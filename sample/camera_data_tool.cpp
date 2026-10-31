#include "SG_CameraControl.h"

#include <cstdint>
#include <cerrno>
#include <cstdlib>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <limits>
#include <map>
#include <sstream>
#include <string>
#include <vector>

namespace {

bool verboseOutput = false;

void PrintUsage(const char* program) {
    std::cout
        << "sgMIX Camera Data Tool\n"
        << "Mode 1: Use Saved Configuration\n"
        << "  Use the camera model or SIPL settings saved by the configuration tool.\n"
        << "  V4L2:\n"
        << "    " << program << " --v4l2 --video <video_device> [options] <command>\n"
        << "  SIPL:\n"
        << "    " << program << " --sipl [--cam N] [options] <command>\n"
        << "  Examples:\n"
        << "    " << program << " --v4l2 --video /dev/video6 all\n"
        << "    " << program << " --v4l2 --video /dev/video6 get-sn\n"
        << "    " << program << " --sipl --cam 7 all\n"
        << "    " << program << " --sipl --cam 7 get-calibration\n"
        << "Mode 2: Specify Configuration Manually\n"
        << "  Use this mode when no configuration has been saved.\n"
        << "  V4L2:\n"
        << "    " << program << " --v4l2 --video <video_device> --model <model> [--bus N] [--eeprom-addr ADDR] <command>\n"
        << "  SIPL:\n"
        << "    " << program << " --sipl [--cam N] \\\n"
        << "      --json <path> --config <name> [--mask \"MASK ...\"] <command>\n"
        << "  Examples:\n"
        << "    " << program << " --v4l2 --video /dev/video7 --model SHW3G all\n"
        << "    " << program << " --sipl --cam 7 \\\n"
        << "      --json camera.json --config SHW3G_8CH \\\n"
        << "      --mask \"0x1000 0x0000\" all\n"
        << "  Notes:\n"
        << "    Put the command last; target and configuration options may be reordered.\n"
        << "    SIPL always requires a camera JSON file, even without saved settings.\n"
        << "    --json and --config must be specified together.\n"
        << "    Specify --mask once, with space-separated masks quoted in driver transport order.\n"
        << "    Masks accept decimal or 0x-prefixed hexadecimal values.\n"
        << "    Use --cam to select an enabled Driver CAM (zero-based), not a module channel.\n"
        << "    Without --cam, the configuration must select exactly one sensor.\n"
        << "Read Commands (V4L2 / SIPL)\n"
        << "  all                         Read all information and summarize results\n"
        << "  device-info                 Read device information\n"
        << "  get-sn                      Read the serial number\n"
        << "  get-resolution              Read the stored resolution\n"
        << "  get-image                   Read image configuration\n"
        << "  get-calibration             Read camera calibration\n"
        << "  get-imu                     Read IMU calibration\n"
        << "Options\n"
        << "  --video PATH                V4L2 video device under /dev/ (required with --v4l2)\n"
        << "  --model MODEL               Override the saved V4L2 video-node model\n"
        << "  --verbose                   With all: add SDK members and full precision\n"
        << "  --bus N                     Override the I2C bus in V4L2 mode\n"
        << "  --eeprom-addr ADDR          EEPROM_CAM only: override the 7-bit EEPROM address (e.g. 0x50)\n"
        << "  --cam N                     Select a SIPL Driver CAM (zero-based decimal)\n"
        << "  -h, --help                  Show this help\n";
}

bool ParseUint16(const char* text, std::uint16_t& value) {
    char* end = nullptr;
    const unsigned long parsed = std::strtoul(text, &end, 10);
    if (text == end || *end != '\0' || parsed == 0 ||
        parsed > std::numeric_limits<std::uint16_t>::max()) {
        return false;
    }
    value = static_cast<std::uint16_t>(parsed);
    return true;
}

bool ParseI2cBus(const char* text, int& value) {
    if (text[0] == '-') return false;

    char* end = nullptr;
    const unsigned long parsed = std::strtoul(text, &end, 10);
    if (text == end || *end != '\0' ||
        parsed > static_cast<unsigned long>(std::numeric_limits<int>::max())) {
        return false;
    }
    value = static_cast<int>(parsed);
    return true;
}

const char* PlatformName(sgmix::Platform platform) {
    switch (platform) {
    case sgmix::Platform::Auto: return "Auto";
    case sgmix::Platform::Nvidia: return "Nvidia";
    case sgmix::Platform::Rockchip: return "Rockchip";
    case sgmix::Platform::Hobot: return "Hobot";
    case sgmix::Platform::Intel: return "Intel";
    }
    return "Unknown";
}

const char* CalibrationModelName(sgmix::CameraCalibration::Model model) {
    using Model = sgmix::CameraCalibration::Model;
    switch (model) {
    case Model::Unknown: return "Unknown";
    case Model::Pinhole: return "Pinhole";
    case Model::Fisheye: return "Fisheye";
    case Model::CMei: return "CMei";
    case Model::OmniRadtan: return "OmniRadtan";
    case Model::DoubleSphere: return "DoubleSphere";
    case Model::DoubleSphereRadtan: return "DoubleSphereRadtan";
    case Model::Fisheye620: return "Fisheye620";
    }
    return "Unknown";
}

void PrintError(const char* api, const std::string& error) {
    std::cerr << api << " failed";
    if (!error.empty()) {
        std::cerr << ": " << error;
    }
    std::cerr << '\n';
}

template <typename T>
void PrintField(const char* name, const T& value) {
    std::cout << std::left << std::setw(24) << name << ": " << value << '\n';
}

template <typename T>
void PrintNestedField(const char* name, const T& value) {
    std::cout << "  " << std::left << std::setw(18) << name << ": " << value << '\n';
}

const char* PixelFormatName(sgmix::ImageInfo::PixelFormat value) {
    using Format = sgmix::ImageInfo::PixelFormat;
    switch (value) {
    case Format::Yuyv: return "YUYV";
    case Format::Uyvy: return "UYVY";
    case Format::Unknown: return "Unknown";
    }
    return "Unknown";
}

const char* TriggerModeName(sgmix::ImageInfo::TriggerMode value) {
    using Mode = sgmix::ImageInfo::TriggerMode;
    switch (value) {
    case Mode::Enabled: return "Enabled";
    case Mode::Disabled: return "Disabled";
    case Mode::Unknown: return "Unknown";
    }
    return "Unknown";
}

struct OutputContext {
    bool sipl = false;
    std::uint32_t port = UINT32_MAX;
    std::string model;
    std::string videoDevice;
};

OutputContext outputContext;
constexpr const char* kUnavailable = "Unavailable (blank or invalid)";

std::string Hex(unsigned value) {
    std::ostringstream text;
    text << "0x" << std::hex << std::uppercase << std::setw(2)
         << std::setfill('0') << value;
    return text.str();
}

std::string Number(double value) {
    if (std::isnan(value)) return "NaN";
    if (std::isinf(value)) return value < 0 ? "-Inf" : "+Inf";
    std::ostringstream text;
    text << std::setprecision(verboseOutput ? 17 : 10) << value;
    return text.str();
}

// Stored float32 metadata needs fewer display digits than double intrinsics.
std::string Number(float value) {
    if (verboseOutput || !std::isfinite(value)) return Number(static_cast<double>(value));
    std::ostringstream text;
    text << std::setprecision(6) << value;
    return text.str();
}

std::string StorageSource(const sgmix::DeviceInfo* device = nullptr) {
    const auto& model = device && !device->model.empty() ? device->model : outputContext.model;
    if (model.empty()) return "Camera Storage"; // The SDK did not resolve the storage identity.
    return model == "SG3S-ISX031C-GMSL2-Hxxx" || model == "SG3S-ISX031C-GMSL2F-Hxxx" ||
           model == "SG3S11AFLK" || model == "SHF3L" || model == "SHF3H"
        ? "Sensor Flash" : "EEPROM";
}

void PrintSection(const std::string& source, const char* name) {
    std::cout << '\n' << '[' << source << " - " << name << "]\n";
}

void PrintStored(const char* name, const std::string& value, const std::string& member) {
    PrintField(name, value + (verboseOutput ? "  [" + member + ']' : ""));
}

void PrintCrc(unsigned index, bool passed) {
    const auto label = "CRC32_" + std::to_string(index) + " check";
    PrintField(label.c_str(), passed ? "PASS" : "Unavailable");
}

void PrintRuntime(const sgmix::DeviceInfo* info) {
    PrintSection("Runtime Information", "Configuration / Driver / SDK");
    const auto connection = outputContext.sipl
        ? "SIPL / " + (outputContext.port == UINT32_MAX ? std::string("Auto (single sensor)")
                       : "CAM" + std::to_string(outputContext.port))
        : "V4L2 / I2C";
    PrintField("Connection", connection);
    const auto& model = info && !info->model.empty() ? info->model : outputContext.model;
    PrintField("Camera model", model.empty() ? "Unknown" : model);
    PrintField("Sensor", info && !info->sensor.chip.empty() ? info->sensor.chip : "Unknown");
    PrintField("Platform", info ? PlatformName(info->platform) : (outputContext.sipl ? "Nvidia" : "Unknown"));
    const auto& video = info && !info->videoDevice.empty() ? info->videoDevice : outputContext.videoDevice;
    PrintField("Video device", video.empty() ? "N/A" : video);
    if (!info || !verboseOutput) return;
    if (info->sensor.endpoint) {
        PrintField("I2C bus", info->sensor.endpoint->bus);
        PrintField("Sensor address", Hex(info->sensor.endpoint->address));
    }
    if (info->eeprom && info->eeprom->endpoint)
        PrintField("EEPROM address", Hex(info->eeprom->endpoint->address));
}

void PrintCameraInfo(const std::string& storage, const sgmix::DeviceInfo* device,
                     const sgmix::ImageInfo* image, bool imageReadFailed = false) {
    PrintSection(storage, "Camera Info");
    PrintStored("Sensor ID", device && device->sensor.id ? Hex(*device->sensor.id) : kUnavailable, "DeviceInfo.sensor.id");
    PrintStored("ISP ID", device && device->isp && device->isp->id ? Hex(*device->isp->id) : kUnavailable, "DeviceInfo.isp->id");
    if (image) {
        PrintStored("Resolution Width", std::to_string(image->width) + " px", "ImageInfo.width");
        PrintStored("Resolution Height", std::to_string(image->height) + " px", "ImageInfo.height");
        PrintStored("FPS", std::to_string(image->framesPerSecond), "ImageInfo.framesPerSecond");
        PrintStored("Image Format", std::string(PixelFormatName(image->pixelFormat)) + " (" +
                    Hex(static_cast<unsigned>(image->pixelFormat)) + ")", "ImageInfo.pixelFormat");
        PrintStored("Trigger Mode", std::string(TriggerModeName(image->triggerMode)) + " (" +
                    Hex(static_cast<unsigned>(image->triggerMode)) + ")", "ImageInfo.triggerMode");
        PrintStored("Embedded Data", image->embeddedDataEnabled ? "Enabled" : "Disabled", "ImageInfo.embeddedDataEnabled");
    }
    // GetDeviceInfo does not expose a separate Camera Info CRC result.
    const bool validIds = device && device->sensor.id && device->isp && device->isp->id;
    PrintCrc(1, !imageReadFailed && (image || validIds));
}

void PrintLens(const std::string& storage, const sgmix::DeviceInfo* info) {
    PrintSection(storage, "Lens Info");
    if (info && info->lens) {
        const auto& lens = *info->lens;
        PrintStored("Field Of View", std::to_string(lens.fieldOfViewDegrees) + " deg",
                    "DeviceInfo.lens->fieldOfViewDegrees");
        PrintStored("Focal Length", Number(lens.focalLengthMillimeters) + " mm",
                    "DeviceInfo.lens->focalLengthMillimeters");
        PrintStored("F/#", Number(lens.fNumber), "DeviceInfo.lens->fNumber");
        PrintStored("Manufacturer ID", Hex(static_cast<unsigned char>(lens.manufacturerId)), "DeviceInfo.lens->manufacturerId");
        PrintStored("Lens Index", std::to_string(lens.index), "DeviceInfo.lens->index");
        const auto name = lens.type == sgmix::DeviceInfo::LensType::Aa ? "AA" :
                          lens.type == sgmix::DeviceInfo::LensType::Threaded ? "Threaded" : "Unknown";
        PrintStored("Lens Type", std::string(name) + " (" + Hex(static_cast<unsigned>(lens.type)) + ")", "DeviceInfo.lens->type");
    } else PrintField("Status", kUnavailable);
    PrintCrc(2, info && info->lens);
}

void PrintRms(const std::string& storage, const sgmix::CameraCalibration* value, bool inputFile = false) {
    PrintSection(storage, "RMS Info");
    PrintStored("RMS", value && value->reprojectionErrorPixels
                ? Number(*value->reprojectionErrorPixels) + " px" : kUnavailable, "CameraCalibration.reprojectionErrorPixels");
    if (inputFile) PrintField("CRC32_3 check", "Not checked - input file");
    else PrintCrc(3, value && value->reprojectionErrorPixels);
}

void PrintIntrinsics(const char* side, const sgmix::CameraCalibration::Intrinsics& value) {
    using Model = sgmix::CameraCalibration::Model;
    const bool left = std::string(side) == "left";
    const auto prefix = std::string("CameraCalibration.") + side + (left ? "." : "->");
    const bool pinhole = value.model == Model::Pinhole || value.model == Model::Fisheye620;
    const bool radtan = pinhole || value.model == Model::CMei || value.model == Model::OmniRadtan ||
                        value.model == Model::DoubleSphereRadtan;
    const bool xi = value.model == Model::CMei || value.model == Model::OmniRadtan ||
                    value.model == Model::DoubleSphere || value.model == Model::DoubleSphereRadtan;
    const bool alpha = value.model == Model::DoubleSphere || value.model == Model::DoubleSphereRadtan;
    struct Parameter { const char* name; const char* member; double number; bool used; };
    const Parameter parameters[] = {
        {"fx", "fx", value.fx, true}, {"fy", "fy", value.fy, true},
        {"cx", "cx", value.cx, true}, {"cy", "cy", value.cy, true},
        {"pinhole-k1", "k1", value.k1, radtan}, {"pinhole-k2", "k2", value.k2, radtan},
        {"pinhole-p1", "p1", value.p1, radtan}, {"pinhole-p2", "p2", value.p2, radtan},
        {"pinhole-k3", "k3", value.k3, pinhole}, {"pinhole-k4", "k4", value.k4, pinhole},
        {"pinhole-k5", "k5", value.k5, pinhole}, {"pinhole-k6", "k6", value.k6, pinhole},
        {"fisheye-k1", "fisheyeK1", value.fisheyeK1, value.model == Model::Fisheye},
        {"fisheye-k2", "fisheyeK2", value.fisheyeK2, value.model == Model::Fisheye},
        {"fisheye-k3", "fisheyeK3", value.fisheyeK3, value.model == Model::Fisheye},
        {"fisheye-k4", "fisheyeK4", value.fisheyeK4, value.model == Model::Fisheye},
        {"Omni-xi/ds-xi", "xi", value.xi, xi}, {"ds-alpha", "alpha", value.alpha, alpha},
    };
    unsigned index = 0;
    for (const auto& parameter : parameters) {
        if (index == 0) std::cout << "\n  Intrinsics:\n";
        if (index == 4) std::cout << "\n  Distortion Coefficients:\n";
        auto text = Number(parameter.number);
        if (!parameter.used && value.model != Model::Unknown)
            text += std::string(" (unused by ") + CalibrationModelName(value.model) + ')';
        PrintStored(parameter.name, text, prefix + parameter.member);
        ++index;
    }
}

void PrintLeft(const std::string& storage, const sgmix::CameraCalibration* value, bool inputFile = false) {
    PrintSection(storage, "Cam0 - Left Camera");
    if (value) {
        PrintStored("Image Width", std::to_string(value->left.width) + " px",
                    "CameraCalibration.left.width");
        PrintStored("Image Height", std::to_string(value->left.height) + " px",
                    "CameraCalibration.left.height");
        PrintStored("Model", std::string(CalibrationModelName(value->left.model)) + " (" +
                    Hex(static_cast<unsigned>(value->left.model)) + ")", "CameraCalibration.left.model");
        PrintIntrinsics("left", value->left);
    } else PrintField("Status", "Read failed");
    if (inputFile) PrintField("CRC32_4 check", "Not checked - input file");
    else PrintCrc(4, value != nullptr);
}

void PrintSerial(const std::string& storage, const std::string* serial) {
    PrintSection(storage, "SN");
    PrintStored("Serial Number", serial ? *serial : kUnavailable,
                "SG_GetCameraSerialNumber / DeviceInfo.serialNumber");
    PrintCrc(5, serial != nullptr);
}

void PrintRight(const std::string& storage, const sgmix::CameraCalibration* value, bool inputFile = false) {
    PrintSection(storage, "Cam1 - Right Camera");
    if (value && value->right) PrintIntrinsics("right", *value->right);
    else {
        PrintField("Intrinsics", kUnavailable);
        PrintField("Distortion", kUnavailable);
    }
    if (inputFile) PrintField("CRC32_6 check", "Not checked - input file");
    else PrintCrc(6, value && value->right);
}

void PrintStereo(const std::string& storage, const sgmix::CameraCalibration* value, bool inputFile = false) {
    PrintSection(storage, "Stereo Extrinsics");
    if (value && value->stereo) {
        for (unsigned i = 0; i < 9; ++i) {
            const auto label = "R[" + std::to_string(i / 3) + ',' + std::to_string(i % 3) + ']';
            PrintStored(label.c_str(), Number(value->stereo->rotation[i]),
                        "CameraCalibration.stereo->rotation[" + std::to_string(i) + ']');
        }
        const char* labels[] = {"Tx", "Ty", "Tz"};
        for (unsigned i = 0; i < 3; ++i)
            PrintStored(labels[i], Number(value->stereo->translation[i]) + " mm",
                        "CameraCalibration.stereo->translation[" + std::to_string(i) + ']');
    } else { PrintField("R", kUnavailable); PrintField("T", kUnavailable); }
    if (inputFile) PrintField("CRC32_7 check", "Not checked - input file");
    else PrintCrc(7, value && value->stereo);
}

// IMU always uses EEPROM in V4L2. In SIPL it uses the selected storage reader.
std::string ImuSource(const std::string& primary) {
    return outputContext.sipl ? primary : "EEPROM";
}

void PrintImuData(const std::string& storage, const sgmix::ImuCalibration* value, bool erased, bool inputFile = false) {
    const char* status = erased ? "NOT PROGRAMMED" : "Read failed";
    PrintSection(storage, "IMU - Camera / IMU Calibration");
    if (!value) {
        PrintField("IMU Gyroscope", status);
        PrintField("IMU Accelerometer", status);
        PrintField("T_cam_imu", status);
        PrintField("Timeshift_cam_imu", status);
    } else {
        std::cout << "\n  IMU Gyroscope:\n";
        PrintStored("gyr_n_x", Number(value->gyr_n_x), "ImuCalibration.gyr_n_x");
        PrintStored("gyr_w_x", Number(value->gyr_w_x), "ImuCalibration.gyr_w_x");
        PrintStored("gyr_n_y", Number(value->gyr_n_y), "ImuCalibration.gyr_n_y");
        PrintStored("gyr_w_y", Number(value->gyr_w_y), "ImuCalibration.gyr_w_y");
        PrintStored("gyr_n_z", Number(value->gyr_n_z), "ImuCalibration.gyr_n_z");
        PrintStored("gyr_w_z", Number(value->gyr_w_z), "ImuCalibration.gyr_w_z");
        PrintStored("gyr_noise_density", Number(value->gyr_noise_density), "ImuCalibration.gyr_noise_density");
        PrintStored("gyr_random_walk", Number(value->gyr_random_walk), "ImuCalibration.gyr_random_walk");
        std::cout << "\n  IMU Accelerometer:\n";
        PrintStored("acc_n_x", Number(value->acc_n_x), "ImuCalibration.acc_n_x");
        PrintStored("acc_w_x", Number(value->acc_w_x), "ImuCalibration.acc_w_x");
        PrintStored("acc_n_y", Number(value->acc_n_y), "ImuCalibration.acc_n_y");
        PrintStored("acc_w_y", Number(value->acc_w_y), "ImuCalibration.acc_w_y");
        PrintStored("acc_n_z", Number(value->acc_n_z), "ImuCalibration.acc_n_z");
        PrintStored("acc_w_z", Number(value->acc_w_z), "ImuCalibration.acc_w_z");
        PrintStored("acc_noise_density", Number(value->acc_noise_density), "ImuCalibration.acc_noise_density");
        PrintStored("acc_random_walk", Number(value->acc_random_walk), "ImuCalibration.acc_random_walk");
        std::cout << "\n  T_cam_imu:\n";
        PrintStored("T_cam_imu_R0_0", Number(value->T_cam_imu_R0_0), "ImuCalibration.T_cam_imu_R0_0");
        PrintStored("T_cam_imu_R0_1", Number(value->T_cam_imu_R0_1), "ImuCalibration.T_cam_imu_R0_1");
        PrintStored("T_cam_imu_R0_2", Number(value->T_cam_imu_R0_2), "ImuCalibration.T_cam_imu_R0_2");
        PrintStored("T_cam_imu_R1_0", Number(value->T_cam_imu_R1_0), "ImuCalibration.T_cam_imu_R1_0");
        PrintStored("T_cam_imu_R1_1", Number(value->T_cam_imu_R1_1), "ImuCalibration.T_cam_imu_R1_1");
        PrintStored("T_cam_imu_R1_2", Number(value->T_cam_imu_R1_2), "ImuCalibration.T_cam_imu_R1_2");
        PrintStored("T_cam_imu_R2_0", Number(value->T_cam_imu_R2_0), "ImuCalibration.T_cam_imu_R2_0");
        PrintStored("T_cam_imu_R2_1", Number(value->T_cam_imu_R2_1), "ImuCalibration.T_cam_imu_R2_1");
        PrintStored("T_cam_imu_R2_2", Number(value->T_cam_imu_R2_2), "ImuCalibration.T_cam_imu_R2_2");
        PrintStored("T_cam_imu_T0", Number(value->T_cam_imu_T0) + " m", "ImuCalibration.T_cam_imu_T0");
        PrintStored("T_cam_imu_T1", Number(value->T_cam_imu_T1) + " m", "ImuCalibration.T_cam_imu_T1");
        PrintStored("T_cam_imu_T2", Number(value->T_cam_imu_T2) + " m", "ImuCalibration.T_cam_imu_T2");
        std::cout << "\n  Timeshift_cam_imu:\n";
        PrintStored("Timeshift_cam_imu", Number(value->Timeshift_cam_imu) + " s", "ImuCalibration.Timeshift_cam_imu");
    }
    if (inputFile) PrintField("CRC32_8 check", "Not checked - input file");
    else if (erased) PrintField("CRC32_8 check", "Not checked - erased");
    else PrintCrc(8, value != nullptr);
    PrintSection(storage, "IMU - Factory Calibration");
    if (!value) {
        for (const char* group : {"Gyro Bias", "Gyro K Matrix", "Gyro Scale", "Gyro Misalignment",
                                  "Acc Bias", "Acc K Matrix", "Acc Scale", "Acc Misalignment"})
            PrintField(group, status);
    } else {
        std::cout << "\n  Gyro Bias:\n";
        PrintStored("gyro_bias_x", Number(value->gyro_bias_x), "ImuCalibration.gyro_bias_x");
        PrintStored("gyro_bias_y", Number(value->gyro_bias_y), "ImuCalibration.gyro_bias_y");
        PrintStored("gyro_bias_z", Number(value->gyro_bias_z), "ImuCalibration.gyro_bias_z");
        std::cout << "\n  Gyro K Matrix:\n";
        PrintStored("gyro_K0_0", Number(value->gyro_K0_0), "ImuCalibration.gyro_K0_0");
        PrintStored("gyro_K0_1", Number(value->gyro_K0_1), "ImuCalibration.gyro_K0_1");
        PrintStored("gyro_K0_2", Number(value->gyro_K0_2), "ImuCalibration.gyro_K0_2");
        PrintStored("gyro_K1_0", Number(value->gyro_K1_0), "ImuCalibration.gyro_K1_0");
        PrintStored("gyro_K1_1", Number(value->gyro_K1_1), "ImuCalibration.gyro_K1_1");
        PrintStored("gyro_K1_2", Number(value->gyro_K1_2), "ImuCalibration.gyro_K1_2");
        PrintStored("gyro_K2_0", Number(value->gyro_K2_0), "ImuCalibration.gyro_K2_0");
        PrintStored("gyro_K2_1", Number(value->gyro_K2_1), "ImuCalibration.gyro_K2_1");
        PrintStored("gyro_K2_2", Number(value->gyro_K2_2), "ImuCalibration.gyro_K2_2");
        std::cout << "\n  Gyro Scale:\n";
        PrintStored("gyro_scale_x", Number(value->gyro_scale_x), "ImuCalibration.gyro_scale_x");
        PrintStored("gyro_scale_y", Number(value->gyro_scale_y), "ImuCalibration.gyro_scale_y");
        PrintStored("gyro_scale_z", Number(value->gyro_scale_z), "ImuCalibration.gyro_scale_z");
        std::cout << "\n  Gyro Misalignment:\n";
        PrintStored("gyro_M0_0", Number(value->gyro_M0_0), "ImuCalibration.gyro_M0_0");
        PrintStored("gyro_M0_1", Number(value->gyro_M0_1), "ImuCalibration.gyro_M0_1");
        PrintStored("gyro_M0_2", Number(value->gyro_M0_2), "ImuCalibration.gyro_M0_2");
        PrintStored("gyro_M1_0", Number(value->gyro_M1_0), "ImuCalibration.gyro_M1_0");
        PrintStored("gyro_M1_1", Number(value->gyro_M1_1), "ImuCalibration.gyro_M1_1");
        PrintStored("gyro_M1_2", Number(value->gyro_M1_2), "ImuCalibration.gyro_M1_2");
        PrintStored("gyro_M2_0", Number(value->gyro_M2_0), "ImuCalibration.gyro_M2_0");
        PrintStored("gyro_M2_1", Number(value->gyro_M2_1), "ImuCalibration.gyro_M2_1");
        PrintStored("gyro_M2_2", Number(value->gyro_M2_2), "ImuCalibration.gyro_M2_2");
        std::cout << "\n  Acc Bias:\n";
        PrintStored("acc_bias_x", Number(value->acc_bias_x), "ImuCalibration.acc_bias_x");
        PrintStored("acc_bias_y", Number(value->acc_bias_y), "ImuCalibration.acc_bias_y");
        PrintStored("acc_bias_z", Number(value->acc_bias_z), "ImuCalibration.acc_bias_z");
        std::cout << "\n  Acc K Matrix:\n";
        PrintStored("acc_K0_0", Number(value->acc_K0_0), "ImuCalibration.acc_K0_0");
        PrintStored("acc_K0_1", Number(value->acc_K0_1), "ImuCalibration.acc_K0_1");
        PrintStored("acc_K0_2", Number(value->acc_K0_2), "ImuCalibration.acc_K0_2");
        PrintStored("acc_K1_0", Number(value->acc_K1_0), "ImuCalibration.acc_K1_0");
        PrintStored("acc_K1_1", Number(value->acc_K1_1), "ImuCalibration.acc_K1_1");
        PrintStored("acc_K1_2", Number(value->acc_K1_2), "ImuCalibration.acc_K1_2");
        PrintStored("acc_K2_0", Number(value->acc_K2_0), "ImuCalibration.acc_K2_0");
        PrintStored("acc_K2_1", Number(value->acc_K2_1), "ImuCalibration.acc_K2_1");
        PrintStored("acc_K2_2", Number(value->acc_K2_2), "ImuCalibration.acc_K2_2");
        std::cout << "\n  Acc Scale:\n";
        PrintStored("acc_scale_x", Number(value->acc_scale_x), "ImuCalibration.acc_scale_x");
        PrintStored("acc_scale_y", Number(value->acc_scale_y), "ImuCalibration.acc_scale_y");
        PrintStored("acc_scale_z", Number(value->acc_scale_z), "ImuCalibration.acc_scale_z");
        std::cout << "\n  Acc Misalignment:\n";
        PrintStored("acc_M0_0", Number(value->acc_M0_0), "ImuCalibration.acc_M0_0");
        PrintStored("acc_M0_1", Number(value->acc_M0_1), "ImuCalibration.acc_M0_1");
        PrintStored("acc_M0_2", Number(value->acc_M0_2), "ImuCalibration.acc_M0_2");
        PrintStored("acc_M1_0", Number(value->acc_M1_0), "ImuCalibration.acc_M1_0");
        PrintStored("acc_M1_1", Number(value->acc_M1_1), "ImuCalibration.acc_M1_1");
        PrintStored("acc_M1_2", Number(value->acc_M1_2), "ImuCalibration.acc_M1_2");
        PrintStored("acc_M2_0", Number(value->acc_M2_0), "ImuCalibration.acc_M2_0");
        PrintStored("acc_M2_1", Number(value->acc_M2_1), "ImuCalibration.acc_M2_1");
        PrintStored("acc_M2_2", Number(value->acc_M2_2), "ImuCalibration.acc_M2_2");
    }
    if (inputFile) PrintField("CRC32_9 check", "Not checked - input file");
    else if (erased) PrintField("CRC32_9 check", "Not checked - erased");
    else PrintCrc(9, value != nullptr);
}

void PrintCapabilities(const sgmix::DeviceInfo* info, const sgmix::CameraCalibration* calibration = nullptr) {
    PrintSection("SDK Results", "Derived Status");
    if (calibration) PrintField("Calibration Type", calibration->right ? "Stereo" : "Monocular");
    PrintField("ISP Control", info ? (info->ispControlSupported ? "Supported" : "Not supported") : "Unavailable");
    PrintField("Firmware OTA", info ? (info->firmwareUpdateSupported ? "Supported" : "Not supported") : "Unavailable");
}

void PrintDeviceInfo(const sgmix::DeviceInfo& info) {
    const auto storage = StorageSource(&info);
    PrintRuntime(&info);
    PrintCameraInfo(storage, &info, nullptr);
    PrintLens(storage, &info);
    PrintSerial(storage, info.serialNumber ? &*info.serialNumber : nullptr);
    PrintCapabilities(&info);
}

void PrintCalibration(const sgmix::CameraCalibration& value, bool inputFile = false) {
    const auto storage = inputFile ? std::string("Input File") : StorageSource();
    PrintRms(storage, &value, inputFile);
    PrintLeft(storage, &value, inputFile);
    PrintRight(storage, &value, inputFile);
    PrintStereo(storage, &value, inputFile);
    PrintSection("SDK Results", "Derived Status");
    PrintField("Calibration Type", value.right ? "Stereo" : "Monocular");
}

void PrintImuCalibration(const sgmix::ImuCalibration& value, bool inputFile = false) {
    PrintImuData(inputFile ? "Input File" : ImuSource(StorageSource()), &value, false, inputFile);
}

bool GetDeviceInfo(sgmix::CameraControl& camera) {
    sgmix::DeviceInfo info;
    std::string error;
    if (!camera.SG_GetDeviceInfo(info, error)) { PrintError("SG_GetDeviceInfo", error); return false; }
    PrintDeviceInfo(info);
    return true;
}

bool GetSerialNumber(sgmix::CameraControl& camera) {
    std::string value, error;
    if (!camera.SG_GetCameraSerialNumber(value, error)) { PrintError("SG_GetCameraSerialNumber", error); return false; }
    std::cout << "SN: " << value << '\n';
    return true;
}

bool GetResolution(sgmix::CameraControl& camera) {
    std::uint16_t width = 0, height = 0;
    std::string error;
    if (!camera.SG_GetCameraResolution(width, height, error)) { PrintError("SG_GetCameraResolution", error); return false; }
    std::cout << "Resolution: " << width << 'x' << height << '\n';
    return true;
}

bool GetImageInfo(sgmix::CameraControl& camera) {
    sgmix::ImageInfo info;
    std::string error;
    if (!camera.SG_GetImageInfo(info, error)) { PrintError("SG_GetImageInfo", error); return false; }
    PrintCameraInfo(StorageSource(), nullptr, &info);
    return true;
}

bool GetCalibration(sgmix::CameraControl& camera) {
    sgmix::CameraCalibration value{};
    std::string error;
    if (!camera.SG_GetCalibration(value, error)) { PrintError("SG_GetCalibration", error); return false; }
    PrintCalibration(value);
    return true;
}

bool GetImuCalibration(sgmix::CameraControl& camera) {
    sgmix::ImuCalibration value{};
    std::string error;
    if (!camera.SG_GetImuCalibration(value, error)) {
        if (error == "IMU calibration is not programmed") {
            PrintImuData(ImuSource(StorageSource()), nullptr, true);
            return true;
        }
        PrintError("SG_GetImuCalibration", error);
        return false;
    }
    PrintImuCalibration(value);
    return true;
}

template <typename T> struct ReadResult {
    T value{};
    bool ok = false;
    std::string error;
};

struct StoredResolution { std::uint16_t width = 0, height = 0; };

bool GetAll(sgmix::CameraControl& camera) {
    ReadResult<sgmix::DeviceInfo> device;
    ReadResult<std::string> serial;
    ReadResult<StoredResolution> resolution;
    ReadResult<sgmix::ImageInfo> image;
    ReadResult<sgmix::CameraCalibration> calibration;
    ReadResult<sgmix::ImuCalibration> imu;
    // Collect every existing read independently before printing in storage order.
    device.ok = camera.SG_GetDeviceInfo(device.value, device.error);
    serial.ok = camera.SG_GetCameraSerialNumber(serial.value, serial.error);
    resolution.ok = camera.SG_GetCameraResolution(resolution.value.width, resolution.value.height, resolution.error);
    image.ok = camera.SG_GetImageInfo(image.value, image.error);
    calibration.ok = camera.SG_GetCalibration(calibration.value, calibration.error);
    imu.ok = camera.SG_GetImuCalibration(imu.value, imu.error);
    const bool erased = !imu.ok && imu.error == "IMU calibration is not programmed";
    const auto* info = device.ok ? &device.value : nullptr;
    const auto* calib = calibration.ok ? &calibration.value : nullptr;
    const auto storage = StorageSource(info);
    std::cout << "sgMIX Camera Information\n" << std::string(70, '=') << '\n';
    PrintRuntime(info);
    std::cout << '\n' << std::string(70, '=') << '\n' << storage
              << " DATA - IN PROTOCOL ORDER\n" << std::string(70, '=') << '\n';
    PrintCameraInfo(storage, info, image.ok ? &image.value : nullptr, !image.ok);
    if (!image.ok) {
        PrintField("Image Configuration", "Read failed");
        if (resolution.ok) {
            PrintField("Stored Resolution", std::to_string(resolution.value.width) + " x " +
                       std::to_string(resolution.value.height) + " (separate resolution read)");
        }
    }
    PrintLens(storage, info);
    PrintRms(storage, calib);
    PrintLeft(storage, calib);
    PrintSerial(storage, serial.ok ? &serial.value : nullptr);
    PrintRight(storage, calib);
    PrintStereo(storage, calib);
    PrintImuData(ImuSource(storage), imu.ok ? &imu.value : nullptr, erased);
    PrintCapabilities(info, calib);
    std::cout << "\n[Read Results]\n";
    PrintField("Device Information", device.ok ? "OK" : "FAILED");
    PrintField("Serial Number", serial.ok ? "OK" : "FAILED");
    PrintField("Stored Resolution", resolution.ok ? "OK" : "FAILED");
    PrintField("Image Configuration", image.ok ? "OK" : "FAILED");
    PrintField("Camera Calibration", calibration.ok ? "OK" : "FAILED");
    PrintField("IMU Calibration", imu.ok ? "OK" : (erased ? "NOT PROGRAMMED" : "FAILED"));
    const unsigned passed = device.ok + serial.ok + resolution.ok + image.ok + calibration.ok + imu.ok;
    const unsigned empty = erased ? 1 : 0;
    const unsigned failed = 6 - passed - empty;
    std::cout << '\n' << std::string(70, '=') << "\nRead completed: " << passed
              << " successful, " << empty << " not programmed, " << failed << " failed.\n";
    std::cout.flush();
    if (!device.ok) PrintError("SG_GetDeviceInfo", device.error);
    if (!serial.ok) PrintError("SG_GetCameraSerialNumber", serial.error);
    if (!resolution.ok) PrintError("SG_GetCameraResolution", resolution.error);
    if (!image.ok) PrintError("SG_GetImageInfo", image.error);
    if (!calibration.ok) PrintError("SG_GetCalibration", calibration.error);
    if (!imu.ok && !erased) PrintError("SG_GetImuCalibration", imu.error);
    return failed == 0;
}

} // namespace

int main(int argc, char* argv[]) {
    if (argc == 2 && (std::string(argv[1]) == "--help" || std::string(argv[1]) == "-h")) {
        PrintUsage(argv[0]);
        return 0;
    }
    if (argc < 3) {
        PrintUsage(argv[0]);
        return 2;
    }

    const std::string mode = argv[1];
    const bool sipl = mode == "--sipl";
    const bool namedV4l2 = mode == "--v4l2";
    if (!sipl && !namedV4l2) {
        std::cerr << "Unknown mode: " << mode << '\n';
        PrintUsage(argv[0]);
        return 2;
    }
    const int commandIndex = argc - 1;
    const std::string command = argv[commandIndex];
    const std::map<std::string, int> argumentCounts = {
        {"all", 0}, {"device-info", 0}, {"get-sn", 0}, {"get-resolution", 0},
        {"get-image", 0}, {"get-calibration", 0}, {"get-imu", 0},
    };
    const auto arguments = argumentCounts.find(command);
    if (arguments == argumentCounts.end()) {
        std::cerr << "Unknown command: " << command << "\n\n";
        PrintUsage(argv[0]);
        return 2;
    }
    int optionIndex = 2;
    const int optionEnd = commandIndex;

    sgmix::CameraConfig config;
    bool verbose = false;
    std::uint32_t cameraPort = UINT32_MAX;
    std::string siplJson, siplConfig;
    std::vector<std::uint32_t> siplMasks;
    // Target and configuration options precede the final read command.
    while (optionIndex < optionEnd) {
        const std::string option = argv[optionIndex];
        const bool hasValue = optionIndex + 1 < optionEnd &&
            !std::string(argv[optionIndex + 1]).empty() &&
            std::string(argv[optionIndex + 1]).compare(0, 2, "--") != 0;
        if (option == "--verbose" && !verbose && command == "all") {
            verbose = true; ++optionIndex; continue;
        }
        if (!sipl && option == "--video" && hasValue && config.videoDevice.empty()) {
            config.videoDevice = argv[optionIndex + 1];
            optionIndex += 2;
            continue;
        }
        if (!sipl && option == "--model" && hasValue && config.model.empty()) {
            config.model = argv[optionIndex + 1];
            optionIndex += 2;
            continue;
        }
        if (sipl && (option == "--json" || option == "--config") && hasValue) {
            auto& target = option == "--json" ? siplJson : siplConfig;
            const std::string value = argv[optionIndex + 1];
            if (!target.empty() || value.empty() || value.compare(0, 2, "--") == 0) {
                std::cerr << "Duplicate or empty SIPL configuration option: " << option << '\n';
                return 2;
            }
            target = value;
            optionIndex += 2;
            continue;
        }
        if (sipl && option == "--mask" && hasValue && siplMasks.empty()) {
            std::istringstream values(argv[optionIndex + 1]);
            std::string value;
            if (!(values >> value)) {
                std::cerr << "SIPL mask list must not be empty.\n";
                return 2;
            }
            do {
                char* end = nullptr;
                errno = 0;
                const auto parsed = std::strtoull(value.c_str(), &end, 0);
                if (value.find_first_not_of("0123456789abcdefABCDEFxX") != std::string::npos ||
                    end == value.c_str() || *end != '\0' || errno == ERANGE ||
                    parsed > std::numeric_limits<std::uint32_t>::max()) {
                    std::cerr << "Invalid SIPL mask: " << value << '\n';
                    return 2;
                }
                siplMasks.push_back(static_cast<std::uint32_t>(parsed));
            } while (values >> value);
            optionIndex += 2;
            continue;
        }
        if (sipl && option == "--cam" && hasValue && cameraPort == UINT32_MAX) {
            std::uint16_t port = 0;
            const std::string value = argv[optionIndex + 1];
            if (value.find_first_not_of("0123456789") != std::string::npos ||
                (value.empty() || (value.find_first_not_of("0") != std::string::npos &&
                                  !ParseUint16(argv[optionIndex + 1], port)))) {
                std::cerr << "Invalid SIPL CAM; use a zero-based decimal CAM number.\n";
                return 2;
            }
            cameraPort = port;
            optionIndex += 2;
            continue;
        }
        if (!sipl && option == "--eeprom-addr" && hasValue && !config.eepromI2cAddress) {
            const std::string value = argv[optionIndex + 1];
            char* end = nullptr;
            errno = 0;
            const int base = value.compare(0, 2, "0x") == 0 || value.compare(0, 2, "0X") == 0 ? 16 : 10;
            const auto address = std::strtoul(value.c_str(), &end, base);
            if (value.empty() || value.find_first_not_of("0123456789abcdefABCDEFxX") != std::string::npos ||
                end == value.c_str() || *end != '\0' || errno == ERANGE || address < 0x08 || address > 0x77) {
                std::cerr << "Invalid EEPROM address: " << value
                          << ". Expected a 7-bit address in 0x08..0x77.\n";
                return 2;
            }
            config.eepromI2cAddress = static_cast<std::uint8_t>(address);
            optionIndex += 2;
            continue;
        }
        if (!sipl && option == "--bus" && hasValue && !config.i2cBus) {
            int bus = -1;
            if (!ParseI2cBus(argv[optionIndex + 1], bus)) {
                std::cerr << "Invalid I2C bus: " << argv[optionIndex + 1]
                          << ". Expected a non-negative integer.\n";
                return 2;
            }
            config.i2cBus = bus;
            optionIndex += 2;
            continue;
        }

        std::cerr << "Unknown or incomplete option: " << option << "\n\n";
        PrintUsage(argv[0]);
        return 2;
    }

    if (namedV4l2 && config.videoDevice.compare(0, 5, "/dev/") != 0) {
        std::cerr << "V4L2 requires --video with a device path under /dev/.\n";
        return 2;
    }
    if (!sipl && config.eepromI2cAddress && config.model != "EEPROM_CAM") {
        std::cerr << "--eeprom-addr is only available with --model EEPROM_CAM.\n";
        return 2;
    }
    if (!sipl && config.model == "EEPROM_CAM" && !config.i2cBus) {
        std::cerr << "EEPROM_CAM requires an explicit --bus.\n";
        return 2;
    }
    if (sipl && (siplJson.empty() != siplConfig.empty() ||
                 (!siplMasks.empty() && siplJson.empty()))) {
        std::cerr << "Use --json and --config together; --mask requires both.\n";
        return 2;
    }
    sgmix::CameraControl camera;
    std::string error;
    const bool initialized = sipl ? camera.SG_Init(siplJson, siplConfig, error, siplMasks, cameraPort)
                                  : camera.SG_Init(config, error);
    if (!initialized) {
        PrintError("SG_Init", error);
        return 1;
    }

    verboseOutput = verbose;
    std::cout << std::setprecision(verboseOutput ? 17 : 10);
    outputContext = {sipl, cameraPort, config.model, config.videoDevice};

    bool success = false;
    if (command == "all") {
        success = GetAll(camera);
    } else if (command == "device-info") {
        success = GetDeviceInfo(camera);
    } else if (command == "get-sn") {
        success = GetSerialNumber(camera);
    } else if (command == "get-resolution") {
        success = GetResolution(camera);
    } else if (command == "get-image") {
        success = GetImageInfo(camera);
    } else if (command == "get-calibration") {
        success = GetCalibration(camera);
    } else if (command == "get-imu") {
        success = GetImuCalibration(camera);
    } else {
        std::cerr << "Unknown command: " << command << "\n\n";
        PrintUsage(argv[0]);
        return 2;
    }

    return success ? 0 : 1;
}
