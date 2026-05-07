# Overview  
This SDK provides fundamental control capabilities for the camera module, including:

+ Image flipping
+ Image mirroring
+ Test pattern generation
+ Camera information retrieval
+ Camera intrinsic parameter acquisition
+ Remote firmware upgrade (OTA)

In addition, the SDK supports parameter configuration and functional control of the camera ISP (Image Signal Processor), providing developers with a comprehensive interface for **image acquisition, processing, and optimization**.

---

# Supported Platforms  
This SDK is primarily designed for the **NVIDIA Jetson platform** and supports the following environments:

**Operating System**

+ JetPack 6.0+

**Hardware Platforms**

+ Jetson Orin NX
+ Jetson Orin Nano
+ Jetson Orin AGX
+ Jetson Thor

**Supported Camera Devices**

Only Sensing cameras are supported:

+ SG3S-ISX031C-GMSL2F-Hxxx
+ SG3S-ISX031C-GMSL2-Hxxx
+ SG2-ISX021C-GMSL2-Hxxx
+ SG3S11AFLK
+ S36
+ S56
+ SHW3G
+ SHW3H
+ SHF3L

**The camera driver is not included in this SDK package.To obtain the required driver for your platform, please contact our technical support team:** [<font style="color:rgb(68, 147, 248);">Sales@sensing-world.com</font>](mailto:Sales@sensing-world.com)

---

# SDK Feature Overview  
The SDK provides the following core functionalities:

### 1. Basic Camera Control
+ Image Flip
+ Image Mirror
+ Test Pattern Generation
+ Camera Resolution Configuration
+ Trigger Mode Switching

### 2. Camera Information Retrieval
+ Module information query
+ Camera intrinsic parameter acquisition

### 3. Remote Upgrade and Management
+ OTA firmware upgrade

### 4. ISP Function Control (Network authorization required)
Adjust ISP parameters such as:

+ Exposure
+ Gain
+ Color settings
+ Other image tuning parameters

### 5. IMU Support (S56 only)
+ Accelerometer data acquisition
+ Gyroscope data acquisition

### 6. Video Stream Control
Control and manage video streaming from the camera.

### 7. Stereo Depth Function (S36 only)
Generate stereo depth images using supported stereo cameras.

### 8. Distortion Correction
Lens distortion correction functionality.

### 9. Exposure Time Control(EBD)
_(Supported only on SG3S-ISX031C-GMSL2F-Hxxx, SG3S-ISX031C-GMSL2-Hxxx, and S36)_

---

# Quick Start and Installation Guide 

## Installation SDK
```plain
git clone https://github.com/SENSING-Technology/sgMIX.git
sudo apt install git-lfs 
git lfs install
git lfs pull
```

## Dependency Installation
Install the required dependencies before using the SDK.

```plain
sudo apt update
sudo apt-get install libglfw3-dev libgtk-3-dev libzmq3-dev libcurl4-openssl-dev libjsoncpp-dev cmake
sudo apt install nvidia-jetpack
```

## Installation Using the Package  
1. Copy the SDK installation package to the Jetson device, for example:

```plain
/home/nvidia/SENSING_sgMIX.deb
```

2. Install the package:

```plain
sudo dpkg -i SENSING_sgMIX.deb
```

---

# Installation Directory
After installation, the SDK will be located at:

```plain
/opt/sgMIX
```

Directory structure:

```plain
/opt/sgMIX
├── app/            # Applications integrating SDK functionality
├── bin/            # Binary file directory
├── include/        # Public header files containing API declarations
├── lib/            # Precompiled static/dynamic libraries
├── samples/        # Sample programs demonstrating API usage
├── CMakeLists.txt  # CMake build configuration
├── Readme.md       # Documentation for installation and usage
└── config/         # Configuration files
```

---

# Sample Programs
After installation, sample programs can be found in:

```plain
/opt/sgMIX/sample
```

The `main.cpp` example demonstrates basic SDK usage and can be compiled and executed directly.

### Build and Run
```plain
cd /opt/sgMIX/
mkdir build && cd build
cmake ..
make -j6

sudo ./main
```

If a supported Sensing camera is connected and functioning correctly, the program will output the **camera intrinsic parameters**.

---

# Available Example Codes
```plain
/opt/sgMIX/sample
├── DistortionCorrect.cpp    # Distortion correction example
├── Frame.cpp                # Video stream operation example
├── FrameImu.cpp             # Video stream with IMU data example
├── GetEBD.cpp               # Exposure time acquisition example
├── Imu.cpp                  # IMU testing example
├── main.cpp                 # Basic camera operation example
└── sDepth.cpp               # Stereo depth generation example
```

To compile different examples, modify the **TODO section** in the `CMakeLists.txt` file and change the source file name.

---

# GUI Tools
After installation, two GUI tools are available in:

```plain
cd /opt/sgMIX/app
```

Applications:

+ **SgMixViewer** — graphical interface for image display
+ **CameraProducer** — camera data acquisition tool

Before running them, grant execution permission:

```plain
chmod a+x SgMixViewer CameraProducer
```

Run the viewer:

```plain
sudo ./SgMixViewer
```



