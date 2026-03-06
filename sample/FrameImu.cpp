/*
 * FrameImu.cpp
 * 
 * Brief Description:
 * This program demonstrates camera frame timestamp monitoring, latency measurement,
 * and IMU (Inertial Measurement Unit) data acquisition using multi-threading with the SG_CameraControl API.
 * 
 * Features:
 * - Initialize camera device and start video capture
 * - Initialize IMU sensors for S56 camera type (Note: IMU functionality is only supported by S56 cameras)
 * - Create a separate monitoring thread to track frame timestamps
 * - Calculate and display latency between frame capture and current system time
 * - Retrieve and display IMU data (acceleration and angular velocity) - only available with S56 cameras
 * - Implement proper thread synchronization and resource management
 * - Run continuous monitoring for 5 minutes with clean shutdown
 * 
 * Note:
 * - IMU functionality is exclusively available on S56 camera models
 */
#include <iostream>
#include <thread>
#include <atomic>
#include <chrono>
#include "SG_CameraControl.h"

std::string device_video = "/dev/video4";
std::string cameraType = "S56";

void TimestampMonitorThread(sgmix::CameraControl& camera, std::atomic<bool>& is_running) {
    while (is_running.load()) {
        std::this_thread::sleep_for(std::chrono::seconds(2));
        
        if (!is_running.load()) break;
        
        int64_t timestamp;
        std::string error;
        
        if (camera.SG_GetLatestFrameTimestamp(timestamp, error)) {
            int64_t system_ts = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count();
            
            int64_t latency_ms = system_ts - timestamp / 1000;  
            
            std::cout << "[Monitor] Frame timestamp: " << timestamp 
                      << " | System time: " << system_ts 
                      << " | Latency: " << latency_ms << " ms" << std::endl;
        } else {
            std::cerr << "[Monitor] Failed to get timestamp: " << error << std::endl;
        }

        if (cameraType == "S56") {
            sgmix::IMUData imu_data;
            if(!camera.SG_GetImuData(imu_data, error)){
                std::cerr << "Failed to get imu data: " << error << std::endl;
            }
            std::cout << "imu accelerated data: " << imu_data.accel_x << " " << imu_data.accel_y << " " << imu_data.accel_z << std::endl;
            std::cout << "imu angular velocity data: " << imu_data.anglvel_x << " " << imu_data.anglvel_y << " " << imu_data.anglvel_z << std::endl;
        }
    }
}

int main() {
    sgmix::CameraControl cameraCtrl;
    std::string error;
    
    if (!cameraCtrl.SG_InitVideoDevice(device_video, cameraType, error)) {
        std::cerr << "Failed to init video device: " << error << std::endl;
        return -1;
    }

    if (!cameraCtrl.SG_Initialize("", error)) {
        std::cerr << "Failed to initialize camera: " << error << std::endl;
        return -1;
    }

    if (!cameraCtrl.SG_StartCapture(error)) {
        std::cerr << "Failed to start capture: " << error << std::endl;
        return -1;
    }
    
    if (cameraType == "S56") {
        if (!cameraCtrl.SG_InitImuAll(error)) {
            std::cerr << "Failed to init imu all: " << error << std::endl;
            return -1;
        }
    }

    std::cout << "Camera capture started. Monitoring timestamps every 5 seconds..." << std::endl;
    std::cout << "Press Ctrl+C to exit." << std::endl;
    
    std::atomic<bool> is_running{true};
    std::thread monitor_thread(TimestampMonitorThread, std::ref(cameraCtrl), std::ref(is_running));
    
    std::this_thread::sleep_for(std::chrono::minutes(5));  // 运行5分钟

    std::cout << "Shutting down..." << std::endl;
    is_running.store(false);
    
    if (monitor_thread.joinable()) {
        monitor_thread.join();
    }

    cameraCtrl.SG_StopCapture(error);
    cameraCtrl.SG_Shutdown(error);
    
    std::cout << "Camera shutdown successfully." << std::endl;
    return 0;
}
