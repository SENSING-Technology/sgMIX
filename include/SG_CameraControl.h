#pragma once
#include <memory>
#include <optional>
#include <string>
#include <vector>
#include "SG_CameraTypes.h"

namespace sgmix {

/**
 * @class CameraControl
 * @brief Interface class providing camera initialization, trigger control,
 *        serial number retrieval, and intrinsic/extrinsic parameter management.
 */
class CameraControl {
public:
    CameraControl();
    ~CameraControl();
    CameraControl(const CameraControl&) = delete;
    CameraControl& operator=(const CameraControl&) = delete;
    CameraControl(CameraControl&&) noexcept;
    CameraControl& operator=(CameraControl&&) noexcept;

    // =========================================================================
    // Initialization
    // =========================================================================
    /**
     * @brief Initialize a camera from the supplied configuration.
     * @param config Camera model, device route, platform, and optional I2C overrides.
     * @param error Output string for error information (if any).
     * @return True on success, false otherwise.
     *
     * @note An empty model is loaded for config.videoDevice (/dev/videoN) from
     *       /opt/sgMIX/etc/sgmix.conf, which must select the v4l2 framework.
     *       An explicit model takes precedence. Other omitted values use defaults
     *       or automatic detection. Each call replaces
     *       prior connection overrides. Failure leaves the object uninitialized.
     */
    bool SG_Init(const CameraConfig& config, std::string& error);

    /**
     * Initialize an Init-only SIPL EEPROM session (no capture Start()).
     * cameraPort is the 0-based driver connector number (CAM), derived from transport/link,
     * before link masks are applied. UINT32_MAX (the default) automatically binds a single sensor.
     * With an explicit jsonPath, linkMasks select links in transport order;
     * empty means all JSON-configured links, as before.
     * With an empty jsonPath, load JSON, omitted configName and omitted linkMasks
     * from /opt/sgMIX/etc/sgmix.conf (SGMIX_FRAMEWORK must be sipl).
     * Saved masks must select at least one link. Explicit configName/masks win.
     * Multiple enabled cameras require an explicit cameraPort for each object.
     * Disabled and invalid ports are rejected before hardware Init. Module channels
     * are discovered from the expanded Query topology, independently of EEPROM layout.
     * No pipeline ID is required. Configuration-tool ports follow driver transport/link mapping.
     * Calls with identical JSON contents, config name and masks share a session.
     * Uses the project's shared EEPROM offsets, encoding and CRC protocol.
     * Supports GMSL module capture and unambiguous single-EEPROM data access with
     * one virtual channel per sensor. Supports device/image info,
     * serial number, resolution, camera and IMU calibration reads.
     * Supports GW5/ISX031 firmware OTA with a matched JP7.2 MAX96717/F
     * mono driver and existing NITO updates for mono RAW modules. Image controls
     * reuse model sequences on active channels with verified MAX96717/F routes.
     * Other device writes remain unsupported. Capture uses SG_StartCapture separately.
     * No I2C information is required. Release all bindings before changing config.
     * Calls on one CameraControl, including reinitialization, must be serialized
     * by the caller. Do not run another camera owner against the same hardware.
     */
    bool SG_Init(const std::string& jsonPath,
                 const std::string& configName, std::string& error,
                 const std::vector<std::uint32_t>& linkMasks = {},
                 std::uint32_t cameraPort = UINT32_MAX);

    // =========================================================================
    // Frame Capture
    // =========================================================================
    /**
     * @brief Start capture on the initialized camera.
     * @param config Capture source, size, frame rate, and pixel format constraints.
     * @param error Output string for error information (if any).
     * @return True when streaming is initiated, false otherwise.
     *
     * @note Uses the bound video device or an explicit Argus sensor-id without
     *       reading EEPROM image settings. SG_Init must succeed first.
     *       Asynchronous negotiation/device errors are reported by SG_GetFrame.
     *       SIPL uses its bound native pipeline. Multi-channel modules require
     *       siplChannel in Query sensor order. Use one CameraControl object per
     *       channel, initialized with the same port. EEPROM support is independent.
     *       Reads are independent, not synchronized stereo pairs. correctDistortion enables
     *       synchronous NV12 correction before SG_GetFrame returns.
     */
    bool SG_StartCapture(const CaptureConfig& config, std::string& error);

    /**
     * @brief Consume a new captured frame within the specified timeout.
     * @param frame Output owned image, plane layout, sequence, and timestamps.
     * @param timeoutMs Wait timeout in milliseconds; zero polls without waiting.
     * @param error Output string for error information (if any).
     * @return True on success, false on timeout, stop, EOS, or pipeline error.
     *
     * @note Real-time capture keeps one pending sample and drops older samples.
     *       Returned data remains valid after the next read, stop, or destruction.
     *       Failure clears frame. Only one reader is allowed; SG_StopCapture may
     *       interrupt a pending read. Serialize all other operations on this
     *       object, including initialization, moves, and destruction.
     */
    bool SG_GetFrame(Frame& frame, std::uint32_t timeoutMs, std::string& error);

    /**
     * @brief Stop capture and release its resources.
     * @param error Output string for error information (if any).
     * @return True on success, false otherwise.
     *
     * @note Repeated calls succeed. Object destruction also stops capture.
     */
    bool SG_StopCapture(std::string& error);

    /**
     * @brief Correct an existing NV12 frame without starting capture.
     * @param input Owned CPU image; input pixels and metadata remain unchanged.
     * @param output Owned, tightly packed NV12 image at the same resolution.
     * @param error Output error; failure clears output except for in-place calls.
     * @param intrinsics Optional per-sensor calibration. When supplied, no camera
     *        initialization or EEPROM access is needed. Otherwise an initialized
     *        camera's mono calibration is read once and cached until stop,
     *        reinitialization. This never writes calibration.
     * @return True on success, false for unsupported formats/models, invalid
     *         image layouts/dimensions, VPI failures, or a build without VPI.
     *
     * @note Supports Pinhole and equidistant Fisheye at calibration resolution.
     *       Output retains sequence/PTS/capture time. Input/output must differ.
     *       CUDA resources are created lazily, reused for matching parameters,
     *       and released on stop/destruction. Completion is synchronous and
     *       includes two CPU row copies. Serialize calls with other frame
     *       operations; SG_StopCapture waits for active correction to finish.
     */
    bool SG_CorrectFrame(const Frame& input, Frame& output, std::string& error,
                         const std::optional<CameraCalibration::Intrinsics>& intrinsics = std::nullopt);

    // =========================================================================
    // Device Information
    // =========================================================================
    /**
     * @brief Get the effective device route, component identities, and capabilities.
     * @param info Output structure for device information.
     * @param error Output string for error information (if any).
     * @return True on success, false otherwise.
     *
     * @note The camera must be initialized. This function reads identity, lens, and
     *       serial-number sections from EEPROM or sensor flash and validates their
     *       CRC values. Unknown or invalid optional sections remain empty; a required
     *       hardware-access failure is reported through error.
     */
    bool SG_GetDeviceInfo(DeviceInfo& info, std::string& error);

    /**
     * @brief Read the CRC-validated image configuration of the current camera.
     * @param info Output resolution, frame rate, pixel format, trigger mode,
     *        and embedded-data state.
     * @param error Output string for error information (if any).
     * @return True on success and valid CRC32_1, false otherwise.
     *
     * @note The complete Camera Info block is read from EEPROM or sensor flash
     *       according to the camera model before its fields are decoded.
     */
    bool SG_GetImageInfo(ImageInfo& info, std::string& error);

    /**
     * @brief Retrieve the CRC-validated serial number of the current camera.
     * @param serialnumber Output string for the camera serial number.
     * @param error Output string for error information (if any).
     * @return True on successful read and valid CRC32_5, false otherwise.
     *
     * @note CRC32_5 covers only the valid serial-number characters. The first
     *       0x00/0xFF terminator and all remaining padding bytes are excluded.
     */
    bool SG_GetCameraSerialNumber(std::string& serialnumber, std::string& error);

    /**
     * @brief Read the resolution stored in the Camera Info area.
     * @param width Output image width in pixels.
     * @param height Output image height in pixels.
     * @param error Output string for error information (if any).
     * @return True on success and valid CRC, false otherwise.
     */
    bool SG_GetCameraResolution(uint16_t& width, uint16_t& height, std::string& error);

    /**
     * @brief Read CRC-validated camera calibration data.
     * @param calibration Output structure for mono or stereo calibration data.
     * @param error Output string for error information (if any).
     * @return True on success, false on a read or CRC validation failure.
     *
     * @note Data is read from EEPROM or sensor flash according to the camera model.
     */
    bool SG_GetCalibration(CameraCalibration& calibration, std::string& error);

    /**
     * @brief Read CRC-validated IMU calibration data from EEPROM.
     * @param calibration Output structure for IMU calibration data.
     * @param error Output string for error information (if any).
     * @return True on success, false otherwise.
     */
    bool SG_GetImuCalibration(ImuCalibration& calibration, std::string& error);

    // =========================================================================
    // Image Processing
    // =========================================================================
    /**
     * @brief Get the image flip state.
     * @param state Output state; true when vertical flip is enabled.
     * @param error Output string for error information (if any).
     * @return True on success, false otherwise.
     */
    bool SG_GetCameraImageFlip(bool& state, std::string& error);

    /**
     * @brief Get the image mirror state.
     * @param state Output state; true when horizontal mirror is enabled.
     * @param error Output string for error information (if any).
     * @return True on success, false otherwise.
     */
    bool SG_GetCameraImageMirror(bool& state, std::string& error);

    /**
     * @brief Get the test-pattern generation state.
     * @param state Output state; true when test-pattern output is enabled.
     * @param error Output string for error information (if any).
     * @return True on success, false otherwise.
     */
    bool SG_GetCameraImageTestpattern(bool& state, std::string& error);

    /**
     * @brief Enable or disable image flip.
     * @param state True to enable vertical flip; false to disable it.
     * @param error Output string for error information (if any).
     * @return True on success, false otherwise.
     */
    bool SG_SetCameraImageFlip(bool state, std::string& error);

    /**
     * @brief Enable or disable image mirror.
     * @param state True to enable horizontal mirror; false to disable it.
     * @param error Output string for error information (if any).
     * @return True on success, false otherwise.
     */
    bool SG_SetCameraImageMirror(bool state, std::string& error);

    /**
     * @brief Enable or disable test-pattern output.
     * @param state True to enable test-pattern output; false to disable it.
     * @param error Output string for error information (if any).
     * @return True on success, false otherwise.
     */
    bool SG_SetCameraImageTestpattern(bool state, std::string& error);

    /**
     * @brief Update camera firmware or SIPL tuning data.
     * @param urlPath Local update file or existing encoded update-package URL.
     * @param error Output string for error information.
     * @note NVIDIA V4L2 S36 updates both eyes sequentially from either video node.
     *       Both nodes must be saved as S36, and all camera captures must be stopped.
     *       Both routes are checked before erase; no full-image readback comparison is performed.
     *       A failure stops further writes and can leave different firmware in each eye.
     *       Power-cycle the module after completion, then read both running versions.
     * @note SIPL S36 upgrades physical left then right, then runs the supplier
     *       address-configuration sequence without immediate right-address verification.
     * @note SIPL firmware OTA requires all shared captures to be stopped. The
     *       library verifies the matched serializer route and available EEPROM
     *       identity before protocol access. GW5 requires EEPROM metadata;
     *       ISX031 can use its driver route and bounded version query without it.
     *       Once a firmware write/reset sequence is attempted, release all
     *       shared camera objects and initialize again, including on failure.
     * @note SIPL mono RAW updates replace the existing module NITO
     *       file (up to 16 MiB), reload it and verify capture with all shared
     *       captures stopped. Success saves the previous file as .bak; failure
     *       restores and verifies it. Capture remains stopped. Objects can be
     *       reused unless rollback fails and the error requires reinitialization.
     * @return True on success, false otherwise.
     */
    bool SG_PerformOTAUpdate(std::string urlPath, std::string& error);

    /**
     * @brief Configure the right S36 sensor address as 0x1B without firmware erase/write.
     * @note SIPL matched dual-ISX031/MAX9295D only. Stop all shared captures first.
     *       The old right-eye version is not required. The supplier GPIO/address
     *       sequence runs under the session lock and restores the original aliases.
     *       No immediate sensor-address readback comparison is performed.
     *       Release and reinitialize the session after any attempted recovery.
     */
    bool SG_RestoreS36RightAddress(std::string& error);


    /**
     * @brief Recover an ISX031 camera whose firmware can no longer boot.
     * @param i2cBus Linux I2C adapter number, for example 30 for /dev/i2c-30.
     * @param serializerI2cAddr Serializer 7-bit I2C address, normally 0x40.
     * @param sensorI2cAddr ISX031 7-bit I2C address, normally 0x1A.
     * @param firmwarePath Path to a complete 512 KiB ISX031 firmware image.
     * @param error Output string for error information.
     * @return True on success, false otherwise.
     *
     * @warning Before calling, isolate the failed camera on the selected bus or
     *          switch the GMSL route so only the failed camera is reachable.
     *          Do not interrupt power while this operation is running.
     * @note This recovery path intentionally bypasses SG_Init().
     */
    bool SG_RecoverISX031Firmware(int i2cBus,
                                  uint8_t serializerI2cAddr,
                                  uint8_t sensorI2cAddr,
                                  const std::string& firmwarePath,
                                  std::string& error);

    /**
     * @brief Activate camera license using a network authentication code.
     * @param authCode Authentication code provided by vendor.
     * @param error Output string for error information.
     * @return True on success, false otherwise.
     */
    bool SG_ActivateNetworkLicense(std::string authCode, std::string& error);

    /**
     * @brief Get the current firmware version of the camera.
     * @param version Output string with firmware version.
     * @param error Output string for error information.
     * @return True on success, false otherwise.
     */
    bool SG_GetFirmwareVersion(std::string& version, std::string& error);

    // =========================================================================
    // ISP Parameter Control
    // @note These functions require a valid network license to use.
    // =========================================================================
    /**
     * @brief Get the camera brightness setting.
     * @param brightness_percent Output brightness in the range 0-100.
     * @param error Output string for error information (if any).
     * @return True on success, false otherwise.
     */
    bool SG_GetCameraBrightness(int& brightness_percent, std::string& error);
    /**
     * @brief Set the camera brightness.
     * @param brightness_percent Brightness in the range 0-100.
     * @param error Output string for error information (if any).
     * @return True on success, false otherwise.
     */
    bool SG_SetCameraBrightness(int brightness_percent, std::string& error);

    /**
     * @brief Get the camera contrast setting.
     * @param contrast_percent Output contrast in the range 0-100.
     * @param error Output string for error information (if any).
     * @return True on success, false otherwise.
     */
    bool SG_GetCameraContrast(int& contrast_percent, std::string& error);
    /**
     * @brief Set the camera contrast.
     * @param contrast_percent Contrast in the range 0-100.
     * @param error Output string for error information (if any).
     * @return True on success, false otherwise.
     */
    bool SG_SetCameraContrast(int contrast_percent, std::string& error);

    /**
     * @brief Get the camera saturation setting.
     * @param saturation_percent Output saturation in the range 0-100.
     * @param error Output string for error information (if any).
     * @return True on success, false otherwise.
     */
    bool SG_GetCameraSaturation(int& saturation_percent, std::string& error);
    /**
     * @brief Set the camera saturation.
     * @param saturation_percent Saturation in the range 0-100.
     * @param error Output string for error information (if any).
     * @return True on success, false otherwise.
     */
    bool SG_SetCameraSaturation(int saturation_percent, std::string& error);

    /**
     * @brief Get the camera sharpness setting.
     * @param sharpness_percent Output sharpness in the range 0-100.
     * @param error Output string for error information (if any).
     * @return True on success, false otherwise.
     */
    bool SG_GetCameraSharpness(int& sharpness_percent, std::string& error);
    /**
     * @brief Set the camera sharpness.
     * @param sharpness_percent Sharpness in the range 0-100.
     * @param error Output string for error information (if any).
     * @return True on success, false otherwise.
     */
    bool SG_SetCameraSharpness(int sharpness_percent, std::string& error);

    /**
     * @brief Get the camera denoise level.
     * @param denoise_percent Output denoise level in the range 0-100.
     * @param error Output string for error information (if any).
     * @return True on success, false otherwise.
     */
    bool SG_GetCameraDenoise(int& denoise_percent, std::string& error);
    /**
     * @brief Set the camera denoise level.
     * @param denoise_percent Denoise level in the range 0-100.
     * @param error Output string for error information (if any).
     * @return True on success, false otherwise.
     */
    bool SG_SetCameraDenoise(int denoise_percent, std::string& error);

    /**
     * @brief Enable or disable automatic exposure mode.
     * @param state True for automatic exposure; false for manual exposure.
     * @param error Output string for error information (if any).
     * @return True on success, false otherwise.
     */
    bool SG_SetExposureMode(bool state, std::string& error);
    /**
     * @brief Get the current exposure mode.
     * @param state Output state; true for automatic exposure, false for manual.
     * @param error Output string for error information (if any).
     * @return True on success, false otherwise.
     */
    bool SG_GetExposureMode(bool& state, std::string& error);

    /**
     * @brief Get the current exposure time.
     * @param val Output exposure time in milliseconds.
     * @param error Output string for error information (if any).
     * @return True on success, false otherwise.
     */
    bool SG_GetExposureTime(float& val, std::string& error);
    /**
     * @brief Set the manual exposure time.
     * @param val Exposure time in milliseconds.
     * @param error Output string for error information (if any).
     * @return True on success, false otherwise.
     */
    bool SG_SetManualExposure(float val, std::string& error);

    /**
     * @brief Get the current white-balance mode.
     * @param wb_mode Output state; true for automatic mode, false for manual.
     * @param error Output string for error information (if any).
     * @return True on success, false otherwise.
     */
    bool SG_GetCameraWhiteBalanceMode(bool& wb_mode, std::string& error);
    /**
     * @brief Set the white-balance mode.
     * @param wb_mode True for automatic mode; false for manual mode.
     * @param error Output string for error information (if any).
     * @return True on success, false otherwise.
     */
    bool SG_SetCameraWhiteBalanceMode(bool wb_mode, std::string& error);

    /**
     * @brief Get the manual white-balance RGB gains.
     * @param wb_R Output red-channel gain.
     * @param wb_G Output green-channel gain.
     * @param wb_B Output blue-channel gain.
     * @param error Output string for error information (if any).
     * @return True on success, false otherwise.
     */
    bool SG_GetCameraWhiteBalance(int& wb_R, int& wb_G, int& wb_B, std::string& error);
    /**
     * @brief Set the manual white-balance RGB gains.
     * @param wb_R Red-channel gain.
     * @param wb_G Green-channel gain.
     * @param wb_B Blue-channel gain.
     * @param error Output string for error information (if any).
     * @return True on success, false otherwise.
     */
    bool SG_SetCameraWhiteBalance(int wb_R, int wb_G, int wb_B, std::string& error);

    /**
     * @brief Get the sensor analog-gain value.
     * @param val Output analog-gain value.
     * @param error Output string for error information (if any).
     * @return True on success, false otherwise.
     */
    bool SG_GetGainValue(int& val, std::string& error);
    /**
     * @brief Set the manual sensor analog-gain value.
     * @param val Analog-gain value.
     * @param error Output string for error information (if any).
     * @return True on success, false otherwise.
     */
    bool SG_SetManualGain(int val, std::string& error);

private:
    // Device-storage writers are internal maintenance APIs. Keep their ABI symbols.
    bool SG_SetCameraSerialNumber(const std::string& serialnumber, std::string& error);
    bool SG_SetCameraResolution(uint16_t width, uint16_t height, std::string& error);
    bool SG_SetCalibration(const CameraCalibration& calibration, std::string& error);
    bool SG_SetImuCalibration(const ImuCalibration& calibration, std::string& error);

    class Impl;
    std::unique_ptr<Impl> impl_;  ///< Internal implementation (PImpl idiom).
};

} // namespace sgmix
