#ifndef SIPL_CAMERA_H
#define SIPL_CAMERA_H

#include "stereo_camera.h"
#include <string>
#include <vector>
#include <memory>

// SIPL 后端参数，从 camera id 字符串解析：
//   "sipl"                                        → 全部默认值
//   "sipl:<json_path>:<config_name>:<mask0>:<mask1>" → 覆盖指定项
//   "sipl:<json_path>:<config_name>:<mask0,mask1>:yuv|isp" → 可选第4段指定输出路径
//      isp（默认）: RAW 模组，ISP0 硬件 ISP → NV12 → VIC → BGR（S56C/S56）
//      yuv:         YUV 模组，ICP 直出 UYVY → CPU 转 BGR（S36，不经过 ISP）
//   可选第5段 "swap|noswap"：pipeline0/1 与标定左/右相反时需交换（模组接线决定，
//      按型号档案 swap_lr 下发；s56c 实测需 swap）
struct SiplCamParams {
    std::string json_path;               // 平台查询 JSON（s56c.json）
    std::string config_name;             // SensorSystemConfig 名称（如 S56C_2CH）
    std::vector<uint32_t> link_masks;    // 每个 transport 的 link 掩码（如 0x0100, 0x0000）
    bool yuv_mode = false;               // true=YUV 模组 ICP 直出路径
    bool swap_lr = true;                 // true=pipeline 序与物理左右相反，路由时交换

    static SiplCamParams parse(const std::string& spec);
};

// 进程级单例：管理唯一的 NvSIPLCameraInterface 实例。
// S56C 双目 = 1 个 GMSL module（2 个 VB1940 sensor 走同一 link 的 VC0/VC1），
// 必须由同一个 interface 统一 SetPlatformCfg/Init/Start。
class SiplCameraManager {
public:
    static SiplCameraManager& getInstance();
    ~SiplCameraManager();

    // 启动 SIPL 采集（幂等：已运行时直接返回）
    // width/height 为期望输出尺寸（VIC 会把 ISP0 输出缩放到该尺寸）
    bool start(const SiplCamParams& params, int width, int height, int fps);
    void stop();

    bool isRunning() const { return running_; }
    int sensorCount() const { return sensor_count_; }
    uint32_t sensorId(int sensor_idx) const;

    // 阻塞取一帧（sensor_idx: 0=左/VC0, 1=右/VC1），超时返回 false
    bool captureFrame(int sensor_idx, CameraFrame& out, int timeout_ms);

private:
    SiplCameraManager();
    struct Impl;
    std::unique_ptr<Impl> impl_;
    bool running_ = false;
    int sensor_count_ = 0;
    std::vector<uint32_t> sensor_ids_;
};

// CameraBase 适配器：一个实例对应一个 sensor（0=左，1=右）
class SiplCamera : public CameraBase {
public:
    SiplCamera(int sensor_idx, const SiplCamParams& params);
    ~SiplCamera() override;

    bool open(int width, int height, int fps) override;
    void close() override;
    bool isOpened() const override;
    bool captureFrame(CameraFrame& frame) override;
    int getWidth() const override { return width_; }
    int getHeight() const override { return height_; }

private:
    int sensor_idx_;
    SiplCamParams params_;
    int width_ = 0;
    int height_ = 0;
};

#endif // SIPL_CAMERA_H
