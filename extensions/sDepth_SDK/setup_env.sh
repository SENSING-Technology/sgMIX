#!/bin/bash
# =============================================================================
# SDepth++ AGX Thor 环境一键安装脚本
# 作用：安装编译 + 运行所需的环境依赖（CUDA / TensorRT / OpenCV / GStreamer /
#       OpenSSL / JSON / v4l2 工具），并做版本验证。
#
# 不包含：相机驱动（SIPL 包 或 GMSL2x8 包，二者互斥，需按阶段单独 install.sh）
#
# 红线（务必遵守）：
#   - 不安装 nvidia-jetpack（会升级固件破坏相机驱动）
#   - 不安装 nvidia-l4t-gstreamer（会把内核从 39.2.0 升到 39.2.1，导致
#     sgcam_gmsl2.ko 符号 CRC 不匹配）
#   - 不执行 apt autoremove（会误删 CUDA/TensorRT 关键包）
# =============================================================================
set -e

echo "==> [1/4] 更新 apt 源"
sudo apt-get update

echo "==> [2/4] 安装基础编译工具 + 核心依赖（对应 readme.txt 4.2 节）"
sudo apt-get install -y \
    cmake build-essential pkg-config \
    libopencv-dev libssl-dev \
    libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev \
    cuda-toolkit tensorrt

echo "==> [3/4] 安装可选/辅助依赖"
# nlohmann-json：CMake 中是 QUIET，缺了也能编译，装上更稳；失败不中断
sudo apt-get install -y nlohmann-json3-dev || echo "  !!! nlohmann-json3-dev 安装失败/无此包（可选，可忽略）"
# v4l2-ctl：GMSL2x8 阶段 load_modules.sh 需要，此处提前装好
sudo apt-get install -y v4l-utils

echo "==> [4/4] 环境验证"
echo "---- cmake ----"; cmake --version | head -1 || true
echo "---- CUDA (期望 13.2) ----"; /usr/local/cuda/bin/nvcc --version 2>/dev/null | tail -1 || (command -v nvcc >/dev/null && nvcc --version | tail -1) || echo "  未找到 nvcc"
echo "---- TensorRT (期望 libnvinfer.so.10) ----"; ls -l /usr/lib/aarch64-linux-gnu/libnvinfer.so.10 2>/dev/null || echo "  未找到 libnvinfer.so.10"
echo "---- OpenCV (Candidate 期望 4.8.0，必须来自 NVIDIA Jetson 源) ----"; apt-cache policy libopencv-dev | grep -iE 'Candidate|安装候选' || true
echo "---- GStreamer ----"; pkg-config --modversion gstreamer-1.0 gstreamer-app-1.0 2>/dev/null || true
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
echo "若上面 OpenCV Candidate 不是 4.8.0 / libnvinfer.so.10 缺失，"
echo "说明 apt 源不是 NVIDIA Jetson 源，请先修正源再重跑本脚本。"