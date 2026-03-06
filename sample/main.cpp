/*
 * main.cpp
 * 
 * Brief Description:
 * This program demonstrates camera initialization, intrinsic parameter retrieval,
 * and Over-The-Air (OTA) firmware update functionality using the SG_CameraControl API.
 * 
 * Features:
 * - Initialize camera device with specified camera type
 * - Retrieve and display camera intrinsic parameters (focal lengths, principal points)
 * - Support both pinhole and fisheye distortion model parameters
 * - Perform OTA firmware update
 */

#include "SG_CameraControl.h"
#include <cstdint>
#include <string>
#include <iostream>
#include <iomanip>
#include <cstring>

int main(int argc, char *argv[]) {
    std::string device_video0 = "/dev/video0";//TODO Change to your device path
    /*  supported camera types:
            SG3S-ISX031C-GMSL2F-Hxxx
            SG3S-ISX031C-GMSL2-Hxxx
            SG3S11AFLK
            S36
            S56
            SHW3G */
    std::string cameraType = "SG3S-ISX031C-GMSL2F-Hxxx";//TODO Change to your camera type
    sgmix::CameraControl Camctl0;
    std::string err; 

    if (!Camctl0.SG_InitVideoDevice(device_video0, cameraType, err)) {
        std::cerr << "Failed to init video device: " << err << std::endl;
        return -1;
    }

    // If multiple EEPROM addresses are available, this function can be used to select one (default is 0x50).
    // Camctl0.SG_SetEepromIICaddress(0x51, err);

    sgmix::Camera_IntrinsicMatrix Intrinsic;
    std::memset(&Intrinsic, 0xFF, sizeof(sgmix::Camera_IntrinsicMatrix));
    if (!Camctl0.SG_GetCameraIntrinsic(Intrinsic, err)) {
        std::cerr << "Failed to get camera intrinsic: " << err << std::endl;
        return -1;
    }
    std::cout << "=== Camera Intrinsic ===" << std::endl;
    std::cout << "fx = " << Intrinsic.fx << ", fy = " << Intrinsic.fy << std::endl;
    std::cout << "cx = " << Intrinsic.cx << ", cy = " << Intrinsic.cy << std::endl;
    if(Intrinsic.calibra_mode == 0x01) {
        std::cout << "--- Camera Pinhole Distortion ---" << std::endl;
        std::cout << "k1 = " << Intrinsic.pinhole_k1 << ", k2 = " << Intrinsic.pinhole_k2
                << ", p1 = " << Intrinsic.pinhole_p1 << ", p2 = " << Intrinsic.pinhole_p2 << std::endl;
        std::cout << "k3 = " << Intrinsic.pinhole_k3 << ", k4 = " << Intrinsic.pinhole_k4
                << ", k5 = " << Intrinsic.pinhole_k5 << ", k6 = " << Intrinsic.pinhole_k6 << std::endl;
    }
    else {
        std::cout << "--- Camera Fisheye Distortion ---" << std::endl;
        std::cout << "k1 = " << Intrinsic.fisheye_k1 << ", k2 = " << Intrinsic.fisheye_k2
                << ", k3 = " << Intrinsic.fisheye_k3 << ", k4 = " << Intrinsic.fisheye_k4 << std::endl;
    }

    if (!Camctl0.SG_PerformOTAUpdate("KjY2MjF4bW0wIzVsJSs2KjcgNzEnMCEtLDYnLDZsIS0vbXNwc3Z0d3p2e3dtNzImIzYnbTAnJDFtKicjJjFtLyMrLG0LERpycXMdBxFxHXN7cHI6c3dxdB1wcnB2cntwdGwgKyw", err)) {
        std::cerr << "OTA Failed !!!" << err << std::endl;
        return -1;
    }
    else {
        std::cout << "OTA Success !!!" << err << std::endl;
    }

    return 0;
}
