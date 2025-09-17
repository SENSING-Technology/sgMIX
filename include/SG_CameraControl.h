#pragma once
#include <string>
#include "SG_CameraTypes.h"

namespace sgmix {

/**
 * @class CameraControl
 * @brief Interface class providing camera initialization, trigger control, 
 *        serial number retrieval, and intrinsic/extrinsic parameter management.
 */
class CameraControl {
public:
    CameraControl();   ///< Constructor, initializes the internal implementation.
    ~CameraControl();  ///< Destructor, releases allocated resources.

    /**
     * @brief Initialize the specified video device.
     * @param device_video Path to the video device (e.g. "/dev/video0").
     * @param cameraType The model of the camera to be initialized.
     * @param error Output string for error information (if any).
     * @return True on success, false otherwise.
     */
    bool SG_InitVideoDevice(std::string device_video, std::string cameraType, std::string& error);

    /**
     * @brief Get the I2C address of the current camera sensor.
     * @param i2caddr Output 7-bit I2C address.
     * @param error Output string for error information (if any).
     * @return True on success, false otherwise.
     */
    bool SG_GetSensorI2CAddr(uint8_t& i2caddr, std::string& error);

    /**
     * @brief Retrieve the serial number of the current camera.
     * @param serialnumber Output string for the camera serial number.
     * @param error Output string for error information (if any).
     * @return True on success, false otherwise.
     */
    bool SG_GetCameraSerialNumber(std::string& serialnumber, std::string& error);

    /**
     * @brief Set the EEPROM I2C address of the camera.
     * @param i2caddr I2C address to set.
     * @param error Output string for error information (if any).
     * @return True on success, false otherwise.
     *
     * @note If multiple EEPROM addresses are available, this function can be used to select one.
     */
    bool SG_SetEepromIICaddress(uint8_t i2caddr, std::string& error);

    /**
     * @brief Read the intrinsic matrix of the current camera.
     * @param camintrinsic Output structure containing the camera intrinsic parameters.
     * @param error Output string for error information (if any).
     * @return True on success, false otherwise.
     */
    bool SG_GetCameraIntrinsic(Camera_IntrinsicMatrix& camintrinsic, std::string& error);

    /**
     * @brief Retrieve intrinsic parameters for left and right cameras,
     *        along with extrinsic parameters (rotation and translation) 
     *        between the stereo pair.
     * @param Letfcamintrinsic Output intrinsic matrix of the left camera.
     * @param Rightcamintrinsic Output intrinsic matrix of the right camera.
     * @param extrinsic Output extrinsic parameters (rotation and translation).
     * @param error Output string for error information (if any).
     * @return True on success, false otherwise.
     */
    bool SG_GetStereoCameraIntrinsic(Camera_IntrinsicMatrix& Letfcamintrinsic, 
                                     Camera_IntrinsicMatrix& Rightcamintrinsic,
                                     Camera_ExtrinsicMatrix& extrinsic, std::string& error);

    /** @brief Get the image flip state. */
    bool SG_GetCameraImageFlip(bool& state, std::string& error);

    /** @brief Get the image mirror state. */
    bool SG_GetCameraImageMirror(bool& state, std::string& error);

    /** @brief Get the test pattern generation state. */
    bool SG_GetCameraImageTestpattern(bool& state, std::string& error);

    /** @brief Enable/disable image flip. */
    bool SG_SetCameraImageFlip(bool state, std::string& error);

    /** @brief Enable/disable image mirror. */
    bool SG_SetCameraImageMirror(bool state, std::string& error);

    /** @brief Enable/disable test pattern output. */
    bool SG_SetCameraImageTestpattern(bool state, std::string& error);

    /**
     * @brief Perform OTA (Over-the-Air) firmware update.
     * @param urlPath URL to the update package.
     * @param error Output string for error information.
     * @return True on success, false otherwise.
     */
    bool SG_PerformOTAUpdate(std::string urlPath, std::string& error);

    /**
     * @brief Get camera hardware and software information.
     * @param info Output structure with detailed camera information.
     * @param error Output string for error information.
     * @return True on success, false otherwise.
     */
    bool SG_GetCameraInfo(CameraInfo& info, std::string& error); 

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
    /** @brief Get the camera brightness setting (0-100%). */
    bool SG_GetCameraBrightness(int& brightness_percent, std::string& error);
    /** @brief Set the camera brightness (0-100%). */
    bool SG_SetCameraBrightness(int brightness_percent, std::string& error);

    /** @brief Get the camera contrast setting (0-100%). */
    bool SG_GetCameraContrast(int& contrast_percent, std::string& error);
    /** @brief Set the camera contrast (0-100%). */
    bool SG_SetCameraContrast(int contrast_percent, std::string& error);

    /** @brief Get the camera saturation setting (0-100%). */
    bool SG_GetCameraSaturation(int& saturation_percent, std::string& error);
    /** @brief Set the camera saturation (0-100%). */
    bool SG_SetCameraSaturation(int saturation_percent, std::string& error);

    /** @brief Get the camera sharpness setting (0-100%). */
    bool SG_GetCameraSharpness(int& sharpness_percent, std::string& error);
    /** @brief Set the camera sharpness (0-100%). */
    bool SG_SetCameraSharpness(int sharpness_percent, std::string& error);

    /** @brief Get the camera denoise level (0-100%). */
    bool SG_GetCameraDenoise(int& denoise_percent, std::string& error);
    /** @brief Set the camera denoise level (0-100%). */
    bool SG_SetCameraDenoise(int denoise_percent, std::string& error);

    /** @brief Enable or disable automatic exposure mode. */
    bool SG_SetExposureMode(bool state, std::string& error);
    /** @brief Get the current exposure mode (auto/manual). */
    bool SG_GetExposureMode(bool& state, std::string& error);

    /** @brief Get the current exposure time in milliseconds. */
    bool SG_GetExposureTime(float& val, std::string& error);
    /** @brief Set manual exposure time (ms). */
    bool SG_SetManualExposure(float val, std::string& error);

    /** @brief Get the white balance mode (auto/manual). */
    bool SG_GetCameraWhiteBalanceMode(bool& wb_mode, std::string& error);
    /** @brief Set the white balance mode (auto/manual). */
    bool SG_SetCameraWhiteBalanceMode(bool wb_mode, std::string& error);

    /** @brief Get the manual white balance RGB gains. */
    bool SG_GetCameraWhiteBalance(int& wb_R, int& wb_G, int& wb_B, std::string& error);
    /** @brief Set the manual white balance RGB gains. */
    bool SG_SetCameraWhiteBalance(int wb_R, int wb_G, int wb_B, std::string& error);

    /** @brief Get the analog gain value of the sensor. */
    bool SG_GetGainValue(int& val, std::string& error);
    /** @brief Set manual analog gain value. */
    bool SG_SetManualGain(int val, std::string& error);

private:
    class Impl;
    Impl* impl_;  ///< Internal implementation pointer (PImpl idiom).
};

} // namespace sgmix
