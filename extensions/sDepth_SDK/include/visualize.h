#ifndef VISUALIZE_H
#define VISUALIZE_H

#include "safe_queue.h"
#include "tensorrt_inference.h"
#include "depth_visualizer.h"
#include <opencv2/opencv.hpp>
#include <thread>
#include <atomic>
#include <string>
#include <mutex>

class VisualizeThread {
public:
    VisualizeThread(SafeQueue<InferenceResult>& input_queue, double focal_length, double baseline, bool enable_visualization);
    ~VisualizeThread();

    void start();
    void stop();
    void join();

private:
    void run();
    cv::Mat disparity_to_depth(const cv::Mat& disparity);
    cv::Mat visualize_depth(const cv::Mat& depth);
    cv::Mat vis_disparity_cpp(const cv::Mat& disparity, float invalid_thres = 416.0f);
    static void onMouse(int event, int x, int y, int flags, void* userdata);

    SafeQueue<InferenceResult>& input_queue_;
    double focal_length_;
    double baseline_;
    bool enable_visualization_ = true;
    
    std::mutex mouse_mutex_;
    cv::Mat latest_depth_map_;
    int left_image_width_ = 0;
    int left_image_height_ = 0;
    cv::Point clicked_point_{-1, -1};
    float clicked_depth_m_ = 0.0f;
    bool has_clicked_ = false;
    
    std::mutex visualization_mutex_;
    VisualizationMode current_mode_ = VisualizationMode::FIXED_RANGE;
    ColorMap current_colormap_ = ColorMap::TURBO;
    DepthVisualizer depth_visualizer_;
    
    bool show_depth_map_ = false;                       // 显示深度图还是视差图
    bool use_fixed_range_for_display_ = true;           // 显示时使用固定范围还是动态范围
    DepthColorMappingMethod current_depth_color_method_ = DepthColorMappingMethod::LINEAR; // 深度颜色映射方式
    
    // 新增参数
    float disp_gamma_ = 0.80f;      // 幂律映射的 gamma 值（0 ~ 1），越小远处拉伸越强
    float log_alpha_  = 1.0f;       // 对数映射的缩放系数，增大可拉伸低视差区域
    
    std::thread thread_;
    std::atomic<bool> running_{false};
};

#endif // VISUALIZE_H