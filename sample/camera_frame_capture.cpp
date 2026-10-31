// Unified V4L2 / Argus / SIPL capture, optional preview and correction.
#include "SG_CameraControl.h"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <csignal>
#include <future>
#include <iostream>
#include <limits>
#include <map>
#include <mutex>
#include <set>
#include <sstream>
#include <stdexcept>
#include <thread>

#ifndef SGMIX_SAMPLE_ENABLE_GSTREAMER
#define SGMIX_SAMPLE_ENABLE_GSTREAMER 0
#endif
#if SGMIX_SAMPLE_ENABLE_GSTREAMER
#include <gst/app/gstappsrc.h>
#include <gst/video/video.h>
#endif

namespace {

using Format = sgmix::Frame::PixelFormat;
struct UsageError : std::runtime_error {
    using std::runtime_error::runtime_error;
};

struct Options {
    sgmix::CameraConfig device;
    sgmix::CaptureConfig capture;
    std::vector<std::uint32_t> ports, masks;
    std::optional<std::uint32_t> channel;
    std::string json, config;
    std::uint32_t frames = 30;
    bool sipl = false, stereo = false, view = false, correct = false, help = false;
};

std::uint64_t number(const std::string& text, std::uint64_t maximum = UINT32_MAX, int base = 10) {
    try {
        if (text.empty() || text.front() < '0' || text.front() > '9')
            throw UsageError("Invalid number: " + text);
        if (base == 10 && text.find_first_not_of("0123456789") != std::string::npos)
            throw UsageError("Expected a decimal number: " + text);
        std::size_t used = 0;
        const auto value = std::stoull(text, &used, base);
        if (used != text.size() || value > maximum)
            throw UsageError("Number out of range: " + text);
        return value;
    } catch (const std::invalid_argument&) {
        throw UsageError("Invalid number: " + text);
    } catch (const std::out_of_range&) {
        throw UsageError("Number out of range: " + text);
    }
}

const char* pixelFormatName(Format value) {
    switch (value) {
        case Format::Uyvy: return "UYVY";
        case Format::Yuyv: return "YUYV";
        case Format::Nv12: return "NV12";
        case Format::Bgr: return "BGR";
    }
    return "Unknown";
}

void usage(const char* program) {
    std::cout
        << "sgMIX Camera Frame Capture\n\n"
        << "Mode 1: Use Saved Configuration\n"
        << "  Use the camera model or SIPL settings saved by the configuration tool.\n\n"
        << "  V4L2:\n"
        << "    " << program << " --v4l2 --video <video_device> [options]\n"
        << "  NVIDIA Argus:\n"
        << "    " << program << " --v4l2 --video <video_device> --argus [options]\n"
        << "  SIPL:\n"
        << "    " << program << " --sipl [--cam N] [options]\n\n"
        << "  Examples:\n"
        << "    " << program << " --v4l2 --video /dev/video6 --view\n"
        << "    " << program << " --v4l2 --video /dev/video0 --argus --view\n"
        << "    " << program << " --sipl --cam 6 --cam 7 --view\n\n"
        << "Mode 2: Specify Configuration Manually\n"
        << "  Use this mode when no configuration has been saved.\n\n"
        << "  V4L2:\n"
        << "    " << program << " --v4l2 --video <video_device> --model <model> [options]\n"
        << "  SIPL:\n"
        << "    " << program << " --sipl [--cam N] \\\n"
        << "      --json <path> --config <name> [--mask \"MASK ...\"] [options]\n\n"
        << "  Examples:\n"
        << "    " << program << " --v4l2 --video /dev/video7 --model SHW3G --view\n"
        << "    " << program << " --sipl --cam 7 \\\n"
        << "      --json camera.json --config SHW3G_8CH \\\n"
        << "      --mask \"0x1000 0x0000\" --view\n\n"
        << "Capture Options\n"
        << "  --frames N                  Frames per channel; default 30\n"
        << "  --view                      Live preview; unlimited frames unless --frames is set\n"
        << "  --correct                   Apply calibrated distortion correction using NV12\n"
        << "  -h, --help                  Show this help\n\n"
        << "V4L2 / Argus Options\n"
        << "  --video PATH                Video device, e.g. /dev/video7\n"
        << "  --model MODEL               Override the saved camera model\n"
        << "  --bus N                     Override the known I2C bus\n"
        << "  --eeprom-addr ADDR          Host-visible 7-bit EEPROM address (0x08..0x77)\n"
        << "  --width W --height H        Set capture dimensions; omit both to negotiate\n"
        << "  --argus                     Use NVIDIA Argus; sensor-id from /dev/videoN\n"
        << "  --isx031-ebd                Decode the first ISX031 EBD row (V4L2 only)\n\n"
        << "SIPL Options\n"
        << "  --cam N                     Select a Driver CAM; repeat for multiple cameras\n"
        << "  --channel N                 Select one module-local Query channel\n"
        << "  --stereo                    Capture channels 0 and 1 on one CAM\n"
        << "  --json PATH                 Camera JSON file\n"
        << "  --config NAME               Platform configuration name in the JSON file\n"
        << "  --mask \"MASK ...\"           Quoted masks in driver transport order\n\n"
        << "Additional Examples\n"
        << "  Capture 100 frames:\n"
        << "    " << program << " --v4l2 --video /dev/video6 --frames 100\n\n"
        << "  Decode ISX031 first-row EBD (no row-count argument):\n"
        << "    " << program << " --v4l2 --video /dev/video6 --isx031-ebd\n\n"
        << "  Preview with distortion correction:\n"
        << "    " << program << " --sipl --cam 7 --view --correct\n\n"
        << "  Preview both channels of a stereo module:\n"
        << "    " << program << " --sipl --cam 7 --stereo --view\n\n"
        << "  Preview one channel of a module:\n"
        << "    " << program << " --sipl --cam 7 --channel 1 --view\n\n"
        << "Notes\n"
        << "  Press Ctrl+C to stop capture.\n"
        << "  Driver CAM numbers are zero-based; selected links must be enabled.\n"
        << "  Without --cam, the SIPL configuration must select exactly one sensor.\n"
        << "  --channel and --stereo each require one explicit --cam; use only one of them.\n"
        << "  Use --json and --config together; specify --mask once.\n"
        << "  --argus maps /dev/videoN to Argus sensor-id N.\n"
        << "  Argus correction uses calibration dimensions when width and height are omitted.\n"
        << "  EBD extraction cannot be combined with Argus or distortion correction.\n"
        << "  Channels run independently; frames are not paired by exposure time.\n"
        << "  Preview and sample-side YUV-to-NV12 conversion require GStreamer support.\n";
}

Options parse(int argc, char** argv) {
    Options options;
    bool namedV4l2 = false;
    std::set<std::string> seen;
    for (int i = 1; i < argc; ++i) {
        std::string key = argv[i];
        if (key == "--help" || key == "-h") {
            options.help = true;
            return options;
        }
        if (key.empty() || key.front() != '-')
            throw UsageError("Unexpected positional argument: " + key);
        if (key != "--cam" && seen.count(key))
            throw UsageError("Repeated option: " + key);
        seen.insert(key);
        const auto value = [&]() -> std::string {
            if (i + 1 >= argc || std::string(argv[i + 1]).empty() || std::string(argv[i + 1]).rfind("--", 0) == 0)
                throw UsageError("Missing value for " + key);
            return argv[++i];
        };
        if (key == "--sipl")
            options.sipl = true;
        else if (key == "--v4l2")
            namedV4l2 = true;
        else if (key == "--video")
            options.device.videoDevice = value();
        else if (key == "--cam")
            options.ports.push_back(static_cast<std::uint32_t>(number(value(), UINT32_MAX - 1U)));
        else if (key == "--stereo")
            options.stereo = true;
        else if (key == "--view")
            options.view = true;
        else if (key == "--correct")
            options.correct = true;
        else if (key == "--frames") {
            options.frames = number(value());
            if (!options.frames)
                throw UsageError("frames must be positive");
        } else if (key == "--model") {
            options.device.model = value();
            if (options.device.model == "auto")
                options.device.model.clear();
        } else if (key == "--eeprom-addr") {
            const auto text = value();
            const int base = text.compare(0, 2, "0x") == 0 || text.compare(0, 2, "0X") == 0 ? 16 : 10;
            const auto address = number(text, 0x77, base);
            if (address < 0x08)
                throw UsageError("EEPROM address must be a host-visible 7-bit address in 0x08..0x77");
            options.device.eepromI2cAddress = static_cast<std::uint8_t>(address);
        } else if (key == "--bus")
            options.device.i2cBus = number(value(), std::numeric_limits<int>::max());
        else if (key == "--width")
            options.capture.width = number(value());
        else if (key == "--height")
            options.capture.height = number(value());
        else if (key == "--argus")
            options.capture.source = sgmix::CaptureConfig::Source::Argus;
        else if (key == "--channel")
            options.channel = number(value());
        else if (key == "--json")
            options.json = value();
        else if (key == "--config")
            options.config = value();
        else if (key == "--mask") {
            std::istringstream values(value());
            std::string mask;
            if (!(values >> mask)) throw UsageError("SIPL mask list must not be empty");
            do {
                options.masks.push_back(number(mask, UINT32_MAX, 0));
            } while (values >> mask);
        }
        else if (key == "--isx031-ebd")
            options.capture.embeddedTopLines = 1;
        else
            throw UsageError("Unknown option: " + key);
    }
    if (options.view && !seen.count("--frames"))
        options.frames = 0;
    if (options.sipl && namedV4l2)
        throw UsageError("Choose --v4l2 or --sipl, not both");
    if (!options.sipl && !namedV4l2)
        throw UsageError("Choose --v4l2 or --sipl");
    if (options.sipl) {
        std::set<std::uint32_t> uniquePorts;
        for (const auto port : options.ports)
            if (!uniquePorts.insert(port).second)
                throw UsageError("SIPL ports must be distinct");
        if (options.stereo && options.channel)
            throw UsageError("Choose --stereo or --channel, not both");
        if ((options.stereo || options.channel) && options.ports.size() != 1)
            throw UsageError("--stereo/--channel requires one explicit CAM port");
        if (options.json.empty() != options.config.empty() || (!options.masks.empty() && options.json.empty()))
            throw UsageError("Use --json and --config together; --mask requires both");
        for (const auto* key : {"--video", "--model", "--bus", "--eeprom-addr", "--width", "--height",
                                "--argus", "--isx031-ebd"})
            if (seen.count(key))
                throw UsageError(std::string(key) + " is not a SIPL option; native output comes from the driver");
        if (options.ports.empty())
            options.ports.push_back(UINT32_MAX);
    } else {
        if (options.device.videoDevice.rfind("/dev/video", 0) != 0)
            throw UsageError("Provide --v4l2 --video /dev/videoN, or use --sipl --cam PORT");
        if (seen.count("--video") && !namedV4l2)
            throw UsageError("--video requires --v4l2");
        for (const auto* key : {"--cam", "--stereo", "--channel", "--json", "--config", "--mask"})
            if (seen.count(key))
                throw UsageError(std::string(key) + " requires --sipl");
        if (options.capture.source == sgmix::CaptureConfig::Source::Argus)
            options.capture.argusSensorId = number(options.device.videoDevice.substr(10));
        if (seen.count("--width") != seen.count("--height") ||
            bool(options.capture.width) != bool(options.capture.height))
            throw UsageError("Set width and height together");
        if (options.correct) {
            options.capture.outputFormat = Format::Nv12;
            options.capture.correctDistortion = true;
        }
        if (options.capture.embeddedTopLines && (options.correct || options.capture.argusSensorId))
            throw UsageError("--isx031-ebd requires uncorrected V4L2 input");
    }
    return options;
}

static_assert(std::atomic<bool>::is_always_lock_free, "Signal handling needs a lock-free stop flag");
std::atomic<bool> stopRequested{false};
void requestStop(int) {
    stopRequested.store(true, std::memory_order_relaxed);
}

void initialize(bool needGstreamer) {
    stopRequested.store(false);
    std::signal(SIGINT, requestStop);
    std::signal(SIGTERM, requestStop);
#if SGMIX_SAMPLE_ENABLE_GSTREAMER
    if (!needGstreamer)
        return;
    GError* error = nullptr;
    if (!gst_init_check(nullptr, nullptr, &error)) {
        const std::string message = error ? error->message : "Cannot initialize GStreamer";
        if (error)
            g_error_free(error);
        throw std::runtime_error(message);
    }
#else
    if (needGstreamer)
        throw std::runtime_error("--view requires GStreamer App/Video development packages; rebuild the sample");
#endif
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

// Wrap caller-owned CPU pixels only for the duration of a synchronous conversion.
struct MappedFrame {
    GstVideoFrame video{};
    GstBuffer* buffer = nullptr;

    MappedFrame(const sgmix::Frame& frame, GstVideoInfo info, GstMapFlags access) {
        const auto flags = access == GST_MAP_READ ? GST_MEMORY_FLAG_READONLY : GstMemoryFlags(0);
        buffer = gst_buffer_new_wrapped_full(flags, const_cast<std::uint8_t*>(frame.data.data()),
                                            frame.data.size(), 0, frame.data.size(), nullptr, nullptr);
        if (!buffer)
            throw std::runtime_error("Cannot wrap conversion pixels");
        if (!gst_video_frame_map(&video, &info, buffer, access)) {
            gst_buffer_unref(buffer);
            buffer = nullptr;
            throw std::runtime_error("Cannot map conversion pixels");
        }
    }
    ~MappedFrame() {
        gst_video_frame_unmap(&video);
        gst_buffer_unref(buffer);
    }
    MappedFrame(const MappedFrame&) = delete;
    MappedFrame& operator=(const MappedFrame&) = delete;
};

struct Nv12Converter {
    GstVideoConverter* converter = nullptr;
    GstVideoInfo previous{};

    ~Nv12Converter() {
        if (converter)
            gst_video_converter_free(converter);
    }
    Nv12Converter() = default;
    Nv12Converter(const Nv12Converter&) = delete;
    Nv12Converter& operator=(const Nv12Converter&) = delete;

    void convert(const sgmix::Frame& input, sgmix::Frame& output) {
        if (&input == &output)
            throw std::runtime_error("Conversion needs a separate output Frame");
        auto sourceInfo = videoInfo(input);
        if (input.height % 2)
            throw std::runtime_error("NV12 conversion requires an even height");
        const size_t pixels = size_t(input.width) * input.height;
        output.data.resize(pixels + pixels / 2);
        output.width = input.width;
        output.height = input.height;
        output.pixelFormat = sgmix::Frame::PixelFormat::Nv12;
        output.planeCount = 2;
        output.planes = {};
        output.planes[0] = {0, input.width, input.height};
        output.planes[1] = {pixels, input.width, input.height / 2};
        auto destinationInfo = videoInfo(output);
        destinationInfo.colorimetry = sourceInfo.colorimetry;
        if (!converter || !gst_video_info_is_equal(&previous, &sourceInfo)) {
            if (converter)
                gst_video_converter_free(converter);
            converter = gst_video_converter_new(&sourceInfo, &destinationInfo, nullptr);
            if (!converter)
                throw std::runtime_error("Cannot create NV12 converter");
            previous = sourceInfo;
        }
        MappedFrame source(input, sourceInfo, GST_MAP_READ);
        MappedFrame destination(output, destinationInfo, GST_MAP_WRITE);
        gst_video_converter_frame(converter, &source.video, &destination.video);
        output.sequence = input.sequence;
        output.ptsNanoseconds = input.ptsNanoseconds;
        output.hostTimestampMicroseconds = input.hostTimestampMicroseconds;
        output.siplMetadata = input.siplMetadata;
        output.embeddedMetadata = input.embeddedMetadata;
    }
};

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
struct Nv12Converter {
    void convert(const sgmix::Frame&, sgmix::Frame&) {
        throw std::runtime_error("YUV-to-NV12 conversion requires GStreamer Video development packages; rebuild the sample");
    }
};
struct Display {
    Display() {
        throw std::runtime_error("Preview support was not built");
    }
    void show(const sgmix::Frame&) {}
};
#endif

struct Channel {
    std::uint32_t port = UINT32_MAX;
    std::optional<std::uint32_t> index;
    std::string name;
    std::unique_ptr<sgmix::CameraControl> camera;
    sgmix::CaptureConfig capture;
    std::optional<sgmix::CameraCalibration::Intrinsics> intrinsics;
    std::unique_ptr<Nv12Converter> converter;
    std::unique_ptr<Display> display;
    // Each worker owns its capture, conversion and correction buffers.
    std::unique_ptr<sgmix::Frame> frame = std::make_unique<sgmix::Frame>();
    std::unique_ptr<sgmix::Frame> nv12 = std::make_unique<sgmix::Frame>();
    std::unique_ptr<sgmix::Frame> corrected = std::make_unique<sgmix::Frame>();
};

std::vector<Channel> open(const Options& options) {
    std::vector<std::pair<std::uint32_t, std::optional<std::uint32_t>>> selected;
    if (options.sipl) {
        for (const auto port : options.ports) {
            if (options.stereo) {
                selected.push_back({port, 0});
                selected.push_back({port, 1});
            } else
                selected.push_back({port, options.channel});
        }
    } else
        selected.push_back({UINT32_MAX, {}});
    std::vector<Channel> channels;
    channels.reserve(selected.size());
    std::map<std::uint32_t, sgmix::CameraCalibration> calibrations;
    for (const auto& selection : selected) {
        Channel channel;
        channel.port = selection.first;
        channel.index = selection.second;
        channel.name = options.sipl ? "port=" + (channel.port == UINT32_MAX ? std::string("auto") : std::to_string(channel.port)) +
            " channel=" + std::to_string(channel.index.value_or(0)) : "video=" + options.device.videoDevice;
        channel.camera = std::make_unique<sgmix::CameraControl>();
        std::string error;
        const bool initialized = options.sipl
            ? channel.camera->SG_Init(options.json, options.config, error, options.masks, channel.port)
            : channel.camera->SG_Init(options.device, error);
        if (!initialized)
            throw std::runtime_error(channel.name + ": " + error);
        channel.capture = options.capture;
        channel.capture.siplChannel = channel.index;
        if (options.sipl && options.correct) {
            if (!calibrations.count(channel.port)) {
                sgmix::CameraCalibration calibration;
                if (!channel.camera->SG_GetCalibration(calibration, error))
                    throw std::runtime_error(channel.name + ": " + error);
                calibrations.emplace(channel.port, std::move(calibration));
            }
            const auto& calibration = calibrations.at(channel.port);
            const auto index = channel.index.value_or(0);
            if (index == 0)
                channel.intrinsics = calibration.left;
            else if (index == 1 && calibration.right)
                channel.intrinsics = *calibration.right;
            else
                throw std::runtime_error(channel.name + ": calibration is unavailable for this channel");
            channel.converter = std::make_unique<Nv12Converter>();
        }
        if (options.view)
            channel.display = std::make_unique<Display>();
        channels.push_back(std::move(channel));
    }
    return channels;
}

void printFrame(const Channel& channel, const sgmix::Frame& frame) {
    std::cout << channel.name << " frame=" << frame.sequence << " size=" << frame.width << 'x' << frame.height
              << " format=" << pixelFormatName(frame.pixelFormat)
              << " bytes=" << frame.data.size() << " host-us=" << frame.hostTimestampMicroseconds;
    if (frame.ptsNanoseconds)
        std::cout << " pts-ns=" << *frame.ptsNanoseconds;
    if (frame.siplMetadata) {
        const auto& metadata = *frame.siplMetadata;
        std::cout << " sof-tsc=" << metadata.captureStartTsc << " eof-tsc=" << metadata.captureEndTsc << " sensor-frame=";
        if (metadata.sensorFrameSequence)
            std::cout << *metadata.sensorFrameSequence;
        else
            std::cout << "unavailable";
        std::cout << " sensor-timestamp-raw=";
        if (metadata.sensorTimestamp)
            std::cout << *metadata.sensorTimestamp;
        else
            std::cout << "unavailable";
        std::cout << " top-ebd-bytes=" << metadata.embeddedDataTop.size()
                  << " bottom-ebd-bytes=" << metadata.embeddedDataBottom.size();
        std::cout << " num-exposures=" << metadata.numExposures;
        const auto printValues = [](const char* name, const auto& values) {
            std::cout << ' ' << name << "=[";
            const char* separator = "";
            for (const auto value : values) {
                std::cout << separator << value;
                separator = ",";
            }
            std::cout << ']';
        };
        printValues("exposure-s", metadata.exposureTimesSeconds);
    }
    if (frame.embeddedMetadata) {
        const auto& metadata = *frame.embeddedMetadata;
        std::cout << " ebd-bytes=" << metadata.raw.size();
        if (metadata.exposureLines)
            std::cout << " exposure-lines=" << *metadata.exposureLines;
        if (metadata.exposureTimeSeconds)
            std::cout << " exposure-s=" << *metadata.exposureTimeSeconds;
    }
    std::cout << '\n';
}

void capture(std::vector<Channel>& channels, const Options& options) {
    for (auto& channel : channels) {
        std::string error;
        if (!channel.camera->SG_StartCapture(channel.capture, error))
            throw std::runtime_error(channel.name + ": " + error);
    }
    std::atomic<bool> stop{false};
    std::mutex output;
    std::vector<std::future<void>> workers;
    workers.reserve(channels.size());
    const auto stopCapture = [&] {
        stop.store(true);
        std::string firstError;
        for (auto& channel : channels) {
            std::string error;
            if (!channel.camera->SG_StopCapture(error) && firstError.empty())
                firstError = channel.name + ": " + error;
        }
        return firstError;
    };
    try {
        for (auto& channel : channels) {
            workers.emplace_back(std::async(std::launch::async, [&, current = &channel] {
                try {
                    std::string error;
                    for (std::uint64_t count = 0; !stop.load() && !stopRequested.load() &&
                         (!options.frames || count < options.frames); ++count) {
                        if (!current->camera->SG_GetFrame(*current->frame, 3000, error)) {
                            if (stop.load() || stopRequested.load())
                                break;
                            throw std::runtime_error(error);
                        }
                        if (stop.load() || stopRequested.load())
                            break;
                        if (options.capture.embeddedTopLines &&
                            (!current->frame->embeddedMetadata || !current->frame->embeddedMetadata->exposureLines))
                            throw std::runtime_error("--isx031-ebd requires a supported ISX031 model and a valid first EBD row");
                        auto* image = &current->frame;
                        if (options.sipl && options.correct) {
                            if ((*image)->pixelFormat != Format::Nv12) {
                                current->converter->convert(**image, *current->nv12);
                                image = &current->nv12;
                            }
                            if (stop.load() || stopRequested.load())
                                break;
                            if (!current->camera->SG_CorrectFrame(**image, *current->corrected, error, *current->intrinsics)) {
                                if (stop.load() || stopRequested.load())
                                    break;
                                throw std::runtime_error(error);
                            }
                            image = &current->corrected;
                        }
                        if (stop.load() || stopRequested.load())
                            break;
                        // Insert processing/inference here. Each worker owns its channel and buffers.
                        {
                            std::lock_guard<std::mutex> guard(output);
                            printFrame(*current, **image);
                        }
                        if (current->display)
                            current->display->show(**image);
                    }
                } catch (const std::exception& error) {
                    stop.store(true);
                    throw std::runtime_error(current->name + ": " + error.what());
                } catch (...) {
                    stop.store(true);
                    throw;
                }
            }));
        }
        while (!stop.load() && !stopRequested.load()) {
            if (std::all_of(workers.begin(), workers.end(), [](auto& worker) {
                    return worker.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready;
                })) break;
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
        }
    } catch (...) {
        stopCapture();
        for (auto& worker : workers)
            worker.wait();
        throw;
    }
    const auto stopError = stopCapture();
    for (auto& worker : workers)
        worker.get();
    if (!stopError.empty())
        throw std::runtime_error(stopError);
}

} // namespace

int main(int argc, char** argv) {
    try {
        if (argc < 2) {
            usage(argv[0]);
            return 2;
        }
        const auto options = parse(argc, argv);
        if (options.help) {
            usage(argv[0]);
            return 0;
        }
        initialize(options.view || (options.sipl && options.correct && SGMIX_SAMPLE_ENABLE_GSTREAMER));
        auto channels = open(options);
        std::cout << "Capturing " << channels.size() << " channel(s), one worker per channel; Ctrl+C stops capture.\n";
        capture(channels, options);
    } catch (const UsageError& error) {
        std::cerr << error.what() << "\nUse --help for usage.\n";
        return 2;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
    return 0;
}
