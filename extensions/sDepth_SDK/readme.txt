========================================================================
SDepth++ 双目深度 SDK（S36 / S56C / S56 相机 × NVIDIA Jetson AGX Thor）
readme.txt  版本: 1    日期: 2026-11-03
========================================================================

【目录】
1. 产品简介与 SDK 架构
2. 交付物清单
3. 固定环境要求（必读）
4. 快速开始
5. stereo_depth_app 命令行参数
6. 相机型号配置文件（config/cameras/*.json）
7. 双目标定文件说明（txt / json）
8. 二次开发接口（sdepth_api.h）
9. 加密与授权机制说明
10. 常见问题（FAQ）
11. 重要注意事项（红线）

========================================================================
1. 产品简介与 SDK 架构
========================================================================
本 SDK 在 NVIDIA Jetson AGX Thor 开发板上驱动 SENSING 双目 GMSL2
相机，运行 SDepth++ 双目深度模型，实时输出深度/视差结果。

支持三款双目模组（一套 SDK，通过命令行 --camera 按需切换）：

  ┌──────┬────────────┬──────────────┬───────────────┬──────────────┐
  │ 型号 │ 传感器     │ 输出格式     │ 分辨率@帧率  │ 数据通路     │
  ├──────┼────────────┼──────────────┼───────────────┼──────────────┤
  │ S56C │ 2×VB1940   │ RAW10        │ 2560x1984@30  │ ISP0→NV12    │
  │      │            │              │               │ →VIC→BGR     │
  │ S56  │ 2×VB1940   │ RAW10        │ 2560x1984@30  │ 同 S56C      │
  │ S36  │ 2×ISX031   │ YUV422(UYVY) │ 1920x1536@30  │ ICP 直通→BGR │
  │      │            │              │               │ （不走 ISP） │
  └──────┴────────────┴──────────────┴───────────────┴──────────────┘

  - 三款模组共用同一个 640x480 输入的深度引擎，切换型号不需要换模型。
  - S56C/S56 为 RAW 传感器，走硬件 ISP（需要 NITO，自动曝光由 ACP 控制）。
  - S36 模组内部自带 ISP/AE，直出 UYVY，不需要 NITO 文件。
  - 标定模型：S56C/S56 为 fisheye（4 系数）；S36 为 pinhole rational
    （8 系数，OpenCV 顺序 k1,k2,p1,p2,k3,k4,k5,k6）。

实测性能（AGX Thor, JetPack 7.2, S56C）：
  - 推理稳定 30.0 FPS（单帧 15.3 ms）
  - 左右帧硬件时间戳差 0.008~0.019 ms
  - 连续运行 90 秒 1500+ 帧零错误

SDK 采用"开放源码 + 预编译闭源库"两层结构：

  ┌─────────────────────────────────────────────────────┐
  │ 开放源码（本目录 src/，客户可修改/二次开发）         │
  │   相机采集(SIPL用户态, ISP/ICP 双通路)               │
  │   相机型号配置(config/cameras/*.json)                │
  │   双目帧配对(硬件SOF时间戳)                          │
  │   立体校正(鱼眼/针孔, →640x480)                      │
  │   视差/深度伪彩显示                                  │
  ├─────────────────────────────────────────────────────┤
  │ 闭源推理库 lib/libsdepth.so（预编译，无源码）        │
  │   模型解密 · license 验签 · 设备指纹生成/比对        │
  │   SDepth++ TensorRT 推理核心 (FP16)                  │
  ├─────────────────────────────────────────────────────┤
  │ 交付的加密模型 my_device/（.enc + .license）         │
  └─────────────────────────────────────────────────────┘

两层之间通过 include/sdepth_api.h 的 C 接口衔接。
模型解密与授权校验逻辑全部封装在闭源库内部，SDK 不包含其源码，
加密模型只能在与授权设备硬件匹配的板子上解密运行。

========================================================================
2. 交付物清单
========================================================================
交付内容分为四部分：

① 本 SDK 目录（开放源码 + 预编译闭源库，客户可在板上自行编译）
     src/             开放源码（采集/校正/可视化/示例主程序）
     include/         公共头文件（含闭源库接口 sdepth_api.h）
     lib/             预编译闭源推理库 libsdepth.so
                      （aarch64/JetPack 7.2 专用，请勿删除或改名）
     sipl/            SIPL 相机接口层（含 s36/s56c/s56 平台配置 json）
     config/cameras/  相机型号档案（s36.json / s56c.json / s56.json，
                      定义接口掩码、分辨率、数据通路，详见第 6 节）
     examples/        参考示例代码（不参与编译，二次开发参考）
     encrypt_output/  运行目录（自带 vendor_public.pem 公钥；
                      标定文件与加密模型也放这里）
     setup_env.sh     一键环境安装脚本（CUDA/TensorRT/OpenCV/GStreamer
                      等编译+运行依赖，见 4.2 节）
     readme.txt       本说明

② 加密模型 + 模型 license（厂商按客户设备指纹单独生成，见第 4 节
     第 6/7 步），交付形式为：
     my_device/
       ├── sDepth.enc       加密模型
       └── sDepth.license   模型授权文件

③ 双目标定文件（每台模组一份，txt 文本格式，文件名=模组序列号，
     随机发货；每台相机不同，不能与其他相机混用）。
     程序可直接读取 txt，也可用附带工具转成 json（见第 7 节）。

④ 双目相机 + GMSL2 连接线（硬件，随机发货）

【交付口径（含内部测试/验收场景）】对客户而言，只需要交付 ①+②+③
（即整个 sDepth_SDK/ 目录 + 绑定该设备指纹的 my_device/ 加密模型与
license + 该台相机的标定 txt）即可完整运行，无需厂商侧其他文件。
闭源推理库源码、加解密工具、厂商私钥均为厂商内部资产，不随 SDK 交付。
测试工程师即等同于"客户"：拿到新板后，先按 4.2 用 setup_env.sh 装环境，
再按 4.6 在本机生成指纹并发给厂商；厂商用私钥绑定该设备指纹并对模型
重新加密（4.7），把新的 my_device/ 交付给测试工程师放入后即可运行。
注意：设备指纹随板变化，旧板的 sDepth.enc/license 在新板上无法通过校验，
因此每换一块新板都必须重新走 4.6→4.7（换相机/换型号则无需重新加密）。

【客户须知】两条发货口径的补充说明：

（1）运行方式与二进制一致性：推荐直接用 encrypt_output/ 下预编译的
    stereo_depth_app / device_finger 运行，无需编译、无需 SIPL 头文件。
    若需保证可执行文件与最新源码完全一致（如厂商刚修改过源码），请按
    4.3 重新编译覆盖；否则二者功能一致，仅终端调试日志可能略有差异。

（2）自编译需 SIPL 头文件：本 SDK 已内置 SIPL 头文件（third_party/jetson_sipl/，
    来源为 NVIDIA 官网公开的 SIPL API 包 R39.2.0）。若该目录缺失或损坏，
    按下述方法自行补入（两个选择）：
      a) 向厂商索取"已内置 SIPL 头文件"的交付版本；
      b) 从 NVIDIA 官网下载 SIPL API 包自行补入（板上能上网时最简单，
         全程只是下载/解压/拷文件，不安装任何系统包，不触碰红线）：
           # JetPack 7.2 / L4T R39.2 对应的官方包（wget 直接下）：
           wget https://developer.download.nvidia.com/embedded/L4T/r39_Release_v2.0/release/Jetson_SIPL_API_R39.2.0_aarch64.tbz2
           # 校验完整性（必须输出 OK）：
           echo "784a1e178c4800c7f0ca76a305365834017fc70b  Jetson_SIPL_API_R39.2.0_aarch64.tbz2" | sha1sum -c
           # 解包并把头文件拷进 SDK（保持目录名 jetson_sipl）：
           tar -xf Jetson_SIPL_API_R39.2.0_aarch64.tbz2
           mkdir -p third_party
           cp -r usr/src/jetson_sipl_api/sipl/include third_party/jetson_sipl
         或从"刷机时已勾选 SIPL 组件"的同版本 Thor 上拷贝：
           cp -r /usr/src/jetson_sipl_api/sipl/include third_party/jetson_sipl
    补入后 CMake 会优先使用内置头文件，无需再装 /usr/src/jetson_sipl_api。
    【概念澄清】4.1 安装的 SENSING SIPL 驱动包只含运行时（libnvsipl.so、
    nvsipl_drv、nvsipl_camera），运行预编译程序足够；而 SIPL API 头文件
    是 NVIDIA 另一个独立组件（Jetson_SIPL_API 包，默认不安装），只有
    按 4.3 重编译 SDK 源码时才需要，两者不可混淆。

    【传输陷阱】从 Windows 云盘同步目录（WPS 云盘等）压缩头文件再传到
    板上，文件可能被透明加密成乱码二进制（编译时 NvSIPLCamera.hpp 报满屏
    乱码错误）。补入后务必验证：
      file third_party/jetson_sipl/NvSIPLCamera.hpp
    输出含 "ASCII text" 才可用；显示 "data" 即文件已损坏，换官方下载。

    【高危】头文件未补入时 cmake 不会报错终止，而是静默降级为无 SIPL
    的版本（仅一行易被忽略的 WARNING），且编译产物会覆盖 encrypt_output/
    里的预编译二进制；覆盖后的程序运行 s56c/s56 必然报
    "Could not open device sipl:..."（见 FAQ Q12），原始预编译件无法找回。
    防呆做法：自编译时始终加 -DSIPL_REQUIRED=ON，缺头文件直接配置失败；
    并在编译前备份 encrypt_output/ 下的预编译二进制。

========================================================================
3. 固定环境要求（必读）
========================================================================
本 SDK 为固定环境交付，以下不可替换：

  硬件   : NVIDIA Jetson AGX Thor 开发板（客户自购，TRD1_G2A 版本）
  固件   : JetPack 7.2 / L4T R39.2（必须与出厂一致的版本，严禁混刷）
  TensorRT: 10.16（JetPack 7.2 官方 apt 源版本，提供 libnvinfer.so.10；
            加密模型内的 engine 与 TRT 大版本绑定，严禁自行安装
            独立版 TensorRT 11，否则模型反序列化失败）
  CUDA   : 13.2（JetPack 7.2 官方 apt 源版本）
  OpenCV : 4.8（NVIDIA Jetson apt 源 libopencv-dev 即得；注意 Ubuntu
            官方源为 4.6，不满足要求，务必确认包来自
            repo.download.nvidia.com/jetson 源）
  相机   : SENSING S36 / S56C / S56 双目 GMSL2 相机

相机驱动（SENSING 官方 GitHub，JetPack7.2 / L4T R39.2 下有两个互斥的
驱动包，对应两种采集后端；同一时刻只能装其一，SDK 用 backend=auto
运行时自动识别，无需人工切换）：

  ① SIPL 用户态驱动包（默认，本 SDK 主推，仅此一个目录）：
     TRD1_G2A_AGX_THOR_SIPL_JP7.2_L4TR39.2
     https://github.com/SENSING-Technology/nvidia-jetson-camera-drivers/tree/main/Jetson%20AGX%20Thor%20Devkit/TRD1_G2A/JetPack7.2/TRD1_G2A_AGX_THOR_SIPL_JP7.2_L4TR39.2
     安装：install.sh 把 driver/libnvuddf_* 与 libnvsipl_* 拷到
     /usr/lib/nvsipl_drv/（含 s36/s56c/s56 三款），不生成 /dev/videoN。
     对应 SIPL 后端（s36→ICP/YUV，s56c/s56→ISP），即下面 4.1 的
     nvsipl_camera 点亮方式。

  ② GMSL2x8 内核定制驱动包（V4L2 / Argus 路线）：
     TRD1_G2A_GMSL2x8_JP7.2_L4TR39.2
     https://github.com/SENSING-Technology/nvidia-jetson-camera-drivers/tree/main/Jetson%20AGX%20Thor%20Devkit/TRD1_G2A/JetPack7.2/TRD1_G2A_GMSL2x8_JP7.2_L4TR39.2
     安装：install.sh 替换内核与 ko，之后 sudo ./load_modules.sh 生成
     /dev/video0~7。S36（YUV）走 V4L2 后端，S56C/S56（RAW）走 Argus
     后端（nvargus-daemon + nvarguscamerasrc）。

  两个包互斥：装②覆盖①的内核/驱动，反之亦然。后端与驱动包对应关系：

  【重要：同机做双框架切换验证必读】GMSL2x8 包(②) 的 install.sh 会直接用
  定制内核覆盖 /boot/Image（绕过 dpkg，dpkg 仍显示标准 39.2.0），并替换
  dtbo / 定制 .ko / nito 等多处系统文件；而 SIPL 包(①) 的 install.sh 只装
  用户态库、不回填这些。因此一旦装过②再切回①，系统处于"两套驱动混杂"状态，
  SIPL 相机采集会异常（0 帧 / Platform configuration failed / NvSciBuf 分配
  失败，且往往只在刚刷机的标准内核下第一次能跑通）。实测仅重装内核不足以恢复；
  可靠做法是【重新刷机】回 JetPack 7.2 / L4T R39.2，再单独安装某一个驱动包。
  结论：同一台 Thor 上做双框架验证时，每次切换框架最省事可靠的方式就是重新刷机。
  生产环境（客户）只装一个驱动包，不受此影响。

     驱动包              S36(YUV)         S56C/S56(RAW)
     ────────────────   ──────────────   ──────────────────────
     SIPL 包(①)         sipl(ICP)        sipl(ISP)
     GMSL2x8 包(②)      v4l2(/dev/videoN) argus(nvarguscamerasrc)

  设备节点映射（GMSL2x8 包）：cam0=/dev/video0 … cam7=/dev/video7。
  双目模组必须接偶数口（cam0/2/4/6），且占用该口及其相邻的下一口
  （如 cam0 → 左目 /dev/video0 + 右目 /dev/video1）。

相机接线与端口掩码对应关系（与厂商文档一致，实测确认；
--port 参数按此表解析）：

  CAM0 = "0x0000 0x0001"      CAM1 = "0x0000 0x0002"
  CAM2 = "0x0000 0x0100"      CAM3 = "0x0000 0x0200"
  CAM4 = "0x0001 0x0000"      CAM5 = "0x0002 0x0000"
  CAM6 = "0x0100 0x0000"      CAM7 = "0x0200 0x0000"

  ※ S36 模组只能接 CAM0/CAM2/CAM4/CAM6（每口占两条 GMSL 链路中的
    偶数口）。

========================================================================
4. 快速开始
========================================================================
以下操作均在 Thor 板上执行（默认用户 nvidia）。
下文假设交付包解压后进入 SDK 根目录（即包含 src/、encrypt_output/ 的目录）。

【两条使用路径，先看这里】
路径A · 直接用预编译程序（推荐，测试/验收默认走这条）：
    装驱动(4.1) → 装环境(4.2) → 【跳过编译 4.3】→ 采集自检(4.4) →
    放标定(4.5) → 生成指纹(4.6) → 厂商交付模型(4.7) → 跑深度(4.8)。
    直接使用 encrypt_output/ 里已编好的 stereo_depth_app /
    device_finger，无需源码编译，也无需 SIPL 头文件。

    完整命令序列（以 S56C 接 CAM0 为例，逐条复制执行即可；
    下文 <交付包目录> 指 SDK 根目录，<SN> 指随相机发货的标定
    txt 文件名=模组序列号）：

      # ── 第1步 安装相机驱动（SIPL 包，详见 4.1）──
      cd ~/TRD1_G2A_AGX_THOR_SIPL_JP7.2_L4TR39.2
      chmod a+x ./install.sh
      sudo ./install.sh
      sudo reboot                      # 安装后重启一次确保驱动生效
      # 重启后设最大性能：
      sudo nvpmodel -m 0
      sudo jetson_clocks

      # ── 第2步 安装环境依赖（详见 4.2）──
      sudo bash <交付包目录>/setup_env.sh

      # ── 第3步 采集自检，确认双目出图（详见 4.4，不加载模型）──
      cd <交付包目录>/encrypt_output
      sudo ./stereo_depth_app --camera s56c --port cam0 --capture-test 3 --no-vis
      # 成功标志：生成 capture_test_left_000~002.jpg / right_000~002.jpg

      # ── 第4步 放置标定文件（详见 4.5）──
      # 把随相机发货的 <SN>.txt 拷贝到 encrypt_output/ 目录

      # ── 第5步 生成设备指纹并发给厂商（详见 4.6）──
      sudo ./device_finger --vendor-pub vendor_public.pem --out my_device.finger
      # 把 my_device.finger 发给厂商

      # ── 第6步 收厂商交付的加密模型（详见 4.7）──
      # 收到 my_device/（内含 sDepth.enc + sDepth.license）后放到：
      #   encrypt_output/my_device/

      # ── 第7步 运行深度推理（详见 4.8）──
      sudo ./stereo_depth_app --camera s56c --port cam0 \
        --calib <SN>.txt \
        --model my_device/sDepth.enc \
        --license my_device/sDepth.license \
        --pubkey vendor_public.pem
      # Ctrl+C 退出。S36 换用：--camera s36 --port cam2

    【红线】路径A 不要执行 4.3 重编译：编译产物会直接覆盖
    encrypt_output/ 里的预编译二进制，且本包未内置 SIPL 头文件时
    覆盖后的程序无法驱动 s56c/s56（见第 2 节"客户须知"(2)、FAQ Q12）。
    确需重编译前，先备份 encrypt_output/ 下的三个可执行文件。

路径B · 源码自编译（仅当你需要改源码或做二次开发时）：
    在路径A 第1、2步（驱动 + 环境）基础上，增加以下步骤：

      # ── 第3步 检查 SIPL 头文件（本 SDK 已内置 third_party/jetson_sipl/；
      #        缺失或损坏时按第 2 节"客户须知"(2) 从官网补入）──
      cd <交付包目录>
      # 【必查】输出须含 "ASCII text"（显示 data = 文件损坏，须重新获取）：
      file third_party/jetson_sipl/NvSIPLCamera.hpp

      # ── 第4步 备份预编译二进制，然后编译（详见 4.3）──
      cp encrypt_output/stereo_depth_app encrypt_output/device_finger /tmp/
      rm -rf build && mkdir build && cd build
      cmake .. -DCMAKE_BUILD_TYPE=Release -DSIPL_REQUIRED=ON
      make -j$(nproc)
      # 【必查】cmake 输出必须出现：
      #     SIPL 相机后端已启用 (HAVE_SIPL)
      # 加了 -DSIPL_REQUIRED=ON 时，若第3步没生效会直接报错终止，
      # 不会产出坏二进制；不带该开关则只有一行易漏的 WARNING。

      # ── 第5步 用新编二进制走路径A 第3~7步（自检→标定→指纹→模型→运行）──

    补充：encrypt_output/ 里的 device_finger / sdepth_calib_convert
    不依赖 SIPL 头文件，即使误编译降级也不受影响；受影响的只有
    stereo_depth_app。

------------------------------------------------------------------------
4.1 安装相机驱动
------------------------------------------------------------------------
# 下载并解压上述 SENSING 驱动目录后：
cd ~/TRD1_G2A_AGX_THOR_SIPL_JP7.2_L4TR39.2
chmod a+x ./install.sh
sudo ./install.sh

# 设置最大性能模式
sudo nvpmodel -m 0
sudo jetson_clocks

# 点亮验证（掩码必须与所接接口对应，见第 3 节表格）：
#   S56C 接 CAM0：
sudo nvsipl_camera -t query/trd1_g2a/s56c.json -c S56C_2CH \
  -m "0x0000 0x0001" --enable-camera-hal -s -Z -12
#   S36 接 CAM2（YUV 直通，注意 -012R 与 S56C 不同）：
sudo nvsipl_camera -t query/trd1_g2a/s36.json -c S36_2CH_1920X1536_30FPS \
  -m "0x0000 0x0100" --enable-camera-hal -s -Z -012R

# 预期输出（Ctrl+C 退出）：
#   Frame rate (fps): 29.9xx
#   Frame captured: xxx  Frame Drops: 0  Frame discontinuities: 0
# 丢帧为 0、不连续为 0 即点亮成功。

注：本节为 SIPL 驱动包（默认推荐）流程。若客户环境装的是
GMSL2x8 内核驱动包，则对应 V4L2/Argus 路线：install.sh 后
sudo ./load_modules.sh 生成 /dev/video0~7，再按第 3 节后端映射
用 --backend 或直接靠 backend=auto 自动适配，详见 FAQ Q10/Q11。

------------------------------------------------------------------------
4.2 安装环境依赖（路径A / 路径B 都需要）
------------------------------------------------------------------------
# 推荐：直接运行 SDK 自带的一键安装脚本（装齐依赖并做版本自检）：
sudo bash setup_env.sh

# 等价的手动逐条安装（不想跑脚本时用）：
# 基础编译工具 + SDK 依赖（均为 apt 官方源，无需手动下载）：
sudo apt-get update
sudo apt-get install -y cmake build-essential pkg-config \
    libopencv-dev libssl-dev \
    libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev \
    cuda-toolkit tensorrt

# 验证（都应正常输出）：
cmake --version | head -1
/usr/local/cuda/bin/nvcc --version | tail -1          # CUDA 13.2
ls /usr/lib/aarch64-linux-gnu/libnvinfer.so.10        # TensorRT 10.x 运行库
apt-cache policy libopencv-dev | grep Candidate       # 应为 4.8.0（NVIDIA 源）
ls /usr/src/jetson_sipl_api                           # SIPL 开发头文件（仅自编译需要；只跑
                # 预编译程序则完全不需要）。CMake 优先用 SDK 内置 third_party/jetson_sipl/，
                # 该目录缺失或损坏时可从 NVIDIA 官网下载 SIPL API 包补入
                # （下载/校验/拷贝命令见第 2 节"客户须知"(2)），
                # 或向厂商索取"已内置 SIPL 头文件"的交付版本。

【红线】严禁执行 sudo apt install nvidia-jetpack（会升级固件并破坏
相机驱动，无法软件恢复）；严禁执行 apt autoremove（会误删 CUDA/
TensorRT 关键包）。详见第 11 节。

------------------------------------------------------------------------
4.3 编译 SDK（可选：仅路径B 需要，路径A 用预编译程序者可跳过）
------------------------------------------------------------------------
cd <交付包目录>
rm -rf build && mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
# 编译产物在 encrypt_output/ 目录：
#   stereo_depth_app      深度应用主程序
#   device_finger         设备指纹生成工具（见 4.6）
#   sdepth_calib_convert  标定 txt→json 转换工具（见第 7 节）
# 说明：深度推理核心为预编译闭源库 lib/libsdepth.so，
#       编译时自动链接，无需也无法重新编译该库。

------------------------------------------------------------------------
4.4 采集自检（不需要模型）
------------------------------------------------------------------------
cd <交付包目录>/encrypt_output

# S36 接 CAM2（YUV/ICP 通路）：
sudo ./stereo_depth_app --camera s36 --port cam2 --capture-test 3 --no-vis

# S56C 接 CAM0（ISP 通路）：
sudo ./stereo_depth_app --camera s56c --port cam0 --capture-test 3 --no-vis

# 预期输出：
#   [Camera] s36 @ cam2 (YUV/ICP, 1920x1536@30, cfg=S36_2CH_1920X1536_30FPS)
#   [CaptureTest] 帧 0 已保存 ... 共 3 对
# 生成 capture_test_left_000~002.jpg / capture_test_right_000~002.jpg，
# 打开确认双目图像正常（左右画面有视差、亮度一致）。
# 注：S36 加载时会自动把 ISX031 的 vc 输入格式修正为 YUV422/UYVY
#     （SDK 对 YUV 模组的标准 query 修正），等价于厂商 nvsipl_camera 的
#     --enable-camera-hal 行为，属正常现象。

------------------------------------------------------------------------
4.5 放置标定文件
------------------------------------------------------------------------
将随相机发货的标定 txt 文件（文件名=模组序列号，如
I06150119.txt）拷贝到 encrypt_output/ 目录下。
程序可直接读取 txt，无需手动转换（格式说明见第 7 节）。

------------------------------------------------------------------------
4.6 生成设备指纹并发给厂商（客户操作）
------------------------------------------------------------------------
cd <交付包目录>/encrypt_output
sudo ./device_finger --vendor-pub vendor_public.pem --out my_device.finger
# 成功输出：Saved fingerprint file: my_device.finger
# 将 my_device.finger 发送给厂商，用于生成绑定本机的加密模型。
# 说明：指纹文件已用厂商公钥加密，不含明文硬件信息。

------------------------------------------------------------------------
4.7 厂商加密模型交付（厂商操作，客户只需了解）
------------------------------------------------------------------------
厂商收到指纹文件后，用内部工具生成绑定该设备的加密模型，交付
my_device/ 目录（内含 sDepth.enc 和
sDepth.license）。加密与签名工具为厂商内部资产，不随
本 SDK 交付。

客户收到后放到板上：
  encrypt_output/my_device/sDepth.enc
  encrypt_output/my_device/sDepth.license
  （vendor_public.pem 在 SDK 的 encrypt_output/ 中已自带）

------------------------------------------------------------------------
4.8 运行深度推理
------------------------------------------------------------------------
cd <交付包目录>/encrypt_output

# S36 接 CAM2（标定直接给 txt）：
sudo ./stereo_depth_app --camera s36 --port cam2 \
  --calib H120UA-D60-H08300044.txt \
  --model my_device/sDepth.enc \
  --license my_device/sDepth.license \
  --pubkey vendor_public.pem

# S56C 接 CAM0：
sudo ./stereo_depth_app --camera s56c --port cam0 \
  --calib I06150119.txt \
  --model my_device/sDepth.enc \
  --license my_device/sDepth.license \
  --pubkey vendor_public.pem

# 弹出深度伪彩窗口：左半为左目画面，右半为深度伪彩。
# 窗口左上角显示 FPS；鼠标左键点击画面任意点，显示该点深度值（米）。
# Ctrl+C 退出。无显示器的场景加 --no-vis，仅输出推理统计。

========================================================================
5. stereo_depth_app 命令行参数
========================================================================
型号与接口（v2.1 新增，推荐用法）：
  --camera <型号>     s36 | s56c | s56
                      读取 config/cameras/<型号>.json，自动确定
                      平台配置、数据通路（ISP/ICP）、分辨率、帧率
  --port <接口>       cam0 ~ cam7（默认 cam2），掩码表见第 3 节
  --backend <后端>    sipl | v4l2 | argus | auto（默认取档案 backend，
                      见下）临时覆盖采集后端，用于排障或强制指定。
                      后端与驱动包对应关系见第 3 节；auto 运行时探测：
                        检测 /dev/video{口}（GMSL2x8 包）→ 按模组选
                          v4l2(YUV) / argus(RAW)
                        否则检测 /usr/lib/nvsipl_drv/libnvuddf_<型号>_*.so
                          （SIPL 包）→ sipl
                        都未命中则报错退出
  --calib <文件>      双目标定文件，.txt / .json 均可（按扩展名自动
                      识别；--calibration 为同义别名，兼容旧脚本）
  --resolution <WxH>  采集分辨率覆盖。仅 ISP 型模组（s56c/s56）有效；
                      S36 分辨率由模组固件与配置锁定（1920x1536），
                      传入会被忽略并打印告警

模型与授权：
  --model <file>      模型文件（.enc 加密模型）
  --license <file>    加密模型配套 license 文件
  --pubkey <file>     厂商公钥（vendor_public.pem）

运行控制：
  --no-vis            关闭可视化窗口（无显示器环境）
  --capture-test <N>  仅采集 N 对帧存 JPEG 自检（不加载模型）
  --fps <1-75>        限制帧率
  --force-uyvy        强制 UYVY 像素格式（GMSL2 特殊场景）
  -h / --help         帮助

底层相机 spec（旧格式，保留兼容；--camera 会覆盖它）：
  --left-cam <spec>   左相机后端（默认 sipl）
  --right-cam <spec>  右相机后端（默认 sipl）
  spec 格式：
    sipl                                  全默认（s56c + 0x0100,0x0000）
    sipl:<json路径>:<配置名>:<掩码0,掩码1>            ISP 通路
    sipl:<json路径>:<配置名>:<掩码0,掩码1>:yuv        ICP/YUV 通路
    可在末尾再加第 5 段控制左右目路由（默认 swap）：
      ...:swap     pipeline0 为物理右目，路由时交换（s56c/s56）
      ...:noswap   pipeline0 为物理左目，不交换（s36）
    （用 --camera 时该段由型号档案的 swap_lr 字段自动下发）

  示例（S56C 换接 CAM4，不走 --camera）：
    sudo ./stereo_depth_app --model ... --calib ... --license ... \
      --pubkey vendor_public.pem \
      --left-cam  "sipl:../sipl/s56c.json:S56C_2CH:0x0001,0x0000" \
      --right-cam "sipl:../sipl/s56c.json:S56C_2CH:0x0001,0x0000"

========================================================================
6. 相机型号配置文件（config/cameras/*.json）
========================================================================
每款模组一份档案，--camera <型号> 即加载 config/cameras/<型号>.json。
档案支持 // 与 /* */ 注释（交付的档案内已带完整中文注释）。
字段说明（以 s36.json 为例）：

  {
    "name": "s36",
    "output": "icp_yuv",                  // isp=RAW模组走ISP0; icp_yuv=YUV直通
    "backend": "auto",                    // 采集后端: auto(推荐)|sipl|v4l2|argus
                                          //   auto 运行时探测（第 3 节两个驱动包，
                                          //   同一时刻只装其一）：
                                          //     /dev/videoN 存在 → YUV走v4l2 / RAW走argus
                                          //     否则查 nvsipl_drv 库 → sipl
                                          //   也可用 --backend 临时覆盖
    "sipl_json": "sipl/s36.json",         // SIPL 平台配置文件（相对 SDK 根）
    "sipl_config": "S36_2CH_1920X1536_30FPS",  // 平台配置名
    "capture_width": 1920,                // 采集分辨率
    "capture_height": 1536,
    "fps": 30,
    "calib_type": "pinhole",              // 标定模型: fisheye | pinhole
    "swap_lr": false,                     // 左右目路由（见下）
    "ports": {                            // 接口→掩码（第 3 节表格）
      "cam0": ["0x0000","0x0001"], "cam2": ["0x0000","0x0100"], ...
    }
  }

swap_lr（v2.2 新增）说明：
  SIPL 按 pipeline 顺序给帧编号，而"pipeline0 是物理左目还是右目"
  由模组内部两颗 sensor 的 GMSL 接线决定，不同型号模组可能相反；
  标定文件 CAM0/CAM1 是相对于物理左右定义的，二者必须对齐。
    true  = pipeline0 为物理右目，路由时交换（s56c / s56）
    false = pipeline0 为物理左目，不交换（s36，实测判定）
  接反的典型症状：深度图大面积在近距饱和区跳变（闪烁）、
  部分区域深度值明显不合常理。判定方法见 FAQ Q5。

新增/修改型号只需编辑此目录下的 json，不需要改代码。

========================================================================
7. 双目标定文件说明（txt / json）
========================================================================
7.1 txt 格式（厂商随模组发货的原始格式，程序可直接读取）
------------------------------------------------------------------------
逐行 key=value，示例如下（# 后为说明，不在文件内）：

  type= fisheye                # 相机模型: fisheye | pinhole
  imageWidth= 2560             # 标定分辨率（须与实际采集一致）
  imageHeight= 1984
  CAM0_fx= 1181.9097474402     # CAM0_ 前缀 = 一路目内参
  CAM0_fy= 1181.7077891728
  CAM0_cx= 1280.6271902124
  CAM0_cy= 994.4774012473
  CAM0_k1= ...                 # 畸变系数: fisheye 为 k1~k4 共 4 个
  ...                          # pinhole 为 k1,k2,p1,p2,k3,k4,k5,k6 共 8 个
  CAM1_fx= ...                 # CAM1_ 前缀 = 另一路目内参
  ...
  R[0,0]= 0.9999967673         # 双目旋转矩阵 3x3，逐元素
  ...
  T[0,0]= -60.1451030943       # 双目平移向量 3x1，单位 mm
  T[1,0]= ...
  T[2,0]= ...

  兼容性说明：
  - 畸变系数按 key 名读取，行顺序无关（如 k4 排在 k3 前不影响）。
  - 数值允许带单位/注释尾缀（如 "2.6700 mm"），解析时自动截断。
  - 分辨率与采集不一致时，SDK 自动按比例缩放内参。

7.2 txt → json 转换工具（可选）
------------------------------------------------------------------------
程序本身可直接读 txt，本工具仅用于查看/归档/与其他工具对接：

  cd <交付包目录>/encrypt_output
  ./sdepth_calib_convert I06150119.txt
      # 生成同目录 I06150119.json，并打印 模型/分辨率/基线/系数数
  ./sdepth_calib_convert I06150119.txt -o calib_s56c.json   # 指定输出

  不需要 sudo。

========================================================================
8. 二次开发接口（sdepth_api.h）
========================================================================
若需把深度推理集成到自己的程序中，直接调用闭源库 C 接口：

  #include "sdepth_api.h"

  /* 1. 生成设备指纹（等价于 device_finger 工具） */
  sdepth_generate_fingerprint("vendor_public.pem", "my_device.finger");

  /* 2. 创建推理后端（加载加密模型 + 授权校验，自动完成） */
  SDepthBackend* b = sdepth_backend_create(
      "my_device/sDepth.enc",
      "my_device/sDepth.license",
      "vendor_public.pem");

  /* 3. 查询模型输入尺寸，逐帧推理 */
  int w = sdepth_backend_input_width(b);
  int h = sdepth_backend_input_height(b);
  cv::Mat disparity;
  sdepth_backend_infer(b, &left_rect, &right_rect, &disparity);

  /* 4. 用完销毁 */
  sdepth_backend_destroy(b);

编译链接方法（详见 examples/simple_pipeline.cpp 完整示例）：
  g++ your_app.cpp -I../include -I/usr/include/opencv4 \
      -L../lib -lsdepth -o your_app

接口约定：
  - 输入图像支持 BGR 8UC3（任意尺寸，库内自动缩放/转 RGB）或
    RGB 32FC3（尺寸等于模型输入尺寸时直通）；
  - 输出视差图 CV_32FC1，尺寸与输入一致，已换算回原图尺度；
  - 所有失败均通过返回值报告，错误描述用 sdepth_last_error()
    获取。

========================================================================
9. 加密与授权机制说明
========================================================================
- 设备指纹：采集设备的 MAC 地址、EMMC ID、GPU UUID 等硬件标识，
  用厂商公钥加密打包为 .finger 文件。
- 模型加密：模型内容经随机密钥加密为 .enc 文件；解密密钥与设备
  指纹哈希绑定后由厂商私钥签名，生成为 .license 文件。
- 运行验证：闭源库加载 .enc 时，用 vendor_public.pem 验签
  license，并比对本机硬件指纹，全部通过才解密并构建 TensorRT
  引擎。解密与校验逻辑封装在 libsdepth.so 内部，SDK 不含
  其源码。
- 绑定对象是"运行模型的 Thor 设备"，与相机无关：换相机（含换型号）
  不用重新加密，换 Thor 板必须重新走 4.6/4.7 流程。

========================================================================
10. 常见问题（FAQ）
========================================================================
Q1: 报 "Operation returning failure: opcode = 0x9 / Parse failed /
    Init failed. status: 10"
A1: 两种可能：① 没有用 sudo 运行（相机 I2C 节点需要 root）；
    ② 掩码与相机接线不匹配，检查 --port 与实际所接 CAM 口
    （见第 3 节掩码表）。

Q2: 相机起不来 / No cameras available
A2: ① 检查是否用 gpioset 占用了 GPIO 线（会阻塞 CameraHAL），先
    释放再运行；② 确认 install.sh 已执行且未重刷固件；③ 排查
    /boot/dtb 下是否残留其他版本 DTB（混刷固件后会出现，需要厂商
    协助清理）。

Q3: S36 报 NOTIF_ERROR_ICP_CAPTURE_FAILURE / 帧率为 0
A3: S36 模组固件分辨率与配置不匹配。模组固件分 1920x1536 和
    1920x1080 两种，config/cameras/s36.json 里的 sipl_config
    必须选与固件一致的配置
    （S36_2CH_1920X1536_30FPS / S36_2CH_1920X1080_30FPS）。

Q4: S36 如何被识别为 YUV422/UYVY（而不是 RAW）？
A4: 标准 SIPL query 不识别厂商 json 里的 yuv422 写法（仅支持
    uyvy/vyuy/yuyv/yvyu/rawXX），SDK 在加载时会自动把 vc 输入格式
    修正为 YUV422/UYVY（等价厂商 nvsipl_camera --enable-camera-hal 的
    内部行为）；无此修正 ICP 缓冲协商会失败并报 GENERIC_ERROR。

Q5: 深度图闪烁（颜色逐帧跳变）
A5: 按以下顺序排查：
    ① 左右目接反（最常见）。典型症状：深度图大面积在"极近/30m"
       之间跳变，部分区域深度值明显不合常理；左上角配对日志
       [Sync] PTS diff 正常（<0.1ms）。
       原因：pipeline 顺序与物理左右的对应关系由模组接线决定，
       不同型号可能相反（s56c 需交换、s36 不需交换）。
       处理：修改 config/cameras/<型号>.json 的 swap_lr
       （true↔false，运行时加载，改完直接重跑，无需重编译）。
       离线判定方法：
         a) sudo ./stereo_depth_app --camera <型号> --port <口> \
              --calib <标定.txt> --capture-test 3
         b) 用两种左右顺序分别喂模型推理（厂商可提供离线脚本），
            视差图空间结构连贯（远处视差小、近处视差大、过渡平滑）
            的顺序即正确。
    ② 左右目未同步。看日志 [Sync] PTS diff：持续 >5ms 说明配对
       窗口没配准，检查是否误接不同型号模组混用、固件是否一致。
    ③ 相机画面本身亮度在闪（不是深度图闪）：S36 为模组内部 AE，
       在荧光灯/LED 频闪环境下两路独立曝光可能出现亮度振荡，
       属传感器行为，非 SDK 问题；s56c/s56 走 ISP0 sensor group
       同步曝光，无此现象。
    ④ 若换过 CAM 口才出现，先核对 --port 与实际接线（第 3 节
       掩码表），掩码选错可能取到相邻口的流。

Q5b: 深度值满屏都是 30m（量程上限）
A5b: 视差全部无效（≤0）或深度换算被放大。排查：
     ① 优先按 Q5① 检查 swap_lr（左右目接反会导致视差大面积
        为负/无效，被钳到量程上限）；
     ② 确认 --calib 用的是该模组自己的 txt（文件名=SN），
        不要混用其他模组的标定；
     ③ 自制 json 标定时注意 translation_vector 单位是米(m)，
        而厂商 txt 的 T 是毫米(mm)——SDK 读 txt 时会自动
        /1000，json 不做换算，单位填错会导致深度整体放大
        1000 倍（表现为满屏 30m）。
     ④ 启动日志应显示 baseline≈0.06（米）；若显示为 60 左右
        说明标定单位是 mm 被直接使用了，按 ③ 修正。

Q6: 加密模型加载失败 / license 校验失败
A6: ① .enc/.license 与设备一一绑定，不能从别的板拷贝使用；
    ② 确认文件路径与 --model/--license/--pubkey 参数一致；
    ③ 若提示 TensorRT engine 反序列化失败，确认板子固件为
       JetPack 7.2 / L4T R39.2 且 TensorRT 为官方源 10.x（engine 与
       TRT 大版本绑定；误装独立版 TensorRT 11 会导致加载失败）；
    ④ 仍失败则联系厂商重新生成。

Q7: 运行报找不到 libsdepth.so
A7: 闭源库位于 SDK 根目录的 lib/ 下，程序按相对路径 $ORIGIN/../lib
    查找。请保持目录结构完整（lib/ 与 encrypt_output/ 为同级目录），
    不要单独移动可执行文件；如必须移动，请设置环境变量
    LD_LIBRARY_PATH 包含 lib 所在目录。

Q8: 终端里编译日志中文乱码
A8: 编码显示差异，不影响编译，关键看 Configuring done / Built
    target stereo_depth_app。

Q9: 运行报权限错误
A9: 必须用 sudo 运行（相机节点与 GPU 设备需要 root 权限）。

Q10: load_modules.sh 报 "Invalid parameters" / 无 /dev/video* /
     argus 报 "No cameras available"（GMSL2x8 定制驱动包环境）
A10: 内核与相机驱动 .ko 版本不匹配（符号 CRC 不一致）。执行
     journalctl -k | grep sgcam 会看到大量：
       sgcam_gmsl2: Unknown symbol tegracam_xxx (err -22)
       sgcam_gmsl2: disagrees about version of symbol ...
     原因：执行过 sudo apt-get install nvidia-l4t-gstreamer（或任何会
     升级 nvidia-l4t-kernel 的包），内核从 39.2.0 被升到 39.2.1，而
     GMSL2x8 定制 .ko 是按 39.2.0 编译的，符号 CRC 对不上。
     处理：重新跑厂商 install.sh 装回配套内核后重启：
       cd <GMSL2x8 驱动包目录> && sudo ./install.sh && sudo reboot
     重启后再 sudo ./load_modules.sh 即可恢复 /dev/video*。
     预防：见第 11 节第 10 条。

     补充（install.sh 恢复不彻底的更深一层）：若 install.sh + 重启 +
     load_modules.sh 后 /dev/video* 虽然回来，但 8 路相机全部报
       tegra-nvcsi ... Failed to create device link (0x180) with 9-00xx
     并伴随 V4L2 采不到一帧（stereo_depth_app --capture-test 输出
     "完成: 0/3 对帧已保存"）、argus 报 NvBufSurfaceFromFd Failed，
     说明内核模块树/设备树已被 gstreamer 升内核污染到 install.sh
     （它只恢复 /boot/Image + tegra-camera.ko/nvhost-nvcsi.ko 两个
     ko + dtbo）无法修好的程度，NVCSI media-graph 建不起 CSI 数据
     通道（此时 v4l2-ctl --list-formats 仍能列格式，因为格式来自设备
     树而非实时传感器，勿被误导）。此情况必须重新刷机到
     JetPack 7.2 / L4T R39.2，再重装 GMSL2x8 驱动包 + generate 按实际
     接线生成 overlay，不要再反复 install.sh。

Q11: 两个驱动包怎么选 / backend=auto 识别不对 / argus 后端双目 RAW 起不来
A11: 分两点：
     ① 两个驱动包（SIPL 包 / GMSL2x8 包，见第 3 节）互斥，同一时刻
        只能装其一。backend=auto 运行时探测（先看 /dev/videoN 是
        GMSL2x8 包正在跑的强信号，再看 nvsipl_drv 库），自动选后端，
        无需人工切换。若识别与预期不符，多半是“刚换包但没重启/残留
        旧 .ko 或旧库”，先确认驱动包已按第 4.1 节正确安装并重启，或
        用 --backend sipl|v4l2|argus 强制指定定位。
     ② argus 后端（GMSL2x8 包上的 RAW 模组 s56c/s56）：nvargus-daemon
        的单个 Argus session 承载不了两个独立 nvarguscamerasrc 同时取流，
        会出现左目超时、右目 30fps、最终 NvBufSurfaceFromFd Failed。
        RAW 单眼调试可用 argus（或按第 11 节第 10 条用 libargus 直采）；
        但双 RAW 立体的官方正解是 SIPL 后端（ISP 通路）。因此 s56c/s56
        双目三目建议用 SIPL 包 + backend=sipl；argus/v4l2 路线主要给
        YUV 模组 s36（V4L2）和 RAW 单眼调试点亮用。

Q12: 运行报 "Error: Could not open device sipl:..."，且日志里是
     "[CaptureThread] Using V4L2 backend"
A12: 编译时 SIPL 头文件缺失，二进制被静默降级为无 SIPL 版本，
     "sipl:..." 设备字符串被当作 V4L2 设备路径去 open，必然失败。
     与相机是否被占用、nvsipl_camera 是否在运行均无关（换时间重试
     无效）。典型诱因：执行过 4.3 自编译且未先补入 SIPL 头文件，
     产物覆盖了 encrypt_output/ 里的预编译二进制。
     确认方法：
       grep SIPL_INCLUDE build/CMakeCache.txt
       看到 SIPL_INCLUDE_DIR-NOTFOUND 即命中本条。
     处理（二选一）：
       ① 按第 2 节"客户须知"(2) 补入 third_party/jetson_sipl/，
          然后 rm -rf build 重编，确认 cmake 输出出现
          "SIPL 相机后端已启用 (HAVE_SIPL)"（推荐加
          -DSIPL_REQUIRED=ON 防呆）；
       ② 换回厂商交付的预编译二进制（路径A）。
     预防：见第 2 节"客户须知"(2) 的【高危】说明。

========================================================================
11. 重要注意事项（红线）
========================================================================
1) 严禁 sudo apt install nvidia-jetpack —— 会触发 UEFI 固件升级并
   覆盖 SENSING 相机驱动，QSPI 升级后无法用软件降级。
2) 严禁 apt autoremove —— 会误删 CUDA/TensorRT 关键包。
3) 固件必须保持 JetPack 7.2 / L4T R39.2，不要混刷其他版本
   （混刷会导致 DTB/ISP 协议不匹配，相机初始化失败）。
4) .enc / .license / .finger 均与设备绑定，不能跨板复制使用。
5) 标定文件与模组 SN 一一绑定（文件名即 SN），不能与其他模组混用。
6) 相机上电后请勿带电插拔 GMSL2 线。
7) 长时间运行建议 sudo jetson_clocks 并保证散热。
8) lib/libsdepth.so 为预编译交付件，请勿删除、改名或对齐
   修改（损坏后将无法推理且无法本地重建）。
9) S36 只能接 CAM0/CAM2/CAM4/CAM6；S36 的 sipl_config 必须与模组
   固件分辨率一致，否则起流后帧率为 0。
10) GMSL2x8 定制驱动包环境下，严禁 sudo apt-get install
    nvidia-l4t-gstreamer（或依赖链里任何会把 nvidia-l4t-kernel 从
    39.2.0 升到 39.2.1 的包），否则内核符号 CRC 变化会导致
    sgcam-gmsl2.ko 加载失败（"Invalid parameters"/Unknown symbol，
    详见 FAQ Q10）。即使之后跑 install.sh 恢复了 /dev/video*，也仍可能
    残留更深的污染：全部相机报 "Failed to create device link (0x180)"
    且采不到帧，此时只能重新刷机回 L4T R39.2（见 FAQ Q10 补充）。
    RAW 相机点亮的官方途径是 libargus（argus_camera，
    来自 nvidia-l4t-jetson-multimedia-api），不依赖 GStreamer 插件，
    不会触发内核升级。
11) 同机做 SIPL ↔ GMSL2x8 双框架切换验证时，GMSL2x8 包的 install.sh 会用
    定制内核覆盖 /boot/Image 并替换 dtbo/定制 .ko/nito 等多处系统文件；切回
    SIPL 后系统处于两套驱动混杂状态，相机采集异常（0 帧 / Platform
    configuration failed / NvSciBuf 分配失败），仅重装内核不足以恢复。可靠
    做法是重新刷机回 JetPack 7.2 / L4T R39.2 后，再单独安装目标驱动包。
    根本原因：两个驱动包针对不同内核与系统配置，不要在同一台设备上反复互切；
    客户环境只装一个包，不受影响。

========================================================================
技术支持：请联系设备/相机供应商。
========================================================================
