#include "stereo_camera.h"
#include "sdepth_preprocess.h"
#include "stereo_backend.h"
#include "tensorrt_inference.h"
#include "visualize.h"
#include "camera_profile.h"
#include <iostream>
#include <string>
#include <memory>
#include <chrono>
#include <thread>
#include <fstream>
#include <unistd.h>
#include <sys/stat.h>
#include <algorithm>
#include <cctype>
#ifdef USE_NLOHMANN_JSON
#include <nlohmann/json.hpp>
#else
#include "json.hpp"
#endif

namespace {

// profile.sipl_json 为相对 SDK 根目录的路径（如 "sipl/s36.json"），
// 按运行目录探测实际位置（encrypt_output/ 下运行也能找到）
std::string locateSiplJson(const std::string& rel) {
    const std::string candidates[] = {rel, "../" + rel};
    for (const auto& p : candidates) {
        std::ifstream f(p);
        if (f.good()) return p;
    }
    return rel;  // 找不到时保留原值，让 SIPL 层报错可见
}

std::string maskToStr(uint32_t m) {
    char buf[16];
    std::snprintf(buf, sizeof(buf), "0x%04x", m);
    return buf;
}

std::string lowerStr(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return s;
}

bool fileExists(const std::string& path) {
    // access(F_OK) 对普通文件、字符设备（/dev/videoN）、目录都有效，
    // 比 stat+S_ISREG 更契合"存在性"判断
    return ::access(path.c_str(), F_OK) == 0;
}

// 自动探测采集后端（两个驱动包互斥，同一时刻系统只会装其一）：
//   - SIPL 包：install.sh 把 libnvuddf_<name>_cameramodule_library.so 拷到
//     /usr/lib/nvsipl_drv/（新版已含 s36）
//   - GMSL2x8 包：替换 /boot/Image + 内核 ko，load_modules.sh 后生成 /dev/videoN
// 返回值 "sipl" / "v4l2" / "argus"，无法判定时返回空串
// 注意优先级：先判 /dev/videoN（GMSL2x8 包 load_modules 后生成的 V4L2 节点，
// 是"当前框架正在运行"的强信号；SIPL 用户态驱动不生成 V4L2 节点），
// 再判 SIPL 驱动库（SIPL 库可能残留于磁盘，但若 GMSL2x8 正在跑应以节点为准）
std::string detectBackend(const std::string& name, int port_idx, bool is_yuv) {
    const std::string dev = "/dev/video" + std::to_string(port_idx);
    if (fileExists(dev)) {
        // GMSL2x8 包内核节点存在：RAW 模组（ISP/Argus）走 nvargus-daemon +
        // nvarguscamerasrc；YUV 模组（ICP 直出）走 V4L2
        return is_yuv ? "v4l2" : "argus";
    }
    const std::string sipl_lib = "/usr/lib/nvsipl_drv/libnvuddf_" + name +
                                 "_cameramodule_library.so";
    if (fileExists(sipl_lib)) return "sipl";
    return "";
}

} // namespace

int main(int argc, char* argv[]) {
    // 前置检查：/dev/nvmap 默认 root:video 只读，非 root 运行时
    // NvmmNvSciBufToNvBufSurface 会直接段错误（无错误码），提前给出可读的提示
    if (geteuid() != 0 && access("/dev/nvmap", W_OK) != 0) {
        std::cerr << "[FATAL] 当前用户无 /dev/nvmap 写权限，相机采集会崩溃。"
                     "请使用 sudo 运行（参考 readme.txt 第 4 节）。" << std::endl;
        return 1;
    }

    std::string model_name;
    std::string calibration_file;
    bool enable_visualization = true;
    int target_fps = 0;
    // 采集后端由型号档案（config/cameras/<name>.json）的 backend 字段决定：
    //   sipl = SIPL 用户态（S56C/S56 默认走此路；新版 SIPL 包也已支持 S36）
    //   v4l2 = 内核驱动 /dev/videoN 节点（YUV/ICP 模组，如 S36）
    //   argus = nvargus-daemon + nvarguscamerasrc（RAW/ISP 模组，如 S56C）
    //   auto = 运行时自动探测（检测 /usr/lib/nvsipl_drv 驱动库 vs /dev/videoN，
    //          再按模组 output 区分 v4l2(YUV) / argus(RAW)）
    // 推荐用 --camera + --port 自动组装；SIPL 后端也可直接给
    // "sipl:<json>:<cfg>:<mask0,mask1>[:yuv|isp]"
    // --backend 可临时代替档案里的 backend（sipl/v4l2/argus/auto）
    std::string left_camera = "sipl";
    std::string right_camera = "sipl";
    std::string camera_name;   // s36 | s56c | s56（config/cameras/<name>.json）
    std::string port_name = "cam2";
    std::string backend_override;  // --backend 显式覆盖（空=用档案值）
    int res_w = 0, res_h = 0;  // --resolution 覆盖采集分辨率（0=用档案默认）
    // 备选后端（--left-cam/--right-cam 直给，绕过型号档案）:
    //std::string left_camera = "nvarguscamerasrc sensor-id=4";   // Argus GStreamer
    //std::string right_camera = "nvarguscamerasrc sensor-id=5";
    //std::string left_camera = "/dev/video2";                    // V4L2
    //std::string right_camera = "/dev/video3";
    std::string license_path;
    std::string pubkey_path;
    bool force_uyvy = false;
    int capture_test_count = 0;  // >0 时仅采集 N 对帧存 JPEG 后退出（不加载模型）

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--model" && i + 1 < argc) {
            model_name = argv[++i];
        } else if ((arg == "--calibration" || arg == "--calib") && i + 1 < argc) {
            calibration_file = argv[++i];
        } else if (arg == "--camera" && i + 1 < argc) {
            camera_name = argv[++i];
        } else if (arg == "--port" && i + 1 < argc) {
            port_name = argv[++i];
        } else if (arg == "--backend" && i + 1 < argc) {
            backend_override = lowerStr(argv[++i]);
            if (backend_override != "sipl" && backend_override != "v4l2" &&
                backend_override != "argus" && backend_override != "auto") {
                std::cerr << "Error: --backend must be sipl / v4l2 / argus / auto." << std::endl;
                return 1;
            }
        } else if (arg == "--resolution" && i + 1 < argc) {
            const std::string r = argv[++i];
            if (std::sscanf(r.c_str(), "%dx%d", &res_w, &res_h) != 2 || res_w <= 0 || res_h <= 0) {
                std::cerr << "Error: --resolution format must be WxH (e.g. 1920x1536)." << std::endl;
                return 1;
            }
        } else if (arg == "--license" && i + 1 < argc) {
            license_path = argv[++i];
        } else if (arg == "--pubkey" && i + 1 < argc) {
            pubkey_path = argv[++i];
        } else if (arg == "--left-cam" && i + 1 < argc) {
            left_camera = argv[++i];
        } else if (arg == "--right-cam" && i + 1 < argc) {
            right_camera = argv[++i];
        } else if (arg == "--no-vis") {
            enable_visualization = false;
        } else if (arg == "--force-uyvy") {
            force_uyvy = true;
        } else if (arg == "--capture-test" && i + 1 < argc) {
            capture_test_count = std::stoi(argv[++i]);
            if (capture_test_count <= 0) {
                std::cerr << "Error: --capture-test value must be positive." << std::endl;
                return 1;
            }
        } else if (arg == "--fps" && i + 1 < argc) {
            int fps = std::stoi(argv[++i]);
            if (fps <= 0) {
                std::cerr << "Error: FPS value must be positive (1-75)." << std::endl;
                return 1;
            } else if (fps > 75) {
                std::cerr << "Error: FPS value exceeds maximum (75)." << std::endl;
                return 1;
            } else {
                target_fps = fps;
            }
        } else if (arg == "-h" || arg == "--help") {
            std::cout << "Usage: " << argv[0] << " [OPTIONS]" << std::endl;
            std::cout << "Options:" << std::endl;
            std::cout << "  --camera <model>          Camera model: s36 | s56c | s56 (loads config/cameras/<model>.json)" << std::endl;
            std::cout << "  --backend <b>             Force backend: sipl | v4l2 | argus | auto (overrides profile)" << std::endl;
            std::cout << "  --port <camN>             GMSL port: cam0..cam7 (default: cam2)" << std::endl;
            std::cout << "  --calib <file>            Stereo calibration file (.txt vendor format or .json)" << std::endl;
            std::cout << "  --model <file>            Model file (.enc encrypted model)" << std::endl;
            std::cout << "  --resolution <WxH>        Override capture resolution (default: from camera profile)" << std::endl;
            std::cout << "  --license <file>          License file for encrypted model" << std::endl;
            std::cout << "  --pubkey <file>           Vendor public key file" << std::endl;
            std::cout << "  --left-cam <device>       Left camera backend string (advanced, overrides --camera)" << std::endl;
            std::cout << "  --right-cam <device>      Right camera backend string (advanced)" << std::endl;
            std::cout << "  --no-vis                  Disable visualization" << std::endl;
            std::cout << "  --force-uyvy              Force UYVY pixel format (Jetson GMSL2 workaround)" << std::endl;
            std::cout << "  --capture-test <N>        Capture N stereo pairs to JPEG (no model), then exit" << std::endl;
            std::cout << "  --fps <1-75>              Limit frame rate (1-75, default: no limit)" << std::endl;
            std::cout << "  -h, --help                Show this help message" << std::endl;
            std::cout << std::endl;
            std::cout << "Camera backends (advanced):" << std::endl;
            std::cout << "  sipl[:<json>:<cfg>:<mask0,mask1>[:yuv|isp]]  SIPL user-space" << std::endl;
            std::cout << "  /dev/videoN                                  V4L2 device" << std::endl;
            std::cout << std::endl;
            std::cout << "Example:" << std::endl;
            std::cout << "  " << argv[0] << " --camera s56c --port cam0 --calib I06150119.txt \\" << std::endl;
            std::cout << "      --model sdepth.enc --license sdepth.license --pubkey vendor_public.pem" << std::endl;
            std::cout << "  " << argv[0] << " --camera s36 --port cam2 --calib H120UA-D60-H08300044.txt \\" << std::endl;
            std::cout << "      --model sdepth.enc --license sdepth.license --pubkey vendor_public.pem" << std::endl;
            std::cout << "  " << argv[0] << " --camera s36 --port cam2 --capture-test 5   (capture-only self test)" << std::endl;
            return 0;
        }
    }

    // ---- 相机型号档案：--camera 指定时自动组装 SIPL 参数与采集分辨率 ----
    int original_width = 0;
    int original_height = 0;
    int cam_fps = 30;
    if (!camera_name.empty()) {
        CameraProfile profile;
        try {
            profile = LoadCameraProfile(camera_name);
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
            return 1;
        }
        // 最终后端：--backend 显式指定 > 档案 backend（auto → 运行时探测）
        std::string backend = profile.backend;
        if (!backend_override.empty()) {
            backend = backend_override;
        }

        // 端口序号：仅 v4l2 后端需要偶数口校验；非 camN 时保留 -1（sipl 走 portMasks）
        int v4l2_idx = -1;
        try {
            v4l2_idx = CameraProfile::PortIndex(port_name);
        } catch (const std::exception&) {
            v4l2_idx = -1;
        }

        if (backend == "auto") {
            backend = detectBackend(profile.name, v4l2_idx, profile.isYuv());
            if (backend.empty()) {
                std::cerr << "Error: 无法自动识别采集后端 —— 未找到 SIPL 驱动库 "
                             "/usr/lib/nvsipl_drv/libnvuddf_" << profile.name
                          << "_cameramodule_library.so，也未找到 /dev/video"
                          << v4l2_idx << "。请确认已安装 SIPL 或 GMSL2x8 驱动包，"
                             "或使用 --backend 显式指定。" << std::endl;
                return 1;
            }
            std::cout << "[Camera] 后端自动识别为: " << backend << std::endl;
        }

        if (backend == "v4l2") {
            // GMSL2x8 驱动包：双目模组需接偶数口（cam0/2/4/6），
            // 驱动生成 videoN（左目）+ videoN+1（右目）两个节点
            if (v4l2_idx < 0 || v4l2_idx % 2 != 0) {
                std::cerr << "Error: " << profile.name << " 为双目模组，v4l2 后端须接偶数端口"
                             "（cam0/2/4/6），当前为 " << port_name << std::endl;
                return 1;
            }
            left_camera = "/dev/video" + std::to_string(v4l2_idx);
            right_camera = "/dev/video" + std::to_string(v4l2_idx + 1);
        } else if (backend == "argus") {
            // GMSL2x8 包上的 RAW 模组：nvargus-daemon + nvarguscamerasrc
            // sensor-id 与 cam 序号一致（sensor-id=N → CAM N → /dev/videoN）
            if (v4l2_idx < 0 || v4l2_idx % 2 != 0) {
                std::cerr << "Error: " << profile.name << " 为双目模组，argus 后端须接偶数端口"
                             "（cam0/2/4/6），当前为 " << port_name << std::endl;
                return 1;
            }
            left_camera = "nvarguscamerasrc sensor-id=" + std::to_string(v4l2_idx);
            right_camera = "nvarguscamerasrc sensor-id=" + std::to_string(v4l2_idx + 1);
        } else {
            // SIPL 后端
            std::vector<uint32_t> masks;
            try {
                masks = profile.portMasks(port_name);
            } catch (const std::exception& e) {
                std::cerr << "Error: " << e.what() << std::endl;
                return 1;
            }

            std::string spec = "sipl:" + locateSiplJson(profile.sipl_json) + ":" +
                               profile.sipl_config + ":" + maskToStr(masks[0]) + "," +
                               maskToStr(masks[1]) + (profile.isYuv() ? ":yuv" : ":isp") +
                               (profile.swap_lr ? ":swap" : ":noswap");
            left_camera = spec;
            right_camera = spec;
        }

        original_width = profile.capture_width;
        original_height = profile.capture_height;
        cam_fps = profile.fps;

        if (res_w > 0 && res_h > 0) {
            if (profile.isYuv() &&
                (res_w != profile.capture_width || res_h != profile.capture_height)) {
                std::cerr << "[Warn] " << profile.name << " 为 YUV 模组（ICP 直出），"
                          << "分辨率固定 " << profile.capture_width << "x"
                          << profile.capture_height << "，忽略 --resolution "
                          << res_w << "x" << res_h << std::endl;
            } else {
                original_width = res_w;
                original_height = res_h;
            }
        }

        std::cout << "[Camera] " << profile.name << " @ " << port_name
                  << " (backend=" << backend
                  << ", " << (profile.isYuv() ? "YUV/ICP" : "RAW/ISP0") << ", "
                  << original_width << "x" << original_height << "@" << cam_fps;
        if (backend == "v4l2" || backend == "argus") {
            std::cout << ", " << left_camera << " + " << right_camera;
        } else {
            std::cout << ", cfg=" << profile.sipl_config;
        }
        std::cout << ")" << std::endl;
    } else {
        // 未指定型号：兼容旧行为（S56C 默认参数）
        original_width = (res_w > 0) ? res_w : 2560;
        original_height = (res_h > 0) ? res_h : 1984;
    }

    // ---- 采集自测模式：不加载模型，采 N 对帧存 JPEG 后退出 ----
    if (capture_test_count > 0) {
        SafeQueue<StereoFrame> test_queue;
        CaptureThread test_capture(test_queue, left_camera, right_camera,
                                   original_width, original_height, cam_fps, force_uyvy);
        test_capture.start();

        // 等待相机初始化（最多 30s）
        for (int i = 0; i < 3000 && !test_capture.isCameraInitialized(); i++) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        if (!test_capture.isCameraInitialized()) {
            std::cerr << "[CaptureTest] 相机初始化失败" << std::endl;
            test_capture.stop();
            return 1;
        }

        int saved = 0;
        auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(20);
        while (saved < capture_test_count &&
               std::chrono::steady_clock::now() < deadline) {
            StereoFrame f;
            if (test_queue.try_pop(f)) {
                char ln[128], rn[128];
                std::snprintf(ln, sizeof(ln), "capture_test_left_%03d.jpg", saved);
                std::snprintf(rn, sizeof(rn), "capture_test_right_%03d.jpg", saved);
                cv::imwrite(ln, f.left);
                cv::imwrite(rn, f.right);
                uint64_t ts_diff = (f.left_timestamp_ns > f.right_timestamp_ns)
                    ? (f.left_timestamp_ns - f.right_timestamp_ns)
                    : (f.right_timestamp_ns - f.left_timestamp_ns);
                std::cout << "[CaptureTest] 帧 " << saved << " 已保存 (ts diff "
                          << ts_diff / 1e6 << " ms)" << std::endl;
                saved++;
                deadline = std::chrono::steady_clock::now() + std::chrono::seconds(20);
            } else {
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
        }
        test_capture.stop();
        std::cout << "[CaptureTest] 完成: " << saved << "/" << capture_test_count
                  << " 对帧已保存 (capture_test_left/right_XXX.jpg)" << std::endl;
        return (saved == capture_test_count) ? 0 : 1;
    }

    if (model_name.empty()) {
        std::cerr << "Error: --model is required." << std::endl;
        std::cerr << "Use --help for usage information." << std::endl;
        return 1;
    }
    if (calibration_file.empty()) {
        std::cerr << "Error: --calibration is required." << std::endl;
        std::cerr << "Use --help for usage information." << std::endl;
        return 1;
    }

    SafeQueue<StereoFrame> capture_to_preprocess_queue;
    SafeQueue<PreprocessedFrame> preprocess_to_inference_queue;
    SafeQueue<InferenceResult> inference_to_visualize_queue;

    std::unique_ptr<CaptureThread> capture_thread;
    std::unique_ptr<SDepthPreprocessThread> sdepth_preprocess_thread;
    std::unique_ptr<InferenceThread> inference_thread;
    std::unique_ptr<IStereoDepthBackend> backend;

    try {
        backend = createBackend(model_name, license_path, pubkey_path);
        if (!backend) {
            std::cerr << "Error: Failed to create backend for model: " << model_name << std::endl;
            return 1;
        }

        cv::Size model_input_size = backend->getInputSize();

        capture_thread = std::make_unique<CaptureThread>(capture_to_preprocess_queue, left_camera, right_camera, original_width, original_height, cam_fps, force_uyvy);
        capture_thread->start();

        sdepth_preprocess_thread = std::make_unique<SDepthPreprocessThread>(
            capture_to_preprocess_queue,
            preprocess_to_inference_queue,
            calibration_file,
            cv::Size(original_width, original_height),
            model_input_size);
        sdepth_preprocess_thread->start();

        inference_thread = std::make_unique<InferenceThread>(preprocess_to_inference_queue,
                                                             inference_to_visualize_queue,
                                                             std::move(backend),
                                                             true,
                                                             enable_visualization);

        if (target_fps > 0) {
            inference_thread->setTargetFPS(target_fps);
        }

        inference_thread->start();

        if (enable_visualization) {
            double focal_length = sdepth_preprocess_thread->getFocalLength();
            double baseline = sdepth_preprocess_thread->getBaseline();

            std::unique_ptr<VisualizeThread> visualize_thread =
                std::make_unique<VisualizeThread>(inference_to_visualize_queue,
                                                  focal_length,
                                                  baseline,
                                                  true);
            visualize_thread->start();
            visualize_thread->join();
        } else {
            inference_thread->join();
        }

        if (capture_thread) capture_thread->stop();
        if (sdepth_preprocess_thread) sdepth_preprocess_thread->stop();
        if (inference_thread) inference_thread->stop();

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        if (capture_thread) capture_thread->stop();
        if (sdepth_preprocess_thread) sdepth_preprocess_thread->stop();
        if (inference_thread) inference_thread->stop();
        return 1;
    }

    return 0;
}