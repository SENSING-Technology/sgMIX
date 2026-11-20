#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <variant>
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
 *   SG_Init                        input         CameraInitConfig
 *   SG_StartCapture                input         CaptureConfig
 *   SG_GetFrame                    output        Frame
 *   SG_GetCameraTriggerMode        output        SensorTriggerMode
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
    /// Requested trigger configuration for NVIDIA V4L2 capture.
    enum class TriggerMode : std::uint8_t {
        Default,      ///< Leave the driver's current configuration in place.
        FreeRun,      ///< Select camera master mode.
        ExternalSync, ///< Select camera slave mode. The board must receive FSYNC pulses.
    };

    /**
     * Camera model name. If empty, SG_Init loads the model mapped to videoDevice
     * in /opt/sgMIX/etc/sgmix.conf (V4L2 only).
     *
     * Supported camera names:
     *   - "SG3S-ISX031C-GMSL2F-Hxxx"
     *   - "SG3S-ISX031C-GMSL2-Hxxx"
     *   - "SYF2G"
     *   - "SYW2G"
     *   - "SHW3G"
     *   - "S56"
     *   - "S56C"
     *   - "S56D"
     *   - "S56DY"
     *   - "SHW5G"
     *   - "SHF5G"
     *   - "S36"
     *   - "S36A"
     *   - "SDT2BCO1"
     *   - "SG3S11AFLK"
     *   - "SYF3L"
     *   - "SHF3L"
     *   - "M3A"
     *   - "SHF3H"
     *
     * Maintenance/compatibility names (not physical camera model names):
     *   - "GW5_DEVICE"
     *   - "GW6_DEVICE"
     *   - "EEPROM_CAM"
     */
    std::string model;
    std::string videoDevice;
    Platform platform = Platform::Auto;

    // Optional I2C overrides. Leave unset to use the detected route.
    // Addresses are host-visible 7-bit values.
    std::optional<int> i2cBus;
    std::optional<std::uint8_t> sensorI2cAddress;
    std::optional<std::uint8_t> eepromI2cAddress;

    // Applied before streaming by SG_StartCapture. SIPL selects FSYNC in its JSON.
    TriggerMode triggerMode = TriggerMode::Default;
};

/** @brief SIPL camera selection for CameraControl::SG_Init. */
struct SiplCameraConfig {
    // An empty path uses the saved SIPL configuration for omitted settings.
    std::string jsonPath;
    std::string configName;
    // Masks in transport order. Empty with an explicit JSON path uses all configured links.
    std::vector<std::uint32_t> linkMasks;
    // Zero-based CAM connector. Leave unset to bind the only enabled sensor.
    std::optional<std::uint32_t> cameraPort;
};

using CameraInitConfig = std::variant<CameraConfig, SiplCameraConfig>;

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
    std::vector<double> exposureTimesSeconds{};
    // Driver-reported exposure count, independent of exposure-time validity.
    // The exposure-time vector is bounded by the SIPL array capacity.
    std::uint32_t numExposures = 0;
};

/** @brief V4L2/Argus metadata copied from the same sample as the image. */
struct V4l2FrameMetadata {
    // Pipeline PTS in nanoseconds, when available.
    std::optional<std::uint64_t> ptsNanoseconds;

    std::vector<std::uint8_t> raw;
    std::uint32_t topLines = 0;
    // Bytes per copied row; source buffer padding is excluded.
    std::size_t rowBytes = 0;
    // Legacy ISX031 layout only: little-endian exposure lines at offsets 30..32.
    std::optional<std::uint32_t> exposureLines;
    // ISX031 exposure time in seconds. Empty when timing is unavailable.
    std::vector<double> exposureTimesSeconds;
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
    // Host system clock when SG_GetFrame receives the sample (Unix microseconds).
    std::int64_t hostTimestampMicroseconds = 0;
    // Present for successfully delivered SIPL frames only.
    std::optional<SiplFrameMetadata> siplMetadata;
    // Present for successfully delivered V4L2/Argus frames. EBD fields may be empty.
    std::optional<V4l2FrameMetadata> v4l2Metadata;
};

/** @brief Capture constraints; omitted dimensions normally use source negotiation. */
struct CaptureConfig {
    std::uint32_t width = 0;
    std::uint32_t height = 0;
    std::uint32_t framesPerSecond = 0;
    // V4L2 input format is negotiated automatically as UYVY/YUYV/NV12.
    // Empty preserves the native format (NV12 for Argus); BGR conversion is optional.
    std::optional<Frame::PixelFormat> outputFormat;

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

/** @brief Primary sensor synchronization mode returned by SG_GetCameraTriggerMode(). */
enum class SensorTriggerMode : std::uint8_t {
    /// The current mode could not be determined.
    Unknown,

    /// The sensor uses its internal timing to produce frames.
    InternalSync,

    /// External pulses synchronize the sensor's image readout timing.
    ExternalPulseSync,

    /// An external trigger controls when the sensor starts exposure.
    ShutterTrigger,
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
        std::optional<std::uint8_t> id; // Component ID from module storage.
        std::optional<I2cEndpoint> endpoint;
        Location location = Location::Module;
    };

    enum class LensType : std::uint8_t {
        Unknown = 0x00,
        Aa = 0x41,
        Threaded = 0xFF,
    };

    struct Lens {
        std::uint8_t fieldOfViewDegrees = 0;       // 0x0020
        float focalLengthMillimeters = 0.0F;       // 0x0021-0x0024
        float fNumber = 0.0F;                      // 0x0025-0x0028
        char manufacturerId = '\0';                // 0x0029
        std::uint8_t index = 0;                    // 0x002A
        LensType type = LensType::Unknown;         // 0x002B
    };

    std::string model;
    std::string videoDevice;
    Platform platform = Platform::Auto;

    Component sensor; // 0x0000
    std::optional<Component> eeprom;
    std::optional<Component> isp; // 0x0001
    std::optional<Component> serializer;
    std::optional<Component> deserializer;
    std::optional<Lens> lens;

    // Unused serial number bytes are 0xFF.
    std::optional<std::string> serialNumber; // 0x0120-0x0140

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

    /// Trigger flag stored in the module image configuration.
    enum class TriggerMode : std::uint8_t {
        Unknown = 0x00,
        Enabled = 0x01,
        Disabled = 0x02,
    };

    // Width and height are stored in little-endian order.
    std::uint16_t width = 0;                        // 0x0002-0x0003
    std::uint16_t height = 0;                       // 0x0004-0x0005
    // The stored frame rate occupies one byte.
    std::uint16_t framesPerSecond = 0;              // 0x0006
    PixelFormat pixelFormat = PixelFormat::Unknown; // 0x0007
    TriggerMode triggerMode = TriggerMode::Unknown; // 0x0008
    bool embeddedDataEnabled = false;               // 0x0009
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
        std::uint16_t width = 0;                   // 0x0060-0x0061
        std::uint16_t height = 0;                  // 0x0062-0x0063
        Model model = Model::Unknown;              // 0x0064

        double fx = 0.0; // Left 0x0065; right 0x0160
        double fy = 0.0; // Left 0x006D; right 0x0168
        double cx = 0.0; // Left 0x0075; right 0x0170
        double cy = 0.0; // Left 0x007D; right 0x0178

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
    };

    /** @brief Stereo transform from one camera coordinate system to the other. */
    struct Extrinsics {
        std::array<double, 9> rotation{};    // 0x0220-0x0267
        std::array<double, 3> translation{}; // 0x0268-0x027F
    };

    std::optional<float> reprojectionErrorPixels; // 0x0040-0x0043
    Intrinsics left;
    std::optional<Intrinsics> right;
    std::optional<Extrinsics> stereo;
};

// =============================================================================
// IMU calibration -- output from SG_GetImuCalibration
// =============================================================================

struct ImuCalibration {
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
    // Camera-to-IMU translation is in metres. The time shift is in seconds.
    double T_cam_imu_T0;        // 0x0368
    double T_cam_imu_T1;        // 0x0370
    double T_cam_imu_T2;        // 0x0378
    double Timeshift_cam_imu;   // 0x0380

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
