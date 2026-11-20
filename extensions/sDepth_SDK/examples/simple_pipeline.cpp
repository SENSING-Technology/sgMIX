// ============================================================
// 示例 2：完整双目深度流水线最小示例
// ============================================================
// 功能：组装 SDK 四大组件跑通"采集 → 鱼眼校正 → 深度推理 → 显示"。
// 与 stereo_depth_app 功能等价，但去掉了 CLI 解析，代码即文档。
//
// 组件与数据流（线程间用 SafeQueue 解耦）：
//   CaptureThread          双目采集 + 硬件时间戳配对
//     → SafeQueue<StereoFrame>
//   SDepthPreprocessThread 鱼眼立体校正 + resize 到模型输入(640x480)
//     → SafeQueue<PreprocessedFrame>
//   InferenceThread        调用 IStereoDepthBackend 推理（加密 TensorRT）
//     → SafeQueue<InferenceResult>
//   VisualizeThread        视差→深度→伪彩显示（可选，Ctrl+C 退出）
//
// 运行前需准备好（详见 readme.txt 第 4 节）：
//   my_device/sdepth_static.enc      加密模型（本板生成）
//   my_device/sdepth_static.license  授权文件（本板生成）
//   stereo_calibration_s56c_xxxxxxxx.json 双目标定文件
//   vendor_public.pem                    厂商公钥
// ============================================================

#include "stereo_camera.h"
#include "sdepth_preprocess.h"
#include "stereo_backend.h"
#include "tensorrt_inference.h"
#include "visualize.h"
#include <iostream>
#include <memory>
#include <thread>
#include <chrono>
#include <signal.h>

// ---- 按实际路径修改这四个参数 ----
static const char* kModelPath     = "my_device/sdepth_static.enc";
static const char* kLicensePath   = "my_device/sdepth_static.license";
static const char* kCalibPath     = "stereo_calibration_s56c_I05220539.json";
static const char* kPubkeyPath    = "vendor_public.pem";

// Ctrl+C 优雅退出
static volatile sig_atomic_t g_stop = 0;
static void onSigint(int) { g_stop = 1; }

int main() {
    signal(SIGINT, onSigint);

    // S56C 原生分辨率与帧率
    const int kWidth = 2560, kHeight = 1984, kFps = 30;
    // 相机后端："sipl" = SIPL 用户态（Thor 唯一可用路径，默认 CAM6 接法）
    const std::string kCamSpec = "sipl";

    // ---- 1. 创建推理后端（加密 TensorRT 模型）----
    // createBackend 工厂函数创建闭源推理库后端；加密模型(.enc)需同时
    // 提供 license 与厂商公钥，解密与授权校验在 libsdepth.so
// 内部完成。
    auto backend = createBackend(kModelPath, kLicensePath, kPubkeyPath);
    if (!backend) {
        std::cerr << "后端创建失败（检查模型/授权/公钥路径）" << std::endl;
        return 1;
    }
    const cv::Size model_input = backend->getInputSize();  // 640x480

    // ---- 2. 组装流水线 ----
    SafeQueue<StereoFrame> capture_to_preprocess;
    SafeQueue<PreprocessedFrame> preprocess_to_inference;
    SafeQueue<InferenceResult> inference_to_visualize;

    CaptureThread capture(capture_to_preprocess,
                          kCamSpec, kCamSpec,   // 左右相机均走 SIPL
                          kWidth, kHeight, kFps);
    SDepthPreprocessThread preprocess(capture_to_preprocess,
                                       preprocess_to_inference,
                                       kCalibPath,
                                       cv::Size(kWidth, kHeight),
                                       model_input);
    InferenceThread inference(preprocess_to_inference,
                              inference_to_visualize,
                              std::move(backend),
                              /*enable_stats_output=*/true);

    // ---- 3. 启动（可视化线程持有标定的焦距/基线，用于视差→深度）----
    VisualizeThread visualize(inference_to_visualize,
                              preprocess.getFocalLength(),
                              preprocess.getBaseline(),
                              /*enable_visualization=*/true);

    capture.start();
    preprocess.start();
    inference.start();
    visualize.start();

    std::cout << "流水线已启动，Ctrl+C 退出" << std::endl;
    while (!g_stop) {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
    std::cout << "正在停止..." << std::endl;

    // ---- 4. 停止（顺序：上游先停）----
    capture.stop();
    preprocess.stop();
    inference.stop();
    visualize.stop();
    return 0;
}
