# 1. Purpose
This guide describes the SENSING Camera Control and ISP SDK for developers. The SDK provides basic camera module controls, including image flip, mirror, test pattern output, camera information and intrinsic calibration reads, and over-the-air (OTA) firmware updates. It also provides interfaces for image capture, processing, and optimization.

This guide is intended for software developers and system integrators. It introduces the SDK architecture, installation and configuration, camera integration, and API usage through examples to help developers build applications based on SENSING cameras.

# 2. Scope
**SDK V2.0 primarily targets NVIDIA Jetson platforms and the following environment:**

**Operating system**: JetPack 7.2.0

**Hardware platform**: Jetson AGX Thor DK

**Driver frameworks**: V4L2 and SIPL

**Camera models**: This guide covers SG3S-ISX031C-GMSL2F-Hxxx, SG3S-ISX031C-GMSL2-Hxxx, SG3S11AFLK, SHF3L, M3A, SHF3H, SYF3L, SHW3G, SHW5G, SHF5G, S36, S36A, S56, S56C, S56D, SYF2G, SYW2G, SDT2BCO1, and S56DY. See the V4L2 and SIPL tables below for feature support by model.

# 3. SDK Features
**The SDK provides the following core functions:**

1. Camera information reads
    1. Basic camera information: resolution, frame rate, and pixel format.
    2. Lens information: field of view (FOV), focal length, aperture, and lens model.
    3. Camera calibration: left and right intrinsics, distortion coefficients, stereo extrinsics (R/T), and reprojection error.
    4. IMU calibration: gyroscope and accelerometer parameters, bias, scale, misalignment, and camera-to-IMU extrinsics.
    5. Module identification: serial number (SN).
2. Basic camera controls
    1. Image flip
    2. Image mirror
    3. Test pattern output
3. Remote updates and management
    1. OTA firmware updates
4. Video capture control
5. Distortion correction
6. Frame metadata parsing

## V4L2
|  | **<font style="color:rgb(0, 0, 0);">Read Module Information</font>** | **<font style="color:rgb(0, 0, 0);">Read Lens Information</font>** | **<font style="color:rgb(0, 0, 0);">Read Module SN</font>** | **<font style="color:rgb(0, 0, 0);">Read Calibration</font>** | **<font style="color:rgb(0, 0, 0);">Flip/Mirror Control</font>** | **<font style="color:rgb(0, 0, 0);">Test Pattern</font>** | **<font style="color:rgb(0, 0, 0);">OTA</font>** | **<font style="color:rgb(0, 0, 0);">Video Capture</font>** | **<font style="color:rgb(0, 0, 0);">Frame Metadata Parsing</font>** | **<font style="color:rgb(0, 0, 0);">Distortion Correction</font>** |
| --- | :---: | :---: | :---: | :---: | :---: | :---: | :---: | :---: | :---: | :---: |
| **<font style="color:rgb(0, 0, 0);">SG3S-ISX031C-GMSL2F-Hxxx</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** |
| **<font style="color:rgb(0, 0, 0);">SG3S-ISX031C-GMSL2-Hxxx</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** |
| **<font style="color:rgb(0, 0, 0);">SG3S11AFLK</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** |
| **<font style="color:rgb(0, 0, 0);">SHW3G</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | | **<font style="color:rgb(0, 0, 0);">●</font>** |
| **<font style="color:rgb(0, 0, 0);">SHW5G</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | | **<font style="color:rgb(0, 0, 0);">●</font>** |
| **<font style="color:rgb(0, 0, 0);">SHF5G</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | | **<font style="color:rgb(0, 0, 0);">●</font>** |
| **<font style="color:rgb(0, 0, 0);">S36</font>** | **<font style="color:#1f2329;">●</font>** | **<font style="color:#1f2329;">●</font>** | **<font style="color:#1f2329;">●</font>** | **<font style="color:#1f2329;">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** |
| **<font style="color:rgb(0, 0, 0);">S36A</font>** | **<font style="color:#1f2329;">●</font>** | **<font style="color:#1f2329;">●</font>** | **<font style="color:#1f2329;">●</font>** | **<font style="color:#1f2329;">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** |
| **<font style="color:rgb(0, 0, 0);">S56</font>** | **<font style="color:#1f2329;">●</font>** | **<font style="color:#1f2329;">●</font>** | **<font style="color:#1f2329;">●</font>** | **<font style="color:#1f2329;">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | | **<font style="color:rgb(0, 0, 0);">●</font>** |
| **<font style="color:rgb(0, 0, 0);">S56C</font>** | **<font style="color:#1f2329;">●</font>** | **<font style="color:#1f2329;">●</font>** | **<font style="color:#1f2329;">●</font>** | **<font style="color:#1f2329;">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | | **<font style="color:rgb(0, 0, 0);">●</font>** |
| **<font style="color:rgb(0, 0, 0);">S56D</font>** | **<font style="color:#1f2329;">●</font>** | **<font style="color:#1f2329;">●</font>** | **<font style="color:#1f2329;">●</font>** | **<font style="color:#1f2329;">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | | **<font style="color:rgb(0, 0, 0);">●</font>** |
| **<font style="color:rgb(0, 0, 0);">SYF2G</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | | | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | | **<font style="color:rgb(0, 0, 0);">●</font>** |
| **<font style="color:rgb(0, 0, 0);">SYW2G</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | | | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | | **<font style="color:rgb(0, 0, 0);">●</font>** |
| **<font style="color:rgb(0, 0, 0);">SDT2BCO1</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | | | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | | |
| **<font style="color:rgb(0, 0, 0);">S56DY</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | | | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | | |
| **<font style="color:rgb(0, 0, 0);">SHF3L</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** |
| **<font style="color:rgb(0, 0, 0);">M3A</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** |
| **<font style="color:rgb(0, 0, 0);">SHF3H</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** |
| **<font style="color:rgb(0, 0, 0);">SYF3L</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** |

## SIPL
|  | **<font style="color:rgb(0, 0, 0);">Read Module Information</font>** | **<font style="color:rgb(0, 0, 0);">Read Lens Information</font>** | **<font style="color:rgb(0, 0, 0);">Read Module SN</font>** | **<font style="color:rgb(0, 0, 0);">Read Calibration</font>** | **<font style="color:rgb(0, 0, 0);">Flip/Mirror Control</font>** | **<font style="color:rgb(0, 0, 0);">Test Pattern</font>** | **<font style="color:rgb(0, 0, 0);">OTA</font>** | **<font style="color:rgb(0, 0, 0);">Video Capture</font>** | **<font style="color:rgb(0, 0, 0);">Frame Metadata Parsing</font>** | **<font style="color:rgb(0, 0, 0);">Distortion Correction</font>** |
| --- | :---: | :---: | :---: | :---: | :---: | :---: | :---: | :---: | :---: | :---: |
| **<font style="color:rgb(0, 0, 0);">SG3S-ISX031C-GMSL2F-Hxxx</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** |
| **<font style="color:rgb(0, 0, 0);">SG3S-ISX031C-GMSL2-Hxxx</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** |
| **<font style="color:rgb(0, 0, 0);">SG3S11AFLK</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** |
| **<font style="color:rgb(0, 0, 0);">SHW3G</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** |
| **<font style="color:rgb(0, 0, 0);">SHW5G</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** |
| **<font style="color:rgb(0, 0, 0);">SHF5G</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** |
| **<font style="color:rgb(0, 0, 0);">S36</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:#1f2329;">●</font>** | **<font style="color:#1f2329;">●</font>** | **<font style="color:#1f2329;">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** |
| **<font style="color:rgb(0, 0, 0);">S36A</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:#1f2329;">●</font>** | **<font style="color:#1f2329;">●</font>** | **<font style="color:#1f2329;">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** |
| **<font style="color:rgb(0, 0, 0);">S56</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:#1f2329;">●</font>** | **<font style="color:#1f2329;">●</font>** | **<font style="color:#1f2329;">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** |
| **<font style="color:rgb(0, 0, 0);">S56C</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:#1f2329;">●</font>** | **<font style="color:#1f2329;">●</font>** | **<font style="color:#1f2329;">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** |
| **<font style="color:rgb(0, 0, 0);">S56D</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:#1f2329;">●</font>** | **<font style="color:#1f2329;">●</font>** | **<font style="color:#1f2329;">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** |
| **<font style="color:rgb(0, 0, 0);">SYF2G</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | | | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | | **<font style="color:rgb(0, 0, 0);">●</font>** |
| **<font style="color:rgb(0, 0, 0);">SYW2G</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | | | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | | **<font style="color:rgb(0, 0, 0);">●</font>** |
| **<font style="color:rgb(0, 0, 0);">SDT2BCO1</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | | | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | | |
| **<font style="color:rgb(0, 0, 0);">S56DY</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | | | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | | |
| **<font style="color:rgb(0, 0, 0);">SHF3L</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** |
| **<font style="color:rgb(0, 0, 0);">M3A</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** |
| **<font style="color:rgb(0, 0, 0);">SHF3H</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** |
| **<font style="color:rgb(0, 0, 0);">SYF3L</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** | **<font style="color:rgb(0, 0, 0);">●</font>** |

SIPL controls and updates require the access routes described later in this guide. For RAW cameras, OTA refers to updates of matching NITO files. Available frame metadata depends on driver output. Distortion correction also requires valid intrinsics and VPI.

SIPL OTA for SHF2GY and SHW2GY uses a matched 1920×1200 GW5 route. SIPL OTA supports GW6 cameras with a matching stereo driver configuration.

Trigger mode reads support the registered mono ISX031 models. SIPL requires a matched single-sensor route. Use `camera_data_tool get-trigger-mode` to read the current sensor trigger mode.

# 4. Quick Start and Installation
## Obtaining Official SENSING Drivers
JetPack 7.2 V4L2 driver: https://github.com/SENSING-Technology/nvidia-jetson-camera-drivers/tree/main/Jetson%20AGX%20Thor%20Devkit/TRD1_G2A/JetPack7.2/TRD1_G2A_GMSL2x8_JP7.2_L4TR39.2

JetPack 7.2 SIPL driver: https://github.com/SENSING-Technology/nvidia-jetson-camera-drivers/tree/main/Jetson%20AGX%20Thor%20Devkit/TRD1_G2A/JetPack7.2/TRD1_G2A_AGX_THOR_SIPL_JP7.2_L4TR39.2

## Obtaining the SDK
1. Option 1: Obtain a Debian installation package from your FAE:

```plain
SENSING_sgMIX_vX.X.X.deb
```

2. Option 2: Obtain the SDK from the Git repository

```plain
git clone https://github.com/SENSING-Technology/sgMIX.git
cd sgMIX
sudo apt install git-lfs 
git lfs install
git lfs pull
```

## Installing the Debian Package
1. Copy the SDK package to the Jetson device, for example:

```plain
/home/nvidia/SENSING_sgMIX_vX.X.X.deb
```

2. Install the package; dependencies are handled automatically:

```plain
sudo apt update
sudo apt install ./SENSING_sgMIX_vX.X.X.deb
```

APT installs Python 3, CMake, build-essential (GCC/G++ and Make), pkg-config, GStreamer development packages, and required runtime dependencies. The SDK has no direct dependency on the NVIDIA JetPack metapackage.

Before installation, run `apt-get -s install ./SENSING_sgMIX_vX.X.X.deb` and check whether any remaining dependency would update the BSP.

After installation, the SDK files are located in /opt/sgMIX:

```plain
/opt/sgMIX/
├── include/                  # Public API headers
├── lib/                      # Static and shared sgMIX libraries and gateway tool libraries
├── sample/                   # Example sources, including basic/ examples
├── bin/                      # Configuration, licensing, and maintenance tools
├── extensions/               # Extensions supplied for the target platform
│   └── sDepth_SDK/           # Depth application, tools, libraries, and config in the Thor package
├── CMakeLists.txt            # Builds the examples against the installed SDK libraries
├── Readme.md                 # English documentation
└── Readme_cn.md              # Chinese documentation
```

The configuration script creates `/opt/sgMIX/etc/sgmix.conf` when run.

3. Check the package version:

```plain
dpkg-deb -f SENSING_sgMIX_vX.X.X.deb Version
```

4. Check the installed version:

```plain
dpkg-query -W -f='${Version}\n' sgmix-sdk
```

5. Uninstall sgMIX:

```plain
sudo apt purge sgmix-sdk
sudo rm -rf /opt/sgMIX
dpkg -l | grep sgmix-sdk
```

## Generating the sgMIX Configuration
Generate the configuration manually after installation. Installation and upgrades preserve existing settings. Install the SDK and camera driver that match the target BSP. For V4L2, load the driver and check for the actual /dev/videoN nodes. SIPL requires a configuration JSON and matching libnvsipl.so and libnvsipl_query.so libraries.

The configuration tool saves a model for each V4L2 video node, or the JSON, configuration name, and enabled link masks for SIPL. It does not start the camera, scan I²C, or probe the hardware to determine the camera model.

Configure the platform, framework, and camera models, then save the settings to `/opt/sgMIX/etc/sgmix.conf`.

```plain
# Requires Python 3.7 or later
sudo /opt/sgMIX/bin/sgmix-config.py
```

### Selecting the Framework
![](https://cdn.nlark.com/yuque/0/2026/png/57985410/1793448214018-79136ac8-c0da-46ec-aedd-2c32550d3a9e.png)

The script automatically detects the hardware platform. The SIPL option is available only on NVIDIA platforms. Enter the menu number to select the driver framework.

#### V4L2: Configuration by Video Node
This example uses an SG3S-ISX031C-GMSL2F-Hxxx connected to port 7.

![](https://cdn.nlark.com/yuque/0/2026/png/57985410/1793448214099-851d30ee-f4df-4f38-8335-241e1c7a3af5.png)

Run the configuration script and select V4L2.

![](https://cdn.nlark.com/yuque/0/2026/png/57985410/1793448213996-eff5b36d-f46b-4f19-bb63-13004f67996c.png)

Enter 1 and press Enter. The script detects the video nodes in the current environment so that you can assign a model to the node used by the camera.

![](https://cdn.nlark.com/yuque/0/2026/png/57985410/1793448214038-81e1f32b-10d2-43c7-bbf5-e0fd99d2c9d8.png)

The camera in this example uses video7. Enter the corresponding menu number and press Enter to select its model.

![](https://cdn.nlark.com/yuque/0/2026/png/57985410/1793448214080-8ecd3eb6-f6d6-4e4d-82cf-6fc99e9863b4.png)

Verify that video7 is assigned SG3S-ISX031C-GMSL2F-Hxxx, then enter s to save.

![](https://cdn.nlark.com/yuque/0/2026/png/57985410/1793448214980-ed07992b-eb7c-40dc-a5c4-569c4b097fa7.png)

Confirm the configuration and enter y to save.

#### SIPL: Configuration by Port
This example uses an SHW3G connected to port 7.

![](https://cdn.nlark.com/yuque/0/2026/png/57985410/1793448215001-f07419e8-430a-4476-adc4-c9c978704f10.png)

Run the configuration script and select SIPL.

![](https://cdn.nlark.com/yuque/0/2026/png/57985410/1793448215047-8444f3d2-0622-4c56-8ec9-8646f312742f.png)

Enter 2 and press Enter, then enter the SIPL JSON file to use. This example uses the SHW3G configuration.

![](https://cdn.nlark.com/yuque/0/2026/png/57985410/1793448215043-5dfb4dbb-9518-48d2-8ba4-a9b580d3caad.png)

The script parses the ports from the configuration. Select the appropriate camera model for each port.

![](https://cdn.nlark.com/yuque/0/2026/png/57985410/1793448215027-f611cc66-ca10-42c5-9e82-dc73be382d5e.png)

Enter 7 to select port 7, press Enter, and then enter 5 to select SHW3G.

![](https://cdn.nlark.com/yuque/0/2026/png/57985410/1793448215588-4b75eef5-739d-4481-abaa-b826c53a1a80.png)

Verify that port 7 is assigned SHW3G, then enter s and press Enter to save.

![](https://cdn.nlark.com/yuque/0/2026/png/57985410/1793448215741-203edd1a-387e-4c24-965a-4c92704f58cb.png)

For a configuration with two transports, CAM7 corresponds to `0:3` (transport 0, link 3). Check the actual JSON for other configurations:

```plain
sudo /opt/sgMIX/bin/sgmix-config.py --platform nvidia --framework sipl \
  --json /path/to/shw3g.json --config SHW3G_8CH \
  --model none --sipl-model 0:3=SHW3G
```

# 5. Building and Using the Examples
After SDK installation, /opt/sgMIX/sample contains source examples for the current APIs.

Build using the CMake template in the installation directory:

```plain
cd /opt/sgMIX/
mkdir -p build && cd build
cmake ..
make -j"$(nproc)"
```

If the current user cannot write to `/opt/sgMIX`, build in a user directory instead:

```plain
cmake -S /opt/sgMIX -B "$HOME/sgmix-samples-build"
cmake --build "$HOME/sgmix-samples-build" -j6
cd "$HOME/sgmix-samples-build"
```

Run every `./program_name` shown below from the build directory. Use `sudo` when device or target-directory permissions are insufficient.

The default build includes several complete tools and the `basic_*` examples.

| **<font style="color:#1f2329;">Source</font>** | **<font style="color:#1f2329;">Executable</font>** | **<font style="color:#1f2329;">Purpose</font>** |
| --- | --- | --- |
| <font style="color:#1f2329;">camera_data_tool.cpp</font> | <font style="color:#1f2329;">camera_data_tool</font> | <font style="color:#1f2329;">Read camera information, serial numbers, and calibration data</font> |
| <font style="color:#1f2329;">camera_frame_capture.cpp</font> | <font style="color:#1f2329;">camera_frame_capture</font> | <font style="color:#1f2329;">V4L2, Argus, and SIPL capture, preview, and distortion correction</font> |
| <font style="color:#1f2329;">camera_ota.cpp</font> | <font style="color:#1f2329;">camera_ota</font> | <font style="color:#1f2329;">Update camera firmware and supported ISP/NITO files</font> |
| <font style="color:#1f2329;">camera_image_controls.cpp</font> | <font style="color:#1f2329;">camera_image_controls</font> | <font style="color:#1f2329;">Verify flip, mirror, and test pattern APIs with live preview and attempt to restore initial values</font> |
| <font style="color:#1f2329;">camera_calibration_export.cpp</font> | <font style="color:#1f2329;">camera_calibration_export</font> | <font style="color:#1f2329;">Export camera calibration data for sDepth</font> |
| <font style="color:#1f2329;">basic/*.cpp</font> | <font style="color:#1f2329;">Eight basic_* programs</font> | <font style="color:#1f2329;">Minimal examples of individual SDK functions; see the end of the examples section</font> |

`SGMIX_BUILD_FULL_SAMPLES` and `SGMIX_BUILD_BASIC_SAMPLES` both default to `ON`. Disable the other option to build only one group, or build a specific program:

```plain
# Run from a build directory that has already been configured with CMake
cmake --build . --target camera_ota -j6
```

## camera_data_tool: Camera Information and Calibration Reads
`camera_data_tool` reads device information, serial numbers, stored resolution, image configuration, camera calibration, and IMU calibration. It supports V4L2 and SIPL.

```plain
./camera_data_tool --help
```

### 1. Using Saved Configuration
V4L2:

```plain
./camera_data_tool --v4l2 --video /dev/video6 all
./camera_data_tool --v4l2 --video /dev/video6 get-sn
./camera_data_tool --v4l2 --video /dev/video6 get-calibration
```

SIPL:

```plain
./camera_data_tool --sipl --cam 7 all
./camera_data_tool --sipl --cam 7 get-sn
./camera_data_tool --sipl --cam 7 get-calibration
```

`--cam` selects an enabled Driver CAM, numbered from 0. It is not a module-local channel number. If omitted, the configuration must select exactly one sensor. Specify `--cam` only once per command.

### 2. Specifying Configuration Manually
V4L2:

```plain
./camera_data_tool --v4l2 --video /dev/video7 --model SHW3G all
```

SIPL:

```plain
./camera_data_tool --sipl --cam 7 \
  --json /path/to/camera.json \
  --config SHW3G_8CH \
  --mask "0x1000 0x0000" all
```

| **<font style="color:#1f2329;">Option</font>** | **<font style="color:#1f2329;">Description</font>** |
| --- | --- |
| <font style="color:#1f2329;">--v4l2 / --sipl</font> | <font style="color:#1f2329;">Select the mode; place this option first</font> |
| <font style="color:#1f2329;">--video PATH</font> | <font style="color:#1f2329;">V4L2 video node; required with --v4l2</font> |
| <font style="color:#1f2329;">--model MODEL</font> | <font style="color:#1f2329;">Optional V4L2 model override; if omitted, use the saved configuration for the video node</font> |
| <font style="color:#1f2329;">--bus N</font> | <font style="color:#1f2329;">Override the V4L2 I2C bus; this tool has no sensor-address option, so regular camera models should discover the bus from the video node</font> |
| <font style="color:#1f2329;">--eeprom-addr ADDR</font> | <font style="color:#1f2329;">Accepted only with an explicit --model EEPROM_CAM; override the host-accessible 7-bit EEPROM address. This model also requires --bus</font> |
| <font style="color:#1f2329;">--json PATH, --config NAME</font> | <font style="color:#1f2329;">Manually specify the SIPL JSON path and a configuration name within it</font> |
| <font style="color:#1f2329;">--mask "MASK ..."</font> | <font style="color:#1f2329;">SIPL link masks separated by spaces in driver transport order; decimal and 0x hexadecimal values are supported. Specify --mask once</font> |
| <font style="color:#1f2329;">--cam N</font> | <font style="color:#1f2329;">An enabled SIPL Driver CAM</font> |
| <font style="color:#1f2329;">--verbose</font> | <font style="color:#1f2329;">Use only with all; include SDK member names and full numeric precision</font> |

Specify `--json` and `--config` together. `--mask` is used only with manually specified SIPL configuration. The selected port must correspond to a camera enabled in that configuration.

Place multiple masks inside one pair of double quotes, in the same order as `nvsipl_camera -m` for the matching driver package. Specify `--mask` only once.

### 3. Supported Commands
| **<font style="color:#1f2329;">Command</font>** | **<font style="color:#1f2329;">Description</font>** |
| --- | --- |
| <font style="color:#1f2329;">all</font> | <font style="color:#1f2329;">Read all information and summarize the results</font> |
| <font style="color:#1f2329;">device-info</font> | <font style="color:#1f2329;">Read device information and capabilities</font> |
| <font style="color:#1f2329;">get-sn</font> | <font style="color:#1f2329;">Read the camera serial number</font> |
| <font style="color:#1f2329;">get-resolution</font> | <font style="color:#1f2329;">Read the stored resolution</font> |
| <font style="color:#1f2329;">get-image</font> | <font style="color:#1f2329;">Read the stored image configuration</font> |
| <font style="color:#1f2329;">get-calibration</font> | <font style="color:#1f2329;">Read camera calibration, including available stereo calibration data</font> |
| <font style="color:#1f2329;">get-imu</font> | <font style="color:#1f2329;">Read IMU calibration, not live IMU measurements</font> |

`all` continues with the remaining reads even if some items fail. Unprogrammed IMU calibration is reported as `NOT PROGRAMMED` and is not counted as a failure. Other read failures cause a nonzero exit code.

The items available for reading depend on the camera model. Stored image configuration does not describe the current live capture parameters. This tool provides read commands only.

The configuration or driver supplies the model and sensor information. Check the results for SN, resolution, and calibration separately. If a value is reported as `Unavailable` or calibration is missing, check the camera configuration and whether the required data was programmed.

For the saved S36 CAM4 configuration above, module data reads do not require a capture channel:

```plain
sudo ./camera_data_tool --sipl --cam 4 device-info
sudo ./camera_data_tool --sipl --cam 4 all
```

Exit codes: `0` for success, `1` for a read or initialization failure, and `2` for a usage or argument error.

## camera_frame_capture: Camera Capture and Live Preview
camera_frame_capture supports V4L2, NVIDIA Argus, and SIPL capture, with live preview and distortion correction.

Select the capture backend first, then add preview, frame-count, or correction options as needed.

Show help:

```plain
./camera_frame_capture --help
```

### 1. Selecting the Capture Backend
#### V4L2 (YUV Cameras)
Capture through a video device node:

```plain
./camera_frame_capture --v4l2 --video /dev/video6
```

By default, the tool uses the model saved for the video node by the configuration program. You can also specify the model manually:

```plain
./camera_frame_capture --v4l2 --video /dev/video6 --model SG3S11AFLK
```

To request capture dimensions, provide both width and height and use dimensions supported by the driver mode:

```plain
./camera_frame_capture --v4l2 --video /dev/video6 --width 1920 --height 1080
```

#### NVIDIA Argus (RAW Cameras)
RAW models use Argus automatically. The SDK derives the Argus ID from the numeric suffix of the video node.

```plain
./camera_frame_capture --v4l2 --video /dev/video0
```

The video node is used for SDK initialization, while the Argus ID selects the capture source. /dev/videoN maps to Argus ID N. Both must identify the same camera in the actual BSP configuration.

You can also specify the Argus output dimensions:

```plain
./camera_frame_capture --v4l2 --video /dev/video0 --width 1920 --height 1080
```

#### SIPL (YUV and RAW Cameras)
For a single-channel module, use the saved SIPL configuration and select its CAM Port:

```plain
./camera_frame_capture --sipl --cam 7
```

CAM Port numbering starts at 0. The selected port must be enabled in the configuration.

For a multi-channel module, add --channel N to select one module-local channel, or --stereo to select channels 0 and 1 together. Specifying only --cam N does not automatically select a module-local channel.

If the configuration selects exactly one sensor, you can omit the port and let the SDK resolve it:

```plain
./camera_frame_capture --sipl
```

For multiple ports and module-local channel selection, see "SIPL Channels and Configuration" below.

### 2. Frame Count and Live Preview
The following options apply to all three capture backends:

| **<font style="color:rgb(0, 0, 0);">Option</font>** | **<font style="color:rgb(0, 0, 0);">Description</font>** |
| --- | --- |
| <font style="color:rgb(0, 0, 0);">--frames N</font> | <font style="color:rgb(0, 0, 0);">Capture N frames per channel; N must be positive. The default is 30 frames per channel</font> |
| <font style="color:rgb(0, 0, 0);">--view</font> | <font style="color:rgb(0, 0, 0);">Enable live preview. Without --frames, capture continuously until Ctrl+C</font> |

Examples:

```plain
# V4L2 live preview
./camera_frame_capture --v4l2 --video /dev/video6 --view

# Preview 300 Argus frames
./camera_frame_capture --v4l2 --video /dev/video0 --view --frames 300

# SIPL live preview
./camera_frame_capture --sipl --cam 7 --view
```

The tool prints capture logs and does not save image files.

### 3. Enabling Distortion Correction
Add --correct to the capture command:

```plain
# V4L2 preview with correction
./camera_frame_capture --v4l2 --video /dev/video6 --view --correct

# Argus preview with correction
./camera_frame_capture --v4l2 --video /dev/video0 --view --correct

# SIPL preview with correction
./camera_frame_capture --sipl --cam 7 --view --correct
```

Correction requires reading calibration from the selected camera. The following conditions must be met:

+ The SDK must be built with VPI enabled, and the target must have a matching VPI runtime. The current JP7.2 SDK uses VPI 4.1.3.
+ The camera must have valid Pinhole or equidistant Fisheye calibration intrinsics.
+ The captured image dimensions must match the calibration dimensions.
+ The camera calibration storage must be accessible through the correct route.

#### V4L2/Argus: Optional Calibration Read Addresses
For drivers that cannot automatically resolve EEPROM calibration storage, use the following options:

| **<font style="color:rgb(0, 0, 0);">Option</font>** | **<font style="color:rgb(0, 0, 0);">Description</font>** |
| --- | --- |
| <font style="color:rgb(0, 0, 0);">--eeprom-addr ADDR</font> | <font style="color:rgb(0, 0, 0);">Override the host-accessible 7-bit EEPROM address; valid range: 0x08 to 0x77</font> |
| <font style="color:rgb(0, 0, 0);">--bus N</font> | <font style="color:rgb(0, 0, 0);">Override the I2C bus; regular camera models are subject to the SDK initialization restriction below</font> |

For example, discover the I2C bus from the video node, read calibration from EEPROM address 0x50, and apply Argus correction:

```plain
./camera_frame_capture --v4l2 --video /dev/video0 \
  --eeprom-addr 0x50 --view --correct
```

Use addresses appropriate for the actual connection. Decimal and hexadecimal values are accepted; for example, 80 and 0x50 are equivalent.

Address selection follows this order:

```plain
--eeprom-addr specified
    → Use the specified address

--eeprom-addr omitted
    → On NVIDIA platforms, prefer the device-tree eeprom-addr property
    → If the property is absent, use the default address 0x50
```

### 4. SIPL Channels and Configuration
#### Selecting Ports and Module-Local Channels
--cam selects a CAM Port, and --channel selects a channel within that module. Whether explicit channel selection is required depends on the number of channels exposed by the driver. Enabling one CAM does not necessarily mean that only one channel is available.

```plain
# Capture two different CAM Ports; each selected module must expose one capture channel
./camera_frame_capture --sipl --cam 6 --cam 7

# Capture channels 0 and 1 on the same CAM Port
./camera_frame_capture --sipl --cam 4 --stereo

# Capture only channel 1 on the selected CAM Port
./camera_frame_capture --sipl --cam 4 --channel 1

# Live preview of both channels
./camera_frame_capture --sipl --cam 4 --stereo --view
```

These selections have different meanings:

+ --sipl --cam 6 --cam 7: select two CAM Ports.
+ --sipl --cam 4 --stereo: select channels 0 and 1 within one CAM Port.
+ --sipl --cam 4 --channel 1: select channel 1 within one CAM Port.

--stereo and --channel are mutually exclusive, and each requires exactly one explicit CAM Port. The module and driver must provide the selected channels. The driver defines how ports correspond to the physical left and right cameras.

Each capture channel has its own worker thread and preview window. Parallel capture does not establish stereo frame synchronization, and the tool does not pair frames by exposure time.

#### Handling Multi-Channel Errors
If a CAM is selected without a module-local channel, the SDK reports the following error for a multi-channel module:

```plain
Multi-channel SIPL capture requires an explicit siplChannel
```

For an enabled CAM0, choose one of the following as needed:

```plain
# Preview only module-local channel 0
./camera_frame_capture --sipl --cam 0 --channel 0 --view

# Preview only module-local channel 1
./camera_frame_capture --sipl --cam 0 --channel 1 --view

# Preview module-local channels 0 and 1 in separate windows
./camera_frame_capture --sipl --cam 0 --stereo --view
```

An error log showing channel=0 does not mean that channel 0 was explicitly selected. Add --channel 0, or use --stereo to select channels 0 and 1. The module and driver must provide those channels, and the CAM must be enabled in both the configuration and the masks.

For the saved S36 CAM4 configuration at 1920×1536 above:

```plain
# Capture 30 frames from each camera
sudo ./camera_frame_capture --sipl --cam 4 --stereo --frames 30

# Capture only channel 0; use --channel 1 for channel 1
sudo ./camera_frame_capture --sipl --cam 4 --channel 0 --frames 30

# Preview the left and right channels separately
sudo ./camera_frame_capture --sipl --cam 4 --stereo --view
```

This example configuration outputs 1920×1536 UYVY. Normal capture does not require valid calibration. With --correct, the selected channel requires valid intrinsics; --stereo --correct requires valid intrinsics for both cameras.

#### Specifying SIPL Configuration Manually
To override the saved configuration, specify the JSON file and configuration name:

```plain
./camera_frame_capture --sipl --cam 7 \
  --json /path/to/camera.json \
  --config SHW3G_8CH \
  --mask "0x1000 0x0000"
```

| **<font style="color:rgb(0, 0, 0);">Option</font>** | **<font style="color:rgb(0, 0, 0);">Description</font>** |
| --- | --- |
| <font style="color:rgb(0, 0, 0);">--json PATH</font> | <font style="color:rgb(0, 0, 0);">Camera JSON file path</font> |
| <font style="color:rgb(0, 0, 0);">--config NAME</font> | <font style="color:rgb(0, 0, 0);">Configuration name within the JSON file</font> |
| <font style="color:rgb(0, 0, 0);">--mask "MASK ..."</font> | <font style="color:rgb(0, 0, 0);">Link masks in driver transport order, enclosed together in one pair of double quotes; specify this option once</font> |

Provide --json and --config together. --mask requires both.

Selecting a CAM Port does not enable a link disabled by the masks. The port and masks must agree.

SIPL native dimensions, format, and device access routes are defined by the driver configuration. The following options are not accepted:

```plain
--model
--bus
--eeprom-addr
--width
--height
--isx031-ebd
```

### 5. Embedded Data (EBD)
| **<font style="color:rgb(0, 0, 0);">Capture Backend</font>** | **<font style="color:rgb(0, 0, 0);">EBD Access</font>** |
| --- | --- |
| <font style="color:rgb(0, 0, 0);">V4L2</font> | <font style="color:rgb(0, 0, 0);">Use --isx031-ebd to parse EBD on ISX031 family cameras</font> |
| <font style="color:rgb(0, 0, 0);">Argus</font> | <font style="color:rgb(0, 0, 0);">The top-row extraction option above is not supported</font> |
| <font style="color:rgb(0, 0, 0);">SIPL</font> | <font style="color:rgb(0, 0, 0);">Obtain driver-provided embedded data and parsed fields with each frame</font> |

#### V4L2: ISX031 EBD Parsing
```plain
./camera_frame_capture --v4l2 --video /dev/video6 --isx031-ebd
```

This feature applies to the ISX031 family and requires driver support. It cannot be combined with `--correct` or Argus capture.

#### SIPL: EBD Delivered with Frames
Start SIPL capture normally to print EBD in the terminal. For multi-channel modules, specify the channel:

```plain
./camera_frame_capture --sipl --cam 7 --frames 30
./camera_frame_capture --sipl --cam 4 --channel 0 --frames 30
```

The tool prints driver-provided data, including:

| **<font style="color:rgb(0, 0, 0);">Log Field</font>** | **<font style="color:rgb(0, 0, 0);">Description</font>** |
| --- | --- |
| <font style="color:rgb(0, 0, 0);">sof-tsc, eof-tsc</font> | <font style="color:rgb(0, 0, 0);">Raw capture start and end TSC values</font> |
| <font style="color:rgb(0, 0, 0);">sensor-frame</font> | <font style="color:rgb(0, 0, 0);">Sensor frame sequence number</font> |
| <font style="color:rgb(0, 0, 0);">sensor-timestamp-raw</font> | <font style="color:rgb(0, 0, 0);">Raw sensor timestamp</font> |
| <font style="color:rgb(0, 0, 0);">top-ebd-bytes, bottom-ebd-bytes</font> | <font style="color:rgb(0, 0, 0);">Byte counts of raw top and bottom EBD</font> |
| <font style="color:rgb(0, 0, 0);">num-exposures</font> | <font style="color:rgb(0, 0, 0);">Number of exposures</font> |
| <font style="color:rgb(0, 0, 0);">exposure-s</font> | <font style="color:rgb(0, 0, 0);">Array of exposure times in seconds</font> |

Raw EBD is stored in embeddedDataTop and embeddedDataBottom within Frame::siplMetadata. The command-line tool prints byte counts, not the raw bytes.

Available data depends on the camera, driver, and mode. If the driver provides no raw EBD, the byte count is 0. Invalid parsed fields produce empty arrays or unavailable values. SIPL does not accept --isx031-ebd.

### 6. Capture Logs and Pixel Formats
V4L2 input format is negotiated automatically and supports UYVY, YUYV, and NV12. Argus native input is NV12. SIPL native format is determined by the driver.

There are currently no command-line options for pixel format or frame rate. The capture backend and camera configuration determine the frame rate. SIPL correction converts non-NV12 input to NV12 first; this conversion requires GStreamer support.

Common log fields are listed below:

| **<font style="color:rgb(0, 0, 0);">Field</font>** | **<font style="color:rgb(0, 0, 0);">Description</font>** |
| --- | --- |
| <font style="color:rgb(0, 0, 0);">video</font> | <font style="color:rgb(0, 0, 0);">Video node used by V4L2/Argus</font> |
| <font style="color:rgb(0, 0, 0);">port, channel</font> | <font style="color:rgb(0, 0, 0);">SIPL CAM Port and module-local channel</font> |
| <font style="color:rgb(0, 0, 0);">frame</font> | <font style="color:rgb(0, 0, 0);">SDK frame sequence number</font> |
| <font style="color:rgb(0, 0, 0);">size</font> | <font style="color:rgb(0, 0, 0);">Current output image dimensions</font> |
| <font style="color:rgb(0, 0, 0);">format</font> | <font style="color:rgb(0, 0, 0);">Current output pixel format</font> |
| <font style="color:rgb(0, 0, 0);">bytes</font> | <font style="color:rgb(0, 0, 0);">Image data size in bytes for the current frame</font> |
| <font style="color:rgb(0, 0, 0);">host-us</font> | <font style="color:rgb(0, 0, 0);">Host timestamp in microseconds</font> |
| <font style="color:rgb(0, 0, 0);">pts-ns</font> | <font style="color:rgb(0, 0, 0);">Backend-provided PTS in nanoseconds</font> |
| <font style="color:rgb(0, 0, 0);">ebd-bytes</font> | <font style="color:rgb(0, 0, 0);">Byte count of EBD extracted by V4L2</font> |
| <font style="color:rgb(0, 0, 0);">exposure-lines, exposure-s</font> | <font style="color:rgb(0, 0, 0);">Exposure line count and exposure time parsed from V4L2 EBD, printed when valid</font> |

Optional metadata is printed only when provided by the backend. SIPL TSC values and raw sensor timestamps are not equivalent to host time or pipeline PTS.

Exit codes: 0 for success, 1 for initialization or runtime failure, and 2 for command-line usage or argument parsing errors.

## camera_ota: Camera Firmware and ISP Tuning File Updates
`camera_ota` supports firmware updates, NVIDIA ISP tuning file updates, and current running firmware version queries for supported models through V4L2 or SIPL.

Stop other camera applications and all capture tasks before running it. Replace device nodes, CAM numbers, and file paths below with the actual values. Use `sudo` if permissions are insufficient.

Show help:

```plain
./camera_ota --help
```

### 1. Updating Camera Firmware
Use the saved configuration. The following examples use ISX031:

```plain
# V4L2
./camera_ota --v4l2 --video /dev/video7 firmware.bin

# SIPL: an actual ISX031 single-sensor camera
./camera_ota --sipl --cam 7 firmware.bin
```

In V4L2 mode, you can specify the model manually. For regular camera models, the I2C bus is discovered from the video node:

```plain
./camera_ota --v4l2 --video /dev/video7 --model SG3S11AFLK firmware.bin
```

| **<font style="color:#1f2329;">Type</font>** | **<font style="color:#1f2329;">File and Requirements</font>** |
| --- | --- |
| <font style="color:#1f2329;">ISX031 / S36</font> | <font style="color:#1f2329;">512 KiB .bin; power-cycle the camera after writing</font> |
| <font style="color:#1f2329;">GW5 / GW6</font> | <font style="color:#1f2329;">Matching complete firmware image, usually .rom</font> |
| <font style="color:#1f2329;">SIPL firmware</font> | <font style="color:#1f2329;">Supports matching MAX96717/F single-sensor routes, the S36 MAX9295D dual-ISX031 route, the 1920×1200 GW5 MAX96712 route, and matching GW6 stereo configurations</font> |

The file must match the actual camera, BSP, and framework. Use a local path or a download address supplied by your FAE. Ordinary URLs are not accepted directly.

A V4L2 S36 update started from either corresponding video node updates the left and right cameras sequentially; both nodes must have S36 saved as their model. SIPL S36 requires the matching MAX9295D dual-ISX031 route and updates the physical left camera first, then the right camera, followed by right-camera address configuration. Current S36 updates do not compare a complete firmware readback. Power-cycle the camera, reinitialize it, and confirm both running firmware versions.

### 2. Updating NVIDIA ISP Tuning
```plain
# V4L2: SHW3G, SHW5G, and S56 support .isp and .nito
./camera_ota --v4l2 --video /dev/video7 --model SHW3G /path/to/update.nito

# SIPL: update the existing NITO for a matching single-sensor RAW module
./camera_ota --sipl --cam 7 /path/to/SHW3G.nito
```

The file must be nonempty and no larger than 16 MiB.

+ V4L2: Success means that the file was installed. Reload the Argus process or service that uses the file. NITO consumers must use a matching `NVCAMERA_NITO_PATH`.
+ SIPL: Existing NITO updates support matching single-sensor RAW modules and S56/S56C stereo RAW modules. The operation includes reload, frame-output validation, and rollback on failure. Capture remains stopped afterward.

### 3. Specifying SIPL Configuration Manually
```plain
./camera_ota --sipl --cam 7 \
  --json /path/to/camera.json --config SHW3G_8CH \
  --mask "0x1000 0x0000" /path/to/SHW3G.nito
```

Provide `--json` and `--config` together. List `--mask` values in driver transport order and specify the option only once. OTA selects one enabled CAM per operation. If `--cam` is omitted, the configuration must select exactly one sensor.

### 4. Reading the Current Firmware Version
Replace the final file argument with `--get-version`:

```plain
./camera_ota --v4l2 --video /dev/video7 --get-version
./camera_ota --sipl --cam 7 --get-version
```

This command does not query NITO versions.

```plain
Firmware version (running): <current running version>
```

After writing ISX031/S36 firmware, power-cycle the camera before confirming the new running version. V4L2 S36 --get-version reads only the camera associated with the selected video node, so query both nodes separately. SIPL S36 returns the running versions for channel0 and channel1 in one call.

Exit codes: `0` for success, `1` for an operation failure, and `2` for a usage or argument error. A validation failure after firmware writing does not mean that no firmware was written.

## camera_image_controls: Image Control API Verification
`camera_image_controls` supports V4L2, NVIDIA Argus, and SIPL live preview and verifies camera image control APIs. The command must end with `image`.

For an initial state showing the live scene with flip and mirror disabled, the displayed sequence is: **live scene → vertical flip → restored scene → horizontal mirror → restored scene → test pattern**. The program previews the initial image for about 3 seconds, then tests Flip, Mirror, and Testpattern in order. Each test disables and then enables its control, displaying each state for about 3 seconds, and then restores the initial value. After all tests, it restores the initial settings, closes the preview, and stops capture.

Show help:

```plain
./camera_image_controls --help
```

### 1. Selecting the Capture Backend
```plain
# V4L2
./camera_image_controls --v4l2 --video /dev/video6 image

# NVIDIA Argus
./camera_image_controls --v4l2 --video /dev/video0 image

# SIPL: default module-local channel 0
./camera_image_controls --sipl --cam 7 image

# SIPL: select a module-local channel
./camera_image_controls --sipl --cam 4 --channel 1 image
```

Camera model and SIPL settings come from the saved configuration. RAW models use Argus automatically. `/dev/videoN` maps automatically to sensor-id `N`. In SIPL mode, `--channel` requires an explicit `--cam`.

### 2. Controls Verified
| **<font style="color:rgb(0, 0, 0);">Mode</font>** | **<font style="color:rgb(0, 0, 0);">Checks</font>** |
| --- | --- |
| <font style="color:rgb(0, 0, 0);">image</font> | <font style="color:rgb(0, 0, 0);">Flip, mirror, and test pattern: three Get/Set pairs</font> |

### 3. Test Procedure and Results
The program reads and saves the initial values, applies each test value, displays each successful setting for 3 seconds, and reads the state back for comparison. It attempts to restore the initial state after each group. Ctrl+C and runtime errors also trigger attempts to restore settings and stop capture.

Stop other camera applications first and run on the target device in a desktop session with GStreamer preview support.

Supported controls depend on the camera model. Unsupported controls, readback mismatches, and restoration failures produce a final result of `FAIL`. If an initial read fails, the corresponding settings are skipped.

### 4. Expected Output
The terminal prints setting results, readback comparisons, and restoration results. A typical all-pass result is:

```plain
[PASS] Flip Get/Set
[PASS] Mirror Get/Set
[PASS] Testpattern Get/Set
[RESULT] PASS pairs_passed=3/3 failures=0 (visual effects require observation)
```

`PASS` means that API calls, readback comparisons, and restoration checks passed. Image effects still require visual confirmation.

Exit codes: `0` when all checks pass, `1` for runtime or verification failure, `2` for argument errors, and `130` for interruption.

## basic_*: Minimal SDK Examples
Each `.cpp` file in `sample/basic/` demonstrates one function and can be copied into an application. These examples use only public SDK APIs and the C++17 standard library, with matching saved configuration. They do not provide the optional command-line arguments of the complete tools.

### 1. Building
The default build above already includes all eight `basic_*` programs. To build only these minimal examples:

```plain
cmake -S /opt/sgMIX -B "$HOME/sgmix-basic-build" \
  -DSGMIX_BUILD_FULL_SAMPLES=OFF
cmake --build "$HOME/sgmix-basic-build" -j6
cd "$HOME/sgmix-basic-build"
```

You can also compile one source file directly, for example:

```plain
g++ -std=c++17 /opt/sgMIX/sample/basic/sipl_capture.cpp \
  -o basic_sipl_capture \
  -I/opt/sgMIX/include -L/opt/sgMIX/lib \
  -Wl,-rpath,/opt/sgMIX/lib -lsgmix -pthread
```

### 2. Source Files and Usage
| **<font style="color:rgb(0, 0, 0);">Source</font>** | **<font style="color:rgb(0, 0, 0);">Command</font>** | **<font style="color:rgb(0, 0, 0);">Behavior and Main APIs</font>** |
| --- | --- | --- |
| <font style="color:rgb(0, 0, 0);">basic/v4l2_capture.cpp</font> | <font style="color:rgb(0, 0, 0);">./basic_v4l2_capture /dev/video6</font> | <font style="color:rgb(0, 0, 0);">Read 30 frames and print sequence numbers, dimensions, and byte counts. UYVY/YUYV/NV12 input is negotiated automatically. No output format is forced by default; dimensions and frame rate are negotiated</font> |
| <font style="color:rgb(0, 0, 0);">basic/sipl_capture.cpp</font> | <font style="color:rgb(0, 0, 0);">./basic_sipl_capture 7</font> | <font style="color:rgb(0, 0, 0);">Read 30 frames from the single-sensor module on CAM7; the SIPL driver defines dimensions and format</font> |
| <font style="color:rgb(0, 0, 0);">basic/sipl_stereo_capture.cpp</font> | <font style="color:rgb(0, 0, 0);">./basic_sipl_stereo_capture 4</font> | <font style="color:rgb(0, 0, 0);">Use one object and thread for each of CAM4 channels 0 and 1; read 30 frames from each</font> |
| <font style="color:rgb(0, 0, 0);">basic/v4l2_read_sn.cpp</font> | <font style="color:rgb(0, 0, 0);">./basic_v4l2_read_sn /dev/video6</font> | <font style="color:rgb(0, 0, 0);">Initialize and call SG_GetCameraSerialNumber() without starting capture</font> |
| <font style="color:rgb(0, 0, 0);">basic/sipl_read_calibration.cpp</font> | <font style="color:rgb(0, 0, 0);">./basic_sipl_read_calibration 4</font> | <font style="color:rgb(0, 0, 0);">Call SG_GetCalibration() and print intrinsics, including right-camera intrinsics and stereo extrinsics when available. The example does not explicitly start capture</font> |
| <font style="color:rgb(0, 0, 0);">basic/v4l2_correct_capture.cpp</font> | <font style="color:rgb(0, 0, 0);">./basic_v4l2_correct_capture /dev/video6</font> | <font style="color:rgb(0, 0, 0);">Request NV12, set correctDistortion = true, and read 30 corrected frames</font> |
| <font style="color:rgb(0, 0, 0);">basic/v4l2_image_flip.cpp</font> | <font style="color:rgb(0, 0, 0);">./basic_v4l2_image_flip /dev/video6 1 0</font> | <font style="color:rgb(0, 0, 0);">Set vertical flip and horizontal mirror, capture 30 frames, then restore the original values</font> |
| <font style="color:rgb(0, 0, 0);">basic/sipl_image_flip.cpp</font> | <font style="color:rgb(0, 0, 0);">./basic_sipl_image_flip 7 1 0</font> | <font style="color:rgb(0, 0, 0);">Apply flip, mirror, and restoration to an enabled single-sensor SIPL CAM</font> |

The capture examples demonstrate this call sequence:

```plain
SG_Init → SG_StartCapture → SG_GetFrame → SG_StopCapture
```

Process `frame.data` at the indicated comment in the capture loop, interpreting it using `pixelFormat`, image dimensions, and `planes`. Reading into the same `Frame` replaces its contents. Copy or move the `Frame` to retain ownership when passing it between threads or using it later.

The minimal examples do not open preview windows or save images. The correction example requires a matching VPI runtime, valid intrinsics for the target channel, and capture dimensions matching the calibration. Dual-channel order comes from Query; independent reads do not establish synchronized exposure pairing.

These examples have no `--help` option. Running without arguments prints usage. Exit codes: `0` for success, `1` for runtime failure, and `2` for a usage or argument error.

### 3. Flip and Mirror Controls
Use the saved configuration. The argument style matches the other minimal examples:

```plain
# The final two arguments are vertical flip and horizontal mirror: 0 disables, 1 enables
./basic_v4l2_image_flip /dev/video6 1 0
./basic_sipl_image_flip 7 1 0
```

The call sequence is: initialize → start capture and get the first frame → read all original values → set and read back → capture 30 frames → restore original values in reverse order → stop capture. The example logs 30 frames after the setting is applied.

The program prints `before`, `requested`, `readback`, and restoration results. A failed setting may already have changed the device, so restoration is attempted for that parameter. Later readback or capture failures, SIGINT, and SIGTERM also trigger restoration and capture shutdown. Restoration or shutdown failures are reported separately and produce a nonzero exit code.

Readback is checked against the requested boolean state using the public APIs. To assess the visual effect, use the `camera_image_controls` preview.

# SDK Functions and API Reference
The SDK uses C++17. Public types and CameraControl are in the sgmix namespace. The interfaces are defined in include/SG_CameraControl.h and include/SG_CameraTypes.h. Successful initialization is required before calls, except for correction with explicitly supplied intrinsics. Failures return false and provide a diagnostic through error.

## Initialization
Prototype:

```plain
bool SG_Init(const CameraInitConfig& config, std::string& error);
```

For V4L2, pass a `CameraConfig` with the video node. The SDK can read the model from saved settings. A manual I²C bus usually also requires a host-visible 7-bit sensor address. `EEPROM_CAM` requires an explicit bus. See `SG_CameraTypes.h` for address overrides.

```plain
sgmix::CameraConfig config;
config.videoDevice = "/dev/video0";
// Optionally override the saved model:
// config.model = "SG3S-ISX031C-GMSL2-Hxxx";
sgmix::CameraControl camera;
std::string error;
if (!camera.SG_Init(config, error)) {
    // Handle error and stop further operations.
}
```

For SIPL, pass a SiplCameraConfig. This example uses the saved SIPL settings and binds CAM7.

```cpp
sgmix::SiplCameraConfig config;
config.cameraPort = 7;
if (!camera.SG_Init(config, error)) {
    // Handle initialization failure.
}
```

`cameraPort` is the driver CAM number before masking. Zero means CAM0. It can be omitted when exactly one sensor is enabled. Objects using the same configuration share a session. Stop capture and release all bindings before switching configurations. See the SIPL configuration examples above for JSON and link masks.

## Video Capture Control
Prototype:

```plain
/** @brief Start capture on an initialized camera. */
bool SG_StartCapture(const CaptureConfig& config, std::string& error);

/** @brief Consume a new frame; timeoutMs is in milliseconds, 0 polls immediately, and failure clears frame. */
bool SG_GetFrame(Frame& frame, std::uint32_t timeoutMs, std::string& error);

/** @brief Stop capture and release resources; repeated calls are safe, and destruction also stops capture. */
bool SG_StopCapture(std::string& error);
```

Call `SG_Init`, then `SG_StartCapture`. Read frames in a loop with `SG_GetFrame` and call `SG_StopCapture` when finished.

- **Capture settings.** The SDK selects V4L2, Argus, or SIPL based on the initialization settings and model. Leave dimensions and frame rate at zero to use the source settings. For V4L2 and Argus, specify both width and height together. Explicit SIPL dimensions and frame rate must match the JSON. For multi-channel SIPL modules, use one `CameraControl` per channel and set `siplChannel` explicitly.
- **Frame data.** A `Frame` owns its CPU data and remains valid after another read or a stop. Live capture keeps one pending frame and drops older frames if the reader falls behind. `SG_GetFrame` can report asynchronous startup errors.
- **Concurrency.** Each object allows one frame reader. Another thread can call `SG_StopCapture` to interrupt a waiting read. Serialize all other operations. Reinitialization stops capture first. If it fails, initialize successfully before capturing again.

See `SG_CameraTypes.h` for pixel formats, plane layouts, and timestamp fields. See the `camera_frame_capture` example above for capture commands.

## Distortion Correction
Prototype:

```plain
/**
 * @brief Synchronously correct an existing NV12 frame without automatic pixel-format conversion.
 * @param input Input frame; its pixels and metadata remain unchanged.
 * @param output Independent tightly packed NV12 output; must not be the same object as input.
 * @param error Output diagnostic string.
 * @param intrinsics Optional single-sensor intrinsics; explicit values require neither initialization nor EEPROM access.
 * @return True on success; false on failure.
 */
 bool SG_CorrectFrame(const Frame& input, Frame& output, std::string& error, const std::optional<CameraCalibration::Intrinsics>& intrinsics = std::nullopt);
```

Use a two-plane NV12 input at the calibration resolution. VPI must be enabled with runtime version 4.1.3 or later, below 5.0. Pinhole and equidistant fisheye calibration are supported.

If omitted, intrinsics are read from the initialized camera and cached. For stereo, use the intrinsics for the selected channel and supply them explicitly when the SDK cannot resolve the channel. Set `CaptureConfig::correctDistortion` to receive corrected NV12 frames during capture.

## Reading Camera Information
Prototype:

```plain
/**
 * @brief Read device identity, effective routes, lens information, SN, and backend capabilities.
 * @param info Output camera information structure.
 * @param error Output diagnostic string.
 * @return True on success; false on failure.
 */
 bool SG_GetDeviceInfo(DeviceInfo& info, std::string& error);
```

Check optional DeviceInfo fields before use. Capability flags do not indicate an active license or successful runtime update validation. Use the optional sensor.endpoint to inspect the actual access route:

```plain
sgmix::DeviceInfo info;
if (camera.SG_GetDeviceInfo(info, error) && info.sensor.endpoint) {
    const auto address = info.sensor.endpoint->address; // Host-visible 7-bit address
    const auto bus = info.sensor.endpoint->bus;
}
```

## Reading Image Information
Prototype:

```plain
/** @brief Read the stored image configuration. */
bool SG_GetImageInfo(ImageInfo& info, std::string& error);
```

ImageInfo contains width, height, framesPerSecond, pixelFormat, triggerMode, and embeddedDataEnabled. It describes stored configuration, not the currently negotiated capture settings.

## Reading the Camera Serial Number
Prototype:

```plain
/**
 * @brief Read the serial number of the current camera.
 * @param serialnumber Output camera serial number string.
 * @param error        Output diagnostic string.
 * @return True on success; false on failure.
 */
 bool SG_GetCameraSerialNumber(std::string& serialnumber, std::string& error);
```

SN reads use at most 32 bytes.

## Reading the Stored Camera Resolution
Prototype:

```plain
/** @brief Read the stored camera resolution. */
bool SG_GetCameraResolution(uint16_t& width, uint16_t& height, std::string& error);
```

## Camera Intrinsics and Extrinsics
Prototype:

```plain
/**
 * @brief Read camera calibration on demand.
 * @param calibration Output calibration structure; left intrinsics are required.
 * @param error Output diagnostic string.
 * @return True on success; false if a required section cannot be read.
 */
 bool SG_GetCalibration(CameraCalibration& calibration, std::string& error);
```

Left-camera intrinsics are required. Right-camera intrinsics, stereo extrinsics, and RMS are optional. SIPL ISX031 requires the physical model and matching configuration. Stop all shared captures before reading Flash. If restoration fails, release all shared objects and reinitialize.

Use CameraCalibration::right and stereo to access optional right-camera intrinsics and stereo extrinsics:

```plain
sgmix::CameraCalibration calibration;
if (camera.SG_GetCalibration(calibration, error) && calibration.right && calibration.stereo) {
    const auto rightFx = calibration.right->fx;
    const auto translation = calibration.stereo->translation; // Millimeters
}
```

SIPL stereo modules capture each siplChannel separately and do not automatically produce synchronized frame pairs.

## Reading IMU Calibration
Prototype:

```plain
/** @brief Read IMU calibration. */
bool SG_GetImuCalibration(ImuCalibration& calibration, std::string& error);
```

ImuCalibration calibration{}; contains noise parameters, bias, calibration matrices, and the camera-to-IMU transform. Translation is in meters and time offset is in seconds. Unprogrammed data or invalid data returns an error.

## Getting Image Flip State
SIPL image controls require capture to be started on the selected channel. They require a verified MAX96717/F route, or an adapted S56/S56C or S36 MAX9295D route with independent sensor mappings. Use S56 as the product model for S56C and select the channel with siplChannel. S56/S56C MAX9295D image Set calls require exactly one enabled stereo module and one active capture channel.

Prototype:

```plain
/**
 * @brief Get the current vertical image flip state.
 * @param state Output state; true means flip is enabled.
 * @param error Output diagnostic string.
 */
 bool SG_GetCameraImageFlip(bool& state, std::string& error);
```

## Getting Image Mirror State
Prototype:

```plain
/**
 * @brief Get the current horizontal image mirror state.
 * @param state Output state; true means mirror is enabled.
 * @param error Output diagnostic string.
 */
 bool SG_GetCameraImageMirror(bool& state, std::string& error);
```

## Getting Test Pattern State
Prototype:

```plain
/**
 * @brief Get the current test pattern output state.
 * @param state Output state; true means test pattern output is enabled.
 * @param error Output diagnostic string.
 */
 bool SG_GetCameraImageTestpattern(bool& state, std::string& error);
```

## Setting Image Flip
Prototype:

```plain
/**
 * @brief Set the image flip state.
 * @param state True enables vertical flip.
 * @param error Output diagnostic string.
 */
 bool SG_SetCameraImageFlip(bool state, std::string& error);
```

## Setting Image Mirror
Prototype:

```plain
/**
 * @brief Set the image mirror state.
 * @param state True enables horizontal mirror.
 * @param error Output diagnostic string.
 */
 bool SG_SetCameraImageMirror(bool state, std::string& error);
```

## Setting Test Pattern Output
Prototype:

```plain
/**
 * @brief Enable or disable test pattern output.
 * @param state True enables test pattern output.
 * @param error Output diagnostic string.
 */
 bool SG_SetCameraImageTestpattern(bool state, std::string& error);
```

## OTA

Prototype:

```plain
/** @brief Update camera firmware or supported tuning files. */
bool SG_PerformOTAUpdate(std::string urlPath, std::string& error);
/** @brief Pass true for a full GW6 ROM update. */
bool SG_PerformOTAUpdate(std::string urlPath, std::string& error, bool fullRom);
```

Pass an update file that matches the camera. GW6 defaults to a component update. Pass `fullRom=true` for a supported full ROM update.

Stop capture before updating, including all captures in a shared SIPL session. After updating, reinitialize or power-cycle the camera as required and check the running firmware version. See the earlier `camera_ota` section for file requirements, steps, and examples.

## Restoring the S36 Right-Camera Address
Prototype:

```plain
/**
 * @brief Configure the native S36 right-camera address as 0x1B without erasing or writing firmware.
 * @param error Output diagnostic string.
 * @return True when the configuration sequence completes successfully; false on failure.
 */
bool SG_RestoreS36RightAddress(std::string& error);
```

This API supports only the matching SIPL dual-ISX031/MAX9295D S36 route. Stop all captures in the shared session before calling it. After an attempted recovery, release all shared objects, power-cycle the camera, reinitialize, and confirm the address and versions.

## Offline authorization
The SDK loads the device-bound offline license automatically when an ISP control is requested.

## Reading the Running Firmware Version
Prototype:

```plain
/**
 * @brief Read the current running camera firmware version.
 * @param version Output firmware version string.
 * @param error   Output diagnostic string.
 * @return True on success; false on failure.
 */
 bool SG_GetFirmwareVersion(std::string& version, std::string& error);
```

Read the running firmware version after initialization. For SIPL ISX031 or S36, stop all captures in the shared session first. SIPL S36 returns both camera versions. SIPL SDT2BCO1 requires one enabled module per GMSL I²C bus.

## Camera Brightness Control
**ISX031 and S36 support ISP parameter control through V4L2 and SIPL. S36 requires a matched MAX9295D dual-ISX031 route for SIPL.**

| Backend and camera models | Supported ISP controls |
| --- | --- |
| V4L2, ISX031 models registered in the SDK and S36 | Contrast, saturation, sharpness, exposure mode, manual exposure time, analog gain, white balance mode, and RGB white balance. |
| V4L2, SHW3G, SHW5G, and S56 | Brightness, contrast, saturation, sharpness, noise reduction, exposure mode, manual exposure time, and analog gain. |
| SIPL, ISX031 models registered in the SDK and S36 on matched MAX9295D dual-ISX031 routes | Contrast, saturation, sharpness, exposure mode, manual exposure time, analog gain, white balance mode, and RGB white balance. Start capture on the selected channel before calling these APIs. |

SIPL S36 controls apply to the selected active capture channel. The NVIDIA ISP percentage controls read and write tuning files. The SDK does not reload Argus automatically. Actual parameter ranges depend on the camera mode and firmware. SDK builds with network license checks enabled require authorization for the corresponding features.

Prototype:

```plain
/** @note Requires a network license. *//** @brief Get camera brightness (0-100%). */
bool SG_GetCameraBrightness(int& brightness_percent, std::string& error);

/** @brief Set camera brightness (0-100%). */
bool SG_SetCameraBrightness(int brightness_percent, std::string& error);
```

## Camera Contrast Control
Prototype:

```plain
/** @note Requires a network license. *//** @brief Get camera contrast (0-100%). */
bool SG_GetCameraContrast(int& contrast_percent, std::string& error);

/** @brief Set camera contrast (0-100%). */
bool SG_SetCameraContrast(int contrast_percent, std::string& error);
```

## Camera Saturation Control
Prototype:

```plain
/** @note Requires a network license. *//** @brief Get camera saturation (0-100%). */
bool SG_GetCameraSaturation(int& saturation_percent, std::string& error);

/** @brief Set camera saturation (0-100%). */
bool SG_SetCameraSaturation(int saturation_percent, std::string& error);
```

## Camera Sharpness Control
Prototype:

```plain
/** @note Requires a network license. *//** @brief Get camera sharpness (0-100%). */
bool SG_GetCameraSharpness(int& sharpness_percent, std::string& error);

/** @brief Set camera sharpness (0-100%). */
bool SG_SetCameraSharpness(int sharpness_percent, std::string& error);
```

## Camera Noise Reduction Control
Prototype:

```plain
/** @note Requires a network license. *//** @brief Get camera noise reduction (0-100%). */
bool SG_GetCameraDenoise(int& denoise_percent, std::string& error);

/** @brief Set camera noise reduction (0-100%). */
bool SG_SetCameraDenoise(int denoise_percent, std::string& error);
```

## Camera Exposure Control
Prototype:

```plain
/** @note Requires a network license. *//** @brief Set exposure mode (automatic/manual). */bool SG_SetExposureMode(bool state, std::string& error);

/** @brief Get the current exposure mode (automatic/manual). */
bool SG_GetExposureMode(bool& state, std::string& error);

/**
 * @brief Read the current exposure-time control value, not per-frame EBD.
 * @param val Output exposure time in milliseconds.
 * @param error Output diagnostic string.
 * @return True on success; false on failure.
 * @note Requires the corresponding feature license.
 */
 bool SG_GetExposureTime(float& val, std::string& error);

/** @brief Set manual exposure time in milliseconds. */
bool SG_SetManualExposure(float val, std::string& error);
```

state=true selects automatic mode; false selects manual mode. Switch to manual mode before setting exposure time.

## Camera White Balance Control
Prototype:

```plain
/** @note Requires a network license. *//** @brief Get white balance mode (automatic/manual). */
bool SG_GetCameraWhiteBalanceMode(bool& wb_mode, std::string& error);

/** @brief Set white balance mode (automatic/manual). */
bool SG_SetCameraWhiteBalanceMode(bool wb_mode, std::string& error);

/** @brief Get manual white balance RGB gains. */
bool SG_GetCameraWhiteBalance(int& wb_R, int& wb_G, int& wb_B, std::string& error);

/** @brief Set manual white balance RGB gains. */
bool SG_SetCameraWhiteBalance(int wb_R, int wb_G, int wb_B, std::string& error);
```

wb_mode=true selects automatic mode; false selects manual mode. Switch to manual mode before setting RGB gains.

## Camera Analog Gain Control
Prototype:

```plain
/** @brief Get the current sensor analog gain. */
bool SG_GetGainValue(int& val, std::string& error);

/** @brief Set manual analog gain. */
bool SG_SetManualGain(int val, std::string& error);
```

Switch to manual exposure mode before setting gain.

# Known Issues
### ISX031 Calibration Reads
Some V4L2 drivers require you to start capture once before reading calibration. For SIPL, stop all captures in the shared session before reading calibration.

