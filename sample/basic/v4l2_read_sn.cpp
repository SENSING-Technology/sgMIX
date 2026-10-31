// Minimal example; use the matching saved camera configuration.
#include "SG_CameraControl.h"

#include <iostream>
#include <stdexcept>

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " /dev/videoN\n";
        return 2;
    }
    try {
        // Owns the camera connection; its destructor also stops capture on exceptions.
        sgmix::CameraControl camera;
        // sgMIX calls return false on failure and write a diagnostic into error.
        std::string error;
        auto check = [&](bool ok) {
            if (!ok)
                throw std::runtime_error(error);
        };
        // Bind the V4L2 video node; omitted route fields use SDK discovery/defaults.
        sgmix::CameraConfig device;
        device.videoDevice = argv[1]; // Model comes from the saved video-node mapping.
        // Initialize using the model saved for this video node in /opt/sgMIX/etc/sgmix.conf.
        // The saved framework must be v4l2; initialization alone does not start capture.
        check(camera.SG_Init(device, error));
        std::string serial;
        // Read the stored serial number and validate its CRC; no capture session is required.
        check(camera.SG_GetCameraSerialNumber(serial, error));
        std::cout << "SN: " << serial << '\n';
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
    return 0;
}
