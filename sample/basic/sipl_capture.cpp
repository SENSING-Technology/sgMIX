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
        sgmix::CaptureConfig capture;
        // SIPL uses the bound native pipeline; dimensions and format come from the driver.
        // A single-sensor module can omit siplChannel; use the stereo example otherwise.
        // Start streaming after SG_Init; asynchronous pipeline errors surface in SG_GetFrame.
        check(camera.SG_StartCapture(capture, error));
        // Frame owns the returned pixels and metadata; the next read into this object replaces them.
        sgmix::Frame frame;
        for (unsigned i = 0; i < 30; ++i) {
            // Wait up to 3000 milliseconds for a new frame. Timeout/stop/error returns false
            // and clears frame; check() exits this example on any such failure.
            check(camera.SG_GetFrame(frame, 3000, error));
            std::cout << "frame=" << frame.sequence << " size=" << frame.width << 'x' << frame.height
                      << " bytes=" << frame.data.size() << '\n';
            // Process frame.data using pixelFormat, dimensions and planes.
            // Copy or move the Frame before reusing it if another thread needs this image.
        }
        // Stop streaming and release capture resources; repeated stops are safe.
        check(camera.SG_StopCapture(error));
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
    return 0;
}
