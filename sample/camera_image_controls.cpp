// Live image Flip/Mirror/Testpattern checks.
#include "SG_CameraControl.h"

#include <chrono>
#include <cmath>
#include <csignal>
#include <functional>
#include <initializer_list>
#include <iostream>
#include <memory>
#include <optional>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <type_traits>
#include <vector>

#ifndef SGMIX_SAMPLE_ENABLE_GSTREAMER
#define SGMIX_SAMPLE_ENABLE_GSTREAMER 0
#endif
#if SGMIX_SAMPLE_ENABLE_GSTREAMER
#include <gst/app/gstappsrc.h>
#include <gst/video/video.h>
#endif

namespace {
using Camera = sgmix::CameraControl;
constexpr auto kObserveTime = std::chrono::seconds(3);
constexpr auto kRestoreWait = std::chrono::milliseconds(100);
volatile std::sig_atomic_t stopRequested = 0;
void onSignal(int) { stopRequested = 1; }
void checkpoint() {
    if (stopRequested) throw std::runtime_error("Interrupted; attempting to restore initial settings");
}

struct Options {
    std::string video;
    std::optional<std::uint32_t> argusSensorId;
    std::uint32_t port = UINT32_MAX, channel = 0;
    bool help = false;
};
void usage(const char* program) {
    std::cout << "Usage: " << program << " --v4l2 --video /dev/videoN [--argus] image\n"
              << "       " << program << " --sipl [--cam PORT] [--channel CHANNEL] image\n"
              << "Put image last.\n"
              << "image: Flip/Mirror/Testpattern Get/Set with live preview.\n"
              << "--argus selects NVIDIA Argus; /dev/videoN maps to sensor-id N.\n"
              << "SIPL channel defaults to 0. Omit PORT only for one enabled sensor.\n"
              << "Each test value is displayed for 3 seconds; attempts to restore initial API values.\n";
}
std::uint32_t number(const std::string& text) {
    if (text.empty() || text.find_first_not_of("0123456789") != std::string::npos)
        throw std::invalid_argument("PORT, CHANNEL and sensor-id must be unsigned decimal numbers");
    const auto value = std::stoull(text);
    if (value >= UINT32_MAX) throw std::invalid_argument("PORT, CHANNEL or sensor-id is out of range");
    return static_cast<std::uint32_t>(value);
}
Options parse(int argc, char** argv) {
    Options result;
    if (argc == 2 && (std::string(argv[1]) == "--help" || std::string(argv[1]) == "-h")) {
        result.help = true;
        return result;
    }
    if (argc < 3 || (std::string(argv[1]) != "--v4l2" && std::string(argv[1]) != "--sipl"))
        throw std::invalid_argument("Choose --v4l2 or --sipl, and put image last");
    const bool sipl = std::string(argv[1]) == "--sipl";
    const std::string action = argv[argc - 1];
    if (action != "image")
        throw std::invalid_argument("Put image at the end of the command");
    std::set<std::string> seen;
    for (int i = 2; i < argc - 1; ++i) {
        std::string key = argv[i];
        if (!seen.insert(key).second)
            throw std::invalid_argument("Repeated option: " + key);
        if (!sipl && key == "--argus") continue;
        if (i + 1 >= argc - 1 || std::string(argv[i + 1]).empty() ||
            std::string(argv[i + 1]).rfind("--", 0) == 0)
            throw std::invalid_argument("Missing value for " + key);
        const std::string value = argv[++i];
        if (!sipl && key == "--video") {
            if (value.rfind("/dev/video", 0) != 0)
                throw std::invalid_argument("Use a /dev/videoN node");
            number(value.substr(10));
            result.video = value;
        } else if (sipl && key == "--cam") {
            result.port = number(value);
        } else if (sipl && key == "--channel") {
            result.channel = number(value);
        } else {
            throw std::invalid_argument("Unknown option for this mode: " + key);
        }
    }
    if (!sipl && result.video.empty())
        throw std::invalid_argument("--v4l2 requires --video /dev/videoN");
    if (!sipl && seen.count("--argus"))
        result.argusSensorId = number(result.video.substr(10));
    if (sipl && seen.count("--channel") && result.port == UINT32_MAX)
        throw std::invalid_argument("--channel requires an explicit --cam");
    return result;
}

#if SGMIX_SAMPLE_ENABLE_GSTREAMER
inline GstVideoInfo videoInfo(const sgmix::Frame& frame) {
    GstVideoFormat format = GST_VIDEO_FORMAT_UNKNOWN;
    switch (frame.pixelFormat) {
        case sgmix::Frame::PixelFormat::Uyvy:
            format = GST_VIDEO_FORMAT_UYVY;
            break;
        case sgmix::Frame::PixelFormat::Yuyv:
            format = GST_VIDEO_FORMAT_YUY2;
            break;
        case sgmix::Frame::PixelFormat::Nv12:
            format = GST_VIDEO_FORMAT_NV12;
            break;
        case sgmix::Frame::PixelFormat::Bgr:
            format = GST_VIDEO_FORMAT_BGR;
            break;
    }
    if (!frame.width || !frame.height || frame.width > G_MAXINT || frame.height > G_MAXINT ||
        (format != GST_VIDEO_FORMAT_BGR && frame.width % 2) ||
        (format == GST_VIDEO_FORMAT_NV12 && frame.height % 2))
        throw std::runtime_error("Invalid frame dimensions");
    GstVideoInfo info{};
    if (!gst_video_info_set_format(&info, format, frame.width, frame.height) ||
        frame.planeCount != GST_VIDEO_INFO_N_PLANES(&info))
        throw std::runtime_error("Unsupported frame layout");
    for (guint plane = 0; plane < frame.planeCount; ++plane) {
        const auto& layout = frame.planes[plane];
        const size_t rows = plane ? frame.height / 2 : frame.height;
        const size_t rowBytes = size_t(frame.width) *
            (format == GST_VIDEO_FORMAT_BGR ? 3 : frame.planeCount == 2 ? 1 : 2);
        if (layout.rows != rows || layout.stride < rowBytes || layout.stride > G_MAXINT ||
            layout.offset > frame.data.size() || rowBytes > frame.data.size() - layout.offset ||
            rows - 1 > (frame.data.size() - layout.offset - rowBytes) / layout.stride)
            throw std::runtime_error("Invalid frame plane bounds");
        info.offset[plane] = layout.offset;
        info.stride[plane] = static_cast<gint>(layout.stride);
    }
    info.size = frame.data.size();
    return info;
}

class Display {
public:
    // A different sink is used only by camera-free verification.
    explicit Display(const char* sink = "autovideosink") {
        try {
            GError* error = nullptr;
            const std::string description =
                "appsrc name=source is-live=true format=time do-timestamp=true ! videoconvert ! " +
                std::string(sink) + " sync=false";
            pipeline_ = gst_parse_launch(description.c_str(), &error);
            if (error || !pipeline_) {
                const std::string message = error ? error->message : "Cannot create display pipeline";
                if (error)
                    g_error_free(error);
                throw std::runtime_error(message);
            }
            source_ = GST_APP_SRC(gst_bin_get_by_name(GST_BIN(pipeline_), "source"));
            bus_ = gst_element_get_bus(pipeline_);
            if (!source_ || !bus_)
                throw std::runtime_error("Display pipeline is incomplete");
            if (gst_element_set_state(pipeline_, GST_STATE_PLAYING) == GST_STATE_CHANGE_FAILURE)
                throw std::runtime_error("Cannot start display; run inside a desktop session");
        } catch (...) {
            close();
            throw;
        }
    }
    ~Display() {
        close();
    }
    Display(const Display&) = delete;
    Display& operator=(const Display&) = delete;

    void show(const sgmix::Frame& frame) {
        checkError();
        // Keep at most one queued input instead of accumulating display latency.
        guint64 queued = 0;
        g_object_get(source_, "current-level-bytes", &queued, nullptr);
        if (queued)
            return;
        auto info = videoInfo(frame);
        GstCaps* caps = gst_video_info_to_caps(&info);
        if (!caps)
            throw std::runtime_error("Cannot create display caps");
        gst_app_src_set_caps(source_, caps);
        gst_caps_unref(caps);

        // appsrc consumes asynchronously, so give it a copy with an independent lifetime.
        GstBuffer* buffer = gst_buffer_new_allocate(nullptr, frame.data.size(), nullptr);
        if (!buffer)
            throw std::runtime_error("Cannot allocate display buffer");
        if (gst_buffer_fill(buffer, 0, frame.data.data(), frame.data.size()) != frame.data.size() ||
            !gst_buffer_add_video_meta_full(buffer, GST_VIDEO_FRAME_FLAG_NONE,
                GST_VIDEO_INFO_FORMAT(&info), frame.width, frame.height, frame.planeCount,
                info.offset, info.stride)) {
            gst_buffer_unref(buffer);
            throw std::runtime_error("Cannot copy display frame");
        }
        // This API takes ownership of buffer, including on a flow error.
        if (gst_app_src_push_buffer(source_, buffer) != GST_FLOW_OK)
            throw std::runtime_error("Display rejected a frame");
        checkError();
    }

    GstElement* pipeline() const {
        return pipeline_;
    }

private:
    void checkError() {
        GstMessage* message = gst_bus_pop_filtered(bus_, GstMessageType(GST_MESSAGE_ERROR | GST_MESSAGE_EOS));
        if (!message)
            return;
        std::string text = "Display reached end of stream";
        if (GST_MESSAGE_TYPE(message) == GST_MESSAGE_ERROR) {
            GError* error = nullptr;
            gchar* detail = nullptr;
            gst_message_parse_error(message, &error, &detail);
            if (error) {
                text = error->message;
                g_error_free(error);
            }
            g_free(detail);
        }
        gst_message_unref(message);
        throw std::runtime_error("Display: " + text);
    }
    void close() {
        if (pipeline_)
            gst_element_set_state(pipeline_, GST_STATE_NULL);
        if (source_)
            gst_object_unref(source_);
        if (bus_)
            gst_object_unref(bus_);
        if (pipeline_)
            gst_object_unref(pipeline_);
        source_ = nullptr;
        bus_ = nullptr;
        pipeline_ = nullptr;
    }
    GstElement* pipeline_ = nullptr;
    GstAppSrc* source_ = nullptr;
    GstBus* bus_ = nullptr;
};

#else
struct Display {
    Display() { throw std::runtime_error("Preview support was not built"); }
    void show(const sgmix::Frame&) {}
};
#endif


template<class T> std::string valueText(const T& value) {
    std::ostringstream text;
    text << value;
    return text.str();
}
std::string diagnostic(const std::string& error) {
    return error.empty() ? "SDK returned false without a diagnostic; check model support" : error;
}

template<class T> struct Setting {
    const char* label;
    const char* getName;
    const char* setName;
    std::function<bool(T&, std::string&)> get;
    std::function<bool(T, std::string&)> set;
    double tolerance = 0;
    T original{};
    bool known = false, dirty = false;
    bool matches(const T& a, const T& b) const {
        if constexpr (std::is_arithmetic_v<T>)
            return std::abs(double(a)-double(b)) <= tolerance;
        else
            return a == b;
    }
};
template<class T> Setting<T> setting(Camera& camera, const char* label, const char* getName,
                                     const char* setName, bool (Camera::*get)(T&, std::string&),
                                     bool (Camera::*set)(T, std::string&), double tolerance = 0) {
    return {label, getName, setName,
        [&camera,get](T& value, std::string& error) { return (camera.*get)(value,error); },
        [&camera,set](T value, std::string& error) { return (camera.*set)(value,error); }, tolerance};
}

struct Runner {
    unsigned passed = 0, failures = 0;
    std::function<void()> observe;

    template<class T> void snapshot(Setting<T>& control) {
        checkpoint();
        std::string error;
        control.known = control.get(control.original,error);
        if (control.known) {
            std::cout << "[OK] " << control.getName << " initial=" << valueText(control.original) << '\n';
        } else {
            ++failures;
            std::cerr << "[FAIL] " << control.getName << ": " << diagnostic(error) << '\n';
            std::cerr << "[SKIP] " << control.setName << ": initial value unknown\n";
        }
    }
    template<class T> bool verify(Setting<T>& control, T expected) {
        T actual{};
        std::string error;
        if (!control.get(actual,error)) {
            ++failures;
            std::cerr << "[FAIL] " << control.getName << ": " << diagnostic(error) << '\n';
            return false;
        }
        std::cout << "[READBACK] " << control.getName << " expected=" << valueText(expected)
                  << " actual=" << valueText(actual) << '\n';
        if (!control.matches(expected,actual)) {
            ++failures;
            std::cerr << "[FAIL] " << control.label << " readback mismatch\n";
            return false;
        }
        return true;
    }
    template<class T> bool change(Setting<T>& control, T value) {
        checkpoint();
        std::string error;
        control.dirty = true;
        if (!control.set(value,error)) {
            ++failures;
            std::cerr << "[FAIL] " << control.setName << ": " << diagnostic(error) << '\n';
            return false;
        }
        std::cout << "[SET OK] " << control.setName << " value=" << valueText(value)
                  << " -- observe live image for 3 seconds\n";
        observe();
        return verify(control,value);
    }
    template<class T> bool restore(Setting<T>& control) {
        if (!control.known || !control.dirty) return true;
        std::string error;
        if (!control.set(control.original,error)) {
            ++failures;
            std::cerr << "[RESTORE FAIL] " << control.label << ": " << diagnostic(error) << '\n';
            return false;
        }
        std::this_thread::sleep_for(kRestoreWait);
        if (!verify(control,control.original)) {
            std::cerr << "[RESTORE FAIL] " << control.label << '\n';
            return false;
        }
        control.dirty = false;
        std::cout << "[RESTORE OK] " << control.label << '=' << valueText(control.original) << '\n';
        return true;
    }
    template<class T> bool test(Setting<T>& control, std::initializer_list<T> values) {
        if (!control.known) return true;
        bool ok = true;
        std::cout << "[TEST] " << control.label << '\n';
        for (const auto& value : values) {
            checkpoint();
            if (!change(control,value)) ok = false;
        }
        const bool restored = restore(control);
        if (ok && restored) {
            ++passed;
            std::cout << "[PASS] " << control.label << " Get/Set\n";
        }
        return restored;
    }
};
} // namespace

int main(int argc, char** argv) {
    Options options;
    try { options = parse(argc,argv); }
    catch (const std::exception& error) {
        std::cerr << error.what() << '\n'; usage(argv[0]); return 2;
    }
    if (options.help) { usage(argv[0]); return 0; }
    std::cout << std::unitbuf;
    std::signal(SIGINT,onSignal);
    std::signal(SIGTERM,onSignal);
    Camera camera;
    Runner runner;
    auto flip = setting(camera,"Flip","SG_GetCameraImageFlip","SG_SetCameraImageFlip",
        &Camera::SG_GetCameraImageFlip,&Camera::SG_SetCameraImageFlip);
    auto mirror = setting(camera,"Mirror","SG_GetCameraImageMirror","SG_SetCameraImageMirror",
        &Camera::SG_GetCameraImageMirror,&Camera::SG_SetCameraImageMirror);
    auto pattern = setting(camera,"Testpattern","SG_GetCameraImageTestpattern","SG_SetCameraImageTestpattern",
        &Camera::SG_GetCameraImageTestpattern,&Camera::SG_SetCameraImageTestpattern);
    const std::vector<std::function<void()>> cleanup{
        [&]{runner.restore(flip);}, [&]{runner.restore(mirror);}, [&]{runner.restore(pattern);}
    };
    bool captureAttempted = false;
    std::unique_ptr<Display> display;
    try {
#if SGMIX_SAMPLE_ENABLE_GSTREAMER
        GError* gstError = nullptr;
        if (!gst_init_check(nullptr,nullptr,&gstError)) {
            const std::string message = gstError ? gstError->message : "Cannot initialize GStreamer";
            if (gstError) g_error_free(gstError);
            throw std::runtime_error(message);
        }
#else
        throw std::runtime_error("Live preview requires GStreamer App/Video development packages; rebuild this sample");
#endif
        std::string error;
        sgmix::CaptureConfig capture;
        bool initialized;
        if (options.video.empty()) {
            capture.siplChannel = options.channel;
            initialized = camera.SG_Init("","",error,{},options.port);
        } else {
            sgmix::CameraConfig device;
            device.videoDevice = options.video;
            if (options.argusSensorId) {
                capture.source = sgmix::CaptureConfig::Source::Argus;
                capture.argusSensorId = options.argusSensorId;
            }
            initialized = camera.SG_Init(device,error);
        }
        if (!initialized) throw std::runtime_error("SG_Init: "+diagnostic(error));
        captureAttempted = true;
        if (!camera.SG_StartCapture(capture,error))
            throw std::runtime_error("SG_StartCapture: "+diagnostic(error));
        display = std::make_unique<Display>();
        runner.observe = [&] {
            const auto deadline = std::chrono::steady_clock::now()+kObserveTime;
            do {
                checkpoint();
                sgmix::Frame frame;
                error.clear();
                if (!camera.SG_GetFrame(frame,3000,error))
                    throw std::runtime_error("SG_GetFrame: "+diagnostic(error));
                display->show(frame);
            } while (std::chrono::steady_clock::now()<deadline);
        };
        runner.observe();
        auto requireRestored = [](bool ok) {
            if (!ok) throw std::runtime_error("A setting could not be restored; stopping further tests");
        };
        std::cout << "[READY] Image API test: Flip, Mirror, Testpattern\n";
        runner.snapshot(flip); runner.snapshot(mirror); runner.snapshot(pattern);
        requireRestored(runner.test(flip,{false,true}));
        requireRestored(runner.test(mirror,{false,true}));
        requireRestored(runner.test(pattern,{false,true}));
    } catch (const std::exception& error) {
        ++runner.failures;
        std::cerr << "[ABORT] " << error.what() << '\n';
    }
    // No frames or display are needed during recovery, including after Ctrl+C/window failure.
    for (const auto& restore : cleanup) {
        try { restore(); }
        catch (const std::exception& error) {
            ++runner.failures;
            std::cerr << "[RESTORE FAIL] " << error.what() << '\n';
        }
    }
    display.reset();
    if (captureAttempted) {
        std::string error;
        if (!camera.SG_StopCapture(error)) {
            ++runner.failures;
            std::cerr << "[STOP FAIL] " << diagnostic(error) << '\n';
        }
    }
    const unsigned expectedPairs = 3;
    const bool ok = !runner.failures && runner.passed==expectedPairs;
    std::cout << "[RESULT] " << (ok ? "PASS" : "FAIL") << " pairs_passed=" << runner.passed
              << '/' << expectedPairs << " failures=" << runner.failures << " (visual effects require observation)\n";
    if (stopRequested) return 130;
    return ok ? 0 : 1;
}
