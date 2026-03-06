/*
 * Imu.cpp
 * 
 * Brief Description:
 * This program demonstrates how to initialize the camera device and start the IMU (Inertial Measurement Unit) data stream
 * using the SG_CameraControl API.
 * 
 * Features:
 * - Initialize camera device
 * - Start the IMU data stream for sensor data collection
 */

#include "SG_CameraControl.h"
#include <iostream>

using namespace sgmix;

int main() {
    CameraControl camera;
    std::string error;
    if (!camera.SG_InitVideoDevice("/dev/video4", "S56", error)) {
        std::cerr << "Failed to init video device: " << error << std::endl;
        return -1;
    }
    if (!camera.SG_StartImuStream(error)) {
        std::cerr << "Failed to start imu stream: " << error << std::endl;
        return -1;
    }

    return 0;
}