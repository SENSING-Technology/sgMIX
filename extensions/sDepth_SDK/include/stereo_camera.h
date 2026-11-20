#ifndef STEREO_CAMERA_H
#define STEREO_CAMERA_H

#include "safe_queue.h"
#include <opencv2/opencv.hpp>
#include <thread>
#include <atomic>
#include <string>
#include <memory>
#include <queue>
#include <mutex>
#include <linux/videodev2.h>

// GStreamer headers for hardware timestamp support
#include <gst/gst.h>
#include <gst/app/gstappsink.h>

struct CameraFrame {
    cv::Mat image;
    uint64_t timestamp_ns;
    int frame_id;
};

struct StereoFrame {
    cv::Mat left;
    cv::Mat right;
    int frame_id;
    uint64_t left_timestamp_ns;
    uint64_t right_timestamp_ns;
};

class CameraBase {
public:
    virtual ~CameraBase() = default;
    virtual bool open(int width, int height, int fps) = 0;
    virtual void close() = 0;
    virtual bool isOpened() const = 0;
    virtual bool captureFrame(CameraFrame& frame) = 0;
    virtual int getWidth() const = 0;
    virtual int getHeight() const = 0;
};

class V4L2Camera : public CameraBase {
public:
    V4L2Camera(const std::string& device_path, bool force_uyvy = false);
    ~V4L2Camera() override;
    
    bool open(int width, int height, int fps = 30) override;
    void close() override;
    bool isOpened() const override { return fd_ >= 0; }
    bool captureFrame(CameraFrame& frame) override;
    int getWidth() const override { return width_; }
    int getHeight() const override { return height_; }

private:
    bool listFormats();
    bool setFormat(int width, int height);
    bool setFps(int fps);
    bool initMmap();
    bool startStreaming();
    bool stopStreaming();
    uint64_t getTimestamp();
    
    std::string device_path_;
    int fd_ = -1;
    int width_ = 0;
    int height_ = 0;
    int bytes_per_line_ = 0;
    __u32 pixel_format_ = 0;  // actual pixel format from driver
    bool force_uyvy_ = false; // force UYVY conversion (Jetson GMSL2 workaround)
    unsigned int buffer_count_ = 4;
    
    struct v4l2_buffer* buffers_ = nullptr;
    void** buffer_starts_ = nullptr;
};

class ArgusCamera : public CameraBase {
public:
    ArgusCamera(const std::string& pipeline_prefix);
    ~ArgusCamera() override;
    
    bool open(int width, int height, int fps = 30) override;
    void close() override;
    bool isOpened() const override { return pipeline_ != nullptr; }
    bool captureFrame(CameraFrame& frame) override;
    int getWidth() const override { return width_; }
    int getHeight() const override { return height_; }

private:
    std::string pipeline_prefix_;
    GstElement* pipeline_ = nullptr;
    GstElement* appsink_ = nullptr;
    int width_ = 0;
    int height_ = 0;
    int fps_ = 30;
};

class CaptureThread {
public:
    CaptureThread(SafeQueue<StereoFrame>& output_queue, 
                  const std::string& left_cam_id, 
                  const std::string& right_cam_id, 
                  int width, 
                  int height, 
                  int fps = 30,
                  bool force_uyvy = false);
    ~CaptureThread();

    void start();
    void stop();
    bool isCameraInitialized() const { return camera_initialized_; }

private:
    void run();
    void captureLeft();
    void captureRight();
    void matchAndPushFrames();

    SafeQueue<StereoFrame>& output_queue_;
    std::string left_cam_id_, right_cam_id_;
    int frame_width_, frame_height_;
    int cam_fps_ = 30;
    bool force_uyvy_ = false;
    bool use_argus_ = false;
    bool use_sipl_ = false;  // SIPL 用户态后端（Jetson Thor 官方主推路径）
    
    std::thread thread_;
    std::thread left_thread_;
    std::thread right_thread_;
    std::atomic<bool> running_{false};
    std::atomic<bool> camera_initialized_{false};
    
    CameraBase* left_cam_ = nullptr;
    CameraBase* right_cam_ = nullptr;
    
    std::queue<CameraFrame> left_frames_;
    std::queue<CameraFrame> right_frames_;
    std::mutex left_mutex_;
    std::mutex right_mutex_;
    
    int frame_count_ = 0;
    const size_t MAX_BUFFER_SIZE = 10;
    const uint64_t MAX_TIME_DIFF_NS = 10000000; // 10ms
};

#endif // STEREO_CAMERA_H