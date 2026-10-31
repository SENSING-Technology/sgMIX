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
        sgmix::CaptureConfig capture;
        // Request NV12 output, the format accepted by automatic distortion correction.
        capture.outputFormat = sgmix::Frame::PixelFormat::Nv12;
        // Requires a VPI-enabled SDK/runtime and valid mono calibration at the source size.
        // SG_GetFrame returns the corrected image; no separate SG_CorrectFrame call is needed.
        capture.correctDistortion = true;
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
