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
     * @brief Initialize the V4L2/I2C or SIPL camera backend.
     * @param config Backend selection and initialization parameters.
     * @param error Output diagnostic string.
     * @return True on success. A backend initialization failure leaves the object uninitialized.
     *
     * @note V4L2 loads an omitted model from the saved /dev/videoN mapping.
     *       Explicit settings take precedence and replace prior connection overrides.
     * @note SIPL cameraPort is the zero-based Driver CAM connector before link masking.
     *       Omit it only when a single sensor is enabled. With an explicit JSON path,
     *       empty masks select all JSON links. An empty path uses the saved SIPL JSON
     *       and supplies any omitted configuration name and masks.
     * @note Identical SIPL configurations share a session. Release all bindings before
     *       switching configurations. Serialize calls on each CameraControl object.
     *       Call SG_StartCapture separately to start streaming.
     */
    bool SG_Init(const CameraInitConfig& config, std::string& error);

    // =========================================================================
    // Frame Capture
    // =========================================================================
    /**
     * @brief Start capture on the initialized camera.
     * @param config Size, frame rate, pixel format, and optional channel settings.
     * @param error Output error information.
     * @return True when streaming is initiated. SG_GetFrame reports asynchronous errors.
     *
     * @note Call after successful SG_Init. RAW models use Argus, YUV models use V4L2,
     *       and SIPL uses its bound pipeline. Argus defaults to sensor ID N from /dev/videoN.
     *       For SIPL multi-channel modules, select siplChannel in Query sensor order and
     *       initialize one object per channel on the same port. EEPROM access is independent.
     *       Reads return individual frames without stereo synchronization.
     *       correctDistortion applies synchronous NV12 correction in SG_GetFrame.
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
     * @brief Synchronously correct an NV12 frame without starting capture.
     * @param input Owned CPU frame. Pixels and metadata are preserved.
     * @param output Owned, tightly packed NV12 frame at the input resolution, with the same sequence and timestamps.
     * @param error Output error information.
     * @param intrinsics Optional per-sensor calibration. Supplying it bypasses camera initialization
     *        and EEPROM access. Otherwise, mono calibration is read once from the initialized
     *        camera and cached until stop or reinitialization.
     * @return True on success. Failure clears output. Passing the same Frame as input and
     *         output returns false and preserves that Frame.
     *
     * @note Requires VPI and supports pinhole and equidistant fisheye models at calibration
     *       resolution. Serialize with other frame operations. SG_StopCapture waits for
     *       active correction to finish.
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
     * @brief Read the current camera's image configuration.
     * @param info Output resolution, frame rate, pixel format, trigger mode,
     *        and embedded-data state.
     * @param error Output string for error information (if any).
     * @return True on success and valid CRC, false otherwise.
     *
     * @note The complete Camera Info block is read from EEPROM or sensor flash
     *       according to the camera model before its fields are decoded.
     */
    bool SG_GetImageInfo(ImageInfo& info, std::string& error);

    /**
     * @brief Read the current camera's serial number.
     * @param serialnumber Output string for the camera serial number.
     * @param error Output string for error information (if any).
     * @return True on successful read and valid CRC, false otherwise.
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
     * @brief Read camera calibration data.
     * @param calibration Output structure for mono or stereo calibration data.
     * @param error Output string for error information (if any).
     * @return True on success, false on a read or CRC validation failure.
     *
     * @note Data is read from EEPROM or sensor flash according to the camera model.
     */
    bool SG_GetCalibration(CameraCalibration& calibration, std::string& error);

    /**
     * @brief Read IMU calibration data from EEPROM.
     * @param calibration Output structure for IMU calibration data.
     * @param error Output string for error information (if any).
     * @return True on success, false on a read or CRC validation failure.
     */
    bool SG_GetImuCalibration(ImuCalibration& calibration, std::string& error);

    /**
     * @brief Read the current sensor trigger mode on a supported mono ISX031 camera.
     * @note Call after SG_Init(). For an idle SIPL session, the SDK may briefly
     *       start and stop the HAL to prepare the sensor control link.
     * @param mode Output mode, set to Unknown on failure.
     * @param error Output error information, cleared on success.
     * @return True if the sensor registers were read and validated.
     */
    bool SG_GetCameraTriggerMode(SensorTriggerMode& mode, std::string& error);

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
     * @param error Output error information.
     * @return True on success, false otherwise.
     *
     * @note GW6 ROM: updates the default components. V4L2 and the dedicated SIPL stereo
     *       route validate the ROM file without configuration, sensor, or version preflight.
     * @note S36/S36A firmware: updates the two eyes sequentially. For NVIDIA V4L2, either node
     *       can initiate the update, both nodes must use the same product model, and all captures
     *       must be stopped. Failure can leave different firmware on the two eyes. After
     *       a successful V4L2 update, power-cycle the module and check both versions.
     * @note SIPL firmware: stop all shared captures. GW5 requires EEPROM metadata.
     *       After any attempted firmware write or reset, release all shared camera
     *       objects and reinitialize, including on failure.
     * @note SIPL NITO: supports mono RAW and matched S56/S56C RAW stereo modules with
     *       an existing NITO file. Stop all shared captures. The update accepts up to
     *       16 MiB, validates capture, and leaves capture stopped. Success saves the
     *       old file as .bak. Failure restores and verifies it. Reinitialize if
     *       rollback fails and the error requests it.
     */
    bool SG_PerformOTAUpdate(std::string urlPath, std::string& error);

    /** Pass true to update the full ROM on a supported GW6 camera. */
    bool SG_PerformOTAUpdate(std::string urlPath, std::string& error, bool fullRom);

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
     * @brief Get the current firmware version of the camera.
     * @param version Output string with firmware version.
     * @param error Output string for error information.
     * @return True on success, false otherwise.
     */
    bool SG_GetFirmwareVersion(std::string& version, std::string& error);

    // =========================================================================
    // ISP Parameter Control
    // @note These functions require a valid offline license with isp.adjust to use.
    // =========================================================================
    /**
     * @brief Get the camera brightness.
     * @param brightness_percent Output brightness in the range 0-100.
     * @param error Output error information.
     * @return True on success, false otherwise.
     */
    bool SG_GetCameraBrightness(int& brightness_percent, std::string& error);
    /**
     * @brief Set the camera brightness.
     * @param brightness_percent Brightness in the range 0-100.
     * @param error Output error information.
     * @return True on success, false otherwise.
     */
    bool SG_SetCameraBrightness(int brightness_percent, std::string& error);

    /**
     * @brief Get the camera contrast.
     * @param contrast_percent Output contrast in the range 0-100.
     * @param error Output error information.
     * @return True on success, false otherwise.
     */
    bool SG_GetCameraContrast(int& contrast_percent, std::string& error);
    /**
     * @brief Set the camera contrast.
     * @param contrast_percent Contrast in the range 0-100.
     * @param error Output error information.
     * @return True on success, false otherwise.
     */
    bool SG_SetCameraContrast(int contrast_percent, std::string& error);

    /**
     * @brief Get the camera saturation.
     * @param saturation_percent Output saturation in the range 0-100.
     * @param error Output error information.
     * @return True on success, false otherwise.
     */
    bool SG_GetCameraSaturation(int& saturation_percent, std::string& error);
    /**
     * @brief Set the camera saturation.
     * @param saturation_percent Saturation in the range 0-100.
     * @param error Output error information.
     * @return True on success, false otherwise.
     */
    bool SG_SetCameraSaturation(int saturation_percent, std::string& error);

    /**
     * @brief Get the camera sharpness.
     * @param sharpness_percent Output sharpness in the range 0-100.
     * @param error Output error information.
     * @return True on success, false otherwise.
     */
    bool SG_GetCameraSharpness(int& sharpness_percent, std::string& error);
    /**
     * @brief Set the camera sharpness.
     * @param sharpness_percent Sharpness in the range 0-100.
     * @param error Output error information.
     * @return True on success, false otherwise.
     */
    bool SG_SetCameraSharpness(int sharpness_percent, std::string& error);

    /**
     * @brief Get the camera denoise level.
     * @param denoise_percent Output denoise level in the range 0-100.
     * @param error Output error information.
     * @return True on success, false otherwise.
     */
    bool SG_GetCameraDenoise(int& denoise_percent, std::string& error);
    /**
     * @brief Set the camera denoise level.
     * @param denoise_percent Denoise level in the range 0-100.
     * @param error Output error information.
     * @return True on success, false otherwise.
     */
    bool SG_SetCameraDenoise(int denoise_percent, std::string& error);

    /**
     * @brief Enable or disable automatic exposure mode.
     * @param state True for automatic exposure; false for manual exposure.
     * @param error Output error information.
     * @return True on success, false otherwise.
     */
    bool SG_SetExposureMode(bool state, std::string& error);
    /**
     * @brief Get the current exposure mode.
     * @param state Output state; true for automatic exposure, false for manual.
     * @param error Output error information.
     * @return True on success, false otherwise.
     */
    bool SG_GetExposureMode(bool& state, std::string& error);

    /**
     * @brief Get the current exposure time.
     * @param val Output exposure time in milliseconds.
     * @param error Output error information.
     * @return True on success, false otherwise.
     */
    bool SG_GetExposureTime(float& val, std::string& error);
    /**
     * @brief Set the manual exposure time.
     * @param val Exposure time in milliseconds.
     * @param error Output error information.
     * @return True on success, false otherwise.
     */
    bool SG_SetManualExposure(float val, std::string& error);

    /**
     * @brief Get the current white-balance mode.
     * @param wb_mode Output state; true for automatic mode, false for manual.
     * @param error Output error information.
     * @return True on success, false otherwise.
     */
    bool SG_GetCameraWhiteBalanceMode(bool& wb_mode, std::string& error);
    /**
     * @brief Set the white-balance mode.
     * @param wb_mode True for automatic mode; false for manual mode.
     * @param error Output error information.
     * @return True on success, false otherwise.
     */
    bool SG_SetCameraWhiteBalanceMode(bool wb_mode, std::string& error);

    /**
     * @brief Get the manual white-balance RGB gains.
     * @param wb_R Output red-channel gain.
     * @param wb_G Output green-channel gain.
     * @param wb_B Output blue-channel gain.
     * @param error Output error information.
     * @return True on success, false otherwise.
     */
    bool SG_GetCameraWhiteBalance(int& wb_R, int& wb_G, int& wb_B, std::string& error);
    /**
     * @brief Set the manual white-balance RGB gains.
     * @param wb_R Red-channel gain.
     * @param wb_G Green-channel gain.
     * @param wb_B Blue-channel gain.
     * @param error Output error information.
     * @return True on success, false otherwise.
     */
    bool SG_SetCameraWhiteBalance(int wb_R, int wb_G, int wb_B, std::string& error);

    /**
     * @brief Get the sensor analog-gain value.
     * @param val Output analog-gain value.
     * @param error Output error information.
     * @return True on success, false otherwise.
     */
    bool SG_GetGainValue(int& val, std::string& error);
    /**
     * @brief Set the manual sensor analog-gain value.
     * @param val Analog-gain value.
     * @param error Output error information.
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
