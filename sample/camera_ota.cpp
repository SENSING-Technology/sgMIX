// Usage: camera_ota --v4l2 --video VIDEO [options] UPDATE_FILE|--get-version
//    or: camera_ota --sipl [--cam N] [options] UPDATE_FILE|--get-version
#include "SG_CameraControl.h"
#include <cstdint>
#include <exception>
#include <iostream>
#include <limits>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

static uint32_t number(const char* value) {
    std::string text(value);
    size_t used = 0;
    if (text.empty() || text.front() < '0' || text.front() > '9')
        throw std::invalid_argument("Invalid number");
    auto result = std::stoull(text, &used, 0);
    if (used != text.size() || result > std::numeric_limits<uint32_t>::max())
        throw std::invalid_argument("Invalid number");
    return static_cast<uint32_t>(result);
}

static bool isIsx031Model(const std::string& model) {
    return model == "SG3S-ISX031C-GMSL2F-Hxxx" || model == "SG3S-ISX031C-GMSL2-Hxxx" ||
           model == "SG3S11AFLK" || model == "SHF3L" || model == "SHF3H";
}

static uint32_t camNumber(const char* value) {
    const std::string text(value);
    if (text.empty() || text.find_first_not_of("0123456789") != std::string::npos)
        throw std::invalid_argument("Use a zero-based decimal Driver CAM number");
    const auto result = std::stoull(text, nullptr, 10);
    if (result >= UINT32_MAX)
        throw std::out_of_range("Driver CAM number is out of range");
    return static_cast<uint32_t>(result);
}

static void usage(const char* program) {
    std::cout
        << "sgMIX Camera OTA Tool\n\n"
        << "Mode 1: Use Saved Configuration\n"
        << "  Use the camera model or SIPL settings saved by the configuration tool.\n\n"
        << "  V4L2:\n"
        << "    " << program << " --v4l2 --video <video_device> <update_file>\n"
        << "  SIPL:\n"
        << "    " << program << " --sipl [--cam N] <update_file>\n\n"
        << "  Examples:\n"
        << "    " << program << " --v4l2 --video /dev/video7 firmware.bin\n"
        << "    " << program << " --sipl --cam 7 firmware.bin\n"
        << "    " << program << " --sipl --cam 7 SHW3G.nito\n\n"
        << "Mode 2: Specify Configuration Manually\n"
        << "  Use this mode when no configuration has been saved.\n\n"
        << "  V4L2:\n"
        << "    " << program << " --v4l2 --video <video_device> --model <model> [--bus N] <update_file>\n"
        << "  SIPL:\n"
        << "    " << program << " --sipl [--cam N] \\\n"
        << "      --json <path> --config <name> [--mask \"MASK ...\"] <update_file>\n\n"
        << "  Examples:\n"
        << "    " << program << " --v4l2 --video /dev/video7 --model SHW3G tuning.isp\n"
        << "    " << program << " --sipl --cam 7 \\\n"
        << "      --json camera.json --config SHW3G_8CH \\\n"
        << "      --mask \"0x1000 0x0000\" SHW3G.nito\n\n"
        << "Update Files\n"
        << "  .bin                        ISX031 / S36 camera firmware\n"
        << "  .rom                        GW5 / GW6 camera firmware\n"
        << "  .isp / .nito                V4L2 NVIDIA ISP tuning: SHW3G / SHW5G / S56\n"
        << "  .nito                       SIPL NVIDIA ISP tuning: mono RAW / S56 / S56C\n\n"
        << "Read Firmware Version\n"
        << "  Replace <update_file> with --get-version.\n\n"
        << "  Examples:\n"
        << "    " << program << " --v4l2 --video /dev/video7 --get-version\n"
        << "    " << program << " --sipl --cam 7 --get-version\n\n"
        << "  Supported: V4L2 ISX031 / S36 / GW5 / GW6; SIPL configured ISX031 / matched S36 stereo.\n\n"
        << "Options\n"
        << "  --video PATH                V4L2 video device, e.g. /dev/video7\n"
        << "  --model MODEL               Override the saved V4L2 camera model\n"
        << "  --bus N                     Override the V4L2 I2C bus\n"
        << "  --cam N                     Select a SIPL Driver CAM (zero-based decimal)\n"
        << "  --json PATH                 SIPL camera JSON file\n"
        << "  --config NAME               Platform configuration name in the JSON file\n"
        << "  --mask \"MASK ...\"           Quoted masks in driver transport order\n"
        << "  -h, --help                  Show this help\n\n"
        << "Notes\n"
        << "  Put the update file or --get-version last.\n"
        << "  Use --json and --config together; --mask requires both.\n"
        << "  Without --cam, the SIPL configuration must select exactly one sensor.\n"
        << "  Stop all camera capture before updating.\n"
        << "  ISX031 firmware requires a 512 KiB .bin file; ISP tuning is limited to 16 MiB.\n"
        << "  Power-cycle ISX031/S36 after firmware updates, then read the version.\n"
        << "  V4L2 tuning updates require reloading Argus to take effect.\n"
        << "  SIPL NITO updates reload tuning and verify frame output automatically.\n"
        << "  V4L2 S36 updates both eyes sequentially from either video node; both saved models must be S36.\n"
        << "  V4L2 S36 checks both routes before erase; no full-image readback comparison is performed.\n"
        << "  --get-version reads the selected V4L2 eye; read both eyes after the power cycle.\n"
        << "  SIPL S36 writes physical left then right and executes the right address configuration sequence.\n"
        << "  --sipl [--cam N] --restore-right-address: address repair only, without firmware input.\n"
        << "  Address-only repair bypasses old right-eye version preflight; release/reinitialize afterward.\n"
        << "  SIPL firmware updates require a matched driver route.\n"
        << "  SIPL GW6 also requires a verified mono route and 7-bit alias 0x6D.\n"
        << "  Update files may use local paths or SDK-supported encoded download addresses.\n";
}

int main(int argc, char** argv) {
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "--check") {
            std::cerr << "Unknown option: --check\n";
            return 2;
        }
    }
    const bool siplMode = argc > 1 && std::string(argv[1]) == "--sipl";
    const bool namedV4l2 = argc > 1 && std::string(argv[1]) == "--v4l2";
    uint32_t cameraPort = UINT32_MAX;
    if (argc == 2 && (std::string(argv[1]) == "--help" || std::string(argv[1]) == "-h")) {
        usage(argv[0]);
        return 0;
    }
    if (argc < 3 || (!siplMode && !namedV4l2)) {
        usage(argv[0]);
        return 2;
    }
    try {
        const bool sipl = siplMode;
        sgmix::CameraConfig config;
        std::string json, name, path, error;
        std::vector<uint32_t> masks;
        try {
            path = argv[argc - 1];
            if (path.empty() || (path.rfind("--", 0) == 0 && path != "--get-version" && path != "--restore-right-address"))
                throw std::invalid_argument("Put an update file or --get-version last");
            std::set<std::string> seen;
            for (int i = 2; i < argc - 1; ++i) {
                std::string key = argv[i];
                if (seen.count(key))
                    throw std::invalid_argument("Repeated option: " + key);
                seen.insert(key);
                if (i + 1 >= argc - 1 || std::string(argv[i + 1]).empty() ||
                    std::string(argv[i + 1]).rfind("--", 0) == 0)
                    throw std::invalid_argument("Missing value for " + key);
                const char* value = argv[++i];
                if (!sipl && key == "--video") config.videoDevice = value;
                else if (!sipl && key == "--model") config.model = value;
                else if (!sipl && key == "--bus") {
                    const auto bus = number(value);
                    if (bus > static_cast<uint32_t>(std::numeric_limits<int>::max()))
                        throw std::out_of_range("I2C bus number too large");
                    config.i2cBus = static_cast<int>(bus);
                } else if (sipl && key == "--cam") cameraPort = camNumber(value);
                else if (sipl && key == "--json") json = value;
                else if (sipl && key == "--config") name = value;
                else if (sipl && key == "--mask") {
                    std::istringstream values(value);
                    std::string mask;
                    if (!(values >> mask)) throw std::invalid_argument("SIPL mask list must not be empty");
                    do {
                        masks.push_back(number(mask.c_str()));
                    } while (values >> mask);
                }
                else throw std::invalid_argument("Unknown option for this mode: " + key);
            }
            if (!sipl && path=="--restore-right-address")
                throw std::invalid_argument("--restore-right-address requires --sipl");
            if (!sipl && config.videoDevice.rfind("/dev/", 0) != 0)
                throw std::invalid_argument("--v4l2 requires --video with a device path under /dev/");
            if (sipl && (json.empty() != name.empty() || (!masks.empty() && json.empty())))
                throw std::invalid_argument("Use --json and --config together; --mask requires both");
        } catch (const std::exception& ex) {
            std::cerr << ex.what() << '\n';
            usage(argv[0]);
            return 2;
        }
        if (path.empty()) throw std::invalid_argument("Update file must not be empty");
        bool powerCycleRequired = isIsx031Model(config.model) || config.model == "S36";
        {
            sgmix::CameraControl camera;
            const bool initialized = sipl ? camera.SG_Init(json, name, error, masks, cameraPort)
                                          : camera.SG_Init(config, error);
            if (!initialized) throw std::runtime_error("SG_Init: " + error);
            // An application sharing a SIPL session must stop every camera before OTA.
            if (!camera.SG_StopCapture(error))
                throw std::runtime_error("SG_StopCapture: " + error);
            if (path=="--restore-right-address") {
                if (!camera.SG_RestoreS36RightAddress(error))
                    throw std::runtime_error("SG_RestoreS36RightAddress: "+error);
                std::cout<<"SUCCESS: S36 right address configuration written; activation pending; no firmware erased or written.\n";
                std::cout<<"Release and initialize again; verify both versions after a power cycle.\n";
                return 0;
            }
            std::string version;
            const bool versionRead = camera.SG_GetFirmwareVersion(version, error);
            const auto versionError = error;
            if (path == "--get-version") {
                if (!versionRead) throw std::runtime_error("SG_GetFirmwareVersion: " + versionError);
                std::cout << "Firmware version (running): " << version << '\n';
                return 0;
            }
            // ISX031 and matched S36 report running firmware versions before update.
            if (sipl && versionRead) powerCycleRequired = true;
            if ((sipl && !versionRead) || (!sipl && config.model.empty())) {
                sgmix::DeviceInfo info;
                std::string infoError;
                const bool infoRead = camera.SG_GetDeviceInfo(info, infoError);
                // The SDK fills the bound model before optional storage reads.
                // Metadata failure must not become a new V4L2 OTA prerequisite.
                powerCycleRequired = powerCycleRequired || isIsx031Model(info.model) || info.model == "S36";
                if (sipl && !infoRead)
                    throw std::runtime_error("Cannot verify SIPL update target: " + versionError +
                                             "; SG_GetDeviceInfo: " + infoError);
                if (sipl && isIsx031Model(info.model))
                    throw std::runtime_error("SG_GetFirmwareVersion before update: " + versionError);
            }
            std::cout << "[OTA] Start " << (sipl ? "SIPL" : "V4L2")
                      << " update: " << path << std::endl;
            if (versionRead)
                std::cout << "Version before update (running): " << version << '\n';
            else
                std::cerr << "Version before update unavailable: " << versionError << '\n';
            if (!camera.SG_PerformOTAUpdate(path, error))
                throw std::runtime_error("SG_PerformOTAUpdate: " + error);
            // Release the session before using hardware that the update may have reset.
        }
        std::cout << "SUCCESS: " << (sipl ? "SIPL" : "V4L2")
                  << " update completed; session released.\n";
        if (powerCycleRequired) {
            std::cout << "[OTA] ISX031/S36 firmware activation is pending. Completely power off the camera, then power it on again.\n"
                      << "[OTA] After the power cycle, use --get-version to display the new running firmware version.\n";
        }
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "FAILED: " << e.what() << '\n';
        return 1;
    }
}
