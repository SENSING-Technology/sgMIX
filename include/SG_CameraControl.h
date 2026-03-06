#pragma once
#include <string>
#include <functional> 

#include <opencv2/opencv.hpp>

#include "SG_CameraTypes.h"

namespace sgmix {
using FrameCallback = std::function<void(const cv::Mat& frame, int64_t timestamp)>;

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
     * @brief Set the Sensor I2C address of the camera.
     * @param i2caddr I2C address to set.
     * @param error Output string for error information (if any).
     * @return True on success, false otherwise.
     *
     * @note If multiple Sensor addresses are available, this function can be used to select one.
     */
    bool SG_SetSensorIICaddress(uint8_t i2caddr, std::string& error);

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

    /**
     * @brief Get EDB data from camera.
     * @param sensor_frame Output structure for EDB data.
     * @param error Output string for error information.
     * @return True on success, false otherwise.
     *
     * @note Only the S36, SG3S-ISX031C-GMSL2F-Hxxx and SG3S-ISX031C-GMSL2-Hxxx cameras are supported.
     */
    bool SG_GetCameraEBD(SensorDataFrame& sensor_frame, std::string& error);

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

    /** @brief Set the camera resolution. 
     * @param val Resolution value (0: 1920x1080, 1: 1920x1536).
     * @param error Output string for error information.
     * @return True on success, false otherwise.
     *
     * @note Only the S36, SG3S-ISX031C-GMSL2F-Hxxx and SG3S-ISX031C-GMSL2-Hxxx cameras are supported.
     */
    bool SG_SetResolution(int val, std::string& error);

    /** @brief Set the trigger mode (auto/manual). 
     * @param val Trigger mode value (0: auto, 1: manual).
     * @param error Output string for error information.
     * @return True on success, false otherwise.
     *
     * @note Only the S36, SG3S-ISX031C-GMSL2F-Hxxx and SG3S-ISX031C-GMSL2-Hxxx cameras are supported.  
     */ 
    bool SG_SetTriggerMode(int val, std::string& error);

    // =========================================================================
    // IMU Parameter Control
    // =========================================================================
    /** @brief Initialize all IMU devices.
     * @param error Output string for error information.
     * @return True on success, false otherwise.
     *
     * @note Only the S56 cameras are supported.
     */       
    bool SG_InitImuAll(std::string& error);

    /** @brief Start streaming IMU data.
     * @param error Output string for error information.
     * @return True on success, false otherwise.
     *
     * @note Only the S56 cameras are supported.
     */
    bool SG_StartImuStream(std::string& error);

    /** @brief Get the latest IMU data. 
     * @param imu_data Output structure to store IMU data.
     * @param error Output string for error information.
     * @return True on success, false otherwise.
     *
     * @note Only the S56 cameras are supported.
     */
    bool SG_GetImuData(IMUData& imu_data, std::string& error);

    // =========================================================================
    // Image Control
    // =========================================================================
    /** @brief Initialize the camera pipeline with a GStreamer description.
     * @param pipeline_desc GStreamer pipeline description string.
     * @param error Output string for error information.
     * @return True on success, false otherwise.
     */
    bool SG_Initialize(const std::string& pipeline_desc, std::string& error);

    /** @brief Shutdown the camera pipeline.
     * @param error Output string for error information.
     * @return True on success, false otherwise.
     */
    bool SG_Shutdown(std::string& error);

    /** @brief Start video capture. 
     * @param error Output string for error information.
     * @return True on success, false otherwise.
     */
    bool SG_StartCapture(std::string& error);

    /** @brief Stop video capture. 
     * @param error Output string for error information.
     * @return True on success, false otherwise.
     */
    bool SG_StopCapture(std::string& error);

    /** @brief Check if the camera pipeline is initialized. 
     * @param error Output string for error information.
     * @return True if initialized, false otherwise.
     */
    bool SG_IsInitialized(std::string& error);

    /** @brief Get the latest captured frame along with its timestamp. 
     * @param frame Output cv::Mat object to store the frame.
     * @param timestamp Output timestamp value.
     * @param error Output string for error information.
     * @return True on success, false otherwise.
     */
    bool SG_GetFrameWithTimestamp(cv::Mat& frame, int64_t& timestamp, std::string& error);

    /** @brief Get the timestamp of the latest frame captured. 
     * @param timestamp Output timestamp value.
     * @param error Output string for error information.
     * @return True on success, false otherwise.
     */
    bool SG_GetLatestFrameTimestamp(int64_t& timestamp, std::string& error);

    /** @brief Get the latest sensor data frame. 
     * @param sensor_frame Output SensorDataFrame object to store the sensor data.
     * @param error Output string for error information.
     * @return True on success, false otherwise.
     */
    bool SG_GetSensorDataFrame(SensorDataFrame& sensor_frame, std::string& error);
    
    // =========================================================================
    // Stereo Depth Control
    // =========================================================================
    /**
     * @brief Initialize the stereo depth SDK
     * 
     * Initialize parameters and perform image preprocessing
     * 
     * @param frame Left and right eye images for preprocessing during initialization
     * @param config SDK initialization configuration parameters, including camera calibration file path and depth estimation model path
     * @param error Output string for error information (if any).
     * @return true Initialization successful
     * @return false Initialization failed (e.g., file does not exist or parameter error)
     */
    bool SG_StereoInit(StereoFrame frame, StereoConfig config);

    /**
     * @brief Generate stereo depth image for a single frame
     * 
     * Perform stereo rectification and depth processing on input raw images, and output the processed results.
     * 
     * @param stereodepth_image Output stereo depth image
     * @param error Output string for error information (if any).
     * @return true Image processing successful
     * @return false Image processing failed (e.g., SDK not initialized)
     */
    bool SG_StereoProcess(cv::Mat& stereodepth_image);

    /**
     * @brief Release SDK internal resources
     * 
     * Release memory, cache, and other resources allocated by the SDK during initialization or processing.
     * 
     * @param error Output string for error information (if any).
     * @return true Successfully released resources
     * @return false Failed to release resources
     */
    bool SG_StereoDeinit();

    // =========================================================================
    // Distortion correction
    // =========================================================================
    /** 
    * @brief Correct distortion for a single image. 
    *
    * @param input_path Input image path
    * @param output_path Output image path
    * @param camera_params Camera intrinsic matrix and distortion coefficients
    * @param error Output string for error information (if any).
    * @return true Distortion correction successful
    * @return false Distortion correction failed (e.g., file does not exist or parameter error)
    */
    bool SG_DistortionCorrect(const std::string& input_path,
                              const std::string& output_path,
                              const Camera_IntrinsicMatrix& camera_params,
                              std::string& error);
    
private:
    class Impl;
    Impl* impl_;  ///< Internal implementation pointer (PImpl idiom).
};

} // namespace sgmix
