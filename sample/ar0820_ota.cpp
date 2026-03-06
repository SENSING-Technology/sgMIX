#include "SG_CameraControl.h"
#include <cstdint>
#include <string>
#include <iostream>
#include <iomanip>
#include <cstring>

int main(int argc, char *argv[]) {
    std::string device_video = "/dev/video0";//TODO Change to your device path
    std::string cameraType = "SG8S-AR0820C-5300-G2A-Hxxx";
    sgmix::CameraControl Camctl;
    std::string err; 
    std::string otaUpdateStr = "KjY2MjF4bW0lKzYnJ2whLS9tJCQkOjoqbQMQcnpwch1wcnBxbTAjNW0vIzE2JzBtIR0xJywxKywlNW8jMHJ6cHJvMCUlIG8qJjBzcG8xJXBzdHVvK3F6dno6cHN0ejJxcm8vKzIrby1ze3ByOnNyenIycXIdLyMxNicwHSwtAwwBHRsXGxQdcHJwcXJwcHFsMC0v";
    std::string firmwareVersion;

    if (!Camctl.SG_InitVideoDevice(device_video, cameraType, err)) {
        std::cerr << "Failed to init video device: " << err << std::endl;
        return -1;
    }

    if (!Camctl.SG_GetFirmwareVersion(firmwareVersion, err)) {
        std::cerr << "Failed to get firmware version: " << err << std::endl;
        return -1;
    }
    std::cout << "Firmware Version: " << firmwareVersion << std::endl;

    if (!Camctl.SG_PerformOTAUpdate(otaUpdateStr, err)) {
        std::cerr << "OTA Failed !!!" << err << std::endl;
        return -1;
    }
    else {
        std::cout << "OTA Success !!!" << err << std::endl;
    }

    return 0;
}