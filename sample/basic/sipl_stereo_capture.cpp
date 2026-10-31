// Minimal SIPL two-channel capture. Query order defines physical channel order.
#include "SG_CameraControl.h"

#include <atomic>
#include <future>
#include <iostream>
#include <mutex>
#include <stdexcept>

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " PORT\n";
        return 2;
    }
    try {
        const std::string text = argv[1];
        if (text.empty() || text.find_first_not_of("0123456789") != std::string::npos)
            throw std::invalid_argument("PORT must be a zero-based decimal CAM number");
        const auto port = std::stoull(text);
        if (port >= UINT32_MAX)
            throw std::invalid_argument("PORT is out of range");
        // Use one CameraControl per channel. Matching SIPL configurations share a session;
        // these names do not determine physical left/right ordering.
        sgmix::CameraControl left, right;
        // sgMIX calls return false on failure and write a diagnostic into error.
        std::string error;
        auto check = [&](bool ok) {
            if (!ok)
                throw std::runtime_error(error);
        };
        // Both objects bind the same zero-based Driver CAM connector. Empty JSON/config
        // and masks load the saved SIPL settings; channels are selected separately below.
        check(left.SG_Init("", "", error, {}, static_cast<std::uint32_t>(port)));
        check(right.SG_Init("", "", error, {}, static_cast<std::uint32_t>(port)));
        sgmix::CaptureConfig leftConfig, rightConfig;
        // Select module-local sensors in Query order; confirm physical left/right mapping
        // in the driver configuration. These selectors do not synchronize exposure.
        leftConfig.siplChannel = 0;
        rightConfig.siplChannel = 1;
        // Start each channel only after both objects have been initialized.
        check(left.SG_StartCapture(leftConfig, error));
        check(right.SG_StartCapture(rightConfig, error));

        std::atomic<bool> failed{false};
        std::mutex output;
        auto grab = [&](sgmix::CameraControl& camera, unsigned channel) {
            try {
                // Keep this output Frame private to its channel; each read replaces its pixels/metadata.
                sgmix::Frame frame;
                std::string readError; // Private to this worker.
                for (unsigned i = 0; i < 30 && !failed.load(); ++i) {
                    // One reader per CameraControl. Wait up to 3000 ms; false means timeout, stop or error.
                    // Both channels read independently, so thread scheduling does not create stereo pairs.
                    if (!camera.SG_GetFrame(frame, 3000, readError)) {
                        if (failed.load())
                            return;
                        throw std::runtime_error(readError);
                    }
                    {
                        std::lock_guard<std::mutex> guard(output);
                        std::cout << "channel=" << channel << " frame=" << frame.sequence << " size="
                                  << frame.width << 'x' << frame.height << " bytes=" << frame.data.size() << '\n';
                    }
                    // Process this channel's frame.data here. Frames are not exposure-paired.
                }
            } catch (...) {
                if (!failed.exchange(true)) {
                    std::string stopError;
                    // Stop both captures to interrupt the peer worker if it is waiting in SG_GetFrame.
                    left.SG_StopCapture(stopError);
                    right.SG_StopCapture(stopError);
                }
                throw;
            }
        };
        // Futures join before cameras are destroyed, including on worker errors.
        auto leftTask = std::async(std::launch::async, [&] {
            grab(left, 0);
        });
        auto rightTask = std::async(std::launch::async, [&] {
            grab(right, 1);
        });
        leftTask.get();
        rightTask.get();
        // After both workers finish, stop each channel and release capture resources.
        // Destructors also stop capture during exception cleanup.
        check(left.SG_StopCapture(error));
        check(right.SG_StopCapture(error));
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
    return 0;
}
