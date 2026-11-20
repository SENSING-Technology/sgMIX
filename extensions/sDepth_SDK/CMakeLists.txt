cmake_minimum_required(VERSION 3.16)
project(SDepthSDK CXX)

# =====================================================================
# SDepth++ 双目深度 SDK —— 客户侧构建
#
# 本目录为客户交付内容：
#   - 开放源码：相机采集(SIPL)、立体校正、可视化、示例程序
#   - 预编译闭源库：lib/libsdepth.so
#     （模型解密 + license 验签 + 设备指纹生成/比对 + TensorRT 推理核心，
#       通过 include/sdepth_api.h 提供接口）
#
# 构建：mkdir build && cd build && cmake .. && make -j$(nproc)
# 产物：encrypt_output/stereo_depth_app、encrypt_output/device_finger
# =====================================================================

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(OUTPUT_DIR "${CMAKE_BINARY_DIR}/../encrypt_output")
file(MAKE_DIRECTORY ${OUTPUT_DIR})

# 闭源库必须随 SDK 交付（lib/libsdepth.so）
if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/lib/libsdepth.so")
    message(FATAL_ERROR
        "未找到 lib/libsdepth.so —— 闭源推理库缺失，\n"
        "请确认 SDK 交付包完整（lib/ 目录不可删除或单独拷贝）")
endif()

# 运行依赖预检（libsdepth.so 的运行时依赖；缺失仅告警，不阻断编译，
# 但运行 stereo_depth_app 前必须满足，否则加载 libsdepth.so 失败）：
#   TensorRT 10.x : sudo apt install tensorrt（JetPack 7.2 官方源，提供 libnvinfer.so.10）
#   OpenCV 4.8    : sudo apt install libopencv-dev（须来自 NVIDIA Jetson 源，提供 so.408）
find_library(NVINFER_RUNTIME nvinfer PATHS /usr/lib/aarch64-linux-gnu)
if(NOT NVINFER_RUNTIME)
    message(WARNING "未找到 libnvinfer：运行前请 sudo apt install tensorrt（JetPack 7.2 官方源 10.x）")
endif()

if(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm64")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")
else()
    message(WARNING "本 SDK 仅支持 Jetson AGX Thor (aarch64) 平台")
endif()

# 消除 vendor 代码（NVIDIA SIPL 官方封装）的 -Wmissing-field-initializers 警告
add_compile_options(-Wno-missing-field-initializers)

find_package(OpenCV REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(nlohmann_json QUIET)

if(NOT OpenCV_VERSION MATCHES "^4\\.8")
    message(WARNING "OpenCV 应为 4.8.x（NVIDIA Jetson apt 源），当前 ${OpenCV_VERSION}：运行时将报 libopencv_core.so.408 not found")
endif()

pkg_check_modules(GSTREAMER REQUIRED gstreamer-1.0)
pkg_check_modules(GSTREAMER_APP REQUIRED gstreamer-app-1.0)

find_package(CUDA QUIET)
if(CUDA_FOUND)
    message(STATUS "Found CUDA: ${CUDA_VERSION}")
else()
    message(WARNING "CUDA not found. SIPL VIC 转换可能不可用")
endif()

include_directories(include)
include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(${GSTREAMER_INCLUDE_DIRS})
include_directories(${GSTREAMER_APP_INCLUDE_DIRS})

if(nlohmann_json_FOUND)
    message(STATUS "Found nlohmann_json")
    add_compile_definitions(USE_NLOHMANN_JSON)
endif()

if(CUDA_FOUND)
    include_directories(${CUDA_INCLUDE_DIRS})
endif()

# ---------- 闭源推理库（导入预编译 .so） ----------
add_library(sdepth SHARED IMPORTED GLOBAL)
set_target_properties(sdepth PROPERTIES
    IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/lib/libsdepth.so")

# ---------- SIPL 相机栈（Jetson Thor 唯一可用路径） ----------
# 注：nvbufsurface.h 位于 SIPL include 根目录下，无需单独查找。
#
# SIPL 头文件来源（按顺序查找，找到即用）：
#  1) SDK 内置：third_party/jetson_sipl/（随包交付，客户无需单独安装
#     /usr/src/jetson_sipl_api，可直接重编译 SIPL 后端）
#  2) 系统路径：/usr/src/jetson_sipl_api/sipl/include（JetPack SDK Manager
#     勾选 "SIPL" 组件提供）
find_path(SIPL_INCLUDE_DIR NvSIPLCamera.hpp
    HINTS ${CMAKE_CURRENT_SOURCE_DIR}/third_party/jetson_sipl
          /usr/src/jetson_sipl_api/sipl/include)

foreach(sipl_lib nvsipl nvsipl_query nvscibuf nvscisync nvbufsurface nvbufsurface_nvsci nvbufsurftransform)
    string(TOUPPER ${sipl_lib} SIPL_LIB_UPPER)
    find_library(SIPL_LIB_${SIPL_LIB_UPPER} ${sipl_lib}
        HINTS /usr/lib/aarch64-linux-gnu/nvidia /usr/lib/aarch64-linux-gnu /usr/lib/nvsipl_uddf)
    if(SIPL_LIB_${SIPL_LIB_UPPER})
        list(APPEND SIPL_LIBRARIES ${SIPL_LIB_${SIPL_LIB_UPPER}})
    else()
        message(STATUS "SIPL optional lib not found: ${sipl_lib}")
    endif()
endforeach()

# 防呆开关：客户交付场景若需要 SIPL 后端（s56c/s56 必需），
# 加 -DSIPL_REQUIRED=ON，缺头文件时直接配置失败，杜绝产出坏二进制
option(SIPL_REQUIRED "缺少 SIPL 头文件时终止配置（客户自编译建议开启）" OFF)

if(SIPL_INCLUDE_DIR AND SIPL_LIB_NVSIPL AND SIPL_LIB_NVSIPL_QUERY)
    set(SIPL_FOUND TRUE)
    message(STATUS "Found SIPL: ${SIPL_LIB_NVSIPL}")
else()
    set(SIPL_FOUND FALSE)
    if(SIPL_REQUIRED)
        message(FATAL_ERROR
            "未找到 SIPL 头文件 (NvSIPLCamera.hpp)，SIPL 相机后端无法编译。\n"
            "补齐方法（详见 readme 第 2 节\"客户须知\"）：\n"
            "  mkdir -p third_party\n"
            "  cp -r /usr/src/jetson_sipl_api/sipl/include third_party/jetson_sipl\n"
            "补齐后删除 build 目录重新 cmake。")
    else()
        message(WARNING
            "\n"
            "====================================================================\n"
            "  警告：未找到 SIPL 头文件，本次编译将禁用 SIPL 相机后端！\n"
            "  编译出的 stereo_depth_app 运行 s56c/s56 时必然报错：\n"
            "      [CaptureThread] Using V4L2 backend\n"
            "      Error: Could not open device sipl:...\n"
            "  补齐方法（二选一，详见 readme 第 2 节\"客户须知\"）：\n"
            "    a) 向厂商索取已内置 SIPL 头文件的交付版本；\n"
            "    b) 在刷机勾选 SIPL 组件的机器上执行：\n"
            "         mkdir -p third_party\n"
            "         cp -r /usr/src/jetson_sipl_api/sipl/include third_party/jetson_sipl\n"
            "  然后删除 build 目录重新 cmake。\n"
            "  若只使用 GMSL2x8 内核驱动包（V4L2/Argus 后端）可忽略本警告；\n"
            "  否则建议加 -DSIPL_REQUIRED=ON 强制配置失败以防误交付。\n"
            "====================================================================")
    endif()
endif()

# ---------- 主应用 ----------
set(STEREO_SOURCES
    src/stereo_camera.cpp
    src/sdepth_preprocess.cpp
    src/calib_format.cpp
    src/camera_profile.cpp
    src/visualize.cpp
    src/depth_visualizer.cpp
    src/tensorrt_inference.cpp
    src/secure_backend_adapter.cpp
    src/main.cpp
)

if(SIPL_FOUND)
    list(APPEND STEREO_SOURCES
        sipl/NvSiplCameraInterface.cpp
        sipl/NvNvSciBufHelper.cpp
        sipl/NvSiplCoEUtils.cpp
        src/sipl_camera.cpp)
endif()

add_executable(stereo_depth_app ${STEREO_SOURCES})
target_link_libraries(stereo_depth_app
    sdepth
    ${OpenCV_LIBS}
    ${CUDA_LIBRARIES}
    ${GSTREAMER_LIBRARIES}
    ${GSTREAMER_APP_LIBRARIES}
)

if(SIPL_FOUND)
    target_compile_definitions(stereo_depth_app PRIVATE HAVE_SIPL)
    target_include_directories(stereo_depth_app PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/sipl
        ${SIPL_INCLUDE_DIR}
        ${SIPL_INCLUDE_DIR}/query/include
        ${SIPL_INCLUDE_DIR}/nvsci)
    target_link_libraries(stereo_depth_app ${SIPL_LIBRARIES})
    message(STATUS "SIPL 相机后端已启用 (HAVE_SIPL)")
endif()

# 运行时通过 $ORIGIN/../lib 找到闭源库（encrypt_output 与 lib 为同级目录）
set_target_properties(stereo_depth_app PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR}
    BUILD_RPATH "\$ORIGIN/../lib;${CMAKE_CURRENT_SOURCE_DIR}/lib"
    INSTALL_RPATH "\$ORIGIN/../lib")

# ---------- 设备指纹工具（调用闭源库 C 接口） ----------
add_executable(device_finger src/device_fingerprint_tool.cpp)
target_link_libraries(device_finger sdepth)
set_target_properties(device_finger PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR}
    BUILD_RPATH "\$ORIGIN/../lib;${CMAKE_CURRENT_SOURCE_DIR}/lib"
    INSTALL_RPATH "\$ORIGIN/../lib")

# ---------- 标定格式转换工具（txt → json） ----------
add_executable(sdepth_calib_convert
    src/sdepth_calib_convert.cpp
    src/calib_format.cpp)
target_link_libraries(sdepth_calib_convert ${OpenCV_LIBS})
set_target_properties(sdepth_calib_convert PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR})

message(STATUS "=== SDK 客户侧构建配置 ===")
message(STATUS "OpenCV: ${OpenCV_VERSION}")
message(STATUS "闭源库: lib/libsdepth.so (sdepth_* C API)")
message(STATUS "产物: encrypt_output/stereo_depth_app, encrypt_output/device_finger")
