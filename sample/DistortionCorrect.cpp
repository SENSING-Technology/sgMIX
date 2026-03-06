/*
 * DistortionCorrect.cpp
 * 
 * Brief Description:
 * This program performs image distortion correction operations using camera intrinsic parameters
 * to correct input images and save the results to an output file.
 * 
 * Features:
 * - Initialize camera device to obtain camera parameters
 * - Get camera intrinsic matrix
 * - Apply distortion correction algorithm to input image
 * - Save the corrected image to the specified path
 * 
 * Usage:
 * ./DistortionCorrect <input image path> <output image path>
 *
 * Note:
 * - Use the image corresponding to the initialized camera
 */

#include "SG_CameraControl.h"
#include <iostream>

using namespace sgmix;

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cout << "Usage: " << argv[0] << " <input.jpg> <output.jpg>\n";
        return 0;
    }

    CameraControl camera;
    std::string error;
    Camera_IntrinsicMatrix Intrinsic;

    if (!camera.SG_InitVideoDevice("/dev/video4", "S56", error)) {
        std::cerr << "Failed to init video device: " << error << std::endl;
        return -1;
    }
    if (!camera.SG_GetCameraIntrinsic(Intrinsic, error)) {
        std::cerr << "Failed to get camera intrinsic: " << error << std::endl;
        return -1;
    }
    if(!camera.SG_DistortionCorrect(argv[1], argv[2], Intrinsic, error)) {
        std::cerr << "Failed to distortion correct: " << error << std::endl;
        return -1;
    }

    return 0;
}
