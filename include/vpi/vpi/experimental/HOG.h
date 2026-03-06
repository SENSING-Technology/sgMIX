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
 * @file HOG.h
 *
 * Declares functions that implement the Histogram of Oriented Gradients algorithm.
 */

#ifndef NV_VPI_ALGORITHMS_HOG_H
#define NV_VPI_ALGORITHMS_HOG_H

#include "../Export.h"
#include "../Status.h"
#include "../Types.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup VPI_HOG Histogram of Oriented Gradients
 * @ingroup VPI_API_Algorithms
 * @{
 *
 * Extracts Histogram of Oriented Gradients features from input image.
 */

/** @name HOG flags 
 * Defines what features will be returned.
 * @{ */
#define VPI_HOG_CONTRAST_SENSITIVE 0x01   /**< Return contrast sensitive features. */
#define VPI_HOG_CONTRAST_INSENSITIVE 0x02 /**< Return contrast insensitive features. */
#define VPI_HOG_TEXTURE 0x04              /**< Return texture-related features. */

/** Helper flag to return all features. */
#define VPI_HOG_ALL_FEATURES (VPI_HOG_CONTRAST_SENSITIVE | VPI_HOG_CONTRAST_INSENSITIVE | VPI_HOG_TEXTURE)
/** @} */

/**
 * Create a payload for the non-batch version of HOG algorithm.
 *
 * @param[in] backends VPI backends that are eligible to execute the algorithm.
 *                     + Valid values:
 *                       - \ref VPI_BACKEND_CPU
 *                       - \ref VPI_BACKEND_CUDA
 *
 * @param[in] width,height Dimensions of the input image to be used.
 *                         + Must be >= 0.
 *
 * @param[in] features Flags to specify what features will be returned.
 *                     + Must be a bitwise combination of one or more of the following flags:
 *                       - \ref VPI_HOG_CONTRAST_SENSITIVE
 *                       - \ref VPI_HOG_CONTRAST_INSENSITIVE
 *                       - \ref VPI_HOG_TEXTURE
 *                       - \ref VPI_HOG_ALL_FEATURES (it's the combination of all flags above)
 *
 * @param[in] cellSize Cell size, typically 8 or 16 for 8x8 and 16x16 cells respectively.
 *                     + Must be >=2 and <= 32 and power of two.
 *
 * @param[in] numOrientations Number of orientations used. This is typically 18.
 *                            + Must be between 4 and 18.
 *
 * @param[out] outNumFeatures Receives the number of features that will be returned. Pass NULL if not needed.
 *
 * @param[out] payload Pointer to the payload variable that receives the created handle.
 * 
 * @retval #VPI_ERROR_INVALID_ARGUMENT \p payload handle is NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT \p width or \p height outside valid range.
 * @retval #VPI_ERROR_NOT_IMPLEMENTED  HOG is not implemented for given backend.
 * @retval #VPI_ERROR_INVALID_CONTEXT  Current context is destroyed.
 * @retval #VPI_ERROR_OUT_OF_MEMORY    Cannot allocate required resources.
 * @retval #VPI_SUCCESS                Operation executed successfully.
 */
VPI_PUBLIC VPIStatus vpiCreateExtractHOGFeatures(uint64_t backends, int32_t width, int32_t height, int32_t features,
                                                 int32_t cellSize, int32_t numOrientations, int32_t *outNumFeatures,
                                                 VPIPayload *payload);

/**
 * Create a payload for the batch version of HOG algorithm.
 * The input images are assumed to be laid out in memory as a 2D matrix of images. All images must have the same dimensions.
 *
 * @param[in] backends VPI backends that are eligible to execute the algorithm.
 *                     + Valid values:
 *                       - \ref VPI_BACKEND_CPU
 *                       - \ref VPI_BACKEND_CUDA
 *
 * @param[in] maxBatchWidth,maxBatchHeight Maximum number of images horizontally and vertically, respectively.
 *                                         + Must be >= 0.
 * @param[in] imgWidth,imgHeight Dimensions each of the batch images.
 *                               + Must be >= 0.
 *                               + \p imgWidth and \p imgHeight must be multiple of cell width and height respectively.
 *
 * @param[in] features Flags to specify which features will be returned.
 *                     + Must be a bitwise combination of one or more of the following flags:
 *                       - \ref VPI_HOG_CONTRAST_SENSITIVE
 *                       - \ref VPI_HOG_CONTRAST_INSENSITIVE
 *                       - \ref VPI_HOG_TEXTURE
 *                       - \ref VPI_HOG_ALL_FEATURES (it's the combination of all flags above)
 *
 * @param[in] cellSize Cell size, typically 8 or 16 for 8x8 and 16x16 cells respectively.
 *                     + Must be >=2 and <= 32 and power of two.
 *
 * @param[in] numOrientations Number of orientations used. This is typically 18.
 *                            + Must be even and between 4 and 18.
 *
 * @param[out] outNumFeatures Receives the number of features that will be returned. Pass NULL if not needed.
 *
 * @param[out] payload Pointer to a payload handle that will receive the allocated payload.
 * 
 * @retval #VPI_ERROR_INVALID_ARGUMENT \p payload handle is NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT \p maxBatchWidth or \p maxBatchHeight outside valid range.
 * @retval #VPI_ERROR_INVALID_ARGUMENT \p imgWidth or \p imgHeight outside valid range.
 * @retval #VPI_ERROR_INVALID_ARGUMENT Invalid \p features.
 * @retval #VPI_ERROR_INVALID_ARGUMENT \p cellSize or \p numOrientations outside valid range.
 * @retval #VPI_ERROR_INVALID_ARGUMENT \p imgWidth must be a multiple of cell width.
 * @retval #VPI_ERROR_INVALID_ARGUMENT \p imgHeight must be a multiple of cell height.
 * @retval #VPI_ERROR_INVALID_ARGUMENT \p backends refers to an invalid backend.
 * @retval #VPI_ERROR_NOT_IMPLEMENTED  HOG is not implemented for given backend.
 * @retval #VPI_ERROR_INVALID_CONTEXT  Current context is destroyed.
 * @retval #VPI_ERROR_OUT_OF_MEMORY    Cannot allocate required resources.
 * @retval #VPI_SUCCESS                Operation executed successfully.
 */
VPI_PUBLIC VPIStatus vpiCreateExtractHOGFeaturesBatch(uint64_t backends, int32_t maxBatchWidth, int32_t maxBatchHeight,
                                                      int32_t imgWidth, int32_t imgHeight, int32_t features,
                                                      int32_t cellSize, int32_t numOrientations,
                                                      int32_t *outNumFeatures, VPIPayload *payload);

/**
 * Submits a HOG operation to the stream.
 * It handles both batch and non-batch payloads.
 *
 * @param[in] stream The stream where the operation will be queued in.
 *                   + Must not be NULL.
 *                   + Stream must have enabled the backends that will execute the algorithm.
 *
 * @param[in] backend Backend that will execute the algorithm.
 *                    Must be the backend specified during payload creation or 0 as a shorthand to use this backend.
 *
 * @param[in] payload Payload to be submitted along the other parameters.
 *
 * @param[in] input If using a non batch payload, this is the input image to be processed.
 *                  + Must not be NULL.
 *                  + Its dimensions must match what was passed to \ref vpiCreateExtractHOGFeatures.
 *                  + If using a batch payload, the image dimensions must be a multiple of (`imgWidth`,`imgHeight`),
 *                    and it must be at most (`imgWidth*maxBatchWidth`, `imgHeight*maxBatchHeight`).
 *                  + Image must have enabled the backends that will execute the algorithm.
 *
 * @param[out] outFeatures Pointer to an array of images that will receive the features. 
 *                         + Must not be NULL.
 *                         + The dimensions of each image must be (`input.width / cellSize`, `input.height / cellSize`).
 *                         + In case of batch processing, the output features position in the 2D matrix be the same position
 *                           of the corresponding input image.
 *                         + All images must have same format.
 *                         + All image must have enabled the backends that will execute the algorithm.
 *
 * @param[in] numFeatures Number of images in the output array. Must be between 1 and 32.
 *
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p stream is NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p input or \p outFeatures are NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p payload is not generated using vpiCreateExtractHOGFeatures.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p numFeatures outside valid range.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     Batch input width must be a multiple of configured input width.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     Batch input height must be a multiple of configured input height.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p outFeatures must all have the same format.
 * @retval #VPI_ERROR_INVALID_PAYLOAD_TYPE \p payload is invalid.
 * @retval #VPI_ERROR_INVALID_OPERATION    The needed backends aren't enabled in \p stream, \p input or \p outFeatures.
 * @retval #VPI_SUCCESS                    Operation executed successfully.
 */
VPI_PUBLIC VPIStatus vpiSubmitExtractHOGFeatures(VPIStream stream, uint64_t backend, VPIPayload payload, VPIImage input,
                                                 VPIImage *outFeatures, int32_t numFeatures);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* NV_VPI_ALGORITHMS_HOG_H */
