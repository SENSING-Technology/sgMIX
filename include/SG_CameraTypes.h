#pragma once
#include <string>
#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>

#ifdef __cplusplus
namespace sgmix {
#endif

enum {
    PLARFORM_NVIDIA = 1,
    PLARFORM_ROCKCHIP = 2,
};

enum CameraFormat {
    FORMAT_RAW,
    FORMAT_YUV422_YUYV,
    FORMAT_YUV422_UYVY,
};

enum CameraTriggerMode {
    TRIGGER_MODE_UNKNOWN,
    TRIGGER_MODE_SLAVE,
    TRIGGER_MODE_MASTER,
};

enum CameraEBDMode {
    EBD_MODE_UNKNOWN,
    EBD_MODE_ON,
    EBD_MODE_OFF,
};

struct CameraInfo {
    uint16_t imageWidth;
    uint16_t imageHeight;
    uint8_t fps;
    uint8_t imageFormat;
    uint8_t triggerMode;
    uint8_t embeddedData;
};

#pragma pack(push, 1) 
/**
 * @brief Camera intrinsic matrix structure. Includes both pinhole and fisheye model parameters.
 */
struct Camera_IntrinsicMatrix {
    uint16_t imageWidth;
    uint16_t imageHeight;
    uint8_t calibra_mode;   ///< Calibration model type: 1 - pinhole, 2 - fisheye

    double fx;          ///< Focal length in x direction
    double fy;          ///< Focal length in y direction
    double cx;          ///< Principal point coordinate in x
    double cy;          ///< Principal point coordinate in y

    // Pinhole model distortion coefficients
    double pinhole_k1;
    double pinhole_k2;
    double pinhole_p1;
    double pinhole_p2;
    double pinhole_k3;
    double pinhole_k4;
    double pinhole_k5;
    double pinhole_k6;

    // Fisheye model distortion coefficients
    double fisheye_k1;
    double fisheye_k2;
    double fisheye_k3;
    double fisheye_k4;
};

/**
 * @brief Stereo camera extrinsic matrix structure. Includes rotation matrix and translation vector.
 */
struct Camera_ExtrinsicMatrix {
    // Rotation matrix R (3x3)
    double R0_0, R0_1, R0_2;
    double R1_0, R1_1, R1_2;
    double R2_0, R2_1, R2_2;

    // Translation vector T
    double Tx;
    double Ty;
    double Tz;
};

/**
 * @brief IMU data structure containing accelerometer and gyroscope readings.
 */
struct IMUData {
    // int64_t timestamp;
    float accel_x;
    float accel_y;
    float accel_z;
    float anglvel_x;
    float anglvel_y;
    float anglvel_z;
};

/**
 * @brief Sensor data frame structure containing image and IMU data
 */
struct SensorDataFrame {
    /** Timestamp (microsecond precision) */
    int64_t timestamp;
    
    /** Image frame data */
    cv::Mat image;
    
    struct IMUData imu_data;

    double exposure_time=0.0;
};

/**
 * @brief Stereo image frame structure.
 */
struct StereoFrame {
        cv::Mat left_rect;
        cv::Mat right_rect;
};

/**
* @brief Stereo camera initialization configuration structure.
*/
struct StereoConfig {
    /** JSON format camera parameter file path (intrinsics, distortion coefficients, etc.) */
    std::string json_file;


    /** Encryption file path for SDK authorization or encrypted data processing */
    std::string enc_file;


    /** Key file path for decryption or SDK authorization verification */
    std::string key_file;
};

struct DistortionCorrectionResult {
    /** Corrected image data */
    cv::Mat corrected_image;
    
    /** Processing time in milliseconds */
    double processing_time_ms;
    
    /** Error message if correction failed */
    std::string error_message;
    
    /** Success flag */
    bool success;
};

#pragma pack(pop)

#ifdef __cplusplus
} // namespace sgmix
#endif
