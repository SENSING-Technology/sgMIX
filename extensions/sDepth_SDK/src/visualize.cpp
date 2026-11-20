#include "visualize.h"
#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>
#include <chrono>
#include <algorithm>
#include <vector>

VisualizeThread::VisualizeThread(SafeQueue<InferenceResult>& input_queue, 
                                 double focal_length, 
                                 double baseline, 
                                 bool enable_visualization)
    : input_queue_(input_queue), 
      focal_length_(focal_length), 
      baseline_(baseline), 
      enable_visualization_(enable_visualization) {
    
    depth_visualizer_.enableFixedRange(true);
    depth_visualizer_.setFixedDepthRange(0.03f, 30.0f);
}

void VisualizeThread::onMouse(int event, int x, int y, int flags, void* userdata) {
    (void)flags;
    if (event != cv::EVENT_LBUTTONDOWN || userdata == nullptr) return;
    VisualizeThread* self = reinterpret_cast<VisualizeThread*>(userdata);
    std::lock_guard<std::mutex> lock(self->mouse_mutex_);
    if (self->latest_depth_map_.empty()) return;
    if (x < 0 || y < 0 || x >= self->left_image_width_ || y >= self->left_image_height_) return;
    float depth_m = self->latest_depth_map_.at<float>(y, x);
    if (!std::isfinite(depth_m) || depth_m <= 0.0f) return;
    self->clicked_point_ = cv::Point(x, y);
    self->clicked_depth_m_ = depth_m;
    self->has_clicked_ = true;
}

VisualizeThread::~VisualizeThread() {
    stop();
}

void VisualizeThread::start() {
    running_ = true;
    thread_ = std::thread(&VisualizeThread::run, this);
}

void VisualizeThread::stop() {
    running_ = false;
    if (thread_.joinable()) {
        thread_.join();
    }
}

void VisualizeThread::join() {
    if (thread_.joinable()) {
        thread_.join();
    }
}

cv::Mat VisualizeThread::disparity_to_depth(const cv::Mat& disparity) {
    const float fb = static_cast<float>(focal_length_ * baseline_);
    cv::Mat depth(disparity.size(), CV_32FC1, cv::Scalar(0));
    cv::Mat valid_mask = (disparity > 0.1f);
    cv::Mat safe_disp;
    disparity.copyTo(safe_disp);
    safe_disp.setTo(1.0f, ~valid_mask);
    cv::divide(fb, safe_disp, depth, 1.0, CV_32F);
    depth.setTo(0, ~valid_mask);
    cv::threshold(depth, depth, 30.0, 30.0, cv::THRESH_TRUNC);
    return depth;
}

cv::Mat VisualizeThread::visualize_depth(const cv::Mat& depth) {
    std::lock_guard<std::mutex> lock(visualization_mutex_);
    return depth_visualizer_.visualizeDepth(depth, current_mode_, current_colormap_);
}

cv::Mat VisualizeThread::vis_disparity_cpp(const cv::Mat& disp, float invalid_thres) {
    cv::Mat vis_disp = disp.clone();
    cv::Mat invalid_mask = vis_disp >= invalid_thres;

    if (cv::countNonZero(~invalid_mask) == 0) {
        return cv::Mat::zeros(disp.size(), CV_8UC3);
    }

    cv::Mat vis;
    float min_disp, max_disp;
    
    if (use_fixed_range_for_display_) {
        min_disp = 0.0f;
        max_disp = 416.0f;
    } else {
        cv::Mat valid_disp;
        vis_disp.copyTo(valid_disp, ~invalid_mask);
        double tmp_min, tmp_max;
        cv::minMaxLoc(valid_disp, &tmp_min, &tmp_max);
        if (tmp_max - tmp_min < 0.1) {
            tmp_max = tmp_min + 1.0;
        }
        min_disp = static_cast<float>(tmp_min);
        max_disp = static_cast<float>(tmp_max);
    }
    
    cv::Mat vis_float;
    vis_disp.convertTo(vis_float, CV_32F);
    cv::Mat vis_normalized(disp.size(), CV_8U);
    
    // ---- 四种颜色映射模式 ----
    if (current_depth_color_method_ == DepthColorMappingMethod::LINEAR) {
        // 方式1：线性
        vis = (vis_float - min_disp) / (max_disp - min_disp);
        vis = cv::min(cv::max(vis, 0.0f), 1.0f);
        vis = vis * 255.0f;
        vis.convertTo(vis_normalized, CV_8U);
        
    } else if (current_depth_color_method_ == DepthColorMappingMethod::LOG) {
        // 方式2：对数映射（优化：增加缩放系数 log_alpha_）
        const float alpha = log_alpha_;
        cv::Mat disp_scaled = vis_float * alpha;
        cv::Mat disp_log;
        cv::log(disp_scaled + 1.0f, disp_log);
        float max_scaled = max_disp * alpha;
        float log_max = std::log(max_scaled + 1.0f);
        vis = disp_log / log_max;
        vis = cv::min(cv::max(vis, 0.0f), 1.0f);
        vis = vis * 255.0f;
        vis.convertTo(vis_normalized, CV_8U);
        
    } else if (current_depth_color_method_ == DepthColorMappingMethod::HSV_RAINBOW) {
        // 方式3：自定义分段线性
        std::vector<float> breakpoints;
        std::vector<float> cum_colors;

        if (use_fixed_range_for_display_) {
            float total = max_disp - min_disp;   // 416
            // 极远处（0~5% 视差）占 45% 色域，远处（5%~15%）占 30%，中（15%~40%）占 17%，近（40%~100%）占 8%
            breakpoints = { min_disp, 
                            min_disp + 0.05f * total, 
                            min_disp + 0.15f * total,
                            min_disp + 0.40f * total,
                            max_disp };
            cum_colors   = { 0.0f, 0.45f, 0.75f, 0.92f, 1.0f };
        } else {
            float range = max_disp - min_disp;
            // 动态范围：远处（0~33%）占 50%，中（33%~66%）占 25%，近（66%~100%）占 25%
            breakpoints = { min_disp, 
                            min_disp + range * 0.333f,
                            min_disp + range * 0.666f,
                            max_disp };
            cum_colors   = { 0.0f, 0.5f, 0.75f, 1.0f };
        }

        for (int y = 0; y < disp.rows; y++) {
            for (int x = 0; x < disp.cols; x++) {
                float d = vis_float.at<float>(y, x);
                float norm_d;
                if (d <= breakpoints[0]) {
                    norm_d = cum_colors[0];
                } else if (d >= breakpoints.back()) {
                    norm_d = cum_colors.back();
                } else {
                    int idx = 0;
                    while (idx < (int)breakpoints.size() - 2 && d > breakpoints[idx+1])
                        idx++;
                    float t = (d - breakpoints[idx]) / (breakpoints[idx+1] - breakpoints[idx]);
                    norm_d = cum_colors[idx] + t * (cum_colors[idx+1] - cum_colors[idx]);
                }
                norm_d = std::max(0.0f, std::min(1.0f, norm_d));
                vis_normalized.at<uchar>(y, x) = static_cast<uchar>(norm_d * 255.0f);
            }
        }

    } else if (current_depth_color_method_ == DepthColorMappingMethod::POWER_LAW) {
        // 方式4：幂律映射 (gamma < 1 拉伸低视差区域)
        cv::Mat disp_norm = (vis_float - min_disp) / (max_disp - min_disp);
        disp_norm = cv::max(disp_norm, 0.0f);
        cv::pow(disp_norm, disp_gamma_, disp_norm);
        disp_norm = disp_norm * 255.0f;
        disp_norm.convertTo(vis_normalized, CV_8U);
    }

    // 应用 TURBO 色盘
    cv::Mat vis_color;
    cv::applyColorMap(vis_normalized, vis_color, cv::COLORMAP_TURBO);
    vis_color.setTo(cv::Scalar(0, 0, 0), invalid_mask);
    return vis_color;
}

void VisualizeThread::run() {
    bool has_display = enable_visualization_ && (getenv("DISPLAY") != nullptr);
    if (has_display) {
        try {
            cv::namedWindow("Stereo Depth Estimation", cv::WINDOW_AUTOSIZE);
            cv::setMouseCallback("Stereo Depth Estimation", &VisualizeThread::onMouse, this);
        } catch (const cv::Exception&) {
            has_display = false;
        }
    }

    long long total_frames = 0;
    double total_infer_ms_sum = 0.0;
    long long window_frames = 0;
    double window_infer_ms_sum = 0.0;
    double recent_fps = 0.0;
    double recent_avg_infer_ms = 0.0;
    auto start_time = std::chrono::high_resolution_clock::now();
    auto last_log_time = start_time;

    while (running_) {
        InferenceResult result;
        InferenceResult temp_result;
        while (input_queue_.try_pop(temp_result)) {
            result = temp_result;
        }
        if (result.left_image.empty()) {
            input_queue_.wait_and_pop(result);
        }

        if (result.left_image.empty() || result.disparity_map.empty()) continue;

        cv::Mat depth_map = disparity_to_depth(result.disparity_map);
        {
            std::lock_guard<std::mutex> lock(mouse_mutex_);
            latest_depth_map_ = depth_map;
        }

        cv::Mat combined_image;
        if (has_display) {
            cv::Mat right_vis;
            if (show_depth_map_) {
                right_vis = visualize_depth(depth_map);
            } else {
                right_vis = vis_disparity_cpp(result.disparity_map);
            }
            cv::hconcat(result.left_image, right_vis, combined_image);
        } else {
            combined_image = result.left_image.clone();
        }
        left_image_width_ = result.left_image.cols;
        left_image_height_ = result.left_image.rows;

        total_frames++;
        total_infer_ms_sum += result.inference_time_ms;
        window_frames++;
        window_infer_ms_sum += result.inference_time_ms;

        auto now = std::chrono::high_resolution_clock::now();
        double since_last_log = std::chrono::duration<double>(now - last_log_time).count();
        if (since_last_log >= 5.0 && window_frames > 0) {
            recent_fps = window_frames / since_last_log;
            recent_avg_infer_ms = window_infer_ms_sum / static_cast<double>(window_frames);
            last_log_time = now;
            window_frames = 0;
            window_infer_ms_sum = 0.0;
        }

        if (has_display) {
            // --------- 绘制 HUD 信息（仅帧率 + 鼠标点选深度） ---------
            std::string fps_text = "FPS: " + std::to_string(static_cast<int>(std::round(recent_fps)));
            cv::putText(combined_image, fps_text, cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(0, 255, 0), 2);

            // 鼠标点选深度
            {
                std::lock_guard<std::mutex> lock(mouse_mutex_);
                if (has_clicked_) {
                    char buf[64];
                    snprintf(buf, sizeof(buf), "Depth: %.2fm", static_cast<double>(clicked_depth_m_));
                    cv::Point draw_pt = clicked_point_;
                    cv::circle(combined_image, draw_pt, 4, cv::Scalar(0, 0, 255), -1);
                    cv::putText(combined_image, std::string(buf), draw_pt + cv::Point(8, -8), cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 0, 255), 2);
                }
            }

            cv::imshow("Stereo Depth Estimation", combined_image);
            char key = cv::waitKey(1);
            if (key == 'q' || key == 27) {
                running_ = false;
            } else if (key == 'd' || key == 'D') {
                show_depth_map_ = !show_depth_map_;
            } else if (key == 'm' || key == 'M') {
                use_fixed_range_for_display_ = !use_fixed_range_for_display_;
                depth_visualizer_.enableFixedRange(use_fixed_range_for_display_);
            } else if (key == '1') {
                current_depth_color_method_ = DepthColorMappingMethod::LINEAR;
                depth_visualizer_.setDepthColorMappingMethod(DepthColorMappingMethod::LINEAR);
            } else if (key == '2') {
                current_depth_color_method_ = DepthColorMappingMethod::LOG;
                depth_visualizer_.setDepthColorMappingMethod(DepthColorMappingMethod::LINEAR); // 若不支持则回退
            } else if (key == '3') {
                current_depth_color_method_ = DepthColorMappingMethod::HSV_RAINBOW;
                depth_visualizer_.setDepthColorMappingMethod(DepthColorMappingMethod::LINEAR);
            } else if (key == '4') {
                current_depth_color_method_ = DepthColorMappingMethod::POWER_LAW;
                depth_visualizer_.setDepthColorMappingMethod(DepthColorMappingMethod::LINEAR);
            } else if (key == '+' || key == '=') {
                if (current_depth_color_method_ == DepthColorMappingMethod::POWER_LAW) {
                    disp_gamma_ = std::min(disp_gamma_ + 0.01f, 1.0f);
                }
            } else if (key == '-') {
                if (current_depth_color_method_ == DepthColorMappingMethod::POWER_LAW) {
                    disp_gamma_ = std::max(disp_gamma_ - 0.01f, 0.01f);
                }
            } else if (key == '[') {
                if (current_depth_color_method_ == DepthColorMappingMethod::LOG) {
                    log_alpha_ = std::max(log_alpha_ - 0.01f, 0.01f);
                }
            } else if (key == ']') {
                if (current_depth_color_method_ == DepthColorMappingMethod::LOG) {
                    log_alpha_ = std::min(log_alpha_ + 0.01f, 20.0f);
                }
            }
        }
    }
}