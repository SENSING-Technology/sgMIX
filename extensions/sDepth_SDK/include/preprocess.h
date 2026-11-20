#ifndef PREPROCESS_H
#define PREPROCESS_H

#include "safe_queue.h"
#include "stereo_camera.h"
#include <opencv2/opencv.hpp>
#include <thread>
#include <atomic>
#ifdef USE_NLOHMANN_JSON
#include <nlohmann/json.hpp>
#else
#include "json.hpp"
#endif

struct PreprocessedFrame {
    // 用于可视化的矫正后 BGR 图像（8UC3）
    cv::Mat left_rect;
    cv::Mat right_rect;
    // 用于推理的已预处理图像：RGB、float32、[0,1]，尺寸与模型输入一致
    cv::Mat left_model;
    cv::Mat right_model;
    int frame_id;
};

class PreprocessThread {
public:
    /** original_size: 原始相机分辨率; target_size: 校正后输出尺寸，应与模型输入尺寸一致 */
    PreprocessThread(SafeQueue<StereoFrame>& input_queue, SafeQueue<PreprocessedFrame>& output_queue, const std::string& calib_file, const cv::Size& original_size, const cv::Size& target_size);
    ~PreprocessThread();

    void start();
    void stop();
    
    cv::Mat getQMatrix() const { return Q_; }
    double getBaseline() const { return baseline_; }
    double getFocalLength() const { return focal_length_; }

private:
    void run();
    void load_calibration(const std::string& calib_file);
    void setup_stereo_rectification();

    SafeQueue<StereoFrame>& input_queue_;
    SafeQueue<PreprocessedFrame>& output_queue_;
    
    std::thread thread_;
    std::atomic<bool> running_{false};

    cv::Size original_size_;
    cv::Size target_size_;
    cv::Mat left_mtx_, left_dist_, right_mtx_, right_dist_, R_, T_;
    cv::Mat R1_, R2_, P1_, P2_, Q_;
    cv::Mat left_map1_, left_map2_, right_map1_, right_map2_;
    double baseline_ = 0.0;
    double focal_length_ = 0.0;
};

#endif // PREPROCESS_H


