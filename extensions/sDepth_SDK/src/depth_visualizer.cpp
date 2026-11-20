#include "depth_visualizer.h"
#include <algorithm>
#include <cmath>
#include <mutex>

DepthVisualizer::DepthVisualizer() {
    clahe_ = cv::createCLAHE(clahe_clip_limit_, cv::Size(8, 8));
}

void DepthVisualizer::setDepthColorMappingMethod(DepthColorMappingMethod method) {
    std::lock_guard<std::mutex> lock(mapping_mutex_);
    depth_color_mapping_method_ = method;
}

DepthColorMappingMethod DepthVisualizer::getDepthColorMappingMethod() const {
    std::lock_guard<std::mutex> lock(mapping_mutex_);
    return depth_color_mapping_method_;
}

void DepthVisualizer::setFixedDepthRange(float min_depth, float max_depth) {
    fixed_min_depth_ = std::max(0.1f, min_depth);
    fixed_max_depth_ = std::max(fixed_min_depth_ + 0.1f, max_depth);
}

void DepthVisualizer::enableFixedRange(bool enable) {
    use_fixed_range_ = enable;
}

cv::Mat DepthVisualizer::visualizeDepth(const cv::Mat& depth, 
                                       VisualizationMode mode,
                                       ColorMap colormap) {
    return visualizeDepthAdvanced(depth, mode, colormap, 
                                  clahe_clip_limit_, bilateral_d_, bilateral_sigma_color_);
}

cv::Mat DepthVisualizer::visualizeDepthAdvanced(const cv::Mat& depth,
                                               VisualizationMode mode,
                                               ColorMap colormap,
                                               float clahe_clip,
                                               int bilateral_d,
                                               float bilateral_sigma) {
    if (depth.empty()) {
        return cv::Mat();
    }
    
    cv::Mat depth_vis = depth.clone();
    
    cv::Mat valid_mask;
    double min_depth, max_depth, p2, p98;
    computeValidDepthStats(depth_vis, valid_mask, min_depth, max_depth, p2, p98);
    
    if (cv::countNonZero(valid_mask) == 0) {
        return cv::Mat::zeros(depth.size(), CV_8UC3);
    }
    
    cv::Mat result;
    
    if (use_fixed_range_ || mode == VisualizationMode::FIXED_RANGE) {
        switch (mode) {
            case VisualizationMode::BASIC:
            case VisualizationMode::FIXED_RANGE:
                result = visualizeFixedRange(depth_vis, colormap);
                break;
            case VisualizationMode::ENHANCED:
                result = visualizeFixedRangeEnhanced(depth_vis, colormap, clahe_clip, bilateral_d, bilateral_sigma);
                break;
            case VisualizationMode::EDGE_ENHANCED:
                result = visualizeFixedRangeEnhanced(depth_vis, colormap, clahe_clip, bilateral_d, bilateral_sigma);
                {
                    cv::Mat depth_normalized;
                    double scale = 255.0 / (fixed_max_depth_ - fixed_min_depth_);
                    double shift = -fixed_min_depth_ * scale;
                    depth_vis.convertTo(depth_normalized, CV_8U, scale, shift);
                    cv::threshold(depth_normalized, depth_normalized, 0, 255, cv::THRESH_TRUNC);
                    
                    cv::Mat edges;
                    cv::Canny(depth_normalized, edges, 50, 150);
                    cv::Mat edges_colored;
                    cv::cvtColor(edges, edges_colored, cv::COLOR_GRAY2BGR);
                    cv::addWeighted(result, 0.8, edges_colored, 0.2, 0, result);
                }
                break;
            case VisualizationMode::HIGH_CONTRAST:
                result = visualizeFixedRangeEnhanced(depth_vis, colormap, clahe_clip * 1.5f, bilateral_d, bilateral_sigma);
                {
                    cv::Mat gray;
                    cv::cvtColor(result, gray, cv::COLOR_BGR2GRAY);
                    double gamma = 0.7;
                    cv::Mat lookup_table(1, 256, CV_8U);
                    uchar* p = lookup_table.ptr();
                    for (int i = 0; i < 256; ++i) {
                        p[i] = cv::saturate_cast<uchar>(std::pow(i / 255.0, gamma) * 255.0);
                    }
                    cv::Mat gamma_corrected;
                    cv::LUT(gray, lookup_table, gamma_corrected);
                    cv::applyColorMap(gamma_corrected, result, 
                        colormap == ColorMap::TURBO ? cv::COLORMAP_TURBO : cv::COLORMAP_VIRIDIS);
                }
                break;
            default:
                result = visualizeFixedRange(depth_vis, colormap);
                break;
        }
    } else {
        switch (mode) {
            case VisualizationMode::BASIC:
                result = visualizeBasic(depth_vis, colormap);
                break;
            case VisualizationMode::ENHANCED:
                result = visualizeEnhanced(depth_vis, colormap, clahe_clip, bilateral_d, bilateral_sigma);
                break;
            case VisualizationMode::EDGE_ENHANCED:
                result = visualizeEdgeEnhanced(depth_vis, colormap, clahe_clip, bilateral_d, bilateral_sigma);
                break;
            case VisualizationMode::HIGH_CONTRAST:
                result = visualizeHighContrast(depth_vis, colormap);
                break;
            default:
                result = visualizeEnhanced(depth_vis, colormap, clahe_clip, bilateral_d, bilateral_sigma);
                break;
        }
    }
    
    return result;
}

// 固定范围基础可视化
cv::Mat DepthVisualizer::visualizeFixedRange(const cv::Mat& depth, ColorMap colormap) {
    cv::Mat depth_clipped = depth.clone();
    
    cv::threshold(depth_clipped, depth_clipped, fixed_max_depth_, fixed_max_depth_, cv::THRESH_TRUNC);
    cv::threshold(depth_clipped, depth_clipped, fixed_min_depth_, fixed_min_depth_, cv::THRESH_TOZERO);
    
    DepthColorMappingMethod current_method;
    {
        std::lock_guard<std::mutex> lock(mapping_mutex_);
        current_method = depth_color_mapping_method_;
    }
    
    cv::Mat depth_normalized;
    
    if (current_method == DepthColorMappingMethod::LINEAR) {
        double scale = 255.0 / (fixed_max_depth_ - fixed_min_depth_);
        double shift = -fixed_min_depth_ * scale;
        depth_clipped.convertTo(depth_normalized, CV_8U, scale, shift);
        return applyColorMap(depth_normalized, colormap);
        
    } else if (current_method == DepthColorMappingMethod::LOG) {
        cv::Mat depth_log;
        cv::log(depth_clipped + 1.0f, depth_log);
        double log_max = std::log(static_cast<double>(fixed_max_depth_) + 1.0);
        cv::divide(depth_log, log_max, depth_normalized);
        depth_normalized *= 255.0;
        depth_normalized.convertTo(depth_normalized, CV_8U);
        return applyColorMap(depth_normalized, colormap);
        
    } else if (current_method == DepthColorMappingMethod::POWER_LAW) {
        // 新增：幂律映射，与视差图逻辑类似
        cv::Mat depth_float;
        depth_clipped.convertTo(depth_float, CV_64F);
        double min_val, max_val;
        cv::minMaxLoc(depth_float, &min_val, &max_val);
        cv::Mat norm = (depth_float - min_val) / (max_val - min_val);
        cv::Mat pow_norm;
        cv::pow(norm, 0.80, pow_norm); // 固定 gamma = 0.80，你可根据需求调整
        pow_norm *= 255.0;
        pow_norm.convertTo(depth_normalized, CV_8U);
        return applyColorMap(depth_normalized, colormap);
        
    } else {
        // HSV_RAINBOW 或者默认，使用原来的分段线性
        cv::Mat depth_float;
        depth_clipped.convertTo(depth_float, CV_64F);
        depth_normalized.create(depth.size(), CV_8U);
        
        for (int y = 0; y < depth.rows; y++) {
            for (int x = 0; x < depth.cols; x++) {
                float d = depth_float.at<double>(y, x);
                float norm_d;
                if (d < 2.0f) {
                    norm_d = d / 2.0f * 0.5f;
                } else if (d < 5.0f) {
                    norm_d = 0.5f + (d - 2.0f) / 3.0f * 0.25f;
                } else {
                    norm_d = 0.75f + (d - 5.0f) / 25.0f * 0.25f;
                }
                depth_normalized.at<uchar>(y, x) = static_cast<uchar>(norm_d * 255.0f);
            }
        }
        return applyColorMap(depth_normalized, colormap);
    }
}

// 固定范围增强可视化
cv::Mat DepthVisualizer::visualizeFixedRangeEnhanced(const cv::Mat& depth, ColorMap colormap,
                                                     float clahe_clip, int bilateral_d, float bilateral_sigma) {
    cv::Mat depth_clipped = depth.clone();
    
    cv::threshold(depth_clipped, depth_clipped, fixed_max_depth_, fixed_max_depth_, cv::THRESH_TRUNC);
    cv::threshold(depth_clipped, depth_clipped, fixed_min_depth_, fixed_min_depth_, cv::THRESH_TOZERO);
    
    DepthColorMappingMethod current_method;
    {
        std::lock_guard<std::mutex> lock(mapping_mutex_);
        current_method = depth_color_mapping_method_;
    }
    
    cv::Mat depth_normalized;
    bool use_turbo_colormap = true;
    
    if (current_method == DepthColorMappingMethod::LINEAR) {
        double scale = 255.0 / (fixed_max_depth_ - fixed_min_depth_);
        double shift = -fixed_min_depth_ * scale;
        depth_clipped.convertTo(depth_normalized, CV_8U, scale, shift);
        
    } else if (current_method == DepthColorMappingMethod::LOG) {
        cv::Mat depth_log;
        cv::log(depth_clipped + 1.0f, depth_log);
        double log_max = std::log(static_cast<double>(fixed_max_depth_) + 1.0);
        cv::divide(depth_log, log_max, depth_normalized);
        depth_normalized *= 255.0;
        depth_normalized.convertTo(depth_normalized, CV_8U);
        
    } else if (current_method == DepthColorMappingMethod::POWER_LAW) {
        // 新增：幂律映射
        cv::Mat depth_float;
        depth_clipped.convertTo(depth_float, CV_64F);
        double min_val, max_val;
        cv::minMaxLoc(depth_float, &min_val, &max_val);
        cv::Mat norm = (depth_float - min_val) / (max_val - min_val);
        cv::Mat pow_norm;
        cv::pow(norm, 0.45, pow_norm);
        pow_norm *= 255.0;
        pow_norm.convertTo(depth_normalized, CV_8U);
        
    } else {
        // HSV_RAINBOW
        cv::Mat depth_float;
        depth_clipped.convertTo(depth_float, CV_64F);
        depth_normalized.create(depth.size(), CV_8U);
        
        for (int y = 0; y < depth.rows; y++) {
            for (int x = 0; x < depth.cols; x++) {
                float d = depth_float.at<double>(y, x);
                float norm_d;
                if (d < 2.0f) {
                    norm_d = d / 2.0f * 0.5f;
                } else if (d < 5.0f) {
                    norm_d = 0.5f + (d - 2.0f) / 3.0f * 0.25f;
                } else {
                    norm_d = 0.75f + (d - 5.0f) / 25.0f * 0.25f;
                }
                depth_normalized.at<uchar>(y, x) = static_cast<uchar>(norm_d * 255.0f);
            }
        }
        use_turbo_colormap = true;
    }
    
    cv::Mat depth_enhanced;
    clahe_->setClipLimit(clahe_clip);
    clahe_->apply(depth_normalized, depth_enhanced);
    
    cv::Mat depth_filtered;
    cv::bilateralFilter(depth_enhanced, depth_filtered, bilateral_d, bilateral_sigma, bilateral_sigma);
    
    if (use_turbo_colormap) {
        return applyColorMap(depth_filtered, colormap);
    } else {
        return depth_filtered;
    }
}

cv::Mat DepthVisualizer::visualizeBasic(const cv::Mat& depth, ColorMap colormap) {
    cv::Mat depth_normalized;
    cv::normalize(depth, depth_normalized, 0, 255, cv::NORM_MINMAX, CV_8U);
    return applyColorMap(depth_normalized, colormap);
}

cv::Mat DepthVisualizer::visualizeEnhanced(const cv::Mat& depth, ColorMap colormap,
                                          float clahe_clip, int bilateral_d, float bilateral_sigma) {
    cv::Mat valid_mask;
    double min_depth, max_depth, p2, p98;
    computeValidDepthStats(depth, valid_mask, min_depth, max_depth, p2, p98);
    
    cv::Mat depth_clipped;
    cv::threshold(depth, depth_clipped, p98, p98, cv::THRESH_TRUNC);
    cv::threshold(depth_clipped, depth_clipped, p2, p2, cv::THRESH_TOZERO);
    
    cv::Mat depth_normalized;
    cv::normalize(depth_clipped, depth_normalized, 0, 255, cv::NORM_MINMAX, CV_8U);
    
    cv::Mat depth_enhanced;
    clahe_->setClipLimit(clahe_clip);
    clahe_->apply(depth_normalized, depth_enhanced);
    
    cv::Mat depth_filtered;
    cv::bilateralFilter(depth_enhanced, depth_filtered, bilateral_d, bilateral_sigma, bilateral_sigma);
    
    return applyColorMap(depth_filtered, colormap);
}

cv::Mat DepthVisualizer::visualizeEdgeEnhanced(const cv::Mat& depth, ColorMap colormap,
                                              float clahe_clip, int bilateral_d, float bilateral_sigma) {
    cv::Mat enhanced_result = visualizeEnhanced(depth, colormap, clahe_clip, bilateral_d, bilateral_sigma);
    
    cv::Mat valid_mask;
    double min_depth, max_depth, p2, p98;
    computeValidDepthStats(depth, valid_mask, min_depth, max_depth, p2, p98);
    
    cv::Mat depth_clipped;
    cv::threshold(depth, depth_clipped, p98, p98, cv::THRESH_TRUNC);
    cv::threshold(depth_clipped, depth_clipped, p2, p2, cv::THRESH_TOZERO);
    
    cv::Mat depth_normalized;
    cv::normalize(depth_clipped, depth_normalized, 0, 255, cv::NORM_MINMAX, CV_8U);
    
    cv::Mat depth_enhanced;
    clahe_->setClipLimit(clahe_clip);
    clahe_->apply(depth_normalized, depth_enhanced);
    
    cv::Mat depth_filtered;
    cv::bilateralFilter(depth_enhanced, depth_filtered, bilateral_d, bilateral_sigma, bilateral_sigma);
    
    cv::Mat edges;
    cv::Canny(depth_filtered, edges, 50, 150);
    
    cv::Mat edges_colored;
    cv::cvtColor(edges, edges_colored, cv::COLOR_GRAY2BGR);
    
    cv::Mat result;
    cv::addWeighted(enhanced_result, 0.8, edges_colored, 0.2, 0, result);
    
    return result;
}

cv::Mat DepthVisualizer::visualizeHighContrast(const cv::Mat& depth, ColorMap colormap) {
    cv::Mat valid_mask;
    double min_depth, max_depth, p2, p98;
    computeValidDepthStats(depth, valid_mask, min_depth, max_depth, p2, p98);
    
    double p1, p99;
    cv::minMaxLoc(depth, &p1, &p99, nullptr, nullptr, valid_mask);
    
    cv::Mat depth_clipped;
    cv::threshold(depth, depth_clipped, p99, p99, cv::THRESH_TRUNC);
    cv::threshold(depth_clipped, depth_clipped, p1, p1, cv::THRESH_TOZERO);
    
    cv::Mat depth_normalized;
    cv::normalize(depth_clipped, depth_normalized, 0, 255, cv::NORM_MINMAX, CV_8U);
    
    cv::Mat depth_enhanced;
    cv::Ptr<cv::CLAHE> strong_clahe = cv::createCLAHE(5.0, cv::Size(4, 4));
    strong_clahe->apply(depth_normalized, depth_enhanced);
    
    cv::Mat depth_gamma;
    double gamma = 0.7;
    cv::Mat lookup_table(1, 256, CV_8U);
    uchar* p = lookup_table.ptr();
    for (int i = 0; i < 256; ++i) {
        p[i] = cv::saturate_cast<uchar>(std::pow(i / 255.0, gamma) * 255.0);
    }
    cv::LUT(depth_enhanced, lookup_table, depth_gamma);
    
    cv::Mat depth_smooth;
    cv::GaussianBlur(depth_gamma, depth_smooth, cv::Size(3, 3), 0);
    
    return applyColorMap(depth_smooth, colormap);
}

cv::Mat DepthVisualizer::applyColorMap(const cv::Mat& gray_image, ColorMap colormap) {
    cv::Mat colored_image;
    
    switch (colormap) {
        case ColorMap::VIRIDIS:
            cv::applyColorMap(gray_image, colored_image, cv::COLORMAP_VIRIDIS);
            break;
        case ColorMap::JET:
            cv::applyColorMap(gray_image, colored_image, cv::COLORMAP_JET);
            break;
        case ColorMap::PLASMA:
            cv::applyColorMap(gray_image, colored_image, cv::COLORMAP_PLASMA);
            break;
        case ColorMap::INFERNO:
            cv::applyColorMap(gray_image, colored_image, cv::COLORMAP_INFERNO);
            break;
        case ColorMap::TURBO:
            cv::applyColorMap(gray_image, colored_image, cv::COLORMAP_TURBO);
            break;
        default:
            cv::applyColorMap(gray_image, colored_image, cv::COLORMAP_VIRIDIS);
            break;
    }
    
    return colored_image;
}

void DepthVisualizer::computeValidDepthStats(const cv::Mat& depth, cv::Mat& valid_mask,
                                            double& min_depth, double& max_depth,
                                            double& p2, double& p98) {
    cv::threshold(depth, valid_mask, 0.1, 255, cv::THRESH_BINARY);
    valid_mask.convertTo(valid_mask, CV_8U);
    
    cv::minMaxLoc(depth, &min_depth, &max_depth, nullptr, nullptr, valid_mask);
    
    cv::Mat valid_depths;
    depth.copyTo(valid_depths, valid_mask);
    
    std::vector<float> depths_vec;
    valid_depths.reshape(1, 1).copyTo(depths_vec);
    
    if (!depths_vec.empty()) {
        std::sort(depths_vec.begin(), depths_vec.end());
        size_t p2_idx = static_cast<size_t>(depths_vec.size() * 0.02);
        size_t p98_idx = static_cast<size_t>(depths_vec.size() * 0.98);
        
        p2 = depths_vec[p2_idx];
        p98 = depths_vec[p98_idx];
    } else {
        p2 = min_depth;
        p98 = max_depth;
    }
}