# Minimal SDK Examples

Each CPP file demonstrates one task independently, using only the public sgMIX
API and the C++17 standard library. SDK configuration must match the framework,
and headers and runtime libraries must come from the same SDK version.

| Source | Executable and arguments | Behavior |
| --- | --- | --- |
| `v4l2_capture.cpp` | `basic_v4l2_capture /dev/video6` | Read 30 frames; UYVY is the default format |
| `sipl_capture.cpp` | `basic_sipl_capture 7` | Read 30 frames from a single-sensor module on CAM7 |
| `sipl_stereo_capture.cpp` | `basic_sipl_stereo_capture 4` | Read 30 frames per channel from channels 0/1 on CAM4, using a separate thread for each |
| `v4l2_read_sn.cpp` | `basic_v4l2_read_sn /dev/video6` | Read the serial number |
| `sipl_read_calibration.cpp` | `basic_sipl_read_calibration 4` | Read intrinsics; also print right-camera intrinsics and stereo extrinsics when available |
| `v4l2_correct_capture.cpp` | `basic_v4l2_correct_capture /dev/video6` | Read 30 distortion-corrected NV12 frames |
| `v4l2_image_flip.cpp` | `basic_v4l2_image_flip /dev/video6 1 0` | Set vertical flip and horizontal mirror, capture 30 frames, then restore the initial states |
| `sipl_image_flip.cpp` | `basic_sipl_image_flip 7 1 0` | The same control flow on an enabled single-sensor SIPL CAM |

Load the driver first, then run `sudo /opt/sgMIX/bin/sgmix-config.py`.
V4L2 loads the camera model from the saved video-node mapping. SIPL loads the
JSON file, configuration name and masks from the saved configuration. SIPL
arguments use zero-based Driver CAM numbers, and the selected links must be
enabled.

Build and run on the target board:

```bash
cmake -S /opt/sgMIX -B "$HOME/sgmix-basic-build" -DSGMIX_BUILD_FULL_SAMPLES=OFF
cmake --build "$HOME/sgmix-basic-build" -j2
cd "$HOME/sgmix-basic-build"
sudo ./basic_sipl_capture 7
sudo ./basic_sipl_stereo_capture 4
```

You can also compile a single source file directly, for example:

```bash
g++ -std=c++17 /opt/sgMIX/sample/basic/sipl_capture.cpp -o basic_sipl_capture \
  -I/opt/sgMIX/include -L/opt/sgMIX/lib -Wl,-rpath,/opt/sgMIX/lib -lsgmix -pthread
```

The capture examples directly demonstrate
`SG_Init → SG_StartCapture → SG_GetFrame → SG_StopCapture`.
Process `frame.data` at the comment in the capture loop. Interpret the pixels
using `pixelFormat`, dimensions and `planes`. To use an image across threads or
after the next frame read, copy or move the Frame to retain independent ownership.

These examples do not open preview windows or save files. The distortion
correction example requires a matching VPI runtime and valid monocular
calibration; frame dimensions must match the calibration dimensions. Stereo
channels follow Query order. Independent threads do not automatically pair
frames by exposure time. Use `camera_frame_capture` in the parent directory for
preview, optional arguments, metadata and frame-count controls.

## Flip and Mirror Controls

The two control examples use saved configuration and positional arguments:

```bash
# FLIP MIRROR: 0 disables, 1 enables (vertical flip, horizontal mirror).
./basic_v4l2_image_flip /dev/video6 1 0
./basic_sipl_image_flip 7 1 0
```

Each example initializes, starts capture and receives one preparation frame before
reading any controls. It saves all initial values, sets and reads back each requested
value, prints 30 subsequent frames, restores attempted settings in reverse order,
and stops capture. A failed Set may have changed the device, so that setting is also
restored. Runtime errors and SIGINT/SIGTERM trigger the same cleanup; restoration
and stop failures are printed separately and return a nonzero exit code.

Readback checks that the public API's boolean state matches the requested value.
Restoration also uses public Get/Set values, rather than a raw
register backup. Successful readback and frame capture do not verify the visual
effect. These examples open no preview windows and save no images.

Exit codes: 0 success, 1 runtime/control/cleanup failure, 2 usage or argument error.
