#!/bin/bash
# =============================================================================
# SDepth++ AGX Thor 环境一键安装脚本
# 作用：安装编译 + 运行所需的环境依赖（CUDA / TensorRT / OpenCV / GStreamer /
#       OpenSSL / JSON / v4l2 工具），锁定最高性能时钟并配置开机自启
#       （防 GPU 推理触发 SIPL ICP 断流），最后做版本验证。
#
# 不包含：相机驱动（SIPL 包 或 GMSL2x8 包，二者互斥，需按阶段单独 install.sh）
#
# 红线（务必遵守）：
#   - 不安装 nvidia-jetpack（会升级固件破坏相机驱动）
#   - nvidia-l4t-gstreamer 只允许 39.2.0-20260601141651（Argus 跑 s56c 必需，
#     但直接 apt install 会拉最新版并把内核从 39.2.0 升到 39.2.1，导致
#     sgcam_gmsl2.ko 符号 CRC 不匹配——必须显式锁版本，见 [4/6]）
#   - 不执行 apt autoremove（会误删 CUDA/TensorRT 关键包）
#
# 常见故障（本脚本已内置处理）：
#   OpenCV 4.8 来自 NVIDIA Jetson 源（repo.download.nvidia.com），但
#   cmake/gstreamer/libssl 等来自 Ubuntu 官方源 ports.ubuntu.com——该源
#   在国内网络经常不可达，导致 `apt-get update` 失败。旧版脚本因
#   `set -e` 会在第一步直接退出，OpenCV 4.8 根本没装上（且无报错提示）。
#   现改为：自动探测并切换国内镜像、update 失败不阻断、分组安装、
#   最终强制验证，任何失败都会在结尾汇总报告并以非 0 退出。
# =============================================================================
# 注意：不用 set -e —— 任何一步失败都要继续跑完并给出明确诊断，
#       否则就成了"静默失败"，用户以为装好了其实什么都没装。
SUDO="sudo"
if [ "$(id -u)" = "0" ]; then SUDO=""; fi
if ! $SUDO true 2>/dev/null; then
    echo "错误：需要 sudo 权限（请用 sudo bash $0 运行）" >&2
    exit 1
fi

FAILURES=""

# ---------------------------------------------------------------------------
# [1/5] Ubuntu 源可达性探测 + 自动切换国内镜像
#   OpenCV 4.8 / cuda-toolkit / tensorrt 走 NVIDIA Jetson 源，其余走 Ubuntu 源。
#   Ubuntu 官方 ports.ubuntu.com 在国内经常不通，不通则自动切换到可达的
#   国内镜像（原文件自动备份为 *.bak.日期，可随时还原）。
# ---------------------------------------------------------------------------
UBUNTU_MIRRORS=("mirrors.aliyun.com" "mirrors.tuna.tsinghua.edu.cn" "mirrors.ustc.edu.cn" "mirrors.huaweicloud.com")

probe_url() {  # $1=url -> 可达返回 0
    curl -sI --max-time 6 -o /dev/null -w "%{http_code}" "$1" 2>/dev/null | grep -q "200\|301"
}

pick_mirror() {
    for m in "${UBUNTU_MIRRORS[@]}"; do
        echo "    探测镜像 $m ..." >&2
        if probe_url "http://$m/ubuntu-ports/dists/noble/InRelease"; then
            echo "$m"; return 0
        fi
    done
    return 1
}

BACKUP_DIR="/etc/apt/sources-backup"   # 备份放这里：sources.list.d/ 内的非 .list/.sources
                                       # 文件会让 apt 每次都打印 N: Ignoring file 告警

switch_ubuntu_mirror() {  # $1=新镜像域名
    local mirror="$1" f bak
    $SUDO mkdir -p "$BACKUP_DIR"
    for f in /etc/apt/sources.list.d/ubuntu.sources /etc/apt/sources.list; do
        [ -f "$f" ] || continue
        grep -q "ports.ubuntu.com" "$f" || continue
        bak="$BACKUP_DIR/$(basename "$f").bak.$(date +%Y%m%d)"
        [ -f "$bak" ] || $SUDO cp "$f" "$bak"     # 幂等：已有备份不覆盖
        $SUDO sed -i "s#http://ports.ubuntu.com/ubuntu-ports#http://$mirror/ubuntu-ports#g; \
                      s#https://ports.ubuntu.com/ubuntu-ports#http://$mirror/ubuntu-ports#g" "$f"
        echo "  已将 $f 的 Ubuntu 源切换为 $mirror（原文件备份为 $bak）"
    done
}

echo "==> [1/6] 检查 Ubuntu 源可达性"
if probe_url "http://ports.ubuntu.com/ubuntu-ports/dists/noble/InRelease"; then
    echo "  ports.ubuntu.com 可达，保持官方源"
else
    echo "  警告：ports.ubuntu.com 不可达（国内网络常见），尝试自动切换国内镜像"
    MIRROR=$(pick_mirror)
    if [ -n "$MIRROR" ]; then
        switch_ubuntu_mirror "$MIRROR"
    else
        echo "  !!! 所有候选镜像均不可达，将仅依赖本地缓存索引继续（可能失败）" >&2
        FAILURES="$FAILURES ubuntu源不可达"
    fi
fi

echo "==> [2/6] 更新 apt 索引"
# update 部分失败不阻断：本地缓存索引通常仍可完成解析
if ! $SUDO apt-get update; then
    echo "  警告：apt-get update 未完全成功（部分源不可达），继续用已有索引尝试" >&2
fi

install_group() {  # $1=组名, 其余=包列表
    local name="$1"; shift
    echo "---- 安装：$name ----"
    if $SUDO apt-get install -y "$@"; then
        echo "  [$name] 完成"
    else
        echo "  !!! [$name] 安装失败" >&2
        FAILURES="$FAILURES $name"
    fi
}

echo "==> [3/6] 安装核心编译/运行依赖（OpenCV 4.8 / GStreamer / OpenSSL）"
# 核心组单独装：即使后面 cuda/tensorrt 出问题，OpenCV 4.8 也能先装上
install_group "核心依赖" \
    cmake build-essential pkg-config \
    libopencv-dev libssl-dev \
    libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev

echo "==> [4/6] 安装 CUDA / TensorRT（体积较大，请耐心等待）"
# TensorRT 必须锁定 10.16（readme 第 3 节：加密引擎与 TRT 大版本绑定）。
# Jetson r39.2 源已同时提供 11.4.1(+cuda13.4)，不锁版本 apt 会选最高版，
# 导致 libnvinfer.so.10 缺失、模型反序列化失败。--allow-downgrades 使
# 误装 TRT 11 后重跑本脚本可自动纠正。
# 注意：上次运行留下的 hold 会让 apt 判定被锁依赖"not going to be installed"，
# 导致锁定版本安装直接失败——必须先 unhold，装完再 hold。
$SUDO apt-mark unhold tensorrt 'libnvinfer*' >/dev/null 2>&1 || true
# 仅给 tensorrt 包名锁版本不够：libnvinfer-dev/libnvinfer-headers-dev 等同名
# 依赖的候选版都是 11.4.1，apt 解析时优先选高版然后报冲突、绝不回退选 10.16。
# 必须写 apt preferences 把整个 libnvinfer*/libnvonnxparsers* 族全局钉到 10.16，
# 优先级 1001（>1000，允许从误装的 11 降级）。
# 版本用 10.16.* 通配：NVIDIA 源更新 10.16 小版本号时不至于找不到包。
$SUDO tee /etc/apt/preferences.d/sdepth-tensorrt-10 >/dev/null <<'EOF'
Package: tensorrt libnvinfer* libnvonnxparsers* python3-libnvinfer*
Pin: version 10.16.*
Pin-Priority: 1001
EOF

# 保险：pin 未生效就中止，绝不允许静默装成 TRT 11（否则最后验证才发现，客户难排查）
TRT_CAND=$(apt-cache policy tensorrt 2>/dev/null | grep -iE 'Candidate|候选' | head -1 | awk '{print $2}')
if [[ "$TRT_CAND" == 10.16.* ]]; then
    echo "  TensorRT 候选版本: $TRT_CAND（符合 readme 10.16 要求）"
    install_group "CUDA/TensorRT" cuda-toolkit tensorrt --allow-downgrades
else
    echo "  !!! TensorRT 候选版本为 [$TRT_CAND]（期望 10.16.x），已跳过安装以防误装 TRT 11" >&2
    echo "      请检查 /etc/apt/preferences.d/sdepth-tensorrt-10 内容是否正确、NVIDIA 源是否可达" >&2
    FAILURES="$FAILURES tensorrt版本锁定"
fi
# 锁定版本，防止后续 apt upgrade / autoremove 把 TensorRT 升回 11
$SUDO apt-mark hold tensorrt 'libnvinfer*' >/dev/null 2>&1 || true

echo "---- nvidia-l4t-gstreamer 39.2.0（Argus 跑 s56c 必需，锁版本防内核升级）----"
# 直接 apt install 会拉最新版（39.2.1+），连带升级内核，导致 sgcam_gmsl2.ko
# 符号 CRC 不匹配、相机驱动失效——必须显式指定版本，装完 hold 防止后续被升级。
L4T_GST_VER="39.2.0-20260601141651"
$SUDO apt-mark unhold nvidia-l4t-gstreamer >/dev/null 2>&1 || true
if $SUDO apt-get install -y "nvidia-l4t-gstreamer=$L4T_GST_VER"; then
    $SUDO apt-mark hold nvidia-l4t-gstreamer >/dev/null 2>&1 || true
    echo "  nvidia-l4t-gstreamer $L4T_GST_VER 已安装并 hold（防 apt upgrade 升级）"
else
    echo "  !!! nvidia-l4t-gstreamer=$L4T_GST_VER 安装失败" >&2
    echo "      请确认 NVIDIA r39.2 源可达且源中存在该版本：" >&2
    echo "      apt-cache policy nvidia-l4t-gstreamer" >&2
    FAILURES="$FAILURES nvidia-l4t-gstreamer"
fi

echo "---- 可选/辅助依赖（失败可忽略） ----"
$SUDO apt-get install -y nlohmann-json3-dev || echo "  nlohmann-json3-dev 安装失败/无此包（可选，可忽略）"
$SUDO apt-get install -y v4l-utils || echo "  v4l-utils 安装失败（load_modules.sh 阶段再补装）"

# ---------------------------------------------------------------------------
# [5/6] 锁定最高性能时钟（jetson_clocks）+ 开机自启
#   为什么必须做（2026-11 断流排查结论）：
#     默认 DVFS 动态调频下，GPU 推理（TensorRT）启动瞬间的功耗/时钟突变会
#     冲击 GMSL/MIPI 采集链路，SIPL 上报 ICP_CAPTURE_FAILURE(type=201)。
#     该错误一旦触发链路即卡死（后续持续 ICP_CAPTURE_TIMEOUT type=102），
#     进程内重建 SIPL 管道无法恢复，只能重启进程。实测：未锁频约 15~20% 概率
#     在推理启动后 1~2 秒内断流；锁频后 20/20 轮满帧 30FPS 零断流。
#   jetson_clocks 只锁当前会话，重启失效——因此同时安装 systemd 服务
#   实现开机自动锁定。
# ---------------------------------------------------------------------------
echo "==> [5/6] 锁定最高性能时钟（jetson_clocks）并配置开机自启"
if command -v jetson_clocks >/dev/null 2>&1; then
    $SUDO nvpmodel -m 0 2>/dev/null || true   # 最高功耗模式（120W），忽略已设置的报错
    if $SUDO jetson_clocks; then
        echo "  当前会话时钟已锁定"
    else
        echo "  !!! jetson_clocks 执行失败" >&2
        FAILURES="$FAILURES jetson_clocks执行"
    fi

    # 开机自启 systemd 服务（幂等：已存在则覆盖为最新内容）
    JETSON_CLOCKS_PATH="$(command -v jetson_clocks)"
    $SUDO tee /etc/systemd/system/jetson-clocks-lock.service >/dev/null <<EOF
[Unit]
Description=Lock Jetson clocks to max (fix SIPL ICP_CAPTURE_FAILURE under GPU load)
After=multi-user.target

[Service]
Type=oneshot
ExecStart=/usr/sbin/nvpmodel -m 0
ExecStart=$JETSON_CLOCKS_PATH
RemainAfterExit=yes

[Install]
WantedBy=multi-user.target
EOF
    if $SUDO systemctl daemon-reload && $SUDO systemctl enable jetson-clocks-lock.service >/dev/null 2>&1; then
        echo "  开机自启服务 jetson-clocks-lock.service 已启用"
    else
        echo "  !!! 开机自启服务配置失败" >&2
        FAILURES="$FAILURES jetson_clocks自启"
    fi
else
    echo "  !!! 未找到 jetson_clocks（请确认 L4T 工具完整安装）" >&2
    FAILURES="$FAILURES 缺少jetson_clocks"
fi

echo "==> [6/6] 环境验证（无论前面是否出错都执行）"
PASS=0; TOTAL=0
check() {  # $1=描述, $2=命令
    TOTAL=$((TOTAL+1))
    if eval "$2" >/dev/null 2>&1; then
        echo "  [通过] $1"; PASS=$((PASS+1))
    else
        echo "  [失败] $1"; FAILURES="$FAILURES 验证:$1"
    fi
}

echo "---- cmake ----"; cmake --version 2>/dev/null | head -1 || echo "  未找到 cmake"
echo "---- CUDA (期望 13.2) ----"; /usr/local/cuda/bin/nvcc --version 2>/dev/null | tail -1 || (command -v nvcc >/dev/null && nvcc --version | tail -1) || echo "  未找到 nvcc"
echo "---- OpenCV Candidate（期望 4.8.0，NVIDIA 源）----"
apt-cache policy libopencv-dev 2>/dev/null | grep -iE 'Candidate|安装候选' || echo "  查询失败"
# 用 ldconfig 查注册状态（NVIDIA 的 4.8 装在 /usr/lib/，Ubuntu 的 4.6 装在
# /usr/lib/aarch64-linux-gnu/，路径不能写死）
check "OpenCV 4.8 运行库 libopencv_imgproc.so.408 已注册（device_finger 运行必需）" "ldconfig -p | grep -q 'libopencv_imgproc\.so\.408'"
check "OpenCV 4.8 运行库 libopencv_core.so.408 已注册（device_finger 运行必需）" "ldconfig -p | grep -q 'libopencv_core\.so\.408'"
check "TensorRT libnvinfer.so.10 已注册" "ldconfig -p | grep -q 'libnvinfer\.so\.10'"
check "nvidia-l4t-gstreamer 已安装且版本为 $L4T_GST_VER（Argus 跑 s56c 必需，防内核升级）" "[ \"\$(dpkg-query -W -f='\${Version}' nvidia-l4t-gstreamer 2>/dev/null)\" = \"$L4T_GST_VER\" ]"
check "nvidia-l4t-gstreamer 已 hold（防 apt upgrade 连带升级内核）" "apt-mark showhold | grep -qx nvidia-l4t-gstreamer"
check "时钟已锁定最高频（jetson_clocks，防 GPU 推理触发 ICP 断流）" "[ \"$(cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_min_freq 2>/dev/null)\" = \"$(cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_max_freq 2>/dev/null)\" ]"
check "jetson-clocks-lock 开机自启服务已启用" "systemctl is-enabled jetson-clocks-lock.service 2>/dev/null | grep -q enabled"
echo "---- GStreamer ----"; pkg-config --modversion gstreamer-1.0 gstreamer-app-1.0 2>/dev/null || echo "  未找到 gstreamer"
echo "---- v4l2-ctl ----"; command -v v4l2-ctl || echo "  未找到 v4l2-ctl"
echo "---- SIPL 头文件 ----"
if [ -d /usr/src/jetson_sipl_api ]; then
    echo "  已安装: /usr/src/jetson_sipl_api"
else
    echo "  未安装（仅当需要【重新编译】SIPL 后端时才需要；"
    echo "  需通过 JetPack SDK Manager 勾选 SIPL 组件，或向厂商索取头文件包。"
    echo "  若直接使用已编译好的 encrypt_output/stereo_depth_app，则无需此项）"
fi

echo ""
echo "==== 完成 ===="
if [ $PASS -eq $TOTAL ] && [ -z "$FAILURES" ]; then
    echo "全部验证通过（$PASS/$TOTAL）。"
else
    echo "存在未通过项:$FAILURES"
    echo "排查建议："
    echo "  1) 检查网络（NVIDIA 源 repo.download.nvidia.com、Ubuntu 镜像源）"
    echo "  2) apt-cache policy libopencv-dev 的 Candidate 应为 4.8.0 且来自 NVIDIA 源"
    echo "  3) 修正网络/源后重跑本脚本即可续装（已装项自动跳过）"
    exit 1
fi
