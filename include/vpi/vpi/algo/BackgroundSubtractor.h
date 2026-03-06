/*
 * Copyright 2021 NVIDIA Corporation. All rights reserved.
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
 * @file BackgroundSubtractor.h
 *
 * Declares functions that implement background subtractor algorithms.
 */

#ifndef NV_VPI_ALGORITHMS_BACKGROUND_SUBTRACTOR_H
#define NV_VPI_ALGORITHMS_BACKGROUND_SUBTRACTOR_H

#include "../Export.h"
#include "../Status.h"
#include "../Types.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup VPI_BackgroundSubtractor Background Subtractor
 * @ingroup VPI_API_Algorithms
 * @{
 *
 * Separate foreground and background from video.
 * Refer to \ref algo_background_subtractor for more details and usage examples.
 * 
 */

/**
 * Structure that defines the parameters for \ref vpiCreateBackgroundSubtractor
 */
typedef struct
{
    /** Threshold on the squared Mahalanobis distance between the pixel and the model
      * to decide whether a pixel is well described by the background model.
      * This parameter does not affect the background update. */
    float varThreshold;

    /** The algorithm will detect shadows and mark them if this is set to non-zero value */
    uint8_t detectShadow;

    /** Pixel value representing shadow. */
    uint8_t shadowPixelValue;

    /** Learning rate that indicates how fast the background model is learnt.
     *  + This value must be between 0 and 1.
     *    - Minimum of 0 means that the background model is not updated at all
     *    - Maximum of 1 means that the background model is completely reinitialized from the last frame.
     */

    float learningRate;
} VPIBackgroundSubtractorParams;

/**
 * Initializes \ref VPIBackgroundSubtractorParams with default values.
 *
 * Defaults:
 * - varThreshold: 16
 * - detectShadow: 1
 * - shadowPixelValue: 127
 * - learningRate: 0.001
 *
 * @param[out] params Structure to be filled with default values.
 *
 * @retval #VPI_ERROR_INVALID_ARGUMENT \p params is NULL
 * @retval #VPI_SUCCESS                Operation executed successfully.
 */
VPI_PUBLIC VPIStatus vpiInitBackgroundSubtractorParams(VPIBackgroundSubtractorParams *params);

/**
 * Creates payload for \ref vpiSubmitBackgroundSubtractor
 *
 * @param[in] backends VPI backend that will execute the algorithm.
 *                     + Valid values:
 *                       - \ref VPI_BACKEND_CPU
 *                       - \ref VPI_BACKEND_CUDA
 *                     + Backend must be enabled in current context.
 *
 * @param[in] imageWidth, imageHeight Input image dimensions.
 *                                    + Must be > 0.
 *
 * @param[in] inputFormat Input image format.
 *                        + The accepted image formats are:
 *                          - \ref VPI_IMAGE_FORMAT_U8
 *                          - \ref VPI_IMAGE_FORMAT_U16
 *                          - \ref VPI_IMAGE_FORMAT_NV12
 *                          - \ref VPI_IMAGE_FORMAT_NV12_ER
 *                          - \ref VPI_IMAGE_FORMAT_RGB8
 *                          - \ref VPI_IMAGE_FORMAT_RGBA8
 *
 * @param[out] payload Pointer to the payload variable that receives the created handle.
 *
 * @retval #VPI_ERROR_INVALID_ARGUMENT  \p payload is NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT  \p imageWidth or \p imageHeight outside valid range.
 * @retval #VPI_ERROR_INVALID_ARGUMENT  \p backend refers to an invalid backend.
 * @retval #VPI_IMAGE_FORMAT_INVALID    \p inputFormat is not supported.
 * @retval #VPI_ERROR_NOT_IMPLEMENTED   Background Subtractor algorithm is not supported by given backend.
 * @retval #VPI_ERROR_INVALID_CONTEXT   Current context was destroyed.
 * @retval #VPI_ERROR_INVALID_OPERATION Backend isn't enabled in current context.
 * @retval #VPI_ERROR_OUT_OF_MEMORY     Cannot allocate required resources.
 * @retval #VPI_SUCCESS                 Operation executed successfully.
 */
VPI_PUBLIC VPIStatus vpiCreateBackgroundSubtractor(uint64_t backends, int32_t imageWidth, int32_t imageHeight,
                                                   VPIImageFormat inputFormat, VPIPayload *payload);

/**
 * Submits a \ref algo_background_subtractor "Background Subtractor" operation to the stream.
 *
 * Runs background subtractor on the incoming image from the video and outputs an estimate of the foreground mask.
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
 * @param[in] inFrame Incoming frame from the video.
 *                    + Must not be NULL.
 *                    + Must have same dimensions as the one specified during payload creation.
 *                    + Image must have enabled the backends that will execute the algorithm.
 *
 * @param[out] outFGMask The output foreground mask as an 8-bit binary image.
 *                       + Must not be NULL.
 *                       + Must have same dimensions as input image.
 *                       + The only accepted format is \ref VPI_IMAGE_FORMAT_U8.
 *                       + Image must have enabled the backends that will execute the algorithm.
 *
 * @param[out] outBGImage Background image.
 *                        Pass NULL if not needed.
 *                        + Must have same format and dimensions as input image.
 *                        + Image must have enabled the backends that will execute the algorithm.
 *
 * @param[in] params Algorithm control parameters.
 *                   Pass NULL to use the defaults given by \ref vpiInitBackgroundSubtractorParams.
 *
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p stream is NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p params is NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p inFrame or \p outFGMask are NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p payload is not created by vpiCreateBackgroundSubtractor.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p inFrame image dimension does not match the one associated with \p payload.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p outFGMask dimension does not match the one associated with \p payload.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     Learning rate in \p params outside valid range.
 * @retval #VPI_ERROR_INVALID_IMAGE_FORMAT \p inFrame image format does not match the one associated with \p payload.
 * @retval #VPI_ERROR_INVALID_IMAGE_FORMAT \p outFGMask format not supported.
 * @retval #VPI_ERROR_INVALID_IMAGE_FORMAT \p outBGImage format does not match the one associated with \p payload.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p payload is invalid.
 * @retval #VPI_ERROR_INVALID_OPERATION    The needed backends aren't enabled in \p stream, \p inFrame, \p outFGMask or \p outBGImage.
 *
 * @retval #VPI_SUCCESS Operation executed successfully.
 */
VPI_PUBLIC VPIStatus vpiSubmitBackgroundSubtractor(VPIStream stream, uint64_t backend, VPIPayload payload,
                                                   VPIImage inFrame, VPIImage outFGMask, VPIImage outBGImage,
                                                   const VPIBackgroundSubtractorParams *params);

/** @} end of VPI_BackgroundSubtractor */

#ifdef __cplusplus
}
#endif

#endif /* NV_VPI_ALGORITHMS_BACKGROUND_SUBTRACTOR_H */
