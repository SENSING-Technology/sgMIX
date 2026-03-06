/*
 * Copyright 2019-2024 NVIDIA Corporation. All rights reserved.
 *
 * NOTICE TO LICENSEE:
 *
 * This source code and/or documentation ("Licensed Deliverables") are
 * subject to NVIDIA intellectual property rights under U.S. and
 * international Copyright laws.
 *
 * These Licensed Deliverables contained herein is PROPRIETARY and
 * CONFIDENTIAL to NVIDIA and is being provided under the terms and
 * conditions of a form of NVIDIA software license agreement by and
 * between NVIDIA and Licensee ("License Agreement") or electronically
 * accepted by Licensee. Notwithstanding any terms or conditions to
 * the contrary in the License Agreement, reproduction or disclosure
 * of the Licensed Deliverables to any third party without the express
 * written consent of NVIDIA is prohibited.
 *
 * NOTWITHSTANDING ANY TERMS OR CONDITIONS TO THE CONTRARY IN THE
 * LICENSE AGREEMENT, NVIDIA MAKES NO REPRESENTATION ABOUT THE
 * SUITABILITY OF THESE LICENSED DELIVERABLES FOR ANY PURPOSE. IT IS
 * PROVIDED "AS IS" WITHOUT EXPRESS OR IMPLIED WARRANTY OF ANY KIND.
 * NVIDIA DISCLAIMS ALL WARRANTIES WITH REGARD TO THESE LICENSED
 * DELIVERABLES, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY,
 * NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE.
 * NOTWITHSTANDING ANY TERMS OR CONDITIONS TO THE CONTRARY IN THE
 * LICENSE AGREEMENT, IN NO EVENT SHALL NVIDIA BE LIABLE FOR ANY
 * SPECIAL, INDIRECT, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, OR ANY
 * DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS
 * ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THESE LICENSED DELIVERABLES.
 *
 * U.S. Government End Users. These Licensed Deliverables are a
 * "commercial item" as that term is defined at 48 C.F.R. 2.101 (OCT
 * 1995), consisting of "commercial computer software" and "commercial
 * computer software documentation" as such terms are used in 48
 * C.F.R. 12.212 (SEPT 1995) and is provided to the U.S. Government
 * only as a commercial end item. Consistent with 48 C.F.R.12.212 and
 * 48 C.F.R. 227.7202-1 through 227.7202-4 (JUNE 1995), all
 * U.S. Government End Users acquire the Licensed Deliverables with
 * only those rights set forth herein.
 *
 * Any use of the Licensed Deliverables in individual and commercial
 * software must include, in the user documentation and internal
 * comments to the code, the above Disclaimer and U.S. Government End
 * Users Notice.
 */

/**
 * @file Types.h
 *
 * Defines all types needed for programming with VPI.
 */

#ifndef NV_VPI_TYPES_H
#define NV_VPI_TYPES_H

#include "ImageFormat.h"
#include "Interpolation.h"
#include "PixelType.h"
#include "Status.h"
#include "Version.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Defines common types used by several components.
 *
 * @defgroup VPI_Types Common Types
 * @ingroup VPI_API_Misc
 */

/** @name Memory creation flags.
 * These flags can be used when creating images, pyramids or arrays.
 *
 * @ingroup VPI_Types
 * @{
 */

/** VPI Backend types.
 *
 * @ingroup VPI_Stream
 *
 */
typedef enum
{
    VPI_BACKEND_CPU     = (1ULL << 0), /**< CPU backend. */
    VPI_BACKEND_CUDA    = (1ULL << 1), /**< CUDA backend. */
    VPI_BACKEND_PVA     = (1ULL << 2), /**< PVA backend. */
    VPI_BACKEND_VIC     = (1ULL << 3), /**< VIC backend. */
    VPI_BACKEND_NVENC   = (1ULL << 4), /**< NVENC backend. */
    VPI_BACKEND_OFA     = (1ULL << 5), /**< OFA backend. */
    VPI_BACKEND_INVALID = (1ULL << 15) /**< Invalid backend. */
} VPIBackend;

/** Tegra-only backends. */
#define VPI_BACKEND_TEGRA (VPI_BACKEND_VIC | VPI_BACKEND_NVENC | VPI_BACKEND_OFA)

#define VPI_BACKEND_MASK ((VPI_BACKEND_INVALID << 1) - 1)

/** All backends. */
#define VPI_BACKEND_ALL \
    (VPI_BACKEND_CPU | VPI_BACKEND_CUDA | VPI_BACKEND_PVA | VPI_BACKEND_VIC | VPI_BACKEND_NVENC | VPI_BACKEND_OFA)

/** @} */

/** @anchor common_flags @name Common object flags.
 *
 * These flags can be passed to the creation function of the following objects:
 * - \ref VPIImage
 * - \ref VPIPyramid
 * - \ref VPIArray
 * - \ref VPIContext
 * - \ref VPIStream
 * - \ref VPIEvent
 *
 * They might change some of the characteristics of the created object to conform
 * to some expected behavior.
 */
/**@{*/

/**
 * Specifies that the memory will be accessed by only one stream at a time.
 *
 * By default memories are created up so that at multiple streams can access it
 * for reading simultaneously. Doing so might incur in increased system
 * resources utilization. By specifying this flag when creating a VPIImage,
 * VPIArray or VPIPyramid, when it's known that they won't be used concurrently
 * by different streams, better resource allocation is achieved.
 */
#define VPI_EXCLUSIVE_STREAM_ACCESS (1ULL << 16)

/**
 * Flag to restrict memory resources usage.
 *
 * When passed as flags to certain VPI functions, it will instruct them to use
 * less memory resources, in exchange of limiting their functionality.  Please
 * refer to documentation of these functions for further explanation on the
 * trade-offs involved.
 */
#define VPI_RESTRICT_MEM_USAGE (1ULL << 17)

/**
 * Require creation of requested backends.
 *
 * With this flag set, the creation functions will require that the given
 * backends are created. If any fail, the creation function will return an
 * error.
 * If not set, it'll try to enable the given backends, but disable the ones that are
 * incompatible with the object parameters or not enabled in current context.
 * @note When applied to a context, this flag isn't propagated to objects created
 *       when this context is active.
 */
#define VPI_REQUIRE_BACKENDS (1ULL << 18)

/**@}*/

/**
 * Parallel task function pointer type.
 * @ingroup VPI_Context
 */
typedef void (*VPIParallelTask)(int taskId, int threadId, void *vpiData);

/**
 * Parallel for callback function pointer type. A serial (reference) implementation of this
 * function might looks as follows:
 *
 * @code
 * void parallel_for(VPIParallelTask task, int taskCount, void *vpiData, void *userData)
 * {
 *    for (int i = 0; i < taskCount; ++i)
 *    {
 *      task(i, 0, vpiData);
 *    }
 * }
 * @endcode
 *
 * Parallel implementation should have equivalent behavior; that is, run all tasks
 * between 0 and taskCount-1 and block the calling thread until all tasks have been
 * completed. `threadId` parameter should be between 0 and maxThreads-1 (\see vpiContextSetParallelFor).
 * Implementations that have `maxThreads` set to zero can pass an arbitrary `threadId` value to
 * task functions.
 *
 * @ingroup VPI_Context
 */
typedef void (*VPIParallelForCallback)(VPIParallelTask task, int taskCount, void *vpiData, void *userData);

/**
 * Stores the ParallelFor configuration.
 *
 * @ingroup VPI_Context
 */
typedef struct
{
    /**
     * The maximum number of threads used by the parallel_for implementation code. Has to be larger than 0.
     * Setting the number to N means that the parallel for implementation will call task functors with task id
     * between 0 and N-1. Calling task functors with thread id outside of this range is not legal and will result
     * in undefined behavior.
     */
    int maxThreads;

    /**
     * A pointer to the parallel_for implementation. If null is passed, the context will fallback to the default
     * internal parallel_for implementation. parallel_for implementation is required to be thread-safe.
     * Internal API implementation might call this function from different threads at the same time.
     */
    VPIParallelForCallback callback;

    /**
     * A user defined opaque pointer passed to callback function unaltered.
     */
    void *userData;
} VPIParallelForConfig;

/**
 * A handle to OS-specific thread handle.
 *
 * @ingroup VPI_Context
 */
typedef void *VPINativeThreadHandle;

/**
 * A handle to an array.
 * @ingroup VPI_Array
 */
typedef struct VPIArrayImpl *VPIArray;

/**
 * A handle to a context.
 * @ingroup VPI_Context
 */
typedef struct VPIContextImpl *VPIContext;

/**
 * A handle to an event.
 * @ingroup VPI_Event
 */
typedef struct VPIEventImpl *VPIEvent;

/**
 * A handle to a stream.
 * @ingroup VPI_Stream
 */
typedef struct VPIStreamImpl *VPIStream;

/**
 * A handle to an image.
 * @ingroup VPI_Image
 */
typedef struct VPIImageImpl *VPIImage;

/**
 * A handle to an image pyramid.
 * @ingroup VPI_Pyramid
 */
typedef struct VPIPyramidImpl *VPIPyramid;

/**
 * A handle to an algorithm payload.
 * @ingroup VPI_Payload
 */
typedef struct VPIPayloadImpl *VPIPayload;

/**
 * Image border extension specify how pixel values outside of the image domain should be
 * constructed.
 *
 * @ingroup VPI_Types
 */
typedef enum
{
    VPI_BORDER_ZERO = 0, /**< All pixels outside the image are considered to be zero. */
    VPI_BORDER_CLAMP,    /**< Border pixels are repeated indefinitely. */
    VPI_BORDER_REFLECT,  /**< edcba|abcde|edcba */
    VPI_BORDER_MIRROR,   /**< dedcb|abcde|dcbab */
    VPI_BORDER_LIMITED,  /**< Consider image as limited to not access outside pixels. */
    VPI_BORDER_INVALID,  /**< Invalid border. */
} VPIBorderExtension;

/**
 * Policy used when converting between image types.
 * @ingroup VPI_ConvertImageFormat
 */
typedef enum
{
    /** Clamps input to output's type range. Overflows
        and underflows are mapped to the output type's
        maximum and minimum representable value,
        respectively. When output type is floating point,
        clamp behaves like cast. */
    VPI_CONVERSION_CLAMP = 0,

    /** Casts input to the output type. Overflows and
        underflows are handled as per C specification,
        including situations of undefined behavior. */
    VPI_CONVERSION_CAST,

    /** Invalid conversion. */
    VPI_CONVERSION_INVALID = 255,
} VPIConversionPolicy;

/**
 * Stores a float32 pyramidal-based keypoint coordinate
 * The coordinate includes the (x, y) position in an image inside a pyramid, and (octave and layer) selects a
 * specific image on that pyramid.  Pyramids are used to approximate the scale space with one or more resolution
 * levels, also known as octaves, where each octave may have one or more smoothing layers.  One example is in the
 * \ref algo_orb_feature_detector "ORB algorithm", where its input is a pyramid, representing the scale space of
 * its base image, that normally has several octaves, where each octave has a single layer.  Octaves are defined as
 * levels of dyadic-image pyramids, i.e. a \ref VPIPyramid with scale=0.5.
 *
 * @ingroup VPI_Types
 */
typedef struct
{
    float x;      /**< Keypoint's x coordinate. */
    float y;      /**< Keypoint's y coordinate. */
    float octave; /**< Pyramid octave (or pyramid level) where the keypoint is located. */
    float layer;  /**< Pyramid layer within the octave where the keypoint is located. */
} VPIPyramidalKeypointF32;

/**
 * Stores a float32 keypoint coordinate
 * The coordinate is relative to the top-left corner of an image.
 *
 * @ingroup VPI_Types
 */
typedef struct
{
    float x; /**< Keypoint's x coordinate. */
    float y; /**< Keypoint's y coordinate. */
} VPIKeypointF32;

/**
 * Stores a U32 keypoint coordinate
 * The coordinate is relative to the top-left corner of an image.
 *
 * @ingroup VPI_Types
 */
typedef struct
{
    uint32_t x; /**< Keypoint's x coordinate. */
    uint32_t y; /**< Keypoint's y coordinate. */
} VPIKeypointU32;

/**
 * Defines different types of corner scores
 */
typedef enum
{
    VPI_CORNER_SCORE_HARRIS = 0, /**< Use Harris response corner scores. */
    VPI_CORNER_SCORE_FAST   = 1  /**< Use FAST corner scores. */
} VPICornerScore;

/**
 * Length of Brief Descriptor Array
 */
#define VPI_BRIEF_DESCRIPTOR_ARRAY_LENGTH 32

/**
 * Length of Brief Descriptor in bits
 */
#define VPI_BRIEF_DESCRIPTOR_BIT_WIDTH (VPI_BRIEF_DESCRIPTOR_ARRAY_LENGTH * 8)

/**
 * Stores a BRIEF Descriptor
 */
typedef struct
{
    /** Description vector of a BRIEF descriptor */
    uint8_t data[VPI_BRIEF_DESCRIPTOR_ARRAY_LENGTH];
} VPIBriefDescriptor;

/**
 * Stores a generic 2D homography transform.
 * When only scale and translation transformation is needed,
 * these parameters must be arranged in the matrix as follows:
 *
 * \f[ \begin{bmatrix}
 *     s_x &   0 & p_x \\
 *       0 & s_y & p_y \\
 *       0 &   0 &   1
 *   \end{bmatrix}
 * \f]
 *
 * Scaling \f$(s_x,s_y)\f$ is relative to the center of the patch,
 * position \f$(p_x,p_y)\f$ is relative to the top-left of the image.
 *
 * In the general case, given an homogeneous 2D point \f$P(x,y,1)\f$
 * and the matrix \f$M^{3x3}\f$, the Euclidean 2D point \f$O(x,y)\f$ is defined as
 * \f{align}{
 *  T &= M \cdot P \\
 *  O &= (T_x/T_z, T_y/T_z)
 * \f}
 *
 * @ingroup VPI_Types
 */
typedef struct
{
    float mat3[3][3]; /**< 3x3 homogeneous matrix that defines the homography. */
} VPIHomographyTransform2D;

/**
 * Stores a generic 2D bounding box.
 * Although this structure can store a 2D bounding box transformed
 * by any homography, sometimes it stores an axis-aligned bounding box.
 * To retrieve it, do the following:
 *
 * @code
 *   float x = xform.mat3[0][2];
 *   float y = xform.mat3[1][2];
 *   float w = width  * xform.mat3[0][0];
 *   float h = height * xform.mat3[1][1];
 * @endcode
 *
 * @ingroup VPI_Types
 */
typedef struct
{
    VPIHomographyTransform2D xform; /**< Defines the bounding box top left corner and its homography. */
    float width,                    /**< Bounding box width. */
        height;                     /**< Bounding box height. */
} VPIBoundingBox;

/**
 * Stores an axis-aligned 32-bit floating point 2D bounding box.
 *
 * @ingroup VPI_Types
 */
typedef struct
{
    float left;   /**< Defines the bounding box's left corner's coordinate. */
    float top;    /**< Defines the bounding box's top corner's coordinate. */
    float width;  /**< Bounding box width. */
    float height; /**< Bounding box height. */
} VPIAxisAlignedBoundingBoxF32;

/** Object tracking state. */
typedef enum
{
    /** Object isn't being tracked anymore.
     * This state is either set by the user when this object doesn't need to be tracked anymore,
     * or by the tracking function given some tracking confidence criteria.
     * Existing objects with state \ref VPI_TRACKING_STATE_LOST can be reused
     * for new objects.
     * The valid transitions from this state are:
     * - \ref VPI_TRACKING_STATE_LOST
     * - \ref VPI_TRACKING_STATE_NEW. */
    VPI_TRACKING_STATE_LOST = 0,

    /** New object to be tracked.
     * This state is defined by the user for new objects to be tracked.
     * The valid transitions from this state are:
     * - \ref VPI_TRACKING_STATE_LOST
     * - \ref VPI_TRACKING_STATE_TRACKED */
    VPI_TRACKING_STATE_NEW,

    /** Object is being tracked with high confidence.
     * The object is being localized in the input frames and because there's a
     * high confidence (established externally) that the bounding box is accurate,
     * its internal model can be updated with the currently estimated position.
     * The valid transitions from this state are:
     * - \ref VPI_TRACKING_STATE_LOST
     * - \ref VPI_TRACKING_STATE_TRACKED.
     * - \ref VPI_TRACKING_STATE_SHADOW_TRACKED. */
    VPI_TRACKING_STATE_TRACKED,

    /** Object is being tracked with low confidence.
     * The object localized in the frame, but its model is not being updated because
     * there's a low confidence on its bounding box location/size.
     * The valid transitions from this state are:
     * - \ref VPI_TRACKING_STATE_LOST
     * - \ref VPI_TRACKING_STATE_TRACKED.
     * - \ref VPI_TRACKING_STATE_SHADOW_TRACKED. */
    VPI_TRACKING_STATE_SHADOW_TRACKED,
} VPITrackingState;

/**
 * Stores a bounding box that is being tracked by \ref algo_klt_tracker "KLT Tracker".
 * @ingroup VPI_KLTFeatureTracker
 */
typedef struct
{
    /** Bounding box being tracked. */
    VPIBoundingBox bbox;

    /** Tracking status of this bounding box.
     * Accepted values:
     * - `1` tracking information is invalid and shouldn't be relied upon.
     * - `0` tracking information is valid.
     */
    int8_t trackingStatus;

    /** Status of the template related to this bounding box.
     * Accepted values:
     * - `1` template needs updating.
     * - `0` existing template still can be used for tracking, it doesn't need to be updated.
     */
    int8_t templateStatus;

    /*! @{ */
    /** Reserved for future use. */
    uint8_t reserved1, reserved2;
    /*! @} */
} VPIKLTTrackedBoundingBox;

/** Stores information about an object tracked by \ref algo_dcf_tracker "DCF Tracker".
 * @ingroup VPI_DCFTracker
 */
typedef struct
{
    /** Bounding box around the object being tracked. */
    VPIAxisAlignedBoundingBoxF32 bbox;

    /** Tracking status of this bounding box. */
    VPITrackingState state;

    /** Index of the input sequence where the tracked object is in.
     *  If seqIndex < 0, the sequence is assumed to be disabled, i.e.,
     *  object won't be processed. */
    int32_t seqIndex;

    /** Learning rate for DCF filter in exponential moving average.
     * + Must be between 0.0 and 1.0, inclusive. */
    float filterLR;

    /** Learning rate for weights of different feature channels in DCF.
     * + Must be between 0.0 and 1.0, inclusive. */
    float filterChannelWeightsLR;

    /** Pointer to some unspecified user data.
     * This is used to associated some externa data to the tracked object.
     * DCFTracker won't interpret it in any way, it'll just copy it over. */
    void *userData;
} VPIDCFTrackedBoundingBox;

/**
 * Stores the statistics of an image.
 * @ingroup VPI_Types
 */

typedef struct
{
    /** Per channel mean */
    float mean[4];

    /** Per-channel covariance matrix, the square root of the diagonal is the standard deviation.*/
    float covariance[4][4];

    /** Total pixel count*/
    int32_t pixelCount;

    /** Per channel sum*/
    float sum[4];

} VPIStats;

/**
 * Maximum number of matches per descriptor
 */
#define VPI_MAX_MATCHES_PER_DESCRIPTOR (3)

/**
 * Stores the matches between 2 descriptors.
 * @ingroup VPI_Types
 */

typedef struct
{
    /** Match reference index */
    int refIndex[VPI_MAX_MATCHES_PER_DESCRIPTOR];

    /** Distance between query and reference descriptor */
    float distance[VPI_MAX_MATCHES_PER_DESCRIPTOR];

} VPIMatches;

/**
 * Stores the geometric information of a rectangle.
 * @ingroup VPI_Types
 */
typedef struct VPIRectangleIRec
{
    int32_t x, y;          /*< Rectangle origin position (x, y). */
    int32_t width, height; /*< Rectangle size dimensions (width, height). */
} VPIRectangleI;

/**
 * Defines the quality of the optical flow algorithm
 * @ingroup VPI_Types
 */
typedef enum
{
    /** Fast but low quality optical flow implementation. */
    VPI_OPTICAL_FLOW_QUALITY_LOW,

    /** Speed and quality in between of \ref VPI_OPTICAL_FLOW_QUALITY_LOW and \ref VPI_OPTICAL_FLOW_QUALITY_HIGH. */
    VPI_OPTICAL_FLOW_QUALITY_MEDIUM,

    /** Slow but high quality optical flow implementation. */
    VPI_OPTICAL_FLOW_QUALITY_HIGH
} VPIOpticalFlowQuality;

/**
 * Defines the lock modes used by memory lock functions.
 * @ingroup VPI_Types
 */
typedef enum
{
    /** Lock memory only for reading.
     * Writing to the memory when locking for reading leads to undefined behavior. */
    VPI_LOCK_READ = 1,

    /** Lock memory only for writing.
     * Reading to the memory when locking for reading leads to undefined behavior.
     * It is expected that the whole memory is written to. If there are regions not
     * written, it might not be updated correctly during unlock. In this case, it's
     * better to use VPI_LOCK_READ_WRITE.
     *
     * It might be slightly efficient to lock only for writing, specially when
     * performing non-shared memory mapping.
     */
    VPI_LOCK_WRITE = 2,

    /** Lock memory for reading and writing. */
    VPI_LOCK_READ_WRITE = 3
} VPILockMode;

/**
 * Defines the termination criteria macros.
 * @ingroup VPI_Types
 */
#define VPI_TERMINATION_CRITERIA_ITERATIONS (1u << 0) /**< Termination based on maximum number of iterations. */
#define VPI_TERMINATION_CRITERIA_EPSILON (1u << 1)    /**< Termination based on maximum error (epsilon). */

/** Camera intrinsic matrix.
 * \f{bmatrix}{
 *    f_x & s & c_x \\
 *    0 & f_y & c_y
 * \f}
 *
 * Where:
 * - \f$f_x, f_y\f$: focal length in pixels.
 * - \f$s\f$: skew, used to model slanted pixels.
 * - \f$c_x, c_y\f$: principal point.
 *
 * @ingroup VPI_LDC
 *
 */
typedef float VPICameraIntrinsic[2][3];

/** Camera extrinsic matrix.
 * \f{bmatrix}{
 *   R_{3\times3} & T_{3\times1}
 * \f}
 *
 * Where:
 * \f$R_{3\times3}\f$: Rotation matrix.
 * \f$T_{3\times1}\f$: 3D position of the origin of world coordinate system expressed in coordinates of camera-centered system.
 *
 * @ingroup VPI_LDC
 */
typedef float VPICameraExtrinsic[3][4];

/** Represents a 2D perspective transform.
 * \verbatim
   [vx]   [a11 a12 tx] [ux]
   [vy] = [a21 a22 ty] [uy]
   [vw]   [ p0  p1  p] [ 1]
   u'x  = vx/vw
   u'y  = vy/vw
   \endverbatim
 *
 * In these equations,
 * - ux, uy are the inhomogeneous coordinates in the source image.
 * - u'x, u'y are the inhomogeneous coordinates in the destination image.
 * - vx,vy,vw are the homogeneous coordinates in the destination image.
 * - a11,a12,a21,a22 are the2x2 non-singular matrix with the linear component of the transform.
 * - tx,ty is the translation component of the transform.
 * - p0,p1,p are the projective components of the transform. p is usually 1.
 *
 * @ingroup VPI_PerspectiveWarp
 */
typedef float VPIPerspectiveTransform[3][3];

/**
 * Defines the states of the event.
 * @ingroup VPI_Types
 */
typedef enum
{
    /** Event is not signaled yet. */
    VPI_EVENT_STATE_NOT_SIGNALED,

    /** Event has been signaled. */
    VPI_EVENT_STATE_SIGNALED

} VPIEventState;

/** Defines image flip directions.
 * This is used by algorithms such as \ref algo_image_flip
 */
typedef enum
{
    /** Flip horizontally. */
    VPI_FLIP_HORIZ = 1U << 0,

    /** Flip vertically. */
    VPI_FLIP_VERT = 1U << 1,

    /** Flip horizontally and vertically.
     *  This effectively rotates the image 180 degrees.
     */
    VPI_FLIP_BOTH = VPI_FLIP_HORIZ | VPI_FLIP_VERT

} VPIFlipMode;

/** Defines the magnitude of vectors.
 * This is used by algorithms such as \ref algo_canny_edge_detector
 */
typedef enum
{
    /** L1 norm. */
    VPI_NORM_L1,

    /** L2 norm. */
    VPI_NORM_L2,

    /** Hamming norm. */
    VPI_NORM_HAMMING

} VPINormType;

/**
 * Different method to generate gradients.
 */
typedef enum
{
    /** Generate the gradient in x direction and y direction via sobel operator */
    VPI_GRADIENT_SOBEL,

    /** Generate the gradient by using prewitt operator on blurred input image */
    VPI_GRADIENT_BLUR_PREWITT
} VPIGradientMethod;

/** Defines which statistics to be calculated.
 */

/** Calculate pixel count */
#define VPI_STAT_PIXEL_COUNT (1U << 0)

/** Calculate per-channel sum */
#define VPI_STAT_SUM (1U << 1)

/** Calculate per-channel mean */
#define VPI_STAT_MEAN ((1U << 2) | VPI_STAT_SUM | VPI_STAT_PIXEL_COUNT)

/** Calculate covariance matrix with variance of each channel */
#define VPI_STAT_VARIANCE ((1U << 3) | VPI_STAT_MEAN)

/** Calculate full covariance matrix */
#define VPI_STAT_COVARIANCE ((1U << 4) | VPI_STAT_VARIANCE)

/** Stores information about an AprilTag detection from \ref algo_apriltags "AprilTag detector"
 * @ingroup VPI_AprilTags
 */
typedef struct
{
    /** The decoded ID of the tag */
    uint16_t id;
    /** Number of bits corrected for this detection */
    int16_t correctedBits;
    /** A measure of the quality of the binary decoding process: the average
     * difference between the intensity of a data bit versus the decision
     * threshold. Higher numbers roughly indicate better decodes. This is a
     * reasonable measure of detection accuracy only for very small tags. */
    float decisionMargin;
    /** The 3x3 homography matrix describing the projection from an "ideal" tag (with corners
     *  at (-1,1), (1,1), (1,-1), and (-1, -1)) to pixels in the image. */
    VPIHomographyTransform2D homography;
    /** The center of the detection in image pixel coordinates. */
    VPIKeypointF32 center;
    /** The corners of the tag in image pixel coordinates, ordered counter-clockwise. */
    VPIKeypointF32 corners[4];
} VPIAprilTagDetection;

/** Estimation of a transformation from one 3D coordinate system to another with object-space error
 * @ingroup VPI_Types
 */
typedef struct
{
    /** Transformation from one 3D coordinate system to another
     * \f{bmatrix}{
     *   R_{3\times3} & T_{3\times1}
     * \f}
     *
     * Where:
     * \f$R_{3\times3}\f$: Rotation matrix.
     * \f$T_{3\times1}\f$: 3D position of the source system origin expressed in coordinates of the target system.
     *
     * A point in the source coordinate system \f$A\f$ can be transformed to the same point in the
     * target coordinate system \f$B\f$ by the following equation:
     *
     * \f$\begin{bmatrix} x_B \\ y_B \\ z_B \end{bmatrix} = \begin{bmatrix} R_{3\times3} &
     * T_{3\times1} \end{bmatrix} \begin{bmatrix} x_A \\ y_A \\ z_A \\ 1 \end{bmatrix}\f$ */
    float transform[3][4];
    /** Object-space error of the pose estimation */
    float error;
} VPIPose;

#ifdef __cplusplus
}
#endif

#endif /* NV_VPI_TYPES_H */
