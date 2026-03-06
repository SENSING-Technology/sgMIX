/*
 * sDepth.cpp
 * 
 * Brief Description:
 * This program demonstrates stereo depth processing functionality using the SG_CameraControl API
 * to generate depth maps from stereo image pairs.
 * 
 * Features:
 * - Initialize stereo processing with configuration files and encryption key
 * - Load left and right stereo images
 * - Process stereo images to generate depth information
 * - Save the depth processing result as an image file
 * - Properly deinitialize stereo processing resources
 * 
 * Usage:
 * ./sDepth <keyfile_path>
 *
 * Note:
 * - sDepth functionality is exclusively available on S36 camera models
 */

#include <iostream>
#include "SG_CameraControl.h"

using namespace sgmix;

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <keyfile_name> " << std::endl;
        std::cerr << "Example: " << argv[0] << "./demo /home/nvidia/stereodepth.key" << std::endl;
        return 1;
    }
    std::string leftimg_path = "../picture/left0000.png";
    std::string rightimg_path = "../picture/right0000.png";
    CameraControl camera;
    StereoFrame frame;
    StereoConfig config;

    frame.left_rect = cv::imread(leftimg_path);
    frame.right_rect = cv::imread(rightimg_path);
    config.json_file = "../config/stereo_calibration.json";
    config.enc_file = "../config/stereodepth.enc";
    config.key_file = argv[1];
    bool ret = camera.SG_StereoInit(frame, config);
    cv::Mat temp_result;
    ret = camera.SG_StereoProcess(temp_result);
    if (ret) {
        std::cout << "SG_STEREO_PROCESS sucessful!!!" << std::endl;
        cv::imwrite("temp_result.jpg", temp_result);
    }

    camera.SG_StereoDeinit();
    
    return 0;
}