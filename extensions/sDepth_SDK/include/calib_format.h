#ifndef CALIB_FORMAT_H
#define CALIB_FORMAT_H

#include <opencv2/core.hpp>
#include <string>

/**
 * 双目标定数据（统一结构，txt / json 两种来源都解析到该结构）
 *
 * txt 格式（客户交付格式，每台模组一份，文件名=模组序列号）：
 *   type= fisheye | pinhole
 *   imageWidth= 2560 / imageHeight= 1984
 *   CAM0_fx= ... CAM0_fy= ... CAM0_cx= ... CAM0_cy= ...   （左目）
 *   CAM0_k1= ... （fisheye: k1~k4；pinhole: k1,k2,p1,p2[,k3,k4,k5,k6]）
 *   CAM1_* 同上（右目）
 *   R[0,0]= ... R[2,2]= ...   （3x3 旋转）
 *   T[0,0]= ... T[2,0]= ...   （3x1 平移，单位 mm）
 *
 * json 格式（历史格式，字段名保持兼容）：
 *   camera_model / left_camera_matrix / right_camera_matrix /
 *   left_distortion_coefficients / right_distortion_coefficients /
 *   rotation_matrix / translation_vector / calibration_size /
 *   fisheye_balance / fisheye_fov_scale / auto_crop_black_border
 */
struct StereoCalibData {
    std::string camera_model;        // "fisheye" | "pinhole"
    cv::Size image_size{0, 0};       // 标定分辨率（= 采集分辨率时无需缩放内参）
    cv::Mat K_left, K_right;         // 3x3 CV_64F
    cv::Mat dist_left, dist_right;   // 1xN CV_64F（fisheye N=4；pinhole N=5 或 8）
    cv::Mat R, T;                    // 3x3 / 3x1 CV_64F，T 单位 m（txt 源为 mm，解析时已换算）

    // 以下为 json 可选扩展字段（txt 无此概念，用默认值）
    double fisheye_balance = 0.0;
    double fisheye_fov_scale = 1.0;
    bool auto_crop_black_border = true;
};

/**
 * 按扩展名自动识别并加载标定文件（.txt → 客户交付格式；其它 → json）。
 * 解析失败抛 std::runtime_error（带行号/字段名）。
 */
StereoCalibData LoadStereoCalibration(const std::string& path);

/** txt 格式解析（key=value，容错空格；畸变系数按 key 取，与行序无关） */
StereoCalibData ParseStereoCalibrationTxt(const std::string& path);

/** json 格式解析（历史兼容） */
StereoCalibData ParseStereoCalibrationJson(const std::string& path);

/** 导出为 json 标定文件（txt→json 转换工具用） */
void SaveStereoCalibrationJson(const StereoCalibData& calib, const std::string& path);

#endif // CALIB_FORMAT_H
