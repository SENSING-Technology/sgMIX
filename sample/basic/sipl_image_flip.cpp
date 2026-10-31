// Minimal control example; use the matching saved camera configuration.
#include "SG_CameraControl.h"

#include <csignal>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {
volatile std::sig_atomic_t stopRequested = 0;
void onSignal(int) { stopRequested = 1; }
}

int main(int argc, char** argv) {
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " PORT FLIP MIRROR\n";
        return 2;
    }
    std::uint32_t port = 0;
    bool flip = false, mirror = false;
    try {
        const std::string text = argv[1];
        if (text.empty() || text.find_first_not_of("0123456789") != std::string::npos)
            throw std::invalid_argument("PORT must be a zero-based decimal CAM number");
        const auto value = std::stoull(text);
        if (value >= UINT32_MAX) throw std::invalid_argument("PORT is out of range");
        port = static_cast<std::uint32_t>(value);
        const std::string flipText = argv[2], mirrorText = argv[3];
        if ((flipText != "0" && flipText != "1") || (mirrorText != "0" && mirrorText != "1"))
            throw std::invalid_argument("FLIP and MIRROR must be 0 (off) or 1 (on)");
        flip = flipText == "1";
        mirror = mirrorText == "1";
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 2;
    }

    std::signal(SIGINT, onSignal);
    std::signal(SIGTERM, onSignal);
    sgmix::CameraControl camera;
    std::string error;
    bool success = true, captureAttempted = false;
    bool oldFlip = false, oldMirror = false;
    bool flipAttempted = false, mirrorAttempted = false;
    auto check = [&](bool ok, const char* api) {
        if (!ok) throw std::runtime_error(std::string(api) + ": " + error);
        if (stopRequested) throw std::runtime_error("Interrupted; restoring initial settings");
    };
    try {
        // Load saved JSON, config name and masks; PORT selects an enabled Driver CAM.
        check(camera.SG_Init("", "", error, {}, port), "SG_Init");

        sgmix::CaptureConfig capture;
        // Start before controls: SIPL opens the sensor control path during capture.
        // A stereo module needs the corresponding channel-specific full tool.
        captureAttempted = true;
        check(camera.SG_StartCapture(capture, error), "SG_StartCapture");
        sgmix::Frame frame;
        check(camera.SG_GetFrame(frame, 3000, error), "SG_GetFrame (first frame)");

        // Read all initial values before changing either setting.
        check(camera.SG_GetCameraImageFlip(oldFlip, error), "SG_GetCameraImageFlip (initial)");
        check(camera.SG_GetCameraImageMirror(oldMirror, error), "SG_GetCameraImageMirror (initial)");

        flipAttempted = true;
        check(camera.SG_SetCameraImageFlip(flip, error), "SG_SetCameraImageFlip");
        bool actualFlip = false;
        check(camera.SG_GetCameraImageFlip(actualFlip, error), "SG_GetCameraImageFlip (readback)");
        std::cout << "Flip: before=" << oldFlip << " requested=" << flip << " readback=" << actualFlip << '\n';
        if (actualFlip != flip) throw std::runtime_error("SG_GetCameraImageFlip: readback mismatch");

        mirrorAttempted = true;
        check(camera.SG_SetCameraImageMirror(mirror, error), "SG_SetCameraImageMirror");
        bool actualMirror = false;
        check(camera.SG_GetCameraImageMirror(actualMirror, error), "SG_GetCameraImageMirror (readback)");
        std::cout << "Mirror: before=" << oldMirror << " requested=" << mirror << " readback=" << actualMirror << '\n';
        if (actualMirror != mirror) throw std::runtime_error("SG_GetCameraImageMirror: readback mismatch");
        for (unsigned i = 0; i < 30; ++i) {
            check(camera.SG_GetFrame(frame, 3000, error), "SG_GetFrame");
            std::cout << "frame=" << frame.sequence << " size=" << frame.width << 'x' << frame.height
                      << " bytes=" << frame.data.size() << '\n';
        }
    } catch (const std::exception& failure) {
        std::cerr << failure.what() << '\n';
        success = false;
    }

    // A failed Set can still have changed the device. Restore every attempted
    // setting in reverse order, keeping capture active until cleanup is done.
    if (mirrorAttempted) {
        if (!camera.SG_SetCameraImageMirror(oldMirror, error)) {
            std::cerr << "Restore SG_SetCameraImageMirror: " << error << '\n';
            success = false;
        } else {
            std::cout << "Restored Mirror=" << oldMirror << '\n';
        }
    }
    if (flipAttempted) {
        if (!camera.SG_SetCameraImageFlip(oldFlip, error)) {
            std::cerr << "Restore SG_SetCameraImageFlip: " << error << '\n';
            success = false;
        } else {
            std::cout << "Restored Flip=" << oldFlip << '\n';
        }
    }
    if (captureAttempted && !camera.SG_StopCapture(error)) {
        std::cerr << "SG_StopCapture: " << error << '\n';
        success = false;
    }
    return success ? 0 : 1;
}
