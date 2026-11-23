========================================================================
SDepth++ Stereo Depth SDK (S36 / S56C / S56 cameras x NVIDIA Jetson AGX Thor)
readme_en.txt  Version: 2.4    Date: 2026-11-02
========================================================================

[Contents]
1. Product Overview & SDK Architecture
2. Deliverables
3. Fixed Environment Requirements (MUST READ)
4. Quick Start
5. stereo_depth_app Command-line Parameters
6. Camera Profile Files (config/cameras/*.json)
7. Stereo Calibration Files (txt / json)
8. Secondary Development API (sdepth_api.h)
9. Encryption & Licensing Mechanism
10. FAQ
11. Critical Warnings (Red Lines)

========================================================================
1. Product Overview & SDK Architecture
========================================================================
This SDK drives SENSING stereo GMSL2 cameras on an NVIDIA Jetson AGX Thor
developer board and runs the SDepth++ stereo depth model, outputting
depth/disparity results in real time.

Three stereo modules are supported (one SDK, switch via the --camera
command-line option):

  +--------+------------+-------------+---------------+---------------+
  | Model  | Sensor     | Output      | Res @ FPS     | Data Path     |
  +--------+------------+-------------+---------------+---------------+
  | S56C   | 2x VB1940  | RAW10       | 2560x1984@30  | ISP0->NV12    |
  |        |            |             |               | ->VIC->BGR    |
  | S56    | 2x VB1940  | RAW10       | 2560x1984@30  | Same as S56C  |
  | S36    | 2x ISX031  | YUV422(UYVY)| 1920x1536@30  | ICP pass-     |
  |        |            |             |               | through->BGR  |
  |        |            |             |               | (no ISP)      |
  +--------+------------+-------------+---------------+---------------+

  - All three modules share the same 640x480-input depth engine;
    switching models does not require a different model.
  - S56C/S56 are RAW sensors and go through the hardware ISP (requires
    NITO; auto exposure is controlled by ACP).
  - The S36 module has a built-in ISP/AE and outputs UYVY directly;
    no NITO file is needed.
  - Calibration models: S56C/S56 use fisheye (4 coefficients); S36 uses
    pinhole rational (8 coefficients, OpenCV order k1,k2,p1,p2,k3,k4,
    k5,k6).

Measured performance (AGX Thor, JetPack 7.2, S56C):
  - Inference sustained at 30.0 FPS (15.3 ms per frame)
  - Left/right hardware timestamp difference 0.008~0.019 ms
  - 90 seconds of continuous run, 1500+ frames, zero errors

The SDK uses a two-layer structure: "open source + prebuilt closed
library":

  +---------------------------------------------------------------+
  | Open source (src/ in this directory, modifiable by customer)  |
  |   Camera capture (SIPL user space, ISP/ICP dual path)         |
  |   Camera profiles (config/cameras/*.json)                     |
  |   Stereo frame pairing (hardware SOF timestamps)              |
  |   Stereo rectification (fisheye/pinhole, ->640x480)           |
  |   Disparity/depth pseudo-color display                        |
  +---------------------------------------------------------------+
  | Closed inference library lib/libsdepth.so (prebuilt, no       |
  | source): model decryption - license verification - device     |
  | fingerprint generation/comparison - SDepth++ TensorRT         |
  | inference core (FP16)                                         |
  +---------------------------------------------------------------+
  | Delivered encrypted model my_device/ (.enc + .license)        |
  +---------------------------------------------------------------+

The two layers are connected through the C interface in
include/sdepth_api.h. Model decryption and license validation logic are
entirely encapsulated inside the closed library; the SDK does not
contain their source code. The encrypted model can only be decrypted
and run on a board whose hardware matches the license.

========================================================================
2. Deliverables
========================================================================
The delivery consists of four parts:

(1) This SDK directory (open source + prebuilt closed library; the
    customer can build it on the board)
     src/             Open source (capture / rectification /
                      visualization / example main program)
     include/         Public headers (including the closed-library
                      interface sdepth_api.h)
     lib/             Prebuilt closed inference library libsdepth.so
                      (aarch64 / JetPack 7.2 only; do not delete or
                      rename)
     sipl/            SIPL camera interface layer (with s36/s56c/s56
                      platform config json)
     config/cameras/  Camera profile files (s36.json / s56c.json /
                      s56.json; define interface mask, resolution,
                      data path; see Section 6)
     examples/        Reference sample code (not compiled; for
                      secondary development reference)
     encrypt_output/  Runtime directory (vendor_public.pem public key
                      included; calibration files and the encrypted
                      model also go here)
     readme.txt       This document

(2) Encrypted model + model license (generated per customer device
    fingerprint by the vendor; see Section 4, steps 6/7). Delivery
    format:
     my_device/
       |-- sDepth.enc       Encrypted model
       +-- sDepth.license   Model license file

(3) Stereo calibration file (one per module, txt text format, file
    name = module serial number, shipped at random; unique per camera,
    must not be mixed with other cameras).
    The program reads the txt directly; it can also be converted to
    json with the bundled tool (see Section 7).

(4) Stereo camera + GMSL2 cable (hardware, shipped with the camera)

========================================================================
3. Fixed Environment Requirements (MUST READ)
========================================================================
This SDK is delivered for a fixed environment; the following must not
be substituted:

  Hardware : NVIDIA Jetson AGX Thor developer board (customer-purchased,
             TRD1_G2A version)
  Firmware : JetPack 7.2 / L4T R39.2 (must match the factory version;
             flashing other versions is strictly forbidden)
  TensorRT : 10.16 (the JetPack 7.2 official apt repository version,
             providing libnvinfer.so.10; the engine inside the
             encrypted model is bound to the TensorRT major version.
             Installing standalone TensorRT 11 is strictly forbidden,
             otherwise model deserialization fails)
  CUDA     : 13.2 (JetPack 7.2 official apt repository version)
  OpenCV   : 4.8 (from the NVIDIA Jetson apt repository
             libopencv-dev; note the Ubuntu official repository
             provides 4.6, which does NOT meet the requirement. Verify
             the package comes from the
             repo.download.nvidia.com/jetson repository)
  Camera   : SENSING S36 / S56C / S56 stereo GMSL2 camera

Camera drivers (SENSING official GitHub; under JetPack7.2 / L4T R39.2
there are two mutually exclusive driver packages, corresponding to two
capture backends; only one can be installed at a time. The SDK runs
with backend=auto and detects it automatically, no manual switching
needed):

  (1) SIPL user-space driver package (default, recommended by this
      SDK, this directory only):
      TRD1_G2A_AGX_THOR_SIPL_JP7.2_L4TR39.2
      https://github.com/SENSING-Technology/nvidia-jetson-camera-drivers/tree/main/Jetson%20AGX%20Thor%20Devkit/TRD1_G2A/JetPack7.2/TRD1_G2A_AGX_THOR_SIPL_JP7.2_L4TR39.2
      Installation: install.sh copies driver/libnvuddf_* and
      libnvsipl_* into /usr/lib/nvsipl_drv/ (covers s36/s56c/s56);
      no /dev/videoN nodes are created. Corresponds to the SIPL
      backend (s36->ICP/YUV, s56c/s56->ISP), i.e. the nvsipl_camera
      bring-up method in 4.1 below.

  (2) GMSL2x8 custom kernel driver package (V4L2 / Argus route):
      TRD1_G2A_GMSL2x8_JP7.2_L4TR39.2
      https://github.com/SENSING-Technology/nvidia-jetson-camera-drivers/tree/main/Jetson%20AGX%20Thor%20Devkit/TRD1_G2A/JetPack7.2/TRD1_G2A_GMSL2x8_JP7.2_L4TR39.2
      Installation: install.sh replaces the kernel and kernel modules,
      then sudo ./load_modules.sh creates /dev/video0~7. S36 (YUV)
      uses the V4L2 backend; S56C/S56 (RAW) use the Argus backend
      (nvargus-daemon + nvarguscamerasrc).

  The two packages are mutually exclusive: installing (2) overwrites
  (1)'s kernel/drivers and vice versa. Backend-to-driver-package
  mapping:

  [IMPORTANT - read this if you must verify both frameworks on the
   same machine] The GMSL2x8 package (2)'s install.sh directly
  overwrites /boot/Image with the custom kernel (bypassing dpkg; dpkg
  still shows the standard 39.2.0) and also replaces dtbo / custom .ko
  / nito and other system files; the SIPL package (1)'s install.sh
  only installs user-space libraries and does not restore these.
  Therefore, once (2) has been installed and you switch back to (1),
  the system is in a "mixed drivers" state and SIPL camera capture
  breaks (0 frames / Platform configuration failed / NvSciBuf
  allocation failures, often only working the first time right after a
  fresh standard-kernel flash). Reinstalling the kernel alone is not
  sufficient in practice; the reliable fix is to [reflash] JetPack
  7.2 / L4T R39.2 and then install exactly one driver package.
  Conclusion: when verifying both frameworks alternately on the same
  Thor, the easiest reliable way to switch is to reflash each time.
  Production environments (customers) install only one package and are
  unaffected.

     Driver package       S36 (YUV)        S56C/S56 (RAW)
     ------------------   --------------   ----------------------
     SIPL package (1)     sipl(ICP)        sipl(ISP)
     GMSL2x8 package (2)  v4l2(/dev/videoN) argus(nvarguscamerasrc)

  Device node mapping (GMSL2x8 package): cam0=/dev/video0 ...
  cam7=/dev/video7. A stereo module must be connected to an even port
  (cam0/2/4/6) and occupies that port plus the adjacent next port
  (e.g. cam0 -> left /dev/video0 + right /dev/video1).

  Camera wiring to port-mask mapping (consistent with the vendor
  documentation, verified in testing; the --port parameter is
  resolved via this table):

  CAM0 = "0x0000 0x0001"      CAM1 = "0x0000 0x0002"
  CAM2 = "0x0000 0x0100"      CAM3 = "0x0000 0x0200"
  CAM4 = "0x0001 0x0000"      CAM5 = "0x0002 0x0000"
  CAM6 = "0x0100 0x0000"      CAM7 = "0x0200 0x0000"

  ※ The S36 module can only be connected to CAM0/CAM2/CAM4/CAM6 (the
    even port of each two-link GMSL pair).

========================================================================
4. Quick Start
========================================================================
All steps below are executed on the Thor board (default user: nvidia).
The examples assume you have extracted the delivery package and entered
the SDK root directory (the directory containing src/ and
encrypt_output/).

------------------------------------------------------------------------
4.1 Install the camera driver
------------------------------------------------------------------------
# After downloading and extracting the SENSING driver directory above:
cd ~/TRD1_G2A_AGX_THOR_SIPL_JP7.2_L4TR39.2
chmod a+x ./install.sh
sudo ./install.sh

# Set maximum performance mode
sudo nvpmodel -m 0
sudo jetson_clocks

# Bring-up check (the mask must match the actual connector; see the
# table in Section 3):
#   S56C on CAM0:
sudo nvsipl_camera -t query/trd1_g2a/s56c.json -c S56C_2CH \
  -m "0x0000 0x0001" --enable-camera-hal -s -Z -12
#   S36 on CAM2 (YUV pass-through; note -012R differs from S56C):
sudo nvsipl_camera -t query/trd1_g2a/s36.json -c S36_2CH_1920X1536_30FPS \
  -m "0x0000 0x0100" --enable-camera-hal -s -Z -012R

# Expected output (exit with Ctrl+C):
#   Frame rate (fps): 29.9xx
#   Frame captured: xxx  Frame Drops: 0  Frame discontinuities: 0
# Frame drops = 0 and discontinuities = 0 means bring-up succeeded.

Note: this section covers the SIPL driver package (default
recommended). If the customer environment has the GMSL2x8 kernel
driver package installed, use the V4L2/Argus route instead: after
install.sh, run sudo ./load_modules.sh to create /dev/video0~7, then
follow the backend mapping in Section 3 with --backend or rely on
backend=auto; see FAQ Q10/Q11.

------------------------------------------------------------------------
4.2 Install build dependencies
------------------------------------------------------------------------
# Basic build tools + SDK dependencies (all from the official apt
# repositories, no manual downloads):
sudo apt-get update
sudo apt-get install -y cmake build-essential pkg-config \
    libopencv-dev libssl-dev \
    libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev \
    cuda-toolkit tensorrt

# Required for the Argus route (running s56c/s56 on the GMSL2x8
# package): nvidia-l4t-gstreamer. NEVER install it without a version
# (apt would pull the newest release and upgrade the kernel from
# 39.2.0 to 39.2.1, breaking the sgcam_gmsl2.ko symbol CRCs; see FAQ
# Q10). You must pin the version matching the 39.2.0 kernel and hold
# it afterwards (setup_env.sh does this automatically; copy these
# commands verbatim when installing manually):
sudo apt-get install -y nvidia-l4t-gstreamer=39.2.0-20260601141651
sudo apt-mark hold nvidia-l4t-gstreamer

# Verify (all should output normally):
cmake --version | head -1
/usr/local/cuda/bin/nvcc --version | tail -1          # CUDA 13.2
ls /usr/lib/aarch64-linux-gnu/libnvinfer.so.10        # TensorRT 10.x runtime
apt-cache policy libopencv-dev | grep Candidate       # should be 4.8.0 (NVIDIA repo)
dpkg-query -W -f='${Version}' nvidia-l4t-gstreamer    # should be 39.2.0-20260601141651
ls /usr/src/jetson_sipl_api                           # SIPL dev headers (provided by the
                # "SIPL" component of the JetPack BSP/SDK Manager; the SENSING
                # camera driver does not install it. If missing, get the matching
                # header package from the vendor, otherwise the camera backend
                # cannot be compiled)

[RED LINE] Never run sudo apt install nvidia-jetpack (it upgrades
firmware and breaks the camera driver; not recoverable by software);
never run apt autoremove (it removes critical CUDA/TensorRT packages).
See Section 11.

------------------------------------------------------------------------
4.3 Build the SDK
------------------------------------------------------------------------
cd <delivery package directory>
rm -rf build && mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
# Build artifacts are in encrypt_output/:
#   stereo_depth_app      Depth application main program
#   device_finger         Device fingerprint tool (see 4.6)
#   sdepth_calib_convert  Calibration txt->json converter (see Section 7)
# Note: the depth inference core is the prebuilt closed library
#       lib/libsdepth.so; it is linked automatically at build time
#       and cannot be rebuilt from this package.

------------------------------------------------------------------------
4.4 Capture self-check (model not needed)
------------------------------------------------------------------------
cd <delivery package directory>/encrypt_output

# S36 on CAM2 (YUV/ICP path):
sudo ./stereo_depth_app --camera s36 --port cam2 --capture-test 3 --no-vis

# S56C on CAM0 (ISP path):
sudo ./stereo_depth_app --camera s56c --port cam0 --capture-test 3 --no-vis

# Expected output:
#   [Camera] s36 @ cam2 (YUV/ICP, 1920x1536@30, cfg=S36_2CH_1920X1536_30FPS)
#   [CaptureTest] frame 0 saved ... 3 pairs in total
# Generates capture_test_left_000~002.jpg / capture_test_right_000~002.jpg;
# open them to confirm both channels image correctly (visible parallax
# between left/right, consistent brightness).
# Note: when loading, the S36 path automatically corrects the ISX031 vc
#     input format to YUV422/UYVY (the SDK's standard query correction
#     for YUV modules, equivalent to the vendor nvsipl_camera
#     --enable-camera-hal behavior); this is normal.

------------------------------------------------------------------------
4.5 Place the calibration file
------------------------------------------------------------------------
Copy the calibration txt shipped with the camera (file name = module
serial number, e.g. I06150119.txt) into encrypt_output/.
The program reads the txt directly; no manual conversion is needed
(format details in Section 7).

------------------------------------------------------------------------
4.6 Generate the device fingerprint and send it to the vendor (customer)
------------------------------------------------------------------------
cd <delivery package directory>/encrypt_output
sudo ./device_finger --vendor-pub vendor_public.pem --out my_device.finger
# On success: Saved fingerprint file: my_device.finger
# Send my_device.finger to the vendor to generate the encrypted model
# bound to this device.
# Note: the fingerprint file is encrypted with the vendor public key
# and contains no plaintext hardware information.

------------------------------------------------------------------------
4.7 Vendor-side model encryption & delivery (vendor operation; customer
    awareness only)
------------------------------------------------------------------------
After receiving the fingerprint file, the vendor uses internal tools to
generate the encrypted model bound to that device and delivers the
my_device/ directory (containing sDepth.enc and
sDepth.license). The encryption and signing tools are internal vendor
assets and are not delivered with this SDK.

Customer places them on the board:
  encrypt_output/my_device/sDepth.enc
  encrypt_output/my_device/sDepth.license
  (vendor_public.pem is already included in the SDK's encrypt_output/)

------------------------------------------------------------------------
4.8 Run depth inference
------------------------------------------------------------------------
cd <delivery package directory>/encrypt_output

# S36 on CAM2 (calibration given directly as txt):
sudo ./stereo_depth_app --camera s36 --port cam2 \
  --calib H120UA-D60-H08300044.txt \
  --model my_device/sDepth.enc \
  --license my_device/sDepth.license \
  --pubkey vendor_public.pem

# S56C on CAM0:
sudo ./stereo_depth_app --camera s56c --port cam0 \
  --calib I06150119.txt \
  --model my_device/sDepth.enc \
  --license my_device/sDepth.license \
  --pubkey vendor_public.pem

# A pseudo-color depth window pops up: left half is the left-eye
# image, right half is the pseudo-color depth. The window shows FPS in
# the top-left corner; left-click any point in the image to display
# the depth (meters) at that point. Exit with Ctrl+C. For headless
# environments add --no-vis to output inference statistics only.

========================================================================
5. stereo_depth_app Command-line Parameters
========================================================================
Model & interface (added in v2.1, recommended):
  --camera <model>    s36 | s56c | s56
                      Reads config/cameras/<model>.json and
                      automatically determines the platform config,
                      data path (ISP/ICP), resolution, and frame rate
  --port <interface>  cam0 ~ cam7 (default cam2); mask table in
                      Section 3
  --backend <name>    sipl | v4l2 | argus | auto (default: the
                      profile's backend, see below). Temporarily
                      overrides the capture backend, for debugging or
                      forcing. Backend-to-driver-package mapping in
                      Section 3; auto probes at runtime:
                        detect /dev/video{port} (GMSL2x8 package) ->
                          v4l2(YUV) / argus(RAW) by module type
                        else detect
                        /usr/lib/nvsipl_drv/libnvuddf_<model>_*.so
                          (SIPL package) -> sipl
                        neither found -> error exit
  --calib <file>      Stereo calibration file, .txt / .json both
                      accepted (auto-detected by extension;
                      --calibration is an alias for compatibility
                      with older scripts)
  --resolution <WxH>  Capture resolution override. Only valid for ISP
                      modules (s56c/s56); S36 resolution is locked by
                      module firmware and config (1920x1536); passing
                      it is ignored with a warning

Model & licensing:
  --model <file>      Model file (.enc encrypted model)
  --license <file>    License file matching the encrypted model
  --pubkey <file>     Vendor public key (vendor_public.pem)

Run control:
  --no-vis            Disable the visualization window (headless)
  --capture-test <N>  Capture only N frame pairs and save JPEGs for
                      self-check (model not loaded)
  --fps <1-75>        Limit frame rate
  --force-uyvy        Force UYVY pixel format (special GMSL2 cases)
  -h / --help         Help

Low-level camera spec (legacy format, kept for compatibility; --camera
overrides it):
  --left-cam <spec>   Left camera backend (default sipl)
  --right-cam <spec>  Right camera backend (default sipl)
  spec format:
    sipl                                  all defaults (s56c + 0x0100,0x0000)
    sipl:<json path>:<config name>:<mask0,mask1>            ISP path
    sipl:<json path>:<config name>:<mask0,mask1>:yuv        ICP/YUV path
    A 5th trailing segment controls left/right routing (default swap):
      ...:swap     pipeline0 is the physical right eye, swap when
                   routing (s56c/s56)
      ...:noswap   pipeline0 is the physical left eye, no swap (s36)
    (with --camera this segment is issued automatically from the
    profile's swap_lr field)

  Example (S56C moved to CAM4, without --camera):
    sudo ./stereo_depth_app --model ... --calib ... --license ... \
      --pubkey vendor_public.pem \
      --left-cam  "sipl:../sipl/s56c.json:S56C_2CH:0x0001,0x0000" \
      --right-cam "sipl:../sipl/s56c.json:S56C_2CH:0x0001,0x0000"

========================================================================
6. Camera Profile Files (config/cameras/*.json)
========================================================================
One profile per module; --camera <model> loads
config/cameras/<model>.json. Profiles support // and /* */ comments
(delivered profiles contain full comments). Field reference (using
s36.json as an example):

  {
    "name": "s36",
    "output": "icp_yuv",                  // isp=RAW module via ISP0;
                                          // icp_yuv=YUV pass-through
    "backend": "auto",                    // capture backend:
                                          //   auto(recommended)|sipl|v4l2|argus
                                          //   auto probes at runtime
                                          //   (the two driver packages in
                                          //   Section 3, only one installed):
                                          //     /dev/videoN exists -> YUV uses
                                          //       v4l2 / RAW uses argus
                                          //     else check nvsipl_drv libs -> sipl
                                          //   can also be overridden by --backend
    "sipl_json": "sipl/s36.json",         // SIPL platform config (relative to SDK root)
    "sipl_config": "S36_2CH_1920X1536_30FPS",  // platform config name
    "capture_width": 1920,                // capture resolution
    "capture_height": 1536,
    "fps": 30,
    "calib_type": "pinhole",              // calibration model: fisheye | pinhole
    "swap_lr": false,                     // left/right routing (see below)
    "ports": {                            // interface->mask (Section 3 table)
      "cam0": ["0x0000","0x0001"], "cam2": ["0x0000","0x0100"], ...
    }
  }

swap_lr (added in v2.2):
  SIPL numbers frames by pipeline order, while "is pipeline0 the
  physical left or right eye" is determined by the module's internal
  GMSL wiring of its two sensors and may differ between models. The
  calibration file's CAM0/CAM1 are defined relative to the physical
  left/right, so the two must be aligned.
    true  = pipeline0 is the physical right eye; swap when routing
            (s56c / s56)
    false = pipeline0 is the physical left eye; no swap (s36, verified
            in testing)
  Typical symptom of a wrong setting: large-scale depth flickering
  (jumping between near-range saturation), some regions with clearly
  implausible depth values. See FAQ Q5 for how to determine it.

Adding/modifying models only requires editing a json in this
directory; no code changes needed.

========================================================================
7. Stereo Calibration Files (txt / json)
========================================================================
7.1 txt format (vendor's original shipping format; read directly)
------------------------------------------------------------------------
Line-by-line key=value, example below (# suffixes are annotations,
not part of the file):

  type= fisheye                # camera model: fisheye | pinhole
  imageWidth= 2560             # calibration resolution (must match actual capture)
  imageHeight= 1984
  CAM0_fx= 1181.9097474402     # CAM0_ prefix = one eye's intrinsics
  CAM0_fy= 1181.7077891728
  CAM0_cx= 1280.6271902124
  CAM0_cy= 994.4774012473
  CAM0_k1= ...                 # distortion: fisheye has k1~k4 (4 values)
  ...                          # pinhole has k1,k2,p1,p2,k3,k4,k5,k6 (8 values)
  CAM1_fx= ...                 # CAM1_ prefix = the other eye's intrinsics
  ...
  R[0,0]= 0.9999967673         # stereo rotation matrix 3x3, element by element
  ...
  T[0,0]= -60.1451030943       # stereo translation vector 3x1, unit: mm
  T[1,0]= ...
  T[2,0]= ...

  Compatibility notes:
  - Distortion coefficients are read by key name; line order does not
    matter (k4 appearing before k3 is fine).
  - Values may carry units/annotation suffixes (e.g. "2.6700 mm");
    parsing truncates automatically.
  - If the calibration resolution differs from the capture resolution,
    the SDK scales the intrinsics automatically.

7.2 txt -> json conversion tool (optional)
------------------------------------------------------------------------
The program reads txt directly; this tool is only for viewing /
archiving / interoperating with other tools:

  cd <delivery package directory>/encrypt_output
  ./sdepth_calib_convert I06150119.txt
      # Generates I06150119.json in the same directory and prints
      # model / resolution / baseline / coefficient count
  ./sdepth_calib_convert I06150119.txt -o calib_s56c.json   # custom output

  No sudo needed.

========================================================================
8. Secondary Development API (sdepth_api.h)
========================================================================
To integrate depth inference into your own program, call the closed
library's C interface directly:

  #include "sdepth_api.h"

  /* 1. Generate device fingerprint (equivalent to the device_finger tool) */
  sdepth_generate_fingerprint("vendor_public.pem", "my_device.finger");

  /* 2. Create the inference backend (loads encrypted model + license
   *    validation, fully automatic) */
  SDepthBackend* b = sdepth_backend_create(
      "my_device/sDepth.enc",
      "my_device/sDepth.license",
      "vendor_public.pem");

  /* 3. Query model input size; infer frame by frame */
  int w = sdepth_backend_input_width(b);
  int h = sdepth_backend_input_height(b);
  cv::Mat disparity;
  sdepth_backend_infer(b, &left_rect, &right_rect, &disparity);

  /* 4. Destroy when done */
  sdepth_backend_destroy(b);

Build & link (see examples/simple_pipeline.cpp for a complete sample):
  g++ your_app.cpp -I../include -I/usr/include/opencv4 \
      -L../lib -lsdepth -o your_app

Interface contract:
  - Input images: BGR 8UC3 of any size (auto-scaled / converted to RGB
    inside the library), or RGB 32FC3 (passed through when the size
    equals the model input size);
  - Output disparity: CV_32FC1, same size as input, already rescaled
    to the original image scale;
  - All failures are reported via return values; use
    sdepth_last_error() for the error description.

========================================================================
9. Encryption & Licensing Mechanism
========================================================================
- Device fingerprint: hardware identifiers of the device (MAC address,
  EMMC ID, GPU UUID, etc.), packed and encrypted with the vendor
  public key into a .finger file.
- Model encryption: the model content is encrypted with a random key
  into an .enc file; the decryption key is bound to the device
  fingerprint hash and signed with the vendor private key, producing
  the .license file.
- Runtime verification: when loading the .enc, the closed library
  verifies the license with vendor_public.pem and compares it against
  the local hardware fingerprint; only after all checks pass does it
  decrypt and build the TensorRT engine. Decryption and validation
  logic are encapsulated inside libsdepth.so; the SDK contains no such
  source code.
- The binding target is "the Thor device running the model", not the
  camera: changing cameras (including models) requires no re-encryption;
  changing the Thor board requires redoing steps 4.6/4.7.

========================================================================
10. FAQ
========================================================================
Q1: Error "Operation returning failure: opcode = 0x9 / Parse failed /
    Init failed. status: 10"
A1: Two possibilities: (1) not running with sudo (camera I2C nodes
    require root); (2) mask does not match the camera wiring; check
    --port against the actual CAM connector (mask table in Section 3).

Q2: Camera fails to start / No cameras available
A2: (1) Check whether a gpioset is holding GPIO lines (blocks
    CameraHAL); release and retry; (2) confirm install.sh was executed
    and firmware was not reflashed; (3) check /boot/dtb for leftover
    DTBs of other versions (occurs after mixed flashing; needs vendor
    assistance to clean up).

Q3: S36 reports NOTIF_ERROR_ICP_CAPTURE_FAILURE / frame rate is 0
A3: The S36 module firmware resolution does not match the config.
    Module firmware comes in 1920x1536 and 1920x1080 variants; the
    sipl_config in config/cameras/s36.json must match the firmware
    (S36_2CH_1920X1536_30FPS / S36_2CH_1920X1080_30FPS).

Q4: How is the S36 recognized as YUV422/UYVY (instead of RAW)?
A4: The standard SIPL query does not recognize the vendor json's yuv422
    notation (only uyvy/vyuy/yuyv/yvyu/rawXX). The SDK automatically
    corrects the vc input format to YUV422/UYVY at load time
    (equivalent to the vendor nvsipl_camera --enable-camera-hal
    internal behavior); without this correction the ICP buffer
    negotiation fails with GENERIC_ERROR.

Q5: Depth image flickers (colors jump frame by frame)
A5: Troubleshoot in this order:
    (1) Left/right swapped (most common). Typical symptom: depth
        jumps at large scale between "very near / 30m", some regions
        with clearly implausible values; the top-left pairing log
        [Sync] PTS diff is normal (<0.1ms).
        Cause: the pipeline-order-to-physical-left/right mapping is
        determined by the module's internal wiring and may differ
        between models (s56c needs swap, s36 does not).
        Fix: edit swap_lr in config/cameras/<model>.json
        (true<->false; loaded at runtime, rerun directly, no rebuild).
        Offline determination:
          a) sudo ./stereo_depth_app --camera <model> --port <port> \
               --calib <calibration>.txt --capture-test 3
          b) Feed the model with both left/right orders (the vendor
             can provide an offline script); the order whose disparity
             map is spatially coherent (small disparity far away,
             large nearby, smooth transitions) is correct.
    (2) Left/right not synchronized. Check the [Sync] PTS diff log:
        consistently >5ms means the pairing window missed; check for
        mixed modules of different types or inconsistent firmware.
    (3) The camera image itself flickers in brightness (not the depth
        image): on S36 the AE is inside the module; under fluorescent/
        LED flicker the two independent exposures may oscillate in
        brightness - sensor behavior, not an SDK issue; s56c/s56 use
        ISP0 sensor-group synchronized exposure and have no such issue.
    (4) If it appeared only after changing the CAM port, verify --port
        against the actual wiring (mask table in Section 3); a wrong
        mask may capture a stream from an adjacent port.

Q5b: Depth is 30m (range cap) everywhere
A5b: Disparity is all invalid (<=0) or the depth conversion is scaled
     up. Troubleshoot:
     (1) First check swap_lr per Q5(1) (swapped left/right causes
         large-scale negative/invalid disparity, clamped to the range
         cap);
     (2) Confirm --calib is the module's own txt (file name = SN); do
         not use another module's calibration;
     (3) When making your own json calibration, note that
         translation_vector is in meters (m) while the vendor txt's T
         is in millimeters (mm) - the SDK divides by 1000
         automatically for txt but does no conversion for json; a
         wrong unit scales depth by 1000x (symptom: 30m everywhere).
     (4) The startup log should show baseline of about 0.06 (meters);
         if it shows about 60, the calibration unit is mm used
         directly; fix per (3).

Q6: Encrypted model fails to load / license verification fails
A6: (1) .enc/.license are bound to one device; do not copy from
        another board;
    (2) confirm file paths match --model/--license/--pubkey;
    (3) if it reports TensorRT engine deserialization failure, confirm
        the firmware is JetPack 7.2 / L4T R39.2 and TensorRT is the
        official 10.x (engine bound to the TRT major version;
        mistakenly installing standalone TensorRT 11 causes load
        failure);
    (4) otherwise contact the vendor to regenerate.

Q7: Runtime error: libsdepth.so not found
A7: The closed library is in lib/ at the SDK root; the program looks
    it up via the relative path $ORIGIN/../lib. Keep the directory
    structure intact (lib/ and encrypt_output/ must be sibling
    directories); do not move the executable alone. If you must move
    it, set LD_LIBRARY_PATH to include the lib directory.

Q8: Chinese characters garbled in build logs
A8: An encoding display difference only; the build is unaffected.
    Check for "Configuring done" / "Built target stereo_depth_app".

Q9: Runtime permission error
A9: Must run with sudo (camera nodes and GPU devices require root).

Q10: load_modules.sh reports "Invalid parameters" / no /dev/video* /
     argus reports "No cameras available" (GMSL2x8 custom driver
     environment)
A10: Kernel and camera driver .ko version mismatch (symbol CRC
     mismatch). Run journalctl -k | grep sgcam and you will see many:
       sgcam_gmsl2: Unknown symbol tegracam_xxx (err -22)
       sgcam_gmsl2: disagrees about version of symbol ...
     Cause: sudo apt-get install nvidia-l4t-gstreamer was executed
     without a version pin (or any package that upgrades
     nvidia-l4t-kernel), moving the kernel from 39.2.0 to 39.2.1, while
     the GMSL2x8 custom .ko modules were built against 39.2.0; the
     symbol CRCs no longer match. Note: the package itself is required
     for the Argus route (s56c/s56) - it is not forbidden, it must
     simply be pinned to the 39.2.0-matching 39.2.0-20260601141651 and
     held (setup_env.sh already installs it pinned; see Section 11,
     item 10).
     Fix: rerun the vendor install.sh to restore the matching kernel,
     then reboot:
       cd <GMSL2x8 driver package directory> && sudo ./install.sh && sudo reboot
     After reboot, sudo ./load_modules.sh restores /dev/video*.
     Prevention: see Section 11, item 10.

     Further (deeper residue when install.sh's recovery is
     incomplete): if after install.sh + reboot + load_modules.sh the
     /dev/video* nodes are back but all 8 cameras report
       tegra-nvcsi ... Failed to create device link (0x180) with 9-00xx
     with zero captured frames (stereo_depth_app --capture-test shows
     "0/3 frame pairs saved") and argus reports NvBufSurfaceFromFd
     Failed, the kernel module tree / device tree has been polluted
     beyond what install.sh (it only restores /boot/Image + two .ko
     modules tegra-camera.ko/nvhost-nvcsi.ko + dtbo) can repair. The
     NVCSI media graph cannot establish the CSI data channel (note:
     v4l2-ctl --list-formats still lists formats because they come
     from the device tree, not the live sensor - do not be misled).
     This case requires reflashing to JetPack 7.2 / L4T R39.2 and
     reinstalling the GMSL2x8 driver package, generating the overlay
     with generate per the actual wiring; do not keep rerunning
     install.sh.

Q11: How to choose between the two driver packages / backend=auto
     misdetects / argus backend dual-eye RAW fails to start
A11: Two points:
     (1) The two driver packages (SIPL / GMSL2x8, see Section 3) are
         mutually exclusive; only one can be installed at a time.
         backend=auto probes at runtime (first /dev/videoN as a strong
         signal that the GMSL2x8 package is active, then nvsipl_drv
         libs) and picks the backend automatically. If detection
         disagrees with expectations, it is usually "package just
         swapped without reboot / leftover old .ko or old libs";
         first confirm the driver package was installed per Section
         4.1 and the board rebooted, or force with
         --backend sipl|v4l2|argus to isolate.
     (2) argus backend (RAW modules s56c/s56 on the GMSL2x8 package):
         a single nvargus-daemon Argus session cannot carry two
         independent nvarguscamerasrc streams simultaneously, causing
         left-eye timeout, right-eye at 30fps, and eventually
         NvBufSurfaceFromFd Failed. RAW single-eye debugging may use
         argus (or libargus direct capture per Section 11 item 10);
         but the official solution for dual-RAW stereo is the SIPL
         backend (ISP path). So for s56c/s56 stereo/depth use the
         SIPL package + backend=sipl; the argus/v4l2 route mainly
         serves the YUV module s36 (V4L2) and RAW single-eye bring-up
         debugging.

========================================================================
11. Critical Warnings (Red Lines)
========================================================================
1) NEVER sudo apt install nvidia-jetpack - it triggers a UEFI firmware
   upgrade and overwrites the SENSING camera driver; after a QSPI
   upgrade there is no software downgrade.
2) NEVER apt autoremove - it removes critical CUDA/TensorRT packages.
3) Firmware must remain JetPack 7.2 / L4T R39.2; do not flash other
   versions (mixed flashing causes DTB/ISP protocol mismatch and
   camera init failure).
4) .enc / .license / .finger are device-bound; do not copy across
   boards.
5) Calibration files are bound one-to-one to the module SN (the file
   name is the SN); do not mix with other modules.
6) Never hot-plug the GMSL2 cable while the camera is powered.
7) For long runs, use sudo jetson_clocks and ensure cooling.
8) lib/libsdepth.so is a prebuilt deliverable; do not delete, rename,
   or otherwise modify it (if damaged, inference fails and it cannot
   be rebuilt locally).
9) The S36 can only connect to CAM0/CAM2/CAM4/CAM6; the S36 sipl_config
   must match the module firmware resolution, otherwise the frame rate
   is 0 after streaming starts.
10) In a GMSL2x8 custom driver environment (the Argus route for
    s56c/s56 requires nvidia-l4t-gstreamer), nvidia-l4t-gstreamer is
    allowed ONLY at 39.2.0-20260601141651. NEVER run sudo apt-get
    install nvidia-l4t-gstreamer without a version pin (or any package
    whose dependency chain moves nvidia-l4t-kernel from 39.2.0 to
    39.2.1); the kernel symbol CRC change will make sgcam-gmsl2.ko fail
    to load ("Invalid parameters"/Unknown symbol; see FAQ Q10). The
    correct procedure (already built into setup_env.sh):
      sudo apt-get install -y nvidia-l4t-gstreamer=39.2.0-20260601141651
      sudo apt-mark hold nvidia-l4t-gstreamer
    Even if install.sh afterwards restores /dev/video*, deeper residue
    may remain: all cameras report "Failed to create device link
    (0x180)" and capture no frames; only reflashing to L4T R39.2 fixes
    it (see FAQ Q10 addendum). The official way to bring up RAW cameras
    is libargus (argus_camera, from nvidia-l4t-jetson-multimedia-api),
    which does not depend on GStreamer plugins and does not trigger
    kernel upgrades.
11) When verifying SIPL <-> GMSL2x8 framework switching on the same
    machine: the GMSL2x8 package's install.sh overwrites /boot/Image
    with the custom kernel and replaces dtbo/custom .ko/nito and other
    system files; after switching back to SIPL the system is in a
    mixed-driver state and camera capture breaks (0 frames / Platform
    configuration failed / NvSciBuf allocation failure); reinstalling
    the kernel alone does not recover. The reliable approach is to
    reflash JetPack 7.2 / L4T R39.2 and then install exactly one
    target driver package. Root cause: the two driver packages target
    different kernels and system configurations; do not alternate them
    repeatedly on the same device. Customer environments install only
    one package and are unaffected.

========================================================================
Technical support: please contact your device/camera supplier.
========================================================================
