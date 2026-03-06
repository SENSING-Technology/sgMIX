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
 * @file BilateralFilter.h
 *
 * Declares functions that implement the Bilateral Filter algorithm.
 */

#ifndef NV_VPI_ALGORITHMS_BILATERAL_FILTER_H
#define NV_VPI_ALGORITHMS_BILATERAL_FILTER_H

#include "../Export.h"
#include "../Status.h"
#include "../Types.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup VPI_BilateralFilter Bilateral Filter
 * @ingroup VPI_API_Algorithms
 * @{
 *
 * Runs a generic 2D bilateral filter over the input image.
 * Refer to \ref algo_bilat_filter for more details and usage examples.
 */

/**
 * Runs a 2D bilateral filter over an image.
 *
 * @param[in] stream A stream handle where the operation will be queued into.
 *                   + Must not be NULL.
 *                   + Stream must have enabled the backends that will execute the algorithm.
 *
 * @param[in] backend Backend that will execute the algorithm.
 *                    + Valid values: 
 *                      - \ref VPI_BACKEND_CPU
 *                      - \ref VPI_BACKEND_CUDA
 *
 * @param[in] input Input image to be filtered.
 *                  + Must not be NULL.
 *                  + Image must have enabled the backends that will execute the algorithm.
 *                  + The accepted image formats are:
 *                    | Formats                            | CPU | CUDA |
 *                    |------------------------------------|:---:|:----:|
 *                    | \ref VPI_IMAGE_FORMAT_U8           |  *  |   *  |
 *                    | \ref VPI_IMAGE_FORMAT_S8           |  *  |   *  |
 *                    | \ref VPI_IMAGE_FORMAT_U16          |  *  |   *  |
 *                    | \ref VPI_IMAGE_FORMAT_S16          |  *  |   *  |
 *                    | \ref VPI_IMAGE_FORMAT_Y8           |  *  |   *  |
 *                    | \ref VPI_IMAGE_FORMAT_Y8_ER        |  *  |   *  |
 *                    | \ref VPI_IMAGE_FORMAT_Y16          |  *  |   *  |
 *                    | \ref VPI_IMAGE_FORMAT_Y16_ER       |  *  |   *  |
 *                    | \ref VPI_IMAGE_FORMAT_F32          |  *  |   *  |
 *                    | \ref VPI_IMAGE_FORMAT_NV12         |     |   *  |
 *                    | \ref VPI_IMAGE_FORMAT_NV12_ER      |     |   *  |
 *                    | \ref VPI_IMAGE_FORMAT_U8_BL        |     |   *  |
 *                    | \ref VPI_IMAGE_FORMAT_U8_BL16      |     |   *  |
 *                    | \ref VPI_IMAGE_FORMAT_Y8_BL        |     |   *  |
 *                    | \ref VPI_IMAGE_FORMAT_Y8_BL16      |     |   *  |
 *                    | \ref VPI_IMAGE_FORMAT_Y8_ER_BL     |     |   *  |
 *                    | \ref VPI_IMAGE_FORMAT_Y8_ER_BL16   |     |   *  |
 *                    | \ref VPI_IMAGE_FORMAT_NV12_BL      |     |   *  |
 *                    | \ref VPI_IMAGE_FORMAT_NV12_BL16    |     |   *  |
 *                    | \ref VPI_IMAGE_FORMAT_NV12_ER_BL   |     |   *  |
 *                    | \ref VPI_IMAGE_FORMAT_NV12_ER_BL16 |     |   *  |
 *
 * @param[out] output Output image where the result will be written to.
 *                    + Must not be NULL.
 *                    + It must have the same dimensions as input.
 *                    + Its format must be one of the valid formats accept as input.
 *                    + If output format can differ from input format only regarding its memory layout (pitch- or block-linear),
 *                      but only if the input is block-linear.
 *                    + Image must have enabled the backends that will execute the algorithm.
 *
 * @param[in] kernelSize Kernel support dimensions, in pixels.
 *                       + Must be >= 1 and <= 11.
 *                       + Must be odd.
 *
 * @param[in] sigmaRange Standard deviation in color space.
 *                       + Must be > 0.
 *
 * @param[in] sigmaSpace Standard deviation in the coordinate space.
 *                       + Must be > 0.
 *
 * @param[in] border How to handle pixels outside image boundaries.
 *                   + The accepted border extensions are:
 *                     - \ref VPI_BORDER_ZERO
 *                     - \ref VPI_BORDER_CLAMP
 *
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p stream is NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p input or \p output are NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p input and \p output image dimensions or formats don't match.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p input or \p output image dimensions outside valid range.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p kernelSize outside valid range.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p sigmaRange or \p sigmaSpace outside valid range.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p backend is invalid or unknown.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p border not supported.
 * @retval #VPI_ERROR_INVALID_IMAGE_FORMAT \p input or \p output format is not supported.
 * @retval #VPI_ERROR_NOT_IMPLEMENTED      Bilateral Filter algorithm is not supported by given backend.
 * @retval #VPI_ERROR_INVALID_OPERATION    The needed backends aren't enabled in \p stream, \p input or \p output.
 * @retval #VPI_SUCCESS                    Operation executed successfully.
 */
VPI_PUBLIC VPIStatus vpiSubmitBilateralFilter(VPIStream stream, uint64_t backend, VPIImage input, VPIImage output,
                                              int32_t kernelSize, float sigmaRange, float sigmaSpace,
                                              VPIBorderExtension border);
/** @} */

#ifdef __cplusplus
}
#endif

#endif /* NV_VPI_ALGORITHMS_BILATERAL_FILTER_H */
