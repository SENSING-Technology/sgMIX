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
 * @file GaussianFilter.h
 *
 * Declares functions that implement the Gaussian Filter algorithm.
 */

#ifndef NV_VPI_ALGORITHMS_GAUSSIAN_FILTER_H
#define NV_VPI_ALGORITHMS_GAUSSIAN_FILTER_H

#include "vpi/Export.h"
#include "vpi/Status.h"
#include "vpi/Types.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup VPI_GaussianFilter Gaussian Filter
 * @ingroup VPI_API_Algorithms
 * @{
 *
 * Runs a generic 2D Gaussian filter over the input image.
 * Refer to \ref algo_gaussian_filter for more details and usage examples.
 */

/**
 * Runs a 2D Gaussian filter over an image.
 *
 * @param[in] stream The stream where the operation will be queued in.
 *                   + Must not be NULL.
 *                   + Stream must have enabled the backends that will execute the algorithm.
 *
 * @param[in] backend Backend that will execute the algorithm.
 *                    + Accepted values:
 *                      - \ref VPI_BACKEND_CPU
 *                      - \ref VPI_BACKEND_CUDA
 *                      - \ref VPI_BACKEND_PVA
 *
 * @param[in] input Input image to be filtered.
 *                  + Must not be NULL.
 *                  + Image must have enabled the backends that will execute the algorithm.
 *                  + On PVA, image dimensions must be between 64x32 and 3264x2448.
 *                  + The accepted image formats are:
 *                    | Formats                      | CPU | CUDA | PVA |
 *                    |------------------------------|:---:|:----:|:---:|
 *                    | \ref VPI_IMAGE_FORMAT_U8     |  *  |   *  |  *  |
 *                    | \ref VPI_IMAGE_FORMAT_S8     |  *  |   *  |  *  |
 *                    | \ref VPI_IMAGE_FORMAT_U16    |  *  |   *  |  *  |
 *                    | \ref VPI_IMAGE_FORMAT_S16    |  *  |   *  |  *  |
 *                    | \ref VPI_IMAGE_FORMAT_Y8     |  *  |   1  |  *  |
 *                    | \ref VPI_IMAGE_FORMAT_Y8_ER  |  *  |   1  |  *  |
 *                    | \ref VPI_IMAGE_FORMAT_Y16    |  *  |   1  |  *  |
 *                    | \ref VPI_IMAGE_FORMAT_Y16_ER |  *  |   1  |  *  |
 *                    | \ref VPI_IMAGE_FORMAT_F32    |  *  |   *  |     |
 *                    | \ref VPI_IMAGE_FORMAT_RGB8p  |  2  |      |     |
 *                    | \ref VPI_IMAGE_FORMAT_RGBA8p |  2  |      |     |
 *                    | \ref VPI_IMAGE_FORMAT_BGR8p  |  2  |      |     |
 *                    | \ref VPI_IMAGE_FORMAT_BGRA8p |  2  |      |     |
 *                    (1) only for kernel dimensions <= 3x3
 *                    (2) only for kernel dimensions >= 3x3
 *
 * @param[out] output Image where the result will be written to.
 *                    + Must not be NULL.
 *                    + Image must have enabled the backends that will execute the algorithm.
 *                    + Must have same dimensions as input image.
 *
 * @param[in] kernelSizeX, kernelSizeY Gaussian kernel size in X and Y directions respectively.
 *                                     If 0, it'll be `2*ceil(3*sigma)` rounded to the next odd size.
 *                                     + Limited between 1 and 11.
 *                                     + Must be odd.
 *
 * @param[in] sigmaX, sigmaY Standard deviation of the Gaussian kernel in the X and Y directions respectively.
 *                           + It must be a positive value.
 *                           + If `kernelSize==0`, sigma is limited to 2.
 *
 * @param[in] border How to handle pixels outside image boundaries.
 *                   + The accepted border extensions are:
 *                     - \ref VPI_BORDER_ZERO
 *                     - \ref VPI_BORDER_CLAMP
 *
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p stream is NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p input or \p output are NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p kernelSizeX or \p kernelSizeY outside valid range.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p sigmaX or \p sigmaY outside valid range.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p input or \p output image dimensions outside valid range.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p input and \p output image dimensions and format must be the same.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p border not supported.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p backend is invalid or unknown.
 * @retval #VPI_ERROR_INVALID_IMAGE_FORMAT \p input or \p output image format not supported.
 * @retval #VPI_ERROR_NOT_IMPLEMENTED      Gaussian Filter algorithm is not supported by given backend.
 * @retval #VPI_ERROR_INVALID_OPERATION    Backend hardware not available.
 * @retval #VPI_ERROR_INVALID_OPERATION    The needed backends aren't enabled in \p stream, \p input or \p output.
 * @retval #VPI_SUCCESS                    Operation executed successfully.
 */
VPI_PUBLIC VPIStatus vpiSubmitGaussianFilter(VPIStream stream, uint64_t backend, VPIImage input, VPIImage output,
                                             int32_t kernelSizeX, int32_t kernelSizeY, float sigmaX, float sigmaY,
                                             VPIBorderExtension border);

#ifdef __cplusplus
}
#endif

/** @} end of VPI_GaussianFilter */

#endif /* NV_VPI_ALGORITHMS_GAUSSIAN_FILTER_H */
