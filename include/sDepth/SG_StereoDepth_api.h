#pragma once
#include <string>
#include "SG_StereoDepth_def.h"

namespace sDepth {

/**
 * @brief 初始化立体深度SDK
 * 
 * 初始化参数，对图像进行预处理
 * 
 * @param frame 左右目图像，在初始化中进行预处理
 * @param config SDK初始化配置参数，包括相机标定文件路径及深度估计模型路径
 * @return true 初始化成功
 * @return false 初始化失败（例如文件不存在或参数错误）
 */
bool SG_STEREO_INIT(StereoFrame frame, StereoConfig config);

/**
 * @brief 生成单帧立体深度图像
 * 
 * 对输入的原始图像进行立体矫正和深度处理，并输出处理后的结果。
 * 
 * @param stereodepth_image 输出立体深度图像
 * @return true 图像处理成功
 * @return false 图像处理失败（例如SDK未初始化）
 */
bool SG_STEREO_PROCESS(cv::Mat& stereodepth_image);

/**
 * @brief 释放SDK内部资源
 * 
 * 释放SDK在初始化或处理过程中分配的内存、缓存和其他资源。
 * 
 * @return true 成功释放资源
 * @return false 释放资源失败
 */
bool SG_STEREO_DEINIT();

}