/*
 * Copyright 2024 NVIDIA Corporation. All rights reserved.
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
 * @file AprilTags.h
 *
 * Declares functions that implement AprilTag detection and pose estimation
 */

#ifndef NV_VPI_ALGORITHMS_APRIL_TAGS_H
#define NV_VPI_ALGORITHMS_APRIL_TAGS_H

#include "../Export.h"
#include "../Status.h"
#include "../Types.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup VPI_AprilTags AprilTags
 * @ingroup VPI_API_Algorithms
 * @{
 *
 * Detect AprilTags (https://april.eecs.umich.edu/software/apriltag) in an image.
 * Refer to \ref algo_apriltags for more details and usage examples.
 */

/** Specify the family of AprilTags to detect
 *
 * The family is defined by the layout, the number of bits in the tag and the
 * minimum hamming distance between valid codewords.
 */
typedef enum
{
    /** Invalid value */
    VPI_APRILTAG_INVALID = 0,
    /** 16h5 family */
    VPI_APRILTAG_16H5 = 1,
    /** 25h9 family */
    VPI_APRILTAG_25H9 = 2,
    /** 36h10 family */
    VPI_APRILTAG_36H10 = 3,
    /** 36h11 family */
    VPI_APRILTAG_36H11 = 4,
    /** Circle21h7 family */
    VPI_APRILTAG_CIRCLE21H7 = 5,
    /** Circle49h12 family */
    VPI_APRILTAG_CIRCLE49H12 = 6,
    /** Custom48h12 family */
    VPI_APRILTAG_CUSTOM48H12 = 7,
    /** Standard41h12 family */
    VPI_APRILTAG_STANDARD41H12 = 8,
    /** Standard52h13 family */
    VPI_APRILTAG_STANDARD52H13 = 9
} VPIAprilTagFamily;

/** Decode parameters for \ref vpiCreateAprilTagDetector
 *
 * Specifies family to decode and decode properties.
 */
typedef struct
{
    /** Optional array of tag ids from family which represent valid detections.
      * Set to NULL to detect all tags from the family
      * Data can be freed after call to \ref vpiCreateAprilTagDetector. */
    const uint16_t *tagIdFilter;
    /** Size of tagIdFilter array if not NULL.
      *   + Must be >=0.
      *   + Must be less than the number of tags in the family. */
    int32_t tagIdFilterSize;
    /** Maximum number of bits to correct during decoding.
      *   + Must be in range [0,2] */
    int32_t maxBitsCorrected;
    /** Which family to detect */
    VPIAprilTagFamily family;
} VPIAprilTagDecodeParams;

/**
 * Initializes \ref VPIAprilTagDecodeParams with default values.
 *
 * Defaults:
 * - tagIdFilter: NULL
 * - tagIdFilterSize: 0
 * - maxBitsCorrected: 2
 * - family: \ref VPI_APRILTAG_36H11
 *
 * @param[out] params Structure to be filled with default values.
 *
 * @retval #VPI_ERROR_INVALID_ARGUMENT \p params is NULL
 * @retval #VPI_SUCCESS                Operation executed successfully.
 */
VPI_PUBLIC VPIStatus vpiInitAprilTagDecodeParams(VPIAprilTagDecodeParams *params);

/**
 * Creates a \ref algo_apriltags "AprilTag detector" payload.
 * This function allocates all temporary memory needed by the algorithm.
 *
 * @param[in] backends VPI backends that are eligible to execute the algorithm.
 *                     + Valid values:
 *                       - \ref VPI_BACKEND_CPU
 *                       - \ref VPI_BACKEND_PVA
 *                     + Backend must be enabled in current context.
 *                     + If \ref VPI_BACKEND_PVA is used, the \ref VPIStream and input/output buffers must
 *                       support both \ref VPI_BACKEND_PVA and \ref VPI_BACKEND_CPU.
 * @param[in] inputWidth, inputHeight Dimensions of the input image that will be used with this payload.
 *                     + Must be > 0.
 * @param[in] params Decode parameters for the detector.
 *                   Pass NULL to use the defaults given by \ref vpiInitAprilTagDecodeParams.
 *
 * @param[out] payload Pointer to the payload variable that receives the created handle.
 *
 * @retval #VPI_ERROR_INVALID_ARGUMENT  \p payload is NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT  \p backends refers to an invalid backend.
 * @retval #VPI_ERROR_INVALID_ARGUMENT  \p inputWidth or \p inputHeight outside valid range.
 * @retval #VPI_ERROR_INVALID_ARGUMENT  \p params is NULL or contains invalid configuration.
 * @retval #VPI_ERROR_INVALID_OPERATION Backend hardware not available, or backend not available in current context.
 * @retval #VPI_ERROR_NOT_IMPLEMENTED   Algorithm does not support the given backend.
 * @retval #VPI_ERROR_INVALID_CONTEXT   Current context is destroyed.
 * @retval #VPI_ERROR_OUT_OF_MEMORY     Cannot allocate required resources.
 * @retval #VPI_SUCCESS                 Operation executed successfully.
 */
VPI_PUBLIC VPIStatus vpiCreateAprilTagDetector(uint64_t backends, int32_t inputWidth, int32_t inputHeight,
                                               VPIAprilTagDecodeParams const *params, VPIPayload *payload);

/**
 * Submits a \ref algo_apriltags "AprilTag detector" operation to the stream.
 *
 * @param[in] stream The stream where the operation will be queued in.
 *                   + Must not be NULL.
 *                   + Stream must have enabled the backends that will execute the algorithm.
 * @param[in] backend Backend that will execute the algorithm.
 *                    + Must be the backend specified during payload creation or 0 as a shorthand to use this backend.
 * @param[in] payload Payload to be submitted along the other parameters.
 * @param[in] maxDetections Maximum number of detections to be returned in outDetections.
 * @param[in] input Input image used for detection
 *                  + Must not be NULL.
 *                  + Must match dimensions used in \ref vpiCreateAprilTagDetector.
 *                  + Image must have enabled the backends that will execute the algorithm.
 *                  + Must have format VPI_IMAGE_FORMAT_U8
 *
 * @param[out] outDetections Array that will receive the detections. Array size is updated with the number of tags found.
 *                         + Must not be NULL.
 *                         + It must have type \ref VPI_ARRAY_TYPE_APRILTAG_DETECTION.
 *                         + Array must have enabled the backends that will execute the algorithm.
 *                         + If more tags are detected than the array capacity, extra tag detections are discarded.
 *
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p stream is NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p input is NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p input image dimensions do not match dimensions passed to \ref vpiCreateAprilTagDetector.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p backend does not match the backend used to create the payload.
 * @retval #VPI_ERROR_INVALID_IMAGE_FORMAT \p input image format not supported.
 * @retval #VPI_ERROR_INVALID_PAYLOAD_TYPE \p payload is invalid or NULL.
 * @retval #VPI_ERROR_NOT_IMPLEMENTED      Algorithm does not support the given backend.
 * @retval #VPI_ERROR_INVALID_OPERATION    Backend hardware not available.
 * @retval #VPI_ERROR_INVALID_OPERATION    The needed backends aren't enabled in \p stream, \p input or \p outDetections.
 * @retval #VPI_SUCCESS                    Operation executed successfully.
 */
VPI_PUBLIC VPIStatus vpiSubmitAprilTagDetector(VPIStream stream, uint64_t backend, VPIPayload payload,
                                               uint32_t maxDetections, VPIImage input, VPIArray outDetections);

/**
 * Submits a \ref algo_apriltags "AprilTag pose estimation" operation to the stream.
 *
 * @param[in] stream The stream where the operation will be queued in.
 *                   + Must not be NULL.
 *                   + Stream must have enabled the backends that will execute the algorithm.
 * @param[in] backend Backend that will execute the algorithm.
 *                     + Valid values:
 *                       - \ref VPI_BACKEND_CPU
 *                     + Backend must be enabled in current context.
 * @param[in] inDetections Array of AprilTag detections.
 *                         + Must not be NULL.
 *                         + It must have type \ref VPI_ARRAY_TYPE_APRILTAG_DETECTION.
 *                         + Array must have enabled the backends that will execute the algorithm.
 * @param[in] intrinsics Camera intrinsics
 * @param[in] tagSize Tag edge length
 *                         + Must be > 0
 *
 * @param[out] outPoses Array that will receive the pose estimations. Array size will be set to same as inDetections.
 *                         + Must not be NULL.
 *                         + It must have type \ref VPI_ARRAY_TYPE_POSE.
 *                         + Array must have enabled the backends that will execute the algorithm.
 *                         + Capacity must be equal or greater than size of inDetections.
 *
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p stream is NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p inDetections or \p outPoses is NULL.
 * @retval #VPI_ERROR_NOT_IMPLEMENTED      Algorithm does not support the given backend.
 * @retval #VPI_ERROR_INVALID_OPERATION    Backend hardware not available.
 * @retval #VPI_ERROR_INVALID_OPERATION    The needed backends aren't enabled in \p stream, \p inDetections or \p outPoses.
 * @retval #VPI_SUCCESS                    Operation executed successfully.
 */
VPI_PUBLIC VPIStatus vpiSubmitAprilTagPoseEstimation(VPIStream stream, uint64_t backend, VPIArray inDetections,
                                                     const VPICameraIntrinsic intrinsics, float tagSize,
                                                     VPIArray outPoses);

/** @} end of VPI_AprilTags */

#ifdef __cplusplus
}
#endif

#endif /* NV_VPI_ALGORITHMS_APRIL_TAGS_H */
