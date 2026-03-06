/*
 * GetEBD.cpp
 * 
 * Brief Description:
 * This program demonstrates how to retrieve camera EBD (Exposure, Brightness, Dynamic) information
 * using the SG_CameraControl API.
 * 
 * Features:
 * - Initialize camera device
 * - Retrieve camera EBD data (exposure time, brightness, dynamic information)
 * - Display the exposure time value
 *
 * Note:
 * - IMU functionality is exclusively available on S36、SG3S-ISX031C-GMSL2F-Hxxx、SG3S-ISX031C-GMSL2-Hxxx camera models
 * - The camera must be in the state of having started the video stream in order to obtain it.
 */

#include "SG_CameraControl.h"
#include <iostream>

using namespace sgmix;

int main() {
    CameraControl camera;
    std::string error;
    SensorDataFrame frame;
    int resolution=1;
    int trigger_mode=1;
    if (!camera.SG_InitVideoDevice("/dev/video4", "S36", error)) {
        std::cerr << "Failed to init video device: " << error << std::endl;
        return -1;
    }
    if (!camera.SG_GetCameraEBD(frame,error)) {
        std::cerr << "Failed to get camera ebd " << error << std::endl;
        return -1;
    }
    std::cout << "Camera EBD: " << frame.exposure_time << std::endl;
    return 0;
}