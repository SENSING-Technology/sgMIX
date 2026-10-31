#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace sgmix {

/**
 * @file SG_CameraTypes.h
 * @brief Public data types used by CameraControl.
 *
 * Quick guide -- types users normally instantiate:
 *
 *   CameraControl API              Direction     Type
 *   ------------------------------------------------------------------
 *   SG_Init                        input         CameraConfig
 *   SG_StartCapture                input         CaptureConfig
 *   SG_GetFrame                    output        Frame
 *   SG_GetDeviceInfo               output        DeviceInfo
 *   SG_GetImageInfo                output        ImageInfo
 *   SG_GetCalibration              output        CameraCalibration
 *   SG_GetImuCalibration           output        ImuCalibration
 *
 * Platform is shared by initialization and device information. Other helper
 * types are nested in the result that owns them, so the namespace exposes only
 * the types users normally pass to CameraControl.
 */

// =============================================================================
// Initialization -- input to CameraControl::SG_Init
// =============================================================================

enum class Platform : std::uint8_t {
    Auto = 0,
    Nvidia,
    Rockchip,
    Hobot,
    Intel,
};

/** @brief Camera selection and optional low-level route overrides. */
struct CameraConfig {
    /**
     * Camera model name. If empty, SG_Init loads the model mapped to videoDevice
     * in /opt/sgMIX/etc/sgmix.conf (V4L2 only).
     *
     * Supported camera names:
     *   - "SG3S-ISX031C-GMSL2F-Hxxx"
     *   - "SG3S-ISX031C-GMSL2-Hxxx"
     *   - "SHW2GY" (AR0820 / GW5; the previous AR0820 model name is accepted)
     *   - "SHW3G"
     *   - "S56"
     *   - "SHW5G" (shares the S56 control backend)
     *   - "S36"
     *   - "SDT2BCO1" (legacy spelling "SDT2BC01" is accepted)
     *
     * Stored-data reads and capture only (no ISP control or firmware update):
     *   - "SG3S11AFLK", "SHF3L", "SHF3H": ISX031 with integrated ISP / Flash data
     *   - "SG8-ISX028C-G2G-Hxxx": ISX028 with integrated ISP / EEPROM data
     *     (the previous name "ISX028" is accepted)
     *
     * SIPL requires a matching driver and JSON configuration for every model.
     *
     * Maintenance/compatibility names (not physical camera model names):
     *   - "GW5_DEVICE"
     *   - "GW6_DEVICE"
     *   - "EEPROM_CAM"
     */
    std::string model;
    std::string videoDevice;
    Platform platform = Platform::Auto;

    // Each value is an independent override. An omitted bus is discovered
    // from the video device; omitted addresses use discovery/defaults.
    std::optional<int> i2cBus;
    std::optional<std::uint8_t> sensorI2cAddress;
    std::optional<std::uint8_t> eepromI2cAddress;
};

// =============================================================================
// Capture -- input/output for CameraControl frame operations
// =============================================================================

/** @brief SIPL metadata copied from the same image buffer. */
struct SiplFrameMetadata {
    // Raw capture SOF/EOF TSC values; not Unix time or pipeline PTS.
    std::uint64_t captureStartTsc = 0;
    std::uint64_t captureEndTsc = 0;
    std::optional<std::uint64_t> sensorFrameSequence;
    // Sensor-native units and clock domain; present only when marked valid.
    std::optional<std::uint64_t> sensorTimestamp;
    // Owned raw embedded data for this frame; empty when the driver provides none.
    std::vector<std::uint8_t> embeddedDataTop{};
    std::vector<std::uint8_t> embeddedDataBottom{};
    // Driver-parsed exposure times; empty when the block is invalid.
    std::vector<float> exposureTimesSeconds{};
    // Driver-reported exposure count, independent of exposure-time validity.
    // The exposure-time vector is bounded by the SIPL array capacity.
    std::uint32_t numExposures = 0;
};

/** @brief Owned top-row EBD copied from the same V4L2 sample as the image. */
struct EmbeddedFrameMetadata {
    std::vector<std::uint8_t> raw;
    std::uint32_t topLines = 0;
    // Bytes per copied row; source buffer padding is excluded.
    std::size_t rowBytes = 0;
    // Legacy ISX031 layout only: little-endian exposure lines at offsets 30..32.
    std::optional<std::uint32_t> exposureLines;
    // ISX031 legacy 210 MHz timing; present when a valid HMAX is available.
    std::optional<double> exposureTimeSeconds;
};

/** @brief Owned CPU image with tightly packed rows and explicit plane offsets. */
struct Frame {
    enum class PixelFormat : std::uint8_t {
        Uyvy,
        Yuyv,
        Nv12,
        Bgr,
    };

    struct Plane {
        std::size_t offset = 0;
        std::size_t stride = 0;
        std::uint32_t rows = 0;
    };

    std::vector<std::uint8_t> data;
    std::uint32_t width = 0;
    std::uint32_t height = 0;
    PixelFormat pixelFormat = PixelFormat::Uyvy;
    std::array<Plane, 2> planes{};
    std::uint32_t planeCount = 0;

    // Delivered-frame counter, reset on each successful capture start.
    std::uint64_t sequence = 0;
    // Original pipeline PTS, when present. Not a Unix or exposure timestamp.
    std::optional<std::uint64_t> ptsNanoseconds;
    // Host system clock when SG_GetFrame receives the sample (Unix microseconds).
    std::int64_t hostTimestampMicroseconds = 0;
    // Present for successfully delivered SIPL frames only.
    std::optional<SiplFrameMetadata> siplMetadata;
    // Present only when V4L2 top-row EBD extraction was explicitly enabled.
    std::optional<EmbeddedFrameMetadata> embeddedMetadata;
};

/** @brief Capture constraints; omitted dimensions normally use source negotiation. */
struct CaptureConfig {
    enum class Source : std::uint8_t {
        V4l2,
        Argus,
    };

    Source source = Source::V4l2;
    std::uint32_t width = 0;
    std::uint32_t height = 0;
    std::uint32_t framesPerSecond = 0;
    // V4L2 input format is negotiated automatically as UYVY/YUYV/NV12.
    // Empty preserves the native format (NV12 for Argus); BGR conversion is optional.
    std::optional<Frame::PixelFormat> outputFormat;
    // Argus requires an explicit sensor-id; never inferred from /dev/videoN.
    std::optional<std::uint32_t> argusSensorId;

    // Read mono calibration once and return corrected NV12 from SG_GetFrame.
    // Requires a VPI-enabled SDK. First frame includes lazy CUDA initialization;
    // calibration resolution must match the source. Argus uses the calibration
    // dimensions when both width and height are omitted; explicit sizes are kept.
    // V4L2/Argus automatically provide NV12 to this correction path.
    bool correctDistortion = false;

    // SIPL only: select a sensor by its index in the bound module's Query order.
    // Multi-channel modules require an explicit selection; mono accepts omitted
    // or 0. Physical left/right follows the module configuration. No frame pairing implied.
    std::optional<std::uint32_t> siplChannel{};

    // V4L2 UYVY/YUYV only: input height includes these leading EBD rows;
    // returned Frame height excludes them. Zero keeps existing behavior.
    // Leave outputFormat empty; convert the image after EBD extraction.
    std::uint32_t embeddedTopLines = 0;
};

// =============================================================================
// Device identity -- output from CameraControl::SG_GetDeviceInfo
// =============================================================================

/** @brief Detected device identity, component routing and capabilities. */
struct DeviceInfo {
    struct I2cEndpoint {
        int bus = -1;
        std::uint8_t address = 0; // Host-visible 7-bit address.
    };

    enum class Location : std::uint8_t {
        Sensor,
        Module,
        Host,
    };

    struct Component {
        std::string chip; // Empty when the chip model is unknown.
        std::optional<std::uint8_t> id; // Raw EEPROM component ID.
        std::optional<I2cEndpoint> endpoint;
        Location location = Location::Module;
    };

    enum class LensType : std::uint8_t {
        Unknown = 0x00,
        Aa = 0x41,
        Threaded = 0xFF,
    };

    struct Lens {
        std::uint8_t fieldOfViewDegrees = 0;       // EEPROM 0x0020
        float focalLengthMillimeters = 0.0F;       // EEPROM 0x0021-0x0024
        float fNumber = 0.0F;                      // EEPROM 0x0025-0x0028
        char manufacturerId = '\0';                // EEPROM 0x0029
        std::uint8_t index = 0;                    // EEPROM 0x002A
        LensType type = LensType::Unknown;         // EEPROM 0x002B
        // 0x002C-0x003B reserved; CRC32_2 is stored at 0x003C-0x003F.
    };

    std::string model;
    std::string videoDevice;
    Platform platform = Platform::Auto;

    Component sensor; // `chip` is decoded from Sensor ID at EEPROM 0x0000.
    std::optional<Component> eeprom;
    std::optional<Component> isp; // `chip` is decoded from ISP ID at EEPROM 0x0001.
    std::optional<Component> serializer;
    std::optional<Component> deserializer;
    std::optional<Lens> lens;

    // EEPROM 0x0120-0x0140; unused bytes are 0xFF. CRC32_5 at 0x0141-0x0144
    // covers only valid SN characters; terminator and padding are excluded.
    std::optional<std::string> serialNumber;

    bool ispControlSupported = false;
    bool firmwareUpdateSupported = false;
};

// =============================================================================
// Image configuration -- output from CameraControl::SG_GetImageInfo
// =============================================================================

/** @brief Resolution, frame rate and image transport settings. */
struct ImageInfo {
    enum class PixelFormat : std::uint8_t {
        Unknown = 0x00,
        Yuyv = 0x01,
        Uyvy = 0x02,
    };

    enum class TriggerMode : std::uint8_t {
        Unknown = 0x00,
        Enabled = 0x01,
        Disabled = 0x02,
    };

    std::uint16_t width = 0;                       // EEPROM 0x0002-0x0003, little-endian
    std::uint16_t height = 0;                      // EEPROM 0x0004-0x0005, little-endian
    std::uint16_t framesPerSecond = 0;             // EEPROM 0x0006 (stored as uint8_t)
    PixelFormat pixelFormat = PixelFormat::Unknown;// EEPROM 0x0007
    TriggerMode triggerMode = TriggerMode::Unknown;// EEPROM 0x0008
    bool embeddedDataEnabled = false;              // EEPROM 0x0009
    // Sensor ID: 0x0000; ISP ID: 0x0001; CRC32_1: 0x001C-0x001F.
};

// =============================================================================
// Camera calibration -- output from SG_GetCalibration
// =============================================================================

/** @brief Complete mono or stereo camera calibration result. */
struct CameraCalibration {
    enum class Model : std::uint8_t {
        Unknown = 0x00,
        Pinhole = 0x01,
        Fisheye = 0x02,
        CMei = 0x03,
        OmniRadtan = 0x04,
        DoubleSphere = 0x05,
        DoubleSphereRadtan = 0x06,
        Fisheye620 = 0x07,
    };

    /** @brief Intrinsic parameters for one camera sensor. */
    struct Intrinsics {
        // Width/height/model exist once in the shared block at 0x0060-0x0064.
        std::uint16_t width = 0;                   // EEPROM 0x0060-0x0061
        std::uint16_t height = 0;                  // EEPROM 0x0062-0x0063
        Model model = Model::Unknown;              // EEPROM 0x0064

        double fx = 0.0; // Left 0x0065; right 0x0160
        double fy = 0.0; // Left 0x006D; right 0x0168
        double cx = 0.0; // Left 0x0075; right 0x0170
        double cy = 0.0; // Left 0x007D; right 0x0178

        // Shared distortion slots. Their meaning depends on `model`.
        double k1 = 0.0; // Left 0x0085; right 0x0180
        double k2 = 0.0; // Left 0x008D; right 0x0188
        double p1 = 0.0; // Left 0x0095; right 0x0190
        double p2 = 0.0; // Left 0x009D; right 0x0198
        double k3 = 0.0; // Left 0x00A5; right 0x01A0
        double k4 = 0.0; // Left 0x00AD; right 0x01A8
        double k5 = 0.0; // Left 0x00B5; right 0x01B0
        double k6 = 0.0; // Left 0x00BD; right 0x01B8

        double fisheyeK1 = 0.0; // Left 0x00C5; right 0x01C0
        double fisheyeK2 = 0.0; // Left 0x00CD; right 0x01C8
        double fisheyeK3 = 0.0; // Left 0x00D5; right 0x01D0
        double fisheyeK4 = 0.0; // Left 0x00DD; right 0x01D8

        // Used by C'Mei/Omni/Double-Sphere model families as applicable.
        double xi = 0.0;    // Left 0x00E5; right 0x01E0
        double alpha = 0.0; // Left 0x00ED; right 0x01E8
        // Left reserve: 0x00F5-0x011B, CRC32_4: 0x011C-0x011F.
        // Right reserve: 0x01F0-0x0216, CRC32_6: 0x0217-0x021A.
    };

    /** @brief Stereo transform from one camera coordinate system to the other. */
    struct Extrinsics {
        std::array<double, 9> rotation{};    // EEPROM 0x0220-0x0267, row-major
        std::array<double, 3> translation{}; // EEPROM 0x0268-0x027F, millimetres
        // CRC32_7: 0x0280-0x0283.
    };

    std::optional<float> reprojectionErrorPixels; // EEPROM 0x0040-0x0043; CRC32_3: 0x005C-0x005F
    Intrinsics left;
    std::optional<Intrinsics> right;
    std::optional<Extrinsics> stereo;
};

// =============================================================================
// IMU calibration -- output from SG_GetImuCalibration
//
// EEPROM stores each value as a little-endian IEEE-754 double. Addresses below
// describe the storage protocol; CRC values themselves are verified internally.
// =============================================================================

struct ImuCalibration {
    // CRC32_8 covers EEPROM 0x02A0-0x0387; checksum at 0x0388-0x038B.
    double gyr_n_x;             // 0x02A0
    double gyr_w_x;             // 0x02A8
    double gyr_n_y;             // 0x02B0
    double gyr_w_y;             // 0x02B8
    double gyr_n_z;             // 0x02C0
    double gyr_w_z;             // 0x02C8
    double gyr_noise_density;   // 0x02D0
    double gyr_random_walk;     // 0x02D8

    double acc_n_x;             // 0x02E0
    double acc_w_x;             // 0x02E8
    double acc_n_y;             // 0x02F0
    double acc_w_y;             // 0x02F8
    double acc_n_z;             // 0x0300
    double acc_w_z;             // 0x0308
    double acc_noise_density;   // 0x0310
    double acc_random_walk;     // 0x0318

    double T_cam_imu_R0_0;      // 0x0320; matrix continues every 8 bytes
    double T_cam_imu_R0_1;
    double T_cam_imu_R0_2;
    double T_cam_imu_R1_0;
    double T_cam_imu_R1_1;
    double T_cam_imu_R1_2;
    double T_cam_imu_R2_0;
    double T_cam_imu_R2_1;
    double T_cam_imu_R2_2;      // 0x0360
    double T_cam_imu_T0;        // 0x0368, metres
    double T_cam_imu_T1;        // 0x0370, metres
    double T_cam_imu_T2;        // 0x0378, metres
    double Timeshift_cam_imu;   // 0x0380, seconds

    // CRC32_9 covers EEPROM 0x0390-0x050F; checksum at 0x0510-0x0513.
    double gyro_bias_x;         // 0x0390
    double gyro_bias_y;
    double gyro_bias_z;         // 0x03A0

    double gyro_K0_0;           // 0x03A8; matrix continues every 8 bytes
    double gyro_K0_1;
    double gyro_K0_2;
    double gyro_K1_0;
    double gyro_K1_1;
    double gyro_K1_2;
    double gyro_K2_0;
    double gyro_K2_1;
    double gyro_K2_2;           // 0x03E8

    double gyro_scale_x;        // 0x03F0
    double gyro_scale_y;
    double gyro_scale_z;        // 0x0400

    double gyro_M0_0;           // 0x0408; matrix continues every 8 bytes
    double gyro_M0_1;
    double gyro_M0_2;
    double gyro_M1_0;
    double gyro_M1_1;
    double gyro_M1_2;
    double gyro_M2_0;
    double gyro_M2_1;
    double gyro_M2_2;           // 0x0448

    double acc_bias_x;          // 0x0450
    double acc_bias_y;
    double acc_bias_z;          // 0x0460

    double acc_K0_0;            // 0x0468; matrix continues every 8 bytes
    double acc_K0_1;
    double acc_K0_2;
    double acc_K1_0;
    double acc_K1_1;
    double acc_K1_2;
    double acc_K2_0;
    double acc_K2_1;
    double acc_K2_2;            // 0x04A8

    double acc_scale_x;         // 0x04B0
    double acc_scale_y;
    double acc_scale_z;         // 0x04C0

    double acc_M0_0;            // 0x04C8; matrix continues every 8 bytes
    double acc_M0_1;
    double acc_M0_2;
    double acc_M1_0;
    double acc_M1_1;
    double acc_M1_2;
    double acc_M2_0;
    double acc_M2_1;
    double acc_M2_2;            // 0x0508
};

} // namespace sgmix
