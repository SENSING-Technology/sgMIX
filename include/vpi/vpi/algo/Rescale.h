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
 * @file Rescale.h
 *
 * Declares functions that implement the Rescale algorithm.
 */

#ifndef NV_VPI_ALGORITHMS_RESCALE_H
#define NV_VPI_ALGORITHMS_RESCALE_H

#include "../AlgoFlags.h"
#include "../Export.h"
#include "../Status.h"
#include "../Types.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup VPI_Rescale Rescale
 * @ingroup VPI_API_Algorithms
 * @{
 *
 * Changes the dimensions of the input image while stretching/squeezing it.
 * Refer to \ref algo_rescale for more details and usage examples.
 */

/**
 * Changes the size and scale of a 2D image.
 *
 * @param[in] stream The stream where the operation will be queued in.
 *                   + Must not be NULL.
 *                   + Stream must have enabled the backends that will execute the algorithm.
 *
 * @param[in] backend VPI backend that will execute the algorithm.
 *                    + Valid values:
 *                      - \ref VPI_BACKEND_CPU
 *                      - \ref VPI_BACKEND_CUDA
 *                      - \ref VPI_BACKEND_VIC
 *
 * @param[in] input Input image to be rescaled.
 *                  + Must not be NULL.
 *                  + Image must have enabled the backends that will execute the algorithm.
 *                  + Dimensions must be >= 1x1. On VIC, valid range is >= 16x16 and <= 16384x16384.
 *                  + On VIC, when using YUV formats with chroma-subsampling of 4:2:0, 4:2:2 or 4:2:2R,
 *                    the image dimensions must be even.
 *                  + Supported image formats are:
 *                    | Format                             | CPU | CUDA | VIC |
 *                    |------------------------------------|:---:|:----:|:---:|
 *                    | \ref VPI_IMAGE_FORMAT_U8           |  *  |   *  |     |
 *                    | \ref VPI_IMAGE_FORMAT_U16          |  *  |   *  |     |
 *                    | \ref VPI_IMAGE_FORMAT_S8           |  *  |   *  |     |
 *                    | \ref VPI_IMAGE_FORMAT_S16          |  *  |   *  |     |
 *                    | \ref VPI_IMAGE_FORMAT_F32          |  *  |   *  |     |
 *                    | \ref VPI_IMAGE_FORMAT_Y8           |  *  |   *  |  *  |
 *                    | \ref VPI_IMAGE_FORMAT_Y8_ER        |  *  |   *  |  *  |
 *                    | \ref VPI_IMAGE_FORMAT_Y8_BL        |     |      |  *  |
 *                    | \ref VPI_IMAGE_FORMAT_Y8_ER_BL     |     |      |  *  |
 *                    | \ref VPI_IMAGE_FORMAT_Y8_BL16      |     |      |  *  |
 *                    | \ref VPI_IMAGE_FORMAT_Y8_ER_BL16   |     |      |  *  |
 *                    | \ref VPI_IMAGE_FORMAT_Y16          |  *  |   *  |  *  |
 *                    | \ref VPI_IMAGE_FORMAT_Y16_ER       |  *  |   *  |  *  |
 *                    | \ref VPI_IMAGE_FORMAT_Y16_BL       |     |      |  *  |
 *                    | \ref VPI_IMAGE_FORMAT_Y16_ER_BL    |     |      |  *  |
 *                    | \ref VPI_IMAGE_FORMAT_Y16_BL16     |     |      |  *  |
 *                    | \ref VPI_IMAGE_FORMAT_Y16_ER_BL16  |     |      |  *  |
 *                    | \ref VPI_IMAGE_FORMAT_RGB8         |  *  |   *  |     |
 *                    | \ref VPI_IMAGE_FORMAT_BGR8         |  *  |   *  |     |
 *                    | \ref VPI_IMAGE_FORMAT_RGBA8        |  *  |   *  |  *  |
 *                    | \ref VPI_IMAGE_FORMAT_BGRA8        |  *  |   *  |  *  |
 *                    | \ref VPI_IMAGE_FORMAT_RGB8p        |  *  |   *  |     |
 *                    | \ref VPI_IMAGE_FORMAT_BGR8p        |  *  |   *  |     |
 *                    | \ref VPI_IMAGE_FORMAT_RGBA8p       |  *  |   *  |     |
 *                    | \ref VPI_IMAGE_FORMAT_BGRA8p       |  *  |   *  |     |
 *                    | \ref VPI_IMAGE_FORMAT_NV12         |  *  |   *  |  *  |
 *                    | \ref VPI_IMAGE_FORMAT_NV12_ER      |  *  |   *  |  *  |
 *                    | \ref VPI_IMAGE_FORMAT_NV12_BL      |     |      |  *  |
 *                    | \ref VPI_IMAGE_FORMAT_NV12_ER_BL   |     |      |  *  |
 *                    | \ref VPI_IMAGE_FORMAT_NV12_BL16    |     |      |  *  |
 *                    | \ref VPI_IMAGE_FORMAT_NV12_ER_BL16 |     |      |  *  |
 *                    | \ref VPI_IMAGE_FORMAT_NV24         |  *  |   *  |  *  |
 *                    | \ref VPI_IMAGE_FORMAT_NV24_ER      |  *  |   *  |  *  |
 *                    | \ref VPI_IMAGE_FORMAT_NV24_BL      |     |      |  *  |
 *                    | \ref VPI_IMAGE_FORMAT_NV24_ER_BL   |     |      |  *  |
 *                    | \ref VPI_IMAGE_FORMAT_NV24_BL16    |     |      |  *  |
 *                    | \ref VPI_IMAGE_FORMAT_NV24_ER_BL16 |     |      |  *  |
 *                    | \ref VPI_IMAGE_FORMAT_UYVY         |     |      |  *  |
 *                    | \ref VPI_IMAGE_FORMAT_UYVY_ER      |     |      |  *  |
 *                    | \ref VPI_IMAGE_FORMAT_UYVY_BL      |     |      |  *  |
 *                    | \ref VPI_IMAGE_FORMAT_UYVY_ER_BL   |     |      |  *  |
 *                    | \ref VPI_IMAGE_FORMAT_UYVY_BL16    |     |      |  *  |
 *                    | \ref VPI_IMAGE_FORMAT_UYVY_ER_BL16 |     |      |  *  |
 *                    | \ref VPI_IMAGE_FORMAT_YUYV         |     |      |  *  |
 *                    | \ref VPI_IMAGE_FORMAT_YUYV_ER      |     |      |  *  |
 *                    | \ref VPI_IMAGE_FORMAT_YUYV_BL      |     |      |  *  |
 *                    | \ref VPI_IMAGE_FORMAT_YUYV_ER_BL   |     |      |  *  |
 *                    | \ref VPI_IMAGE_FORMAT_YUYV_BL16    |     |      |  *  |
 *                    | \ref VPI_IMAGE_FORMAT_YUYV_ER_BL16 |     |      |  *  |
 *
 * @param[out] output Output image with the desired resulting dimensions.
 *                    + Must not be NULL.
 *                    + Image must have enabled the backends that will execute the algorithm.
 *                    + Dimensions must be >= 1x1. On VIC, valid range is >= 16x16 and <= 16384x16384.
 *                    + On VIC, when using YUV formats with chroma-subsampling of 4:2:0, 4:2:2 or 4:2:2R,
 *                      the image dimensions must be even.
 *                    + On VIC, scale factor (output/input) must be between 1/16.0f and 16.0f.
 *                    + On VIC, input and output formats can be different. Accepted formats are
 *                      the same as \p input, provided that the following constraints are met:
 *                      - No bit depth conversion is allowed, i.e., Y8 to Y16, Y16 to NV12, etc.
 *                      - No conversion from/to Y16_ER, except when only changing layout.
 *                        Example: Y16_ER_BL to Y16_ER is allowed, but Y16_ER_BL to Y16 or Y16_BL isn't.
 *                    + On other backends, input and output formats must be the same.
 *
 * @param[in] interpolationType Interpolation method to be used.
 *                              + Valid values:
 *                                - \ref VPI_INTERP_NEAREST
 *                                - \ref VPI_INTERP_LINEAR (VIC only supports this one)
 *                                - \ref VPI_INTERP_CATMULL_ROM
 *
 * @param[in] border Border extension to be used when sampling pixels outside the image border.
 *                   + The accepted border extensions are:
 *                     - \ref VPI_BORDER_ZERO
 *                     - \ref VPI_BORDER_CLAMP (VIC only supports this one)
 *
 * @param[in] flags Control flags.
 *                  + Valid values are a combination of one or more of the following flags:
 *                    - 0: default, negation of all other flags.
 *                    - \ref VPI_PRECISE : precise, but potentially slower implementation.
 *
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p stream is NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p input or \p output are NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p input or \p output dimensions are outside valid range.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p flags has some unsupported flag(s).
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p border not supported.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p interpolationType not supported.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p backend is invalid or unknown.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p input and \p output formats are not compatible.
 * @retval #VPI_ERROR_INVALID_IMAGE_FORMAT \p input or \p output format not supported.
 * @retval #VPI_ERROR_NOT_IMPLEMENTED      Rescale algorithm is not supported by given backend.
 * @retval #VPI_ERROR_INVALID_OPERATION    Backend hardware not available.
 * @retval #VPI_ERROR_INVALID_OPERATION    The needed backends aren't enabled in \p stream, \p input or \p output.
 * @retval #VPI_SUCCESS                    Operation executed successfully.
 */
VPI_PUBLIC VPIStatus vpiSubmitRescale(VPIStream stream, uint64_t backend, VPIImage input, VPIImage output,
                                      VPIInterpolationType interpolationType, VPIBorderExtension border,
                                      uint64_t flags);

/** @} end of VPI_Rescale */

#ifdef __cplusplus
}
#endif

#endif /* NV_VPI_ALGORITHMS_RESCALE_H */
