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
 * @file Convolution.h
 *
 * Declares functions to perform image filtering with convolution kernels.
 */

#ifndef NV_VPI_ALGORITHMS_CONVOLUTION_H
#define NV_VPI_ALGORITHMS_CONVOLUTION_H

#include "../Export.h"
#include "../Status.h"
#include "../Types.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup VPI_Convolution Convolution
 * @ingroup VPI_API_Algorithms
 * @{
 *
 * Convolves an image with a 2D kernel.
 * Refer to \ref algo_convolution for more details and usage examples regarding Convolution.
 * Refer to \ref algo_sep_convolution for more details and usage examples regarding Separable Convolution.
 *
 * \ref vpiSubmitConvolution is used for generic 2D kernels, separable or not. For separable kernels,
 * it's usually more efficient to use \ref vpiSubmitSeparableConvolution.
 *
 */

/**
 * Runs a generic 2D convolution over an image.
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
 * @param[in] input Input image to be convolved with the kernel.
 *                  + Must not be NULL.
 *                  + On PVA backend, image dimensions must be between 65x33 and 3264x2448.
 *                  + Image must have enabled the backends that will execute the algorithm.
 *                  + The accepted image formats are:
 *                    | Formats                      | CPU | CUDA | PVA |
 *                    |------------------------------|:---:|:----:|:---:|
 *                    | \ref VPI_IMAGE_FORMAT_U8     |  *  |   *  |  *  |
 *                    | \ref VPI_IMAGE_FORMAT_S8     |  *  |   *  |  *  |
 *                    | \ref VPI_IMAGE_FORMAT_U16    |  *  |   *  |  *  |
 *                    | \ref VPI_IMAGE_FORMAT_S16    |  *  |   *  |  *  |
 *                    | \ref VPI_IMAGE_FORMAT_Y8     |  *  |   *  |  *  |
 *                    | \ref VPI_IMAGE_FORMAT_Y8_ER  |  *  |   *  |  *  |
 *                    | \ref VPI_IMAGE_FORMAT_Y16    |  *  |   *  |  *  |
 *                    | \ref VPI_IMAGE_FORMAT_Y16_ER |  *  |   *  |  *  |
 *                    | \ref VPI_IMAGE_FORMAT_F32    |  *  |   *  |     |
 *
 * @param[out] output Output image where the result is written to.
 *                    + Must not be NULL.
 *                    + Must have same dimensions as input image.
 *                    + Image must have enabled the backends that will execute the algorithm.
 *                    + With PVA, it must have same format as input image.
 *                    + With other backends, format may be:
 *                      - Equal to input format.
 *                      - \ref VPI_IMAGE_FORMAT_F32 (no matter what input format).
 *
 * @param[in] kernelWidth, kernelHeight Kernel dimensions.
 *                                      + Must be between 1x1 and 11x11 and can be non-square.
 *
 * @param[in] kernelData Convolution kernel coefficients, in row-major layout.
 *                       The kernel elements are copied to an internal buffer.
 *                       The buffers passed can be deallocated after the call.
 *                       + It cannot be NULL.
 *                       + It must point to a buffer with \p kernelWidth * \p kernelHeight elements,
 *                         if not it'll lead to undefined behavior 
 *                       + On PVA backend, kernel weights are restricted to `|weight|<=1`.
 *
 * @param[in] border How to handle pixels outside image boundaries.
 *                   + The accepted border extensions are:
 *                     - \ref VPI_BORDER_ZERO
 *                     - \ref VPI_BORDER_CLAMP
 *
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p stream is NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p input or \p output are NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p kernelWidth or \p kernelHeight are outside valid range.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p kernelData is NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     Some weight in \p kernelData is outside valid range.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p input or \p output dimensions outside valid range.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p input and \p output image must have same dimensions.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p border not supported.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p backend is invalid or unknown.
 * @retval #VPI_ERROR_INVALID_IMAGE_FORMAT \p input or \p output format not supported.
 * @retval #VPI_ERROR_INVALID_IMAGE_FORMAT \p input and \p output formats are not compatible.
 * @retval #VPI_ERROR_NOT_IMPLEMENTED      Convolution algorithm is not supported by given backend.
 * @retval #VPI_ERROR_INVALID_OPERATION    Backend hardware not available.
 * @retval #VPI_ERROR_INVALID_OPERATION    The needed backends aren't enabled in \p stream, \p input or \p output.
 * @retval #VPI_SUCCESS                    Operation executed successfully.
 */
VPI_PUBLIC VPIStatus vpiSubmitConvolution(VPIStream stream, uint64_t backend, VPIImage input, VPIImage output,
                                          const float *kernelData, int32_t kernelWidth, int32_t kernelHeight,
                                          VPIBorderExtension border);

/**
 * Runs a generic 2D convolution operation over an image, optimized for separable kernels.
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
 * @param[in] input Input image to be convolved with the kernel.
 *                  + Must not be NULL.
 *                  + Image must have enabled the backends that will execute the algorithm.
 *                  + On PVA, image dimensions must be between 32x32 and 3264x2448.
 *                  + The accepted image formats are:
 *                    | Formats                      | CPU | CUDA | PVA |
 *                    |------------------------------|:---:|:----:|:---:|
 *                    | \ref VPI_IMAGE_FORMAT_U8     |  *  |   *  |     |
 *                    | \ref VPI_IMAGE_FORMAT_S8     |  *  |   *  |     |
 *                    | \ref VPI_IMAGE_FORMAT_U16    |  *  |   *  |     |
 *                    | \ref VPI_IMAGE_FORMAT_S16    |  *  |   *  |  *  |
 *                    | \ref VPI_IMAGE_FORMAT_Y8     |  *  |   1  |     |
 *                    | \ref VPI_IMAGE_FORMAT_Y8_ER  |  *  |   1  |     |
 *                    | \ref VPI_IMAGE_FORMAT_Y16    |  *  |   1  |     |
 *                    | \ref VPI_IMAGE_FORMAT_Y16_ER |  *  |   1  |     |
 *                    | \ref VPI_IMAGE_FORMAT_F32    |  *  |   *  |     |
 *                    | \ref VPI_IMAGE_FORMAT_RGB8p  |  2  |      |     |
 *                    | \ref VPI_IMAGE_FORMAT_RGBA8p |  2  |      |     |
 *                    | \ref VPI_IMAGE_FORMAT_BGR8p  |  2  |      |     |
 *                    | \ref VPI_IMAGE_FORMAT_BGRA8p |  2  |      |     |
 *                    (1) only for kernel dimensions <= 3x3
 *                    (2) only for kernel dimensions >= 3x3
 *
 * @param[out] output Output image where the result is written to.
 *                    + Must not be NULL.
 *                    + Must have same dimensions and format as input image.
 *                    + Image must have enabled the backends that will execute the algorithm.
 *
 * @param[in] kernelXSize, kernelYSize Kernel dimensions in X and Y directions.
 *                                     + Must be between 1x1 and 11x11.
 *
 * @param[in] kernelXData, kernelYData Convolution kernel coefficients, in both X and Y directions respectively.
 *                                     The kernel elements are copied to an internal buffer.
 *                                     The buffers passed can be deallocated after the call.
 *                                     + \p kernelXData and \p kernelYData must point to a buffer
 *                                       with \p kernelXSize and \p kernelYSize elements respectively.
 *                                     + They can't be NULL.
 *                                     + On PVA backend, kernel weights are restricted to `|weight|<1`.
 *
 * @param[in] border How to handle pixels outside image boundaries.
 *                   + The accepted border extensions are:
 *                     - \ref VPI_BORDER_ZERO
 *                     - \ref VPI_BORDER_CLAMP
 *
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p stream is NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p input or \p output are NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p kernelXData or \p kernelYData are NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p kernelXSize or \p kernelYSize outside valid range.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     Some weight in \p kernelData is outside valid range.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p input or \p output image dimensions outside valid range..
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p border not supported.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p input and \p output image dimensions and format do not match.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p backend is invalid or unknown.
 * @retval #VPI_ERROR_INVALID_IMAGE_FORMAT \p input and \p output formats aren't supported
 * @retval #VPI_ERROR_NOT_IMPLEMENTED      Separable Convolution algorithm is not supported by given backend.
 * @retval #VPI_ERROR_INVALID_OPERATION    Backend hardware not available.
 * @retval #VPI_ERROR_INVALID_OPERATION    The needed backends aren't enabled in \p stream, \p input or \p output.
 * @retval #VPI_SUCCESS                    Operation executed successfully.
 */
VPI_PUBLIC VPIStatus vpiSubmitSeparableConvolution(VPIStream stream, uint64_t backend, VPIImage input, VPIImage output,
                                                   const float *kernelXData, int32_t kernelXSize,
                                                   const float *kernelYData, int32_t kernelYSize,
                                                   VPIBorderExtension border);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* NV_VPI_ALGORITHMS_CONVOLUTION_H */
