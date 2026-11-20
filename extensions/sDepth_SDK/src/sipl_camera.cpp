// SIPL 用户态相机后端（Jetson AGX Thor / JetPack 7.2 / L4T R39.2）
// 链路: NvSIPLCameraQuery(json) → NvSIPLCamera(ISP0 硬件ISP, BlockLinear NV12)
//        → VIC 转 PitchLinear NV12 → CPU 双平面读取 → OpenCV BGR
// 参考 NVIDIA sipl_coe_unit_sample（NvSiplCameraInterface 为 vendor 复用代码）

#ifdef HAVE_SIPL

#include "sipl_camera.h"
#include "NvSiplCameraInterface.h"

#include <opencv2/imgproc.hpp>

#include <chrono>
#include <condition_variable>
#include <cstdio>
#include <cstdlib>
#include <deque>
#include <fstream>
#include <memory>
#include <mutex>
#include <vector>

namespace {

// 定容帧队列：满时丢最旧（相机持续推进，消费者只需最新帧），弹出带超时
class FrameQueue {
public:
    explicit FrameQueue(size_t max_size) : max_size_(max_size) {}

    void push(CameraFrame&& f) {
        {
            std::lock_guard<std::mutex> lk(m_);
            if (q_.size() >= max_size_) q_.pop_front();
            q_.push_back(std::move(f));
        }
        cv_.notify_one();
    }

    bool pop(CameraFrame& out, int timeout_ms) {
        std::unique_lock<std::mutex> lk(m_);
        if (!cv_.wait_for(lk, std::chrono::milliseconds(timeout_ms),
                          [this] { return !q_.empty(); })) {
            return false;
        }
        out = std::move(q_.front());
        q_.pop_front();
        return true;
    }

    void clear() {
        std::lock_guard<std::mutex> lk(m_);
        q_.clear();
    }

private:
    size_t max_size_;
    std::deque<CameraFrame> q_;
    mutable std::mutex m_;
    std::condition_variable cv_;
};

// 默认 s56c.json 定位：按运行目录与板上固定路径依次探测
std::string locateDefaultJson() {
    static const char* kCandidates[] = {
        "sipl/s56c.json",                 // 工程根目录运行
        "../sipl/s56c.json",              // encrypt_output/ 目录运行
        "/home/nvidia/sDepth/sDepth_SDK/sipl/s56c.json",
    };
    for (const char* p : kCandidates) {
        std::ifstream f(p);
        if (f.good()) return p;
    }
    return "sipl/s56c.json";  // 找不到时保留相对路径，让 SIPL 报错可见
}

} // namespace

// ---------------- SiplCameraManager ----------------

struct SiplCameraManager::Impl {
    std::unique_ptr<NvSiplCameraInterface> camera;

    // per-sensor 状态（pipeline 顺序号：0/1）
    std::vector<std::unique_ptr<FrameQueue>> queues;
    std::vector<NvBufSurface*> staging;  // VIC 输出: PitchLinear NV12（C API 分配；yuv 模式不用）
    std::vector<uint32_t> frame_counters;
    std::atomic<uint32_t> frame_drops{0};
    uint32_t num_sensors = 1;  // onFrame 路由用（双目时交换左右）
    bool yuv_mode = false;     // YUV 模组（ICP 直出 UYVY），start() 时由参数决定
    bool swap_lr = true;       // pipeline 序与物理左右相反时交换（start() 由参数下发）

    void releaseStaging() {
        for (NvBufSurface* s : staging) {
            if (s) NvBufSurfaceDestroy(s);
        }
        staging.clear();
    }

    // 物理左右与标定左右对齐（2026-09-20 极线检测：s56c pipeline0=物理右目，
    // 与标定 left/right 相反，需交换；是否交换由型号档案 swap_lr 决定）
    void routeAndPush(uint32_t sensor_idx, CameraFrame&& cf) {
        const uint32_t logical_idx =
            (swap_lr && num_sensors == 2 && sensor_idx < 2) ? (1u - sensor_idx) : sensor_idx;
        cf.frame_id = static_cast<int>(frame_counters[logical_idx]++);
        queues[logical_idx]->push(std::move(cf));
    }

    // YUV 模组帧处理（ICP capture 回调）: packed UYVY CPU 直读 → BGR → 入队
    // （frame.rawData 已在 ProcessICPFrame 完成 cache invalidate）
    void onYuvFrame(uint32_t sensor_idx, const NvSiplFrame& frame) {
        if (sensor_idx >= queues.size() || frame.rawData == nullptr) return;

        // 首帧诊断：确认 ICP capture 的实际几何
        {
            static bool s_printed[16] = {false};
            if (sensor_idx < 16 && !s_printed[sensor_idx]) {
                s_printed[sensor_idx] = true;
                std::fprintf(stderr,
                    "[SIPL][yuv] sensor%u 首帧: %ux%u pitch=%u rawSize=%zu\n",
                    sensor_idx, frame.width, frame.height, frame.pitch,
                    frame.rawDataSize);
            }
        }

        const int w = static_cast<int>(frame.width);
        const int h = static_cast<int>(frame.height);

        CameraFrame cf;
        cf.image.create(h, w, CV_8UC3);
        {
            // UYVY packed: 单 plane，2 字节/像素（Y0 U Y1 V 交错）
            cv::Mat uyvy(h, w, CV_8UC2, frame.rawData, frame.pitch);
            cv::cvtColor(uyvy, cf.image, cv::COLOR_YUV2BGR_UYVY);
        }

        cf.timestamp_ns = frame.timestamp * 1000ULL;  // SIPL SOF TSC(µs) → ns
        routeAndPush(sensor_idx, std::move(cf));
    }

    // ISP0 就绪帧回调（每 sensor 独立线程）:
    // BlockLinear NV12 --VIC--> PitchLinear NV12 --CPU--> BGR → 入队
    void onFrame(uint32_t sensor_idx, const NvSiplFrame& frame) {
        if (yuv_mode) {
            onYuvFrame(sensor_idx, frame);
            return;
        }
        if (frame.nvbufSurface == nullptr) return;
        if (sensor_idx >= staging.size() || staging[sensor_idx] == nullptr) {
            return;
        }
        NvBufSurface* dst = staging[sensor_idx];

        // 1) VIC 转换（硬件 2D 引擎，BL→PL + 必要时缩放）
        NvBufSurfTransformConfigParams session{};
        session.compute_mode = NvBufSurfTransformCompute_VIC;

        NvBufSurfTransformParams xform{};
        NvBufSurfTransformRect src_rect{}, dst_rect{};
        src_rect.width = frame.nvbufSurface->surfaceList[0].planeParams.width[0];
        src_rect.height = frame.nvbufSurface->surfaceList[0].planeParams.height[0];
        dst_rect.width = dst->surfaceList[0].planeParams.width[0];
        dst_rect.height = dst->surfaceList[0].planeParams.height[0];
        xform.src_rect = &src_rect;
        xform.dst_rect = &dst_rect;

        NvBufSurfTransformSetSessionParams(&session);
        if (NvBufSurfTransform(frame.nvbufSurface, dst, &xform) !=
            NvBufSurfTransformError_Success) {
            ++frame_drops;
            return;
        }

        // 2) CPU 读取双平面 → BGR
        if (NvBufSurfaceMap(dst, 0, 0, NVBUF_MAP_READ) != 0) {
            ++frame_drops;
            return;
        }
        if (NvBufSurfaceMap(dst, 0, 1, NVBUF_MAP_READ) != 0) {
            NvBufSurfaceUnMap(dst, 0, 0);
            ++frame_drops;
            return;
        }
        NvBufSurfaceSyncForCpu(dst, 0, 0);
        NvBufSurfaceSyncForCpu(dst, 0, 1);

        const NvBufSurfacePlaneParams& pp = dst->surfaceList[0].planeParams;
        const int w = static_cast<int>(pp.width[0]);
        const int h = static_cast<int>(pp.height[0]);

        CameraFrame cf;
        cf.image.create(h, w, CV_8UC3);
        {
            // NV12: plane0 = Y(h×w), plane1 = UV 交错(h/2×w/2, 2字节/采样)
            cv::Mat y_plane(h, w, CV_8UC1,
                            dst->surfaceList[0].mappedAddr.addr[0], pp.pitch[0]);
            cv::Mat uv_plane(h / 2, w / 2, CV_8UC2,
                             dst->surfaceList[0].mappedAddr.addr[1], pp.pitch[1]);
            cv::cvtColorTwoPlane(y_plane, uv_plane, cf.image, cv::COLOR_YUV2BGR_NV12);
        }

        NvBufSurfaceUnMap(dst, 0, 0);
        NvBufSurfaceUnMap(dst, 0, 1);

        // SIPL 硬件时间戳（SOF TSC，µs → ns）
        cf.timestamp_ns = frame.timestamp * 1000ULL;
        routeAndPush(sensor_idx, std::move(cf));
    }
};

SiplCameraManager& SiplCameraManager::getInstance() {
    static SiplCameraManager inst;
    return inst;
}

SiplCameraManager::SiplCameraManager() : impl_(std::make_unique<Impl>()) {}

SiplCameraManager::~SiplCameraManager() {
    stop();
}

bool SiplCameraManager::start(const SiplCamParams& params, int width, int height, int fps) {
    (void)fps;  // 帧率由 s56c.json 传感器配置决定（S56C = 30fps）
    if (running_) return true;

    if (width <= 0 || height <= 0) {
        std::fprintf(stderr, "[SIPL] 非法输出尺寸 %dx%d\n", width, height);
        return false;
    }

    auto fail = [&]() {
        impl_->releaseStaging();
        impl_->queues.clear();
        impl_->frame_counters.clear();
        impl_->camera.reset();
        return false;
    };

    // ---- 配置（RAW 模组与 nvsipl_camera 工作命令 -t <json> -c <cfg> -m "<mask>" -Z 对应）----
    // yuv_mode（S36/ISX031）：等价 nvsipl_camera 的 -012R —— 禁 ISP0/1/2、
    // capture 直出（YUV422/UYVY），模组内部完成 ISP/AE，主机侧不注册 ACP。
    NvSiplCameraConfig cfg;
    cfg.platformConfigName = params.config_name;
    cfg.jsonFilePath = params.json_path;
    cfg.linkMasks = params.link_masks;
    cfg.disableAuth = true;       // 等价 -Z
    cfg.enableISP0 = !params.yuv_mode;
    // 实验7：解耦组合，精确复刻 nvsipl_camera 工作命令（不带 -R）的行为——
    //   captureOutputRequested=false（不向用户输出 RAW）
    //   但 ICP buffer 照常注册（CNvSIPLMaster registerWithSipl[ICP]=true 恒真，
    //   capture buffer 是 ISP 输入，由 SIPL 内部管理）
    // 实验1/4/5 证明 captureRequested=true 时 ACP 必挂(6)；实验6 证明不注册 ICP
    // 则 ISP0 注册挂(10)——唯一没试过的组合就是本实验。
    // （yuv_mode 相反：capture 是唯一输出，captureOutputRequested=true）
    cfg.enableICP = true;
    cfg.captureOutputRequested = params.yuv_mode;
    cfg.maxCaptureBuffers = 6;    // 对齐 nvsipl_camera -B 默认 "6 capture, 4 ISP"
    // YUV 模组 SIPL 强制 disableSubframe（libnvsipl: "disableSubframe set to true
    // for YUV format"），显式关闭避免 SetPipelineCfg 属性冲突
    cfg.enableSubframe = !params.yuv_mode;
    cfg.enableAutoControl = !params.yuv_mode; // ACP: /var/nvidia/nvcam/settings/sipl/<Sensor>.nito
    // YUV 模组：标准 query 解析不出 "yuv422"，需在 SetPlatformCfg 前强制
    // vcInfo=YUV422/UYVY（等价厂商 nvsipl_camera CQueryAdapter 的 forcing）
    cfg.forceYuv422Input = params.yuv_mode;
    cfg.useInternalThreads = true;
    cfg.maxISP0Buffers = 4; // 与实际注册的 ISP0 buffer 数一致

    impl_->yuv_mode = params.yuv_mode;
    impl_->swap_lr = params.swap_lr;

    impl_->camera = std::make_unique<NvSiplCameraInterface>();

    if (impl_->camera->SetSiplCameraConfig(cfg) != NVSIPL_STATUS_OK) {
        std::fprintf(stderr, "[SIPL] SetSiplCameraConfig 失败\n");
        return fail();
    }

    // ParseDatabase → ParseJsonFile → GetSensorSystemConfig → ApplyMask
    SIPLStatus st = impl_->camera->SetupCameraSystemConfig();
    if (st != NVSIPL_STATUS_OK) {
        std::fprintf(stderr, "[SIPL] SetupCameraSystemConfig 失败(%d): json=%s config=%s\n",
                     (int)st, params.json_path.c_str(), params.config_name.c_str());
        return fail();
    }

    // SetPlatformCfg → SetPipelineCfg(每 sensor) → Init
    st = impl_->camera->Initialize(cfg);
    if (st != NVSIPL_STATUS_OK) {
        std::fprintf(stderr, "[SIPL] Initialize 失败(%d): 检查相机供电/接线与 gpioset 占用\n", (int)st);
        return fail();
    }

    const uint32_t n = impl_->camera->GetNumSensors();
    if (n == 0) {
        std::fprintf(stderr, "[SIPL] 未发现传感器\n");
        return fail();
    }
    impl_->num_sensors = n;

    // 实验7诊断：dump notification queue —— Init 阶段错误不返回失败而是静默进队列，
    // 后续 RegisterImages/ACP 才报 INVALID_STATE（CNvSIPLMaster: "Pipeline had seen
    // Init Error"）。ACP 失败前先看队列里有没有 NOTIF_INIT_ERROR_FAILURE(223)。
    for (uint32_t k = 0; k < n; k++) {
        const NvSIPLPipelineQueues& q = impl_->camera->GetPipelineQueues(k);
        if (!q.notificationQueue) continue;
        for (int t = 0; t < 16; t++) {
            NvSIPLPipelineNotifier::NotificationData nd{};
            SIPLStatus gs = q.notificationQueue->Get(nd, 1000 /*us*/);
            if (gs != NVSIPL_STATUS_OK) break;  // 队列空（TIMED_OUT/EOF）
            std::fprintf(stderr,
                "[SIPL][notif] sensor%u: type=%d pipeline=%u vc=%u initErrCode=%u\n",
                k, (int)nd.eNotifType, nd.uIndex, nd.uVcIdx,
                static_cast<uint32_t>(nd.initErrCode));
        }
    }

    // 收集 sensor ID（打印映射，便于核对 CAM 口 ↔ sensorId）
    sensor_ids_.clear();
    const auto& sys = impl_->camera->GetSensorSystemConfig();
    for (const auto& module : sys.modules) {
        std::visit([&](const auto& modType) {
            for (const auto& sensorCfg : modType.sensorConfigs) {
                std::visit([&](const auto& c) { sensor_ids_.push_back(c.id); }, sensorCfg);
            }
        }, module.moduleType);
    }
    // 实验9诊断：打印 HAL query 分配的 sensorId（基线 nvsipl_camera 显示 Pipeline 8/9，
    // 需确认 ssc 里的 id 与 SetPipelineCfg/GetImageAttributes 的 index 语义是否一致）
    std::fprintf(stderr, "[SIPL] ssc sensorIds:");
    for (uint32_t id : sensor_ids_) std::fprintf(stderr, " %u", id);
    std::fprintf(stderr, " (numSensors=%u)\n", n);

    impl_->queues.clear();
    impl_->staging.clear();
    impl_->frame_counters.clear();

    // ---- 循环1: 全部 sensor 的缓冲区/同步对象注册（两段式，对齐 nvsipl_camera）----
    // 实验9根因：vendor 交错顺序（sensor0 的 ICP→ISP→ACP 完成后才轮到 sensor1 注册）
    // 与 nvsipl_camera 的两段式不同——S56C 是 sensorGroup trigger 模式，sensor0 的
    // ACP 注册要求组内全部 pipeline 完成 buffer 注册，sensor1 未注册时 ACP → INVALID_STATE。
    // nvsipl_camera main.cpp: 3203-3360 先 forEach 全部 RegisterBuffers+RegisterCPUWaiter，
    // 3380+ 才是独立的 ACP 注册循环。
    for (uint32_t i = 0; i < n; i++) {
        // ICP buffer 注册（实验7：即使 captureOutputRequested=false 也要注册，
        // nvsipl_camera 的 registerWithSipl[ICP] 恒为 true——capture buffer 是 ISP 输入）
        st = impl_->camera->SetupICPBuffers(i, 6);
        if (st != NVSIPL_STATUS_OK) {
            std::fprintf(stderr, "[SIPL] SetupICPBuffers(%u) 失败(%d)\n", i, (int)st);
            return fail();
        }

        if (!params.yuv_mode) {
            st = impl_->camera->SetupISPBuffers(i, 4, true /*BL 输出，VIC 前置*/);
            if (st != NVSIPL_STATUS_OK) {
                std::fprintf(stderr, "[SIPL] SetupISPBuffers(%u) 失败(%d)\n", i, (int)st);
                return fail();
            }

            // staging: PitchLinear NV12（VIC 目标，CPU 可直接读）
            // 注意: Thor R39.2 无 NvBufSurf C++ API（NvAllocate），用 C 版 NvBufSurfaceAllocate
            NvBufSurfaceAllocateParams ap{};
            ap.params.width = static_cast<uint32_t>(width);
            ap.params.height = static_cast<uint32_t>(height);
            ap.params.layout = NVBUF_LAYOUT_PITCH;
            ap.params.colorFormat = NVBUF_COLOR_FORMAT_NV12;
            ap.params.memType = NVBUF_MEM_SURFACE_ARRAY;
            ap.memtag = NvBufSurfaceTag_VIDEO_CONVERT;

            NvBufSurface* surf = nullptr;
            if (NvBufSurfaceAllocate(&surf, 1, &ap) != 0) {
                std::fprintf(stderr, "[SIPL] sensor %u: staging NV12 分配失败\n", i);
                return fail();
            }
            impl_->staging.push_back(surf);
        }

        impl_->queues.push_back(std::make_unique<FrameQueue>(5));
        impl_->frame_counters.push_back(0);

        uint32_t idx = i;
        if (params.yuv_mode) {
            impl_->camera->SetCaptureFrameCallback(
                idx, [this, idx](const NvSiplFrame& f) { impl_->onFrame(idx, f); });
        } else {
            impl_->camera->SetReadyFrameCallback(
                idx, [this, idx](const NvSiplFrame& f) { impl_->onFrame(idx, f); });
        }
    }

    // ---- 循环2: ACP 注册（全部 buffer 注册完成后再做，对齐 nvsipl_camera 两段式）----
    // ACP 只注册 primary sensor（deviceIndex=0，即 pipeline 0）：
    // S56C 双 sensor 同 sensorGroup（trigger 模式），nvsipl_camera 对 secondary
    // 明确跳过 ACP 注册（main.cpp "skipping auto control registration for
    // secondary sensor"）；对 secondary 注册会触发 INVALID_STATE 并污染整组状态。
    // YUV 模组（S36）曝光/AE 在模组内部完成，主机侧无 NITO，整组跳过。
    if (!params.yuv_mode) {
        for (uint32_t i = 0; i < n; i++) {
            if (i == 0) {
                st = impl_->camera->RegisterAutoControlPlugin(i);
                if (st != NVSIPL_STATUS_OK) {
                    // Start() @pre 要求 ISP 输出使能时必须注册 ACP，失败即致命
                    std::fprintf(stderr, "[SIPL] RegisterAutoControlPlugin(primary) 失败(%d)\n", (int)st);
                    return fail();
                }
            } else {
                std::fprintf(stderr, "[SIPL] sensor %u = secondary(sensorGroup)，跳过 ACP 注册\n", i);
            }
        }
    }

    // ---- 启动（一次 Start 覆盖全部 sensor）----
    st = impl_->camera->StartCapture();
    if (st != NVSIPL_STATUS_OK) {
        std::fprintf(stderr, "[SIPL] StartCapture 失败(%d)\n", (int)st);
        return fail();
    }

    sensor_count_ = static_cast<int>(n);
    running_ = true;

    std::fprintf(stderr, "[SIPL] 采集已启动: %u 传感器, 输出 %dx%d (%s)\n",
                 n, width, height,
                 params.yuv_mode ? "ICP UYVY→BGR" : "ISP0+VIC NV12→BGR");
    for (size_t i = 0; i < sensor_ids_.size(); i++) {
        std::fprintf(stderr, "[SIPL]   pipeline[%zu] = sensorId %u\n", i, sensor_ids_[i]);
    }
    return true;
}

void SiplCameraManager::stop() {
    if (!impl_ || (!running_ && !impl_->camera)) return;

    if (impl_->camera) {
        if (impl_->camera->IsCapturing()) {
            impl_->camera->StopCapture();  // 内部 join ISP0/ICP/Event 线程
        }
        impl_->camera.reset();  // 析构触发 Cleanup()（幂等）
    }
    impl_->releaseStaging();
    impl_->queues.clear();
    impl_->frame_counters.clear();

    if (impl_->frame_drops.load() > 0) {
        std::fprintf(stderr, "[SIPL] 采集停止 (累计丢帧 %u)\n", impl_->frame_drops.load());
    } else {
        std::fprintf(stderr, "[SIPL] 采集停止\n");
    }
    running_ = false;
    sensor_count_ = 0;
    sensor_ids_.clear();
}

uint32_t SiplCameraManager::sensorId(int sensor_idx) const {
    if (sensor_idx < 0 || static_cast<size_t>(sensor_idx) >= sensor_ids_.size()) {
        return 0xFFFFFFFFu;
    }
    return sensor_ids_[(size_t)sensor_idx];
}

bool SiplCameraManager::captureFrame(int sensor_idx, CameraFrame& out, int timeout_ms) {
    if (!running_ || sensor_idx < 0 || sensor_idx >= sensor_count_) return false;
    return impl_->queues[(size_t)sensor_idx]->pop(out, timeout_ms);
}

// ---------------- SiplCamera（CameraBase 适配器）----------------

SiplCamera::SiplCamera(int sensor_idx, const SiplCamParams& params)
    : sensor_idx_(sensor_idx), params_(params) {}

SiplCamera::~SiplCamera() {
    close();
}

bool SiplCamera::open(int width, int height, int fps) {
    SiplCameraManager& mgr = SiplCameraManager::getInstance();
    if (!mgr.start(params_, width, height, fps)) {
        return false;
    }
    if (sensor_idx_ >= mgr.sensorCount()) {
        std::fprintf(stderr, "[SIPL] sensor index %d 超出范围（共 %d 个传感器）\n",
                     sensor_idx_, mgr.sensorCount());
        return false;
    }
    width_ = width;
    height_ = height;
    return true;
}

void SiplCamera::close() {
    // 不停 Manager：另一个 sensor 实例可能仍在用。
    // Manager 由 CaptureThread::stop() → SiplCameraManager::stop() 统一停止。
}

bool SiplCamera::isOpened() const {
    const SiplCameraManager& mgr = SiplCameraManager::getInstance();
    return mgr.isRunning() && sensor_idx_ < mgr.sensorCount();
}

bool SiplCamera::captureFrame(CameraFrame& frame) {
    return SiplCameraManager::getInstance().captureFrame(sensor_idx_, frame, 2000);
}

// ---------------- 参数解析 ----------------

// 格式: "sipl"（全默认）
//       "sipl:<json_path>:<config_name>:<mask0,mask1>"（空段保持默认）
//       "sipl:<json_path>:<config_name>:<mask0,mask1>:yuv|isp"（第4段指定输出路径）
// 示例: "sipl:/home/nvidia/SENSING-.../query/trd1_g2a/s56c.json:S56C_2CH:0x0100,0x0000"
//       "sipl:sipl/s36.json:S36_2CH_1920X1536_30FPS:0x0100,0x0000:yuv"
SiplCamParams SiplCamParams::parse(const std::string& spec) {
    SiplCamParams p;
    p.json_path = locateDefaultJson();
    p.config_name = "S56C_2CH";
    p.link_masks = {0x0100u, 0x0000u};  // Transport[0](AB) Link2 = CAM2 接线

    if (spec.rfind("sipl", 0) != 0) return p;
    const size_t colon = spec.find(':');
    if (colon == std::string::npos) return p;

    std::vector<std::string> parts;
    const std::string rest = spec.substr(colon + 1);
    size_t b = 0;
    while (true) {
        const size_t c = rest.find(':', b);
        parts.push_back(rest.substr(b, (c == std::string::npos ? rest.size() : c) - b));
        if (c == std::string::npos) break;
        b = c + 1;
    }

    if (parts.size() >= 1 && !parts[0].empty()) p.json_path = parts[0];
    if (parts.size() >= 2 && !parts[1].empty()) p.config_name = parts[1];
    if (parts.size() >= 3 && !parts[2].empty()) {
        p.link_masks.clear();
        const std::string& m = parts[2];
        size_t b2 = 0;
        while (true) {
            const size_t c2 = m.find_first_of(",+ ", b2);
            const std::string tok =
                m.substr(b2, (c2 == std::string::npos ? m.size() : c2) - b2);
            if (!tok.empty()) {
                p.link_masks.push_back(
                    static_cast<uint32_t>(strtoul(tok.c_str(), nullptr, 0)));
            }
            if (c2 == std::string::npos) break;
            b2 = c2 + 1;
        }
    }
    if (parts.size() >= 4 && !parts[3].empty()) {
        if (parts[3] == "yuv") {
            p.yuv_mode = true;
        } else if (parts[3] == "isp") {
            p.yuv_mode = false;
        } else {
            std::fprintf(stderr,
                "[SIPL] 未识别的输出路径 '%s'（期望 yuv|isp），按 isp 处理\n",
                parts[3].c_str());
        }
    }
    if (parts.size() >= 5 && !parts[4].empty()) {
        if (parts[4] == "swap") {
            p.swap_lr = true;
        } else if (parts[4] == "noswap") {
            p.swap_lr = false;
        } else {
            std::fprintf(stderr,
                "[SIPL] 未识别的左右交换选项 '%s'（期望 swap|noswap），按 swap 处理\n",
                parts[4].c_str());
        }
    }
    return p;
}

#endif // HAVE_SIPL
