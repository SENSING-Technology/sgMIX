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
 * @file RecursiveGaussianFilter.h
 *
 * Declares functions that implement the Recursive Gaussian Filter algorithm.
 */

#ifndef NV_VPI_ALGORITHMS_RECURSIVE_GAUSSIAN_FILTER_H
#define NV_VPI_ALGORITHMS_RECURSIVE_GAUSSIAN_FILTER_H

#include "../Export.h"
#include "../Status.h"
#include "../Types.h"

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup VPI_RecursiveGaussianFilter Recursive Gaussian Filter
 * @ingroup VPI_API_Algorithms
 * @{
 *
 * Runs a generic 2D Gaussian filter over the input image.
 *
 * It allows bigger sigma values than the ones available in
 * \ref vpiSubmitGaussianFilter "direct-convolution-based Gaussian filter".
 * Refer to \ref algo_recursive_gaussian_filter for more details and usage examples.
 */

/**
 * Creates payload for \ref vpiSubmitRecursiveGaussianFilter.
 * This function allocates all temporary memory needed by the algorithm.
 *
 * @param[in] backends VPI backends that are eligible to execute the algorithm.
 *                     + Supported backends:
 *                       - \ref VPI_BACKEND_CPU
 *                       - \ref VPI_BACKEND_CUDA
 *                     + Backend must be enabled in current context.
 *
 * @param[in] maxWidth Maximum width in the input and output images provided for the algorithm.
 *                     + It must be >= 1.
 *
 * @param[in] maxHeight Maximum height in the input and output images provided for the algorithm.
 *                      + It must be >= 1.
 *
 * @param[out] payload Pointer to the payload variable that receives the created handle.
 *                     + It must not be NULL.
 *
 * @retval #VPI_ERROR_INVALID_ARGUMENT  \p payload is NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT  \p backends refers to an invalid backend.
 * @retval #VPI_ERROR_INVALID_ARGUMENT  \p maxWidth or \p maxHeight outside valid range.
 * @retval #VPI_ERROR_INVALID_OPERATION Backend hardware not available.
 * @retval #VPI_ERROR_NOT_IMPLEMENTED   Algorithm does not support the given backend.
 * @retval #VPI_ERROR_INVALID_CONTEXT   Current context is destroyed.
 * @retval #VPI_ERROR_OUT_OF_MEMORY     Cannot allocate required resources.
 * @retval #VPI_ERROR_INVALID_OPERATION Backend isn't enabled in current context.
 * @retval #VPI_SUCCESS                 Operation executed successfully.
 */
VPI_PUBLIC VPIStatus vpiCreateRecursiveGaussianFilter(uint64_t backends, int32_t maxWidth, int32_t maxHeight,
                                                      VPIPayload *payload);

/**
 * Runs a Recursive Gaussian Filter over an image.
 *
 * @param[in] stream The stream where the operation will be queued in.
 *                   + It must not be NULL.
 *                   + Stream must have enabled the backends that will execute the algorithm.
 *
 * @param[in] backend Backend that will execute the algorithm.
 *                    + Valid values:
 *                      - \ref VPI_BACKEND_CPU
 *                      - \ref VPI_BACKEND_CUDA
 *
 * @param[in] payload Payload created by \ref vpiCreateRecursiveGaussianFilter.
 *                    + It must not be NULL.
 *
 * @param[in] input Input image to be filtered.
 *                  + It must not be NULL.
 *                  + Image must have enabled the backends that will execute the algorithm.
 *                  + It must have dimensions less or equal maximum dimensions set in payload.
 *                  + The accepted image formats are:
 *                    - \ref VPI_IMAGE_FORMAT_Y8
 *                    - \ref VPI_IMAGE_FORMAT_Y16
 *                    - \ref VPI_IMAGE_FORMAT_Y8_ER
 *                    - \ref VPI_IMAGE_FORMAT_Y16_ER
 *                    - \ref VPI_IMAGE_FORMAT_U8
 *                    - \ref VPI_IMAGE_FORMAT_S8
 *                    - \ref VPI_IMAGE_FORMAT_U16
 *                    - \ref VPI_IMAGE_FORMAT_S16
 *                    - \ref VPI_IMAGE_FORMAT_F32
 *
 * @param[out] output Image where the result will be written to.
 *                    + Must not be NULL.
 *                    + Image must have enabled the backends that will execute the algorithm.
 *                    + Must have same dimensions and format as input image.
 *
 * @param[in] sigmaX Standard deviation for the Gaussian filter in the X direction.
 *                   + It must be > 0.
 *
 * @param[in] sigmaY Standard deviation for the Gaussian filter in the Y direction.
 *                   + It must be > 0.
 *                   + It must be equal to sigmaX.
 *
 * @param[in] border How to handle pixels outside image boundaries.
 *                   + The accepted border extensions are:
 *                     - \ref VPI_BORDER_ZERO
 *                     - \ref VPI_BORDER_CLAMP
 *                     - \ref VPI_BORDER_REFLECT
 *
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p stream is NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p input or \p output are NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p payload is not generated using vpiCreateRecursiveGaussianFilter.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p input or \p output image dimensions outside valid range.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p input and \p output image dimensions and format must be the same.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p input and \p output dimensions are bigger than the one defined in the \p payload.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p border not supported.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p backend is invalid or unknown.
 * @retval #VPI_ERROR_INVALID_IMAGE_FORMAT \p input or \p output image format not supported.
 * @retval #VPI_ERROR_INVALID_PAYLOAD_TYPE \p payload is invalid.
 * @retval #VPI_ERROR_NOT_IMPLEMENTED      Algorithm does not support the given backend.
 * @retval #VPI_ERROR_INVALID_OPERATION    Backend hardware not available.
 * @retval #VPI_ERROR_INVALID_OPERATION    The needed backends aren't enabled in \p stream, \p input or \p output.
 * @retval #VPI_SUCCESS                    Operation executed successfully.
 */
VPI_PUBLIC VPIStatus vpiSubmitRecursiveGaussianFilter(VPIStream stream, uint64_t backend, VPIPayload payload,
                                                      VPIImage input, VPIImage output, float sigmaX, float sigmaY,
                                                      VPIBorderExtension border);

#ifdef __cplusplus
}
#endif

/** @} */ // end of VPI_RecursiveGaussianFilter

#endif /* NV_VPI_ALGORITHMS_RECURSIVE_GAUSSIAN_FILTER_H */
