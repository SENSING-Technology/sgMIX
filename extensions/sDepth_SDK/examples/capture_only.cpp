// ============================================================
// 示例 1：SIPL 双目采集最小示例（不加载模型）
// ============================================================
// 功能：启动 S56C 双目相机，同步抓取 5 对帧保存为 JPEG。
// 展示 SDK 相机 API 的基础用法：
//   - SiplCamParams::parse()   解析相机配置字符串
//   - SiplCameraManager        进程级单例，管理 SIPL 采集
//   - CameraFrame.timestamp_ns 硬件 SOF 时间戳（左右同步判据）
//
// 编译：由 SDK CMakeLists 的 BUILD_STEREO_APP=ON 一并构建
//       （产物 example_capture_only，位于 encrypt_output/）
// 运行（在 encrypt_output/ 目录下）：
//   sudo ./example_capture_only
// ============================================================

#include "sipl_camera.h"
#include <opencv2/imgcodecs.hpp>
#include <cstdio>
#include <iostream>

int main() {
    // ---- 1. 相机参数 ----
    // "sipl" = 全默认（s56c.json + S56C_2CH + 掩码 0x0100,0x0000 即 CAM6 接法）
    // 换 CAM 口接法时用完整格式，例如 CAM4：
    //   "sipl:../sipl/s56c.json:S56C_2CH:0x0001,0x0000"
    SiplCamParams params = SiplCamParams::parse("sipl");

    // ---- 2. 启动采集 ----
    // S56C 原生分辨率 2560x1984 @ 30fps（由 s56c.json 决定）
    SiplCameraManager& cam = SiplCameraManager::getInstance();
    if (!cam.start(params, 2560, 1984, 30)) {
        std::cerr << "相机启动失败" << std::endl;
        return 1;
    }
    std::cout << "采集已启动: " << cam.sensorCount() << " 个传感器" << std::endl;

    // ---- 3. 同步取帧并保存 ----
    // 说明：captureFrame(sensor_idx) 从各自队列阻塞取帧。
    // 左右帧是否属于同一次曝光，用硬件 SOF 时间戳判断（差 < 1ms 视为同步）。
    const int kWantFrames = 5;
    const uint64_t kSyncTolNs = 1000000ULL;  // 1ms
    int saved = 0, attempts = 0;
    while (saved < kWantFrames && attempts < 100) {
        attempts++;
        CameraFrame left, right;
        if (!cam.captureFrame(0, left, 5000)) break;   // 0 = 左目
        if (!cam.captureFrame(1, right, 5000)) break;  // 1 = 右目

        uint64_t diff = (left.timestamp_ns > right.timestamp_ns)
                            ? left.timestamp_ns - right.timestamp_ns
                            : right.timestamp_ns - left.timestamp_ns;
        if (diff > kSyncTolNs) continue;  // 时间戳超差，丢弃重取

        char ln[64], rn[64];
        std::snprintf(ln, sizeof(ln), "example_left_%02d.jpg", saved);
        std::snprintf(rn, sizeof(rn), "example_right_%02d.jpg", saved);
        cv::imwrite(ln, left.image);
        cv::imwrite(rn, right.image);
        std::cout << "已保存 " << ln << " / " << rn
                  << " (ts diff " << diff / 1e6 << " ms)" << std::endl;
        saved++;
    }

    // ---- 4. 停止 ----
    cam.stop();
    std::cout << "完成: " << saved << "/" << kWantFrames << " 对帧" << std::endl;
    return (saved == kWantFrames) ? 0 : 1;
}
