/*
 * Copyright 2019-2021 NVIDIA Corporation. All rights reserved.
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
 * @file HarrisCorners.h
 *
 * Declares functions that implement the Harris Corner Detector algorithm.
 */

#ifndef NV_VPI_ALGORITHMS_HARRIS_CORNERS_H
#define NV_VPI_ALGORITHMS_HARRIS_CORNERS_H

#include "../Export.h"
#include "../Status.h"
#include "../Types.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup VPI_HarrisCorners Harris Corners
 * @ingroup VPI_API_Algorithms
 * @{
 *
 * Detect Harris keypoints in an image, usually used in keypoint tracking.
 * Refer to \ref algo_harris_corners for more details and usage examples.
 */

/**
 * Structure that defines the parameters for \ref vpiSubmitHarrisCornerDetector
 */
typedef struct
{
    /** Gradient window size.
     *  + Must be 3, 5 or 7. */
    int32_t gradientSize;

    /** Block window size used to compute the Harris Corner score.
     *  + Must be 3, 5 or 7. */
    int32_t blockSize;

    /** Specifies the minimum threshold with which to eliminate Harris Corner scores.
     *  + Threshold is dependent on input image range. For image format U8 and S8 it is in the range of [0-20]. Can be larger for U16 and S16.
     *  + Must be >= 0. */
    float strengthThresh;

    /** Specifies sensitivity threshold from the Harris-Stephens equation. */
    float sensitivity;

    /** Non-maximum suppression radius, set to 0 to disable it.
     *  + On PVA backend, this must be set to 8. */
    float minNMSDistance;
} VPIHarrisCornerDetectorParams;

/** Initializes \ref VPIHarrisCornerDetectorParams with default values.
 *
 * Default values are:
 * - gradientSize:   3
 * - blockSize:      3
 * - strengthThresh: 20
 * - sensitivity:    0.0625
 * - minNMSDistance: 8
 * 
 * @retval #VPI_ERROR_INVALID_ARGUMENT \p params is NULL.
 * @retval #VPI_SUCCESS Operation executed successfully.
 */
VPI_PUBLIC VPIStatus vpiInitHarrisCornerDetectorParams(VPIHarrisCornerDetectorParams *params);

/**
 * Creates a \ref algo_harris_corners "Harris Corner Detector" payload.
 * This function allocates all temporary memory needed by the algorithm.
 *
 * @param[in] backends VPI backends that are eligible to execute the algorithm.
 *                     + Valid values:
 *                       - \ref VPI_BACKEND_CPU
 *                       - \ref VPI_BACKEND_CUDA
 *                       - \ref VPI_BACKEND_PVA
 *                     + Backend must be enabled in current context.
 *
 * @param[in] inputWidth, inputHeight Dimensions of the input image that will be used with this payload.
 *                                    + On PVA dimensions limited to minimum of 160x120, maximum of 3264x2448.
 *                                    + Must be >= 0 (other backends).
 *
 * @param[out] payload Pointer to the payload variable that receives the created handle.
 * 
 * @retval #VPI_ERROR_INVALID_ARGUMENT  \p payload is NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT  \p inputWidth or \p inputHeight outside valid range.
 * @retval #VPI_ERROR_INVALID_ARGUMENT  \p backends refers to an invalid backend.
 * @retval #VPI_ERROR_INVALID_OPERATION Backend hardware not available.
 * @retval #VPI_ERROR_NOT_IMPLEMENTED   Harris Corner Detector algorithm is not supported by given backend.
 * @retval #VPI_ERROR_INVALID_CONTEXT   Current context is destroyed.
 * @retval #VPI_ERROR_OUT_OF_MEMORY     Cannot allocate required resources.
 * @retval #VPI_ERROR_INVALID_OPERATION Backend isn't enabled in current context.
 * @retval #VPI_SUCCESS                 Operation executed successfully.
 */
VPI_PUBLIC VPIStatus vpiCreateHarrisCornerDetector(uint64_t backends, int32_t inputWidth, int32_t inputHeight,
                                                   VPIPayload *payload);

/**
 * Submits a \ref algo_harris_corners "Harris Corner Detector" operation to the stream.
 *
 * @param[in] stream The stream where the operation will be queued in.
 *                   + Must not be NULL.
 *                   + Stream must have enabled the backends that will execute the algorithm.
 *
 * @param[in] backend Backend that will execute the algorithm.
 *                    + Must be the backend specified during payload creation or 0 as a shorthand to use this backend.
 *
 * @param[in] payload Payload to be submitted along the other parameters.
 *
 * @param[in] input Input image from where the Harris corners will be extracted.
 *                  + Must not be NULL.
 *                  + Image must have enabled the backends that will execute the algorithm.
 *                  + The accepted image formats are:
 *                    - \ref VPI_IMAGE_FORMAT_U8
 *                    - \ref VPI_IMAGE_FORMAT_S8
 *                    - \ref VPI_IMAGE_FORMAT_U16
 *                    - \ref VPI_IMAGE_FORMAT_S16 (PVA backend only supports this format)
 *
 * @param[out] outFeatures Array that will receive the detected corners.
 *                         Array size is updated with the number of corners found.
 *                         + Must not be NULL.
 *                         + It must have type \ref VPI_ARRAY_TYPE_KEYPOINT_F32.
 *                         + Array must have enabled the backends that will execute the algorithm.
 *
 * @param[out] outScores Array that will receive the corners' scores.
 *                       Array size matches outFeatures array' size.
 *                       + Must not be NULL.
 *                       + It must have type \ref VPI_ARRAY_TYPE_U32.
 *                       + Array must have enabled the backends that will execute the algorithm.
 *                       + Must have same capacity as outFeatures.
 *
 * @param[in] params Pointer to a \ref VPIHarrisCornerDetectorParams with parameters for this algorithm invocation.
 *                   These parameters can vary in every call and will be copied internally, no need to keep the object around.
 *  
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p stream is NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p input, \p outFeatures or \p outScores are NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p payload is not created by vpiCreateHarrisCornerDetector.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     Strength threshold in \p params outside valid range.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     Output arrays must have same capacity.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     Unsupported \p outFeatures or \p outScores type.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     Invalid \p input dimensions.
 * @retval #VPI_ERROR_INVALID_PAYLOAD_TYPE \p payload is invalid.
 * @retval #VPI_ERROR_INVALID_IMAGE_FORMAT Unsupported input format.
 * @retval #VPI_ERROR_INVALID_OPERATION    The needed backends aren't enabled in \p stream, \p outFeatures or \p outScores.
 * @retval #VPI_SUCCESS                    Operation executed successfully.
 */
VPI_PUBLIC VPIStatus vpiSubmitHarrisCornerDetector(VPIStream stream, uint64_t backend, VPIPayload payload,
                                                   VPIImage input, VPIArray outFeatures, VPIArray outScores,
                                                   const VPIHarrisCornerDetectorParams *params);

/** @} end of VPI_HarrisCorners */

#ifdef __cplusplus
}
#endif

#endif /* NV_VPI_ALGORITHMS_HARRIS_CORNERS_H */
