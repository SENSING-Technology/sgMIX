#pragma once
#include <string>
#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>

namespace sDepth {

/**
 * @brief 立体图像帧结构
 * 
 * 包含一对待处理的左右摄像头图像。
 */
struct StereoFrame {
    /** 左目原始图像 */
    cv::Mat left_rect;

    /** 右目原始图像 */
    cv::Mat right_rect;
};

/**
 * @brief SDK初始化配置参数
 * 
 * 包含SDK初始化所需的各种配置文件路径。
 */
struct StereoConfig {
    /** JSON 格式的相机参数文件路径（内参、畸变系数等） */
    std::string json_file;

    /** 加密文件路径，用于 SDK 授权或加密数据处理 */
    std::string enc_file;

    /** 密钥文件路径，用于解密或 SDK 授权校验 */
    std::string key_file;
};

}