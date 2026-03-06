/*
 * Copyright 2023 NVIDIA Corporation. All rights reserved.
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
 * @file CropScaler.h
 *
 * Declares functions that implement the Crop Scaler algorithm.
 */

#ifndef NV_VPI_ALGORITHMS_CROPSCALER_H
#define NV_VPI_ALGORITHMS_CROPSCALER_H

/**
 * @defgroup VPI_CropScaler Crop Scaler
 * @ingroup VPI_API_Algorithms
 * @{
 *
 * Crops rectangular regions from the input image, rescale them to
 * uniform dimensions and writes them in a single image to be used
 * in further batched image processing.
 */

#include "../Export.h"
#include "../ImageFormat.h"
#include "../Status.h"
#include "../Types.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Creates payload instance for the Crop Scale algorithm.
 *
 * @param[in] backends VPI backends that are eligible to execute the algorithm.
 *                     + Valid values:
 *                       - \ref VPI_BACKEND_CUDA
 *                     + Backend must be enabled in current context.
 * 
 * @param[in] maxFrames Maximum number of frames.
 *                       + Must be >= 1 and <= 128.
 * 
 * @param[in] maxObjects Maximum number of objects.
 *                       + Must be >= 1.
 *
 * @param[out] payload Pointer to the payload variable that receives the created handle.
 *
 * @retval #VPI_ERROR_INVALID_OPERATION Backend isn't enabled in current context.
 * @retval #VPI_ERROR_NOT_IMPLEMENTED   Crop Scaler algorithm is not supported by given backend.
 * @retval #VPI_ERROR_INVALID_CONTEXT   Current context is destroyed.
 * @retval #VPI_ERROR_OUT_OF_MEMORY     Cannot allocate required resources.
 * @retval #VPI_SUCCESS                 Operation executed successfully.
 */

VPI_PUBLIC VPIStatus vpiCreateCropScaler(uint64_t backends, int maxFrames, int maxObjects, VPIPayload *payload);

/**
 * Crops rectangular regions from the input frames and rescale them all to the same dimensions.
 *
 * @param[in] stream The stream where the operation will be queued in.
 *                   + Must not be NULL.
 *                   + Stream must have enabled the backends that will execute the algorithm.
 *
 * @param[in] backend Backend that will execute the algorithm.
 *                    + Must be the backend specified during payload creation or 0 as a shorthand to use this backend.
 *
 * @param[in] payload Payload created by \ref vpiCreateCropScaler.
 *
 * @param[in] frameList List of image frames.
 *                      If an image within \p frameList is NULL, its correponding objects won't be
 *                      processed.
 *                      + Must not be NULL.
 *                      + Must point to an array of at least \p numFrames images.
 *                      + Each image must have enabled the backends that will execute the algorithm.
 *                      + All frames must have the same image format, size, and pitch.
 *                      + The pitch must be a multiple of the element size.
 *                      + Valid image formats:
 *                        | Formats                       | CUDA |
 *                        |-------------------------------|:----:|
 *                        | \ref VPI_IMAGE_FORMAT_RGBA8   |   *  |
 *                        | \ref VPI_IMAGE_FORMAT_NV12_ER |   *  |
 *                        | \ref VPI_IMAGE_FORMAT_NV12    |   *  |
 *
 * @param[in] numFrames Number of input frames in the batch.
 *                      + Must be >= 1 and <= maxFrames estipulated in \ref vpiCreateCropScaler.
 *
 * @param[in] objects Input array with bounding boxes associated with each input frame via its index.
 *                    + Must not be NULL.
 *                    + Must have enabled the backends that will execute the algorithm.
 *                    + The index of the input frame for each object must be >= 0 and < \p numFrames.
 *                      An out-of-bounds index is undefined behavior.
 *                    + The number of objects in the array must be >= 0 and < maxObjects as specified by \ref vpiCreateCropScaler.
 *                    + Valid array types:
 *                      - \ref VPI_ARRAY_TYPE_DCF_TRACKED_BOUNDING_BOX
 *                        Objects with state \ref VPI_TRACKING_STATE_LOST won't be processed, as well as
 *                        objects whose frame in \p frameList is NULL.
 *
 * @param[in] patchWidth The width of the patch to be written to \p outPatches.
 *                       + Must be positive.
 *
 * @param[in] patchHeight The height of the patch to be written to \p outPatches.
 *                       + Must be positive.
 *
 * @param[out] outPatches Image where the cropped and rescale patches will be written to.
 *                        The width of each patch will be equal to \p patchWidth.
 *                        The height of each patch will be equal to \p patchHeight.
 *                        The patch for an object 'o' will then start at row `o * patchHeight`
 *                        and have height 'patchHeight'.
 *                        If image format is different from input frame's, an image format conversion
 *                        will be performed on the fly.
 *                        + Image height must be at least `patchHeight * numObjects`, where 'numObjects' is the
 *                          size of \p inObjects.
 *                        + Image width must be at least `patchWidth`.
 *                        + Valid image formats:
 *                        | Formats                      | CUDA |
 *                        |------------------------------|:----:|
 *                        | \ref VPI_IMAGE_FORMAT_RGB8p  |   *  |
 *                        | \ref VPI_IMAGE_FORMAT_BGR8p  |   *  |
 *                        | \ref VPI_IMAGE_FORMAT_RGBA8p |   *  |
 *                        | \ref VPI_IMAGE_FORMAT_RGBA8  |   *  |
 *                        | \ref VPI_IMAGE_FORMAT_RGB8   |   *  |
 *                        | \ref VPI_IMAGE_FORMAT_BGR8   |   *  |
 *
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p stream is NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p frameList, \p objects or outPatches are NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p payload not created by vpiCreateCropScaler.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p outPatches' height not divisible by the number of objects.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     Input frames have different image size.
 * @retval #VPI_ERROR_INVALID_IMAGE_FORMAT Input frames have different image format.
 * @retval #VPI_ERROR_INVALID_ARRAY_TYPE   Unsupported array type in \p objects.
 * @retval #VPI_ERROR_INVALID_PAYLOAD_TYPE \p payload is invalid.
 * @retval #VPI_ERROR_INVALID_OPERATION    The needed backends aren't enabled in \p stream, some image in
 *                                         \p frameList, \p outPatches or \p objects.
 * @retval #VPI_SUCCESS                    Operation executed successfully.
 */
VPI_PUBLIC VPIStatus vpiSubmitCropScalerBatch(VPIStream stream, uint64_t backend, VPIPayload payload,
                                              VPIImage *frameList, int32_t numFrames, VPIArray objects,
                                              int32_t patchWidth, int32_t patchHeight, VPIImage outPatches);

/** @} end of VPI_CropScaler */

#ifdef __cplusplus
}
#endif

#endif /* NV_VPI_ALGORITHMS_CROPSCALER_H */
