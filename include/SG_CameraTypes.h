#pragma once

namespace sgmix {
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
#pragma pack(pop)
} // namespace sgmix
