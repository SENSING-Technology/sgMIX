/*
 * Frame.cpp
 * 
 * Brief Description:
 * This program demonstrates camera initialization, frame capture, and timestamp retrieval
 * using the SG_CameraControl API.
 * 
 * Features:
 * - Initialize camera device
 * - Start video capture
 * - Retrieve frame with timestamp
 * - Get latest frame timestamp
 * - Properly stop capture and shutdown camera
 */

#include <iostream>
#include <unistd.h>
#include <opencv2/opencv.hpp>
#include "SG_CameraControl.h"

int main() {
    sgmix::CameraControl cameraCtrl;
    sgmix::CameraInfo cameraInfo;
    
    std::string error;

    std::string pipeline_desc = ""; 

    if (!cameraCtrl.SG_InitVideoDevice("/dev/video4", "S56", error)) {
        std::cerr << "Failed to init video device: " << error << std::endl;
        return -1;
    }

    if(!cameraCtrl.SG_GetCameraInfo(cameraInfo, error)) {
        std::cerr << "Failed to get camera info: " << error << std::endl;
        return -1;
    }

    if (!cameraCtrl.SG_Initialize(pipeline_desc, error)) {
        std::cerr << "Failed to initialize camera: " << error << std::endl;
        return -1;
    }
    std::cout << "Camera initialized successfully." << std::endl;

    bool is_initialized = false;
    if (cameraCtrl.SG_IsInitialized(error)) {
        std::cout << "Camera is initialized." << std::endl;
        is_initialized = true;
    } else {
        std::cerr << "Camera is not initialized: " << error << std::endl;
        return -1;
    }

    if (!cameraCtrl.SG_StartCapture(error)) {
        std::cerr << "Failed to start capture: " << error << std::endl;
        return -1;
    }
    std::cout << "Camera capture started." << std::endl;

    cv::Mat frame;
    int64_t timestamp;
    usleep(5000000);// Wait for 5 seconds to ensure there is enough time to obtain one frame

    if (cameraCtrl.SG_GetFrameWithTimestamp(frame, timestamp, error)) {
        std::cout << "Got frame with timestamp: " << timestamp << std::endl;
        cv::imshow("Captured Frame", frame);
        cv::waitKey(0); 
    } else {
        std::cerr << "Failed to get frame: " << error << std::endl;
        return -1;
    }

    if (cameraCtrl.SG_GetLatestFrameTimestamp(timestamp, error)) {
        std::cout << "Latest frame timestamp: " << timestamp << std::endl;
    } else {
        std::cerr << "Failed to get latest frame timestamp: " << error << std::endl;
        return -1;
    }

    if (!cameraCtrl.SG_StopCapture(error)) {
        std::cerr << "Failed to stop capture: " << error << std::endl;
        return -1;
    }
    std::cout << "Capture stopped." << std::endl;

    if (!cameraCtrl.SG_Shutdown(error)) {
        std::cerr << "Failed to shutdown camera: " << error << std::endl;
        return -1;
    }
    std::cout << "Camera shutdown successfully." << std::endl;

    return 0;
}