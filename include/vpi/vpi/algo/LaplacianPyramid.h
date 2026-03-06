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
 * @file LaplacianPyramid.h
 *
 * Declares functions that handle Laplacian pyramids.
 */

#ifndef NV_VPI_ALGORITHMS_LAPLACIAN_PYRAMID_H
#define NV_VPI_ALGORITHMS_LAPLACIAN_PYRAMID_H

#include "../Export.h"
#include "../Status.h"
#include "../Types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup VPI_LaplacianPyramid Laplacian Pyramid Generator
 * @ingroup VPI_API_Algorithms
 * @{
 *
 * Creates a Laplacian pyramid from the input image.
 * Refer to \ref algo_laplacian_pyramid_generator for more details and usage examples.
 */

/**
 * Computes the Laplacian pyramid from the input image.
 *
 * @note The coarsest level of the Laplacian pyramid is equivalent in concept to that of the Gaussian pyramid.
 * However, in cases where the Laplacian pyramid output format has less positive dynamic range than the input format,
 * i.e. input format is \ref VPI_IMAGE_FORMAT_U8 and output format is \ref VPI_IMAGE_FORMAT_S8 or U16 and S16, the pixel values
 * of the output in the coarsest level are divided by 2 in order to avoid overflow.
 *
 * @param[in] stream The stream where the operation will be queued in.
 *                   + Must not be NULL.
 *                   + Stream must have enabled the backends that will execute the algorithm.
 *
 * @param[in] backend VPI backend that will execute the algorithm.
 *                    + Valid values: 
 *                      - \ref VPI_BACKEND_CPU
 *                      - \ref VPI_BACKEND_CUDA
 *
 * @param[in] input Input image.
 *                  + Must not be NULL.
 *                  + Image must have enabled the backends that will execute the algorithm.
 *                  + Supported formats are:
 *                    - \ref VPI_IMAGE_FORMAT_U8
 *                    - \ref VPI_IMAGE_FORMAT_U16
 *                    - \ref VPI_IMAGE_FORMAT_F32
 *                    - \ref VPI_IMAGE_FORMAT_Y8
 *                    - \ref VPI_IMAGE_FORMAT_Y8_ER
 *                    - \ref VPI_IMAGE_FORMAT_Y16
 *                    - \ref VPI_IMAGE_FORMAT_Y16_ER
 *
 * @param[out] output Where the resulting Laplacian pyramid will be written to. 
 *                    It must have been created with the desired scale and number of levels. 
 *                    + Must not be NULL.
 *                    + Dimensions of first level must be the same as \p input.
 *                    + Pyramid must have enabled the backends that will execute the algorithm.
 *                    + Supported formats:
 *                      - \ref VPI_IMAGE_FORMAT_S8 (only for 8-bit inputs)
 *                      - \ref VPI_IMAGE_FORMAT_S16 (only for integer inputs)
 *                      - \ref VPI_IMAGE_FORMAT_F32
 *
 * @param[out] gaussianPyr Defines where the intermediate Gaussian pyramid will be written to. 
 *                         Pass NULL if not needed.
 *                         + Must have same format as \p input.
 *                         + Must have same scale, number of levels and dimensions as \p output.
 *                         + Pyramid must have enabled the backends that will execute the algorithm.
 *
 * @param[in] border Border extension to be used when sampling pixels outside the image border.
 *                   + Valid values:
 *                    | Border                  | CPU | CUDA | PVA |
 *                    |-------------------------|:---:|:----:|:---:|
 *                    | \ref VPI_BORDER_ZERO    |  *  |   *  |  *  |
 *                    | \ref VPI_BORDER_CLAMP   |  *  |   *  |  *  |
 *                    | \ref VPI_BORDER_REFLECT |     |   *  |     |
 *                    | \ref VPI_BORDER_MIRROR  |     |   *  |     |
 *
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p stream is NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p input or \p output are NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p gaussianPyr and \p output must have the same scale, number of levels and dimensions.
 * @retval #VPI_ERROR_INVALID_IMAGE_FORMAT \p input and \p gaussianPyr must have the same format.
 * @retval #VPI_ERROR_INVALID_IMAGE_FORMAT \p output format not compatible with \p input format.
 * @retval #VPI_ERROR_INVALID_IMAGE_FORMAT Unsupported \p input image format.
 * @retval #VPI_ERROR_INVALID_OPERATION    The needed backends aren't enabled in \p stream, \p input, \p output or \p gaussianPyr.
 * @retval #VPI_ERROR_NOT_IMPLEMENTED      Laplacian Pyramid algorithm is not supported by given backend.
 * @retval #VPI_SUCCESS                    Operation executed successfully.
 */

VPI_PUBLIC VPIStatus vpiSubmitLaplacianPyramidGenerator(VPIStream stream, uint64_t backend, VPIImage input,
                                                        VPIPyramid output, VPIPyramid gaussianPyr,
                                                        VPIBorderExtension border);

#ifdef __cplusplus
}
#endif

/** @} */

#endif /* NV_VPI_ALGORITHMS_LAPLACIAN_PYRAMID_H */
