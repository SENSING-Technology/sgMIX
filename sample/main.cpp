#include "SG_CameraControl.h"
#include <cstdint>
#include <string>
#include <iostream>
#include <iomanip>
#include <cstring>

int main(int argc, char *argv[]) {
    std::string device_video0 = "/dev/video0";
    std::string cameraType = "SHW3H";  // "SHW3H" or "SHW3G"
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

    return 0;
}