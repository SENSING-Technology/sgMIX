#ifndef SDEPTH_PREPROCESS_H
#define SDEPTH_PREPROCESS_H

#include "safe_queue.h"
#include "stereo_camera.h"
#include "preprocess.h"
#include <opencv2/opencv.hpp>
#include <thread>
#include <atomic>
#include <string>

/**
 * SDepth 专用前处理线程：
 * - 负责几何矫正和 resize 到模型输入尺寸
 * - 输出 left_rect/right_rect（BGR 8UC3，用于可视化）
 * - 不做 BGR->RGB / 归一化，留给 SDepthCore 内部处理
 */
class SDepthPreprocessThread {
public:
    SDepthPreprocessThread(SafeQueue<StereoFrame>& input_queue,
                            SafeQueue<PreprocessedFrame>& output_queue,
                            const std::string& calib_file,
                            const cv::Size& original_size,
                            const cv::Size& target_size);
    ~SDepthPreprocessThread();

    void start();
    void stop();

    cv::Mat getQMatrix() const { return Q_; }
    double getBaseline() const { return baseline_; }
    double getFocalLength() const { return focal_length_; }

private:
    void run();
    void load_calibration(const std::string& calib_file);
    void setup_stereo_rectification();
    cv::Rect detectNonBlackRoi(const cv::Mat& bgr, int threshold = 5) const;
    void cropBlackBorderAndResize(cv::Mat& img, const cv::Rect& roi) const;

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

    bool use_fisheye_ = false;
    double fisheye_balance_ = 0.0;
    double fisheye_fov_scale_ = 1.0;
    bool auto_crop_black_border_ = true;
    cv::Rect cached_roi_;
    bool roi_initialized_ = false;
};

#endif // SDEPTH_PREPROCESS_H