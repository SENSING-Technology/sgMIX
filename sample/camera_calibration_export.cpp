#include "SG_CameraControl.h"

#include <cerrno>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

namespace {

namespace fs = std::filesystem;

void PrintUsage(const char* program) {
    std::cout
        << "sgMIX Camera Calibration Export\n"
        << "Read the serial number and the stored calibration, then write a text file\n"
        << "named after the serial number (for example I06150119.txt).\n"
        << "\n"
        << "Mode 1: Use Saved Configuration\n"
        << "  V4L2:\n"
        << "    " << program << " <video_device> [--out-dir DIR] [--bus N]\n"
        << "  SIPL:\n"
        << "    " << program << " --sipl [--out-dir DIR] [--port N]\n"
        << "Mode 2: Specify Configuration Manually\n"
        << "  V4L2:\n"
        << "    " << program << " <model> <video_device> [--out-dir DIR] [--bus N] [--eeprom-addr ADDR]\n"
        << "  SIPL:\n"
        << "    " << program << " --sipl --json <path> --config <name> [--mask VALUE ...]\n"
        << "      [--out-dir DIR] [--port N]\n"
        << "\n"
        << "Options\n"
        << "  --out-dir DIR               Output directory (default: current directory)\n"
        << "  --name NAME                 Output file name without the directory; .txt is\n"
        << "                              appended when missing (default: <SN>.txt)\n"
        << "  --bus N                     Override the I2C bus in V4L2 mode\n"
        << "  --eeprom-addr ADDR          Override the 7-bit EEPROM address (e.g. 0x50)\n"
        << "  --json <path> --config <name>   Explicit SIPL configuration\n"
        << "  --mask VALUE                Repeat per device block; decimal or 0x hex\n"
        << "  --port N                    Select a SIPL camera port (zero-based decimal)\n"
        << "  -h, --help                  Show this help\n"
        << "\n"
        << "Examples\n"
        << "  " << program << " /dev/video6 --out-dir /home/user/calib\n"
        << "  " << program << " /dev/video0 --out-dir /home/user/calib --name cam0_calib\n"
        << "  " << program << " --sipl --port 7 --out-dir /home/user/calib\n"
        << "  " << program << " SHW3G /dev/video7 --out-dir .\n"
        << "\n"
        << "The file keeps the existing calibration layout:\n"
        << "  type= / imageWidth= / imageHeight=\n"
        << "  CAM0_fx= ... CAM0_k3=    (CAM0 is the left camera)\n"
        << "  CAM1_fx= ... CAM1_k3=    (CAM1 is the right camera)\n"
        << "  R[i,j]= / T[i,0]=        (stereo extrinsics, millimetres)\n"
        << "It never writes camera storage.\n";
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

bool ParsePort(const char* text, std::uint32_t& value) {
    if (text[0] == '-') return false;

    char* end = nullptr;
    errno = 0;
    const unsigned long parsed = std::strtoul(text, &end, 10);
    if (text == end || *end != '\0' || errno == ERANGE ||
        parsed > std::numeric_limits<std::uint32_t>::max()) {
        return false;
    }
    value = static_cast<std::uint32_t>(parsed);
    return true;
}

bool ParseMask(const char* text, std::uint32_t& value) {
    const std::string input(text);
    if (input.empty() ||
        input.find_first_not_of("0123456789abcdefABCDEFxX") != std::string::npos) {
        return false;
    }

    char* end = nullptr;
    errno = 0;
    const auto parsed = std::strtoull(text, &end, 0);
    if (end == text || *end != '\0' || errno == ERANGE ||
        parsed > std::numeric_limits<std::uint32_t>::max()) {
        return false;
    }
    value = static_cast<std::uint32_t>(parsed);
    return true;
}

bool ParseEepromAddress(const char* text, std::uint8_t& value) {
    const std::string input(text);
    if (input.empty() ||
        input.find_first_not_of("0123456789abcdefABCDEFxX") != std::string::npos) {
        return false;
    }

    char* end = nullptr;
    errno = 0;
    const bool hexadecimal = input.compare(0, 2, "0x") == 0 || input.compare(0, 2, "0X") == 0;
    const auto parsed = std::strtoul(text, &end, hexadecimal ? 16 : 10);
    if (end == text || *end != '\0' || errno == ERANGE || parsed < 0x08 || parsed > 0x77) {
        return false;
    }
    value = static_cast<std::uint8_t>(parsed);
    return true;
}

// The calibration file records the model in lower case, for example "fisheye".
const char* TypeName(sgmix::CameraCalibration::Model model) {
    using Model = sgmix::CameraCalibration::Model;
    switch (model) {
    case Model::Pinhole: return "pinhole";
    case Model::Fisheye: return "fisheye";
    case Model::CMei: return "cmei";
    case Model::OmniRadtan: return "omniradtan";
    case Model::DoubleSphere: return "doublesphere";
    case Model::DoubleSphereRadtan: return "doublesphere_radtan";
    case Model::Fisheye620: return "fisheye620";
    case Model::Unknown: return "unknown";
    }
    return "unknown";
}

void PrintError(const char* api, const std::string& error) {
    std::cerr << api << " failed";
    if (!error.empty()) {
        std::cerr << ": " << error;
    }
    std::cerr << '\n';
}

// Fixed ten decimals, matching the existing calibration text files.
std::string Number(double value) {
    if (std::isnan(value)) return "nan";
    if (std::isinf(value)) return value < 0 ? "-inf" : "inf";

    std::ostringstream text;
    text << std::fixed << std::setprecision(10) << value;
    return text.str();
}

// Keep the serial number as the file name; only characters that cannot appear in
// a file name are replaced, so a normal EEPROM serial is written unchanged.
bool SerialToFileName(const std::string& serial, std::string& name) {
    name.clear();
    for (const char character : serial) {
        const bool safe = (character >= '0' && character <= '9') ||
                          (character >= 'A' && character <= 'Z') ||
                          (character >= 'a' && character <= 'z') ||
                          character == '-' || character == '_' || character == '.';
        name.push_back(safe ? character : '_');
    }
    return !name.empty() && name != "." && name != "..";
}

// A user-supplied name must stay a plain file name; the directory comes from --out-dir.
bool ValidBaseName(const std::string& name) {
    if (name.empty() || name == "." || name == "..") return false;
    return name.find('/') == std::string::npos && name.find('\\') == std::string::npos;
}

// Explicit --name wins; otherwise the serial number names the file.
// Returns 0 on success, 2 for an invalid --name, 1 for an unusable serial number.
int ResolveFileName(const std::string& serial, const std::string& outName, bool nameGiven,
                    std::string& fileName, std::string& error) {
    if (nameGiven) {
        if (!ValidBaseName(outName)) {
            error = "Invalid output file name: '" + outName + "'";
            return 2;
        }
        fileName = outName;
        if (fileName.size() < 4 || fileName.compare(fileName.size() - 4, 4, ".txt") != 0) {
            fileName += ".txt";
        }
        return 0;
    }
    if (!SerialToFileName(serial, fileName)) {
        error = "Serial number cannot be used as a file name: '" + serial + "'";
        return 1;
    }
    fileName += ".txt";
    return 0;
}

// CAM0 is the left camera, CAM1 the right one. The key names, the "key= value"
// spacing, the blank lines and the k1/k2/k4/k3 order match the existing files.
void AppendCamera(std::ostringstream& out, const std::string& prefix,
                  const sgmix::CameraCalibration::Intrinsics& value) {
    using Model = sgmix::CameraCalibration::Model;
    const bool radtan = value.model == Model::Pinhole || value.model == Model::Fisheye620 ||
                        value.model == Model::CMei || value.model == Model::OmniRadtan ||
                        value.model == Model::DoubleSphereRadtan;
    const bool usesXi = value.model == Model::CMei || value.model == Model::OmniRadtan ||
                        value.model == Model::DoubleSphere ||
                        value.model == Model::DoubleSphereRadtan;
    const bool usesAlpha = value.model == Model::DoubleSphere ||
                           value.model == Model::DoubleSphereRadtan;

    out << prefix << "fx= " << Number(value.fx) << '\n'
        << prefix << "fy= " << Number(value.fy) << '\n'
        << prefix << "cx= " << Number(value.cx) << '\n'
        << prefix << "cy= " << Number(value.cy) << '\n';

    if (value.model == Model::Fisheye) {
        // Existing files list the fisheye slots as k1, k2, k4, k3; each label
        // still carries its own slot value.
        out << prefix << "k1= " << Number(value.fisheyeK1) << '\n'
            << prefix << "k2= " << Number(value.fisheyeK2) << '\n'
            << prefix << "k4= " << Number(value.fisheyeK4) << '\n'
            << prefix << "k3= " << Number(value.fisheyeK3) << '\n';
        return;
    }
    if (radtan) {
        out << prefix << "k1= " << Number(value.k1) << '\n'
            << prefix << "k2= " << Number(value.k2) << '\n'
            << prefix << "p1= " << Number(value.p1) << '\n'
            << prefix << "p2= " << Number(value.p2) << '\n';
    }
    if (value.model == Model::Pinhole || value.model == Model::Fisheye620) {
        out << prefix << "k3= " << Number(value.k3) << '\n'
            << prefix << "k4= " << Number(value.k4) << '\n'
            << prefix << "k5= " << Number(value.k5) << '\n'
            << prefix << "k6= " << Number(value.k6) << '\n';
    }
    if (usesXi) {
        out << prefix << "xi= " << Number(value.xi) << '\n';
    }
    if (usesAlpha) {
        out << prefix << "alpha= " << Number(value.alpha) << '\n';
    }
}

std::string BuildCalibrationText(const sgmix::CameraCalibration& calibration) {
    std::ostringstream out;
    out << "type= " << TypeName(calibration.left.model) << '\n'
        << "imageWidth= " << calibration.left.width << '\n'
        << "imageHeight= " << calibration.left.height << '\n';

    AppendCamera(out, "CAM0_", calibration.left);
    if (calibration.right) {
        out << '\n';
        AppendCamera(out, "CAM1_", *calibration.right);
    }

    if (calibration.stereo) {
        // Three blank lines before the extrinsics, as in the existing files.
        out << "\n\n\n";
        const auto& rotation = calibration.stereo->rotation;
        for (unsigned row = 0; row < 3; ++row) {
            for (unsigned column = 0; column < 3; ++column) {
                out << "R[" << row << ',' << column << "]= "
                    << Number(rotation[row * 3 + column]) << '\n';
            }
        }
        const auto& translation = calibration.stereo->translation;
        for (unsigned index = 0; index < 3; ++index) {
            out << "T[" << index << ",0]= " << Number(translation[index]) << '\n';
        }
    }

    return out.str();
}

// Write through a temporary file so an interrupted run never leaves a truncated
// calibration file behind.
bool WriteTextFile(const fs::path& target, const std::string& content, std::string& error) {
    std::error_code code;
    const auto parent = target.parent_path();
    if (!parent.empty()) {
        fs::create_directories(parent, code);
        if (code) {
            error = "Cannot create output directory " + parent.string() + ": " + code.message();
            return false;
        }
    }

    fs::path temporary = target;
    temporary += ".tmp";
    {
        std::ofstream file(temporary, std::ios::binary | std::ios::trunc);
        if (!file) {
            error = "Cannot open " + temporary.string() + " for writing";
            return false;
        }
        file.write(content.data(), static_cast<std::streamsize>(content.size()));
        file.flush();
        if (!file) {
            error = "Cannot write " + temporary.string();
            return false;
        }
    }

    fs::rename(temporary, target, code);
    if (code) {
        std::error_code ignored;
        fs::remove(temporary, ignored);
        error = "Cannot rename " + temporary.string() + " to " + target.string() + ": " +
                code.message();
        return false;
    }
    return true;
}

} // namespace

int main(int argc, char* argv[]) {
    if (argc == 2 && (std::string(argv[1]) == "--help" || std::string(argv[1]) == "-h")) {
        PrintUsage(argv[0]);
        return 0;
    }
    if (argc < 2) {
        PrintUsage(argv[0]);
        return 2;
    }

    bool sipl = false;
    sgmix::CameraConfig config;
    std::string outDir = ".";
    std::string outName;
    std::string siplJson;
    std::string siplConfig;
    std::vector<std::uint32_t> siplMasks;
    std::uint32_t cameraPort = UINT32_MAX;
    std::vector<std::string> positional;
    bool outDirGiven = false;
    bool nameGiven = false;
    bool jsonGiven = false;
    bool configGiven = false;
    bool portGiven = false;

    for (int index = 1; index < argc; ++index) {
        const std::string option = argv[index];
        if (option == "-h" || option == "--help") {
            PrintUsage(argv[0]);
            return 0;
        }
        if (option == "--sipl" && !sipl) {
            sipl = true;
            continue;
        }
        if (option == "--out-dir" && index + 1 < argc && !outDirGiven) {
            outDir = argv[++index];
            if (outDir.empty()) {
                std::cerr << "Empty output directory\n";
                return 2;
            }
            outDirGiven = true;
            continue;
        }
        if (option == "--name" && index + 1 < argc && !nameGiven) {
            outName = argv[++index];
            nameGiven = true;
            continue;
        }
        if (sipl && option == "--json" && index + 1 < argc && !jsonGiven) {
            siplJson = argv[++index];
            jsonGiven = true;
            continue;
        }
        if (sipl && option == "--config" && index + 1 < argc && !configGiven) {
            siplConfig = argv[++index];
            configGiven = true;
            continue;
        }
        if (sipl && option == "--mask" && index + 1 < argc) {
            std::uint32_t mask = 0;
            if (!ParseMask(argv[++index], mask)) {
                std::cerr << "Invalid SIPL mask: " << argv[index] << '\n';
                return 2;
            }
            siplMasks.push_back(mask);
            continue;
        }
        if (sipl && option == "--port" && index + 1 < argc && !portGiven) {
            if (!ParsePort(argv[++index], cameraPort)) {
                std::cerr << "Invalid SIPL port; use a zero-based decimal port number.\n";
                return 2;
            }
            portGiven = true;
            continue;
        }
        if (!sipl && option == "--bus" && index + 1 < argc && !config.i2cBus) {
            int bus = -1;
            if (!ParseI2cBus(argv[++index], bus)) {
                std::cerr << "Invalid I2C bus: " << argv[index]
                          << ". Expected a non-negative integer.\n";
                return 2;
            }
            config.i2cBus = bus;
            continue;
        }
        if (!sipl && option == "--eeprom-addr" && index + 1 < argc && !config.eepromI2cAddress) {
            std::uint8_t address = 0;
            if (!ParseEepromAddress(argv[++index], address)) {
                std::cerr << "Invalid EEPROM address: " << argv[index]
                          << ". Expected a 7-bit address in 0x08..0x77.\n";
                return 2;
            }
            config.eepromI2cAddress = address;
            continue;
        }
        if (!option.empty() && option[0] != '-') {
            positional.push_back(option);
            continue;
        }

        std::cerr << "Unknown or incomplete option: " << option << "\n\n";
        PrintUsage(argv[0]);
        return 2;
    }

    if (sipl) {
        if (!positional.empty()) {
            std::cerr << "SIPL mode does not accept a camera model or video device\n\n";
            PrintUsage(argv[0]);
            return 2;
        }
        if (jsonGiven != configGiven || (!siplMasks.empty() && !jsonGiven)) {
            std::cerr << "Use --json and --config together; --mask requires both.\n";
            return 2;
        }
    } else {
        if (positional.size() == 1) {
            if (positional[0].compare(0, 5, "/dev/") != 0) {
                std::cerr << "A single device argument must start with /dev/\n\n";
                PrintUsage(argv[0]);
                return 2;
            }
            config.videoDevice = positional[0];
        } else if (positional.size() == 2) {
            config.model = positional[0];
            config.videoDevice = positional[1];
        } else {
            std::cerr << "Expected a video device, or a model followed by a video device\n\n";
            PrintUsage(argv[0]);
            return 2;
        }
    }

    sgmix::CameraControl camera;
    std::string error;
    // 新接口把两种后端收敛为 CameraInitConfig 变体；UINT32_MAX 仍表示自动选择端口。
    const bool initialized = sipl
        ? camera.SG_Init(sgmix::SiplCameraConfig{siplJson, siplConfig, siplMasks, cameraPort}, error)
        : camera.SG_Init(config, error);
    if (!initialized) {
        PrintError("SG_Init", error);
        return 1;
    }

    std::string serial;
    if (!camera.SG_GetCameraSerialNumber(serial, error)) {
        PrintError("SG_GetCameraSerialNumber", error);
        return 1;
    }
    std::cout << "SN: " << serial << '\n';

    sgmix::CameraCalibration calibration{};
    if (!camera.SG_GetCalibration(calibration, error)) {
        PrintError("SG_GetCalibration", error);
        return 1;
    }

    std::string fileName;
    const int nameStatus = ResolveFileName(serial, outName, nameGiven, fileName, error);
    if (nameStatus != 0) {
        std::cerr << error << '\n';
        return nameStatus;
    }

    const fs::path target = fs::path(outDir) / fileName;
    const std::string content = BuildCalibrationText(calibration);
    if (!WriteTextFile(target, content, error)) {
        PrintError("WriteTextFile", error);
        return 1;
    }

    std::cout << "Calibration: " << (calibration.right ? "Stereo" : "Monocular")
              << (calibration.stereo ? " with extrinsics" : "") << '\n'
              << "Saved: " << target.string() << '\n';
    return 0;
}
