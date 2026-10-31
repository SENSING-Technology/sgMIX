// Minimal example; use the matching saved camera configuration.
#include "SG_CameraControl.h"

#include <iostream>
#include <stdexcept>

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " PORT\n";
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
        const std::string text = argv[1];
        if (text.empty() || text.find_first_not_of("0123456789") != std::string::npos)
            throw std::invalid_argument("PORT must be a zero-based decimal CAM number");
        const auto port = std::stoull(text);
        if (port >= UINT32_MAX)
            throw std::invalid_argument("PORT is out of range");
        // Empty JSON path, config name and masks load the saved SIPL settings.
        // port selects a zero-based Driver CAM connector, not a module channel index.
        check(camera.SG_Init("", "", error, {}, static_cast<std::uint32_t>(port)));
        // Receives stored calibration; this API reads existing data rather than calibrating a camera.
        sgmix::CameraCalibration calibration;
        // Read and CRC-validate calibration through the initialized SIPL session.
        // SG_StartCapture is not needed for this data read.
        check(camera.SG_GetCalibration(calibration, error));
        // fx/fy are focal lengths and cx/cy are principal-point coordinates, in pixels.
        const auto print = [](const char* name, const sgmix::CameraCalibration::Intrinsics& k) {
            std::cout << name << ": fx=" << k.fx << " fy=" << k.fy << " cx=" << k.cx << " cy=" << k.cy << '\n';
        };
        print("left", calibration.left);
        // Right intrinsics and stereo extrinsics are optional; check each before accessing it.
        if (calibration.right)
            print("right", *calibration.right);
        if (calibration.stereo) {
            std::cout << "rotation:";
            for (const auto value : calibration.stereo->rotation)
                std::cout << ' ' << value;
            std::cout << "\ntranslation:";
            for (const auto value : calibration.stereo->translation)
                std::cout << ' ' << value;
            std::cout << '\n';
        }
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
    return 0;
}
