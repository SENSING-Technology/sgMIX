#ifndef DEPTH_VISUALIZER_H
#define DEPTH_VISUALIZER_H

#include <opencv2/opencv.hpp>
#include <string>

// 深度可视化模式
enum class VisualizationMode {
    BASIC,           // 基础模式：简单归一化
    ENHANCED,        // 增强模式：百分位数裁剪 + CLAHE + 双边滤波
    EDGE_ENHANCED,   // 边缘增强模式：增强模式 + 边缘检测叠加
    HIGH_CONTRAST,   // 高对比度模式：更激进的对比度增强
    FIXED_RANGE      // 【新增】固定范围模式：解决闪烁问题
};

// 颜色映射类型
enum class ColorMap {
    VIRIDIS,
    JET,
    PLASMA,
    INFERNO,
    TURBO
};

// 深度颜色映射方式（运行时切换）
enum class DepthColorMappingMethod {
    LINEAR,      // 方式1：线性归一化 + TURBO
    LOG,         // 方式2：对数归一化 + TURBO
    HSV_RAINBOW, // 方式3：HSV 七色渐变（红-橙-黄-绿-青-蓝-紫）
    POWER_LAW    // 方式4：幂律映射（gamma 可调，拉伸远处层次）
};

// 深度可视化器类
class DepthVisualizer {
public:
    DepthVisualizer();
    ~DepthVisualizer() = default;
    
    // 禁用拷贝构造和赋值
    DepthVisualizer(const DepthVisualizer&) = delete;
    DepthVisualizer& operator=(const DepthVisualizer&) = delete;
    
    // 深度图可视化
    cv::Mat visualizeDepth(const cv::Mat& depth, 
                           VisualizationMode mode = VisualizationMode::ENHANCED,
                           ColorMap colormap = ColorMap::VIRIDIS);
    
    // 高级深度图可视化
    cv::Mat visualizeDepthAdvanced(const cv::Mat& depth,
                                   VisualizationMode mode = VisualizationMode::ENHANCED,
                                   ColorMap colormap = ColorMap::VIRIDIS,
                                   float clahe_clip = 3.0f,
                                   int bilateral_d = 9,
                                   float bilateral_sigma = 75.0f);
    
    // 【新增】设置固定深度范围（解决闪烁的关键）
    void setFixedDepthRange(float min_depth, float max_depth);
    void enableFixedRange(bool enable);
    bool isFixedRangeEnabled() const { return use_fixed_range_; }
    float getFixedMinDepth() const { return fixed_min_depth_; }
    float getFixedMaxDepth() const { return fixed_max_depth_; }
    
    // 设置可视化参数
    void setClaheClipLimit(float clip_limit) { clahe_clip_limit_ = clip_limit; }
    void setBilateralParams(int d, float sigma_color, float sigma_space) {
        bilateral_d_ = d;
        bilateral_sigma_color_ = sigma_color;
        bilateral_sigma_space_ = sigma_space;
    }
    
    // 获取当前参数
    float getClaheClipLimit() const { return clahe_clip_limit_; }
    int getBilateralD() const { return bilateral_d_; }
    float getBilateralSigmaColor() const { return bilateral_sigma_color_; }
    float getBilateralSigmaSpace() const { return bilateral_sigma_space_; }
    
    // 深度颜色映射方式设置/获取（运行时切换）
    void setDepthColorMappingMethod(DepthColorMappingMethod method);
    DepthColorMappingMethod getDepthColorMappingMethod() const;

private:
    // 基础可视化
    cv::Mat visualizeBasic(const cv::Mat& depth, ColorMap colormap);
    
    // 增强可视化
    cv::Mat visualizeEnhanced(const cv::Mat& depth, ColorMap colormap,
                             float clahe_clip, int bilateral_d, float bilateral_sigma);
    
    // 边缘增强可视化
    cv::Mat visualizeEdgeEnhanced(const cv::Mat& depth, ColorMap colormap,
                                 float clahe_clip, int bilateral_d, float bilateral_sigma);
    
    // 高对比度可视化
    cv::Mat visualizeHighContrast(const cv::Mat& depth, ColorMap colormap);
    
    // 【新增】固定范围可视化
    cv::Mat visualizeFixedRange(const cv::Mat& depth, ColorMap colormap);
    cv::Mat visualizeFixedRangeEnhanced(const cv::Mat& depth, ColorMap colormap,
                                        float clahe_clip, int bilateral_d, float bilateral_sigma);
    
    // 应用颜色映射
    cv::Mat applyColorMap(const cv::Mat& gray_image, ColorMap colormap);
    
    // 计算有效深度值的统计信息
    void computeValidDepthStats(const cv::Mat& depth, cv::Mat& valid_mask,
                               double& min_depth, double& max_depth,
                               double& p2, double& p98);
    
    // 可视化参数
    float clahe_clip_limit_ = 3.0f;
    int bilateral_d_ = 9;
    float bilateral_sigma_color_ = 75.0f;
    float bilateral_sigma_space_ = 75.0f;
    
    // 【新增】固定范围参数
    bool use_fixed_range_ = false;
    float fixed_min_depth_ = 0.03f;
    float fixed_max_depth_ = 30.0f;
    
    // 深度颜色映射方式参数
    DepthColorMappingMethod depth_color_mapping_method_ = DepthColorMappingMethod::LINEAR;
    
    // 互斥锁（线程安全）
    mutable std::mutex mapping_mutex_;
    
    // CLAHE对象（可重用）
    cv::Ptr<cv::CLAHE> clahe_;
};

#endif // DEPTH_VISUALIZER_H