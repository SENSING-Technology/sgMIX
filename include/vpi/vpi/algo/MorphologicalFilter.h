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
 * @file MorphologicalFilter.h
 *
 * Declares functions to perform image morphological filtering with binary kernels.
 *
 */

#ifndef NV_VPI_ALGORITHMS_MORPHOLOGICAL_FILTER_H
#define NV_VPI_ALGORITHMS_MORPHOLOGICAL_FILTER_H

#include "../Export.h"
#include "../Status.h"
#include "../Types.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup VPI_Erode Erode
 * @ingroup VPI_API_Algorithms
 * @{
 *
 * Filter an image with a 2D binary kernel composed with the erode morphological operation.
 *
 * Refer to \ref algo_erode for more details and usage examples regarding Erode.
 *
 */

/**
 * Runs a 2D erode over an image.
 *
 * @param[in] stream The stream handle where the operation will be queued in.
 *                   + Must not be NULL.
 *                   + Stream must have enabled the backends that will execute the algorithm.
 *
 * @param[in] backend Backend that will execute the algorithm.
 *                    + Valid values: 
 *                      - \ref VPI_BACKEND_CPU
 *                      - \ref VPI_BACKEND_CUDA
 *                      - \ref VPI_BACKEND_PVA
 *
 * @param[in] input Input image to be eroded with the kernel.
 *                  + Must not be NULL.
 *                  + Image must have enabled the backends that will execute the algorithm.
 *                  + Supported formats:
 *                    - \ref VPI_IMAGE_FORMAT_U8
 *                    - \ref VPI_IMAGE_FORMAT_S8
 *                    - \ref VPI_IMAGE_FORMAT_U16
 *                    - \ref VPI_IMAGE_FORMAT_S16
 *                  + For PVA backend, Image size must be greater than 128x128.
 *
 * @param[out] output Output image where the result is written to.
 *                    + Must not be NULL.
 *                    + Must have same format and dimensions as input image.
 *                    + Image must have enabled the backends that will execute the algorithm.
 *
 * @param[in] kernelWidth, kernelHeight Kernel dimensions.
 *                                      + Must be between 1x1 and 11x11, and can be non-square. (PVA only supports 3x3 and 5x5 square sizes)
 *
 * @param[in] kernelData Erode kernel binary mask, i.e. structuring element or neighborhood definition, in row-major layout.
 *                       The kernel elements are copied to an internal buffer.
 *                       The buffers passed can be deallocated after the call.
 *                       Use NULL for full neighborhood, all elements considered to be 1.
 *                       + If not NULL, it must point to a buffer with \p kernelWidth * \p kernelHeight elements. (PVA only supports 3x3, 5x5 square and 3x3 cross kernels)
 *
 * @param[in] border How to handle pixels outside image boundaries.
 *                   + The accepted border extensions are:
 *                    - \ref VPI_BORDER_ZERO
 *                    - \ref VPI_BORDER_CLAMP
 *                    - \ref VPI_BORDER_REFLECT
 *                    - \ref VPI_BORDER_MIRROR
 *                    - \ref VPI_BORDER_LIMITED (ignore pixels outside image boundaries)
 *
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p stream is NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p input or \p output are NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p kernelWidth or \p kernelHeight outside valid range.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p input and \p output images must have same dimensions and format.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p border not supported.
 * @retval #VPI_ERROR_INVALID_IMAGE_FORMAT \p input format not supported.
 * @retval #VPI_ERROR_NOT_IMPLEMENTED      Erode algorithm is not supported by given backend.
 * @retval #VPI_ERROR_INVALID_OPERATION    The needed backends aren't enabled in \p stream, \p input or \p output.
 * @retval #VPI_SUCCESS                    Operation executed successfully.
 */
VPI_PUBLIC VPIStatus vpiSubmitErode(VPIStream stream, uint64_t backend, VPIImage input, VPIImage output,
                                    const int8_t *kernelData, int32_t kernelWidth, int32_t kernelHeight,
                                    VPIBorderExtension border);

/** @} */

/**
 * @defgroup VPI_Dilate Dilate
 * @ingroup VPI_API_Algorithms
 * @{
 *
 * Filter an image with a 2D binary kernel composed with the dilate morphological operation.
 *
 * Refer to \ref algo_dilate for more details and usage examples regarding Dilate.
 *
 */

/**
 * Runs a 2D dilate over an image.
 *
 * @param[in] stream The stream handle where the operation will be queued in.
 *                   + Must not be NULL.
 *                   + Stream must have enabled the backends that will execute the algorithm.
 *
 * @param[in] backend Backend that will execute the algorithm.
 *                    + Valid values: 
 *                      - \ref VPI_BACKEND_CPU
 *                      - \ref VPI_BACKEND_CUDA
 *                      - \ref VPI_BACKEND_PVA
 *  
 * @param[in] input Input image to be dilated with the kernel.
 *                  + Must not be NULL.
 *                  + Image must have enabled the backends that will execute the algorithm.
 *                  + Supported formats:
 *                    - \ref VPI_IMAGE_FORMAT_U8
 *                    - \ref VPI_IMAGE_FORMAT_S8
 *                    - \ref VPI_IMAGE_FORMAT_U16
 *                    - \ref VPI_IMAGE_FORMAT_S16
 *                  + For PVA backend, Image size must be greater than 128x128.
 *
 * @param[out] output Output image where the result is written to.
 *                    + Must not be NULL.
 *                    + Its image format and dimensions must be the same as input.
 *                    + Image must have enabled the backends that will execute the algorithm.
 *
 * @param[in] kernelWidth, kernelHeight Kernel dimensions.
 *                                      + Must be between 1x1 and 11x11, and can be non-square. (PVA only supports 3x3 and 5x5 square sizes)
 *
 * @param[in] kernelData Dilate kernel binary mask, i.e. structuring element or neighborhood definition, in row-major layout.
 *                       The kernel elements are copied to an internal buffer.
 *                       The buffers passed can be deallocated after the call.
 *                       Use NULL for full neighborhood, all elements considered to be 1.
 *                       + If not NULL, it must point to a buffer with \p kernelWidth * \p kernelHeight elements. (PVA only supports 3x3, 5x5 square and 3x3 cross kernels)
 *
 * @param[in] border How to handle pixels outside image boundaries.
 *                   + The accepted border extensions are:
 *                    - \ref VPI_BORDER_ZERO
 *                    - \ref VPI_BORDER_CLAMP
 *                    - \ref VPI_BORDER_REFLECT (Not supported for PVA)
 *                    - \ref VPI_BORDER_MIRROR  (Not supported for PVA)
 *                    - \ref VPI_BORDER_LIMITED (ignore pixels outside image boundaries)
 *
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p stream is NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p input or \p output are NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p kernelWidth or \p kernelHeight outside valid range.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p input and \p output images must have same dimensions and format.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p border not supported.
 * @retval #VPI_ERROR_INVALID_IMAGE_FORMAT \p input format not supported.
 * @retval #VPI_ERROR_NOT_IMPLEMENTED      Dilate algorithm is not supported by given backend.
 * @retval #VPI_ERROR_INVALID_OPERATION    The needed backends aren't enabled in \p stream, \p input or \p output.
 * @retval #VPI_SUCCESS                    Operation executed successfully.
 */
VPI_PUBLIC VPIStatus vpiSubmitDilate(VPIStream stream, uint64_t backend, VPIImage input, VPIImage output,
                                     const int8_t *kernelData, int32_t kernelWidth, int32_t kernelHeight,
                                     VPIBorderExtension border);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* NV_VPI_ALGORITHMS_MORPHOLOGICAL_FILTER_H */
