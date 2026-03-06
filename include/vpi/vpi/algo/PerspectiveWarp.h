/*
 * Copyright 2020-2022 NVIDIA Corporation. All rights reserved.
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
 * @file PerspectiveWarp.h
 *
 * Declares functions that implement the Perspective Warp algorithm.
 */

#ifndef NV_VPI_ALGORITHMS_PERSPECTIVE_WARP_H
#define NV_VPI_ALGORITHMS_PERSPECTIVE_WARP_H

#include "../AlgoFlags.h"
#include "../Export.h"
#include "../Status.h"
#include "../Types.h"
#include "../WarpMap.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup VPI_PerspectiveWarp Perspective Warp
 * @ingroup VPI_API_Algorithms
 * @{
 *
 * Applies a perspective warp on an image.
 * Refer to \ref algo_persp_warp for more details and usage examples.
 */

/**
 * Submits a \ref algo_persp_warp "Perspective Warp" operation to the stream.
 *
 * @param[in] stream The stream where the operation will be queued in.
 *                   + Must not be NULL.
 *                   + Stream must have enabled the backends that will execute the algorithm.
 *
 * @param[in] backend VPI backends that are eligible to execute the algorithm.
 *                     + Valid values:
 *                       - \ref VPI_BACKEND_CPU
 *                       - \ref VPI_BACKEND_CUDA
 *                       - \ref VPI_BACKEND_VIC
 *
 * @param[in] input Input image to be warped.
 *                  + Must not be NULL.
 *                  + Image must have enabled the backends that will execute the algorithm.
 *                  + On VIC, minimum input dimensions is 64x16 and even.
 *                  + The accepted image formats are:
 *                    | Formats                            | CPU | CUDA | VIC |
 *                    |------------------------------------|:---:|:----:|:---:|
 *                    | \ref VPI_IMAGE_FORMAT_U8           |  *  |   *  |  *  |
 *                    | \ref VPI_IMAGE_FORMAT_U8_BL        |     |      |  *  |
 *                    | \ref VPI_IMAGE_FORMAT_U8_BL16      |     |      |  *  |
 *                    | \ref VPI_IMAGE_FORMAT_S8           |  *  |   *  |     |
 *                    | \ref VPI_IMAGE_FORMAT_U16          |  *  |   *  |     |
 *                    | \ref VPI_IMAGE_FORMAT_S16          |  *  |   *  |     |
 *                    | \ref VPI_IMAGE_FORMAT_F32          |  *  |   *  |     |
 *                    | \ref VPI_IMAGE_FORMAT_Y8_ER        |  *  |   *  |  *  |
 *                    | \ref VPI_IMAGE_FORMAT_Y8_ER_BL     |     |      |  *  |
 *                    | \ref VPI_IMAGE_FORMAT_Y8_ER_BL16   |     |      |  *  |
 *                    | \ref VPI_IMAGE_FORMAT_Y16_ER       |  *  |   *  |     |
 *                    | \ref VPI_IMAGE_FORMAT_RGB8         |  *  |   *  |     |
 *                    | \ref VPI_IMAGE_FORMAT_BGR8         |  *  |   *  |     |
 *                    | \ref VPI_IMAGE_FORMAT_RGBA8        |  *  |   *  |     |
 *                    | \ref VPI_IMAGE_FORMAT_BGRA8        |  *  |   *  |     |
 *                    | \ref VPI_IMAGE_FORMAT_NV12         |     |      |  *  |
 *                    | \ref VPI_IMAGE_FORMAT_NV12_ER      |  *  |   *  |  *  |
 *                    | \ref VPI_IMAGE_FORMAT_NV12_BL      |     |      |  *  |
 *                    | \ref VPI_IMAGE_FORMAT_NV12_BL16    |     |      |  *  |
 *                    | \ref VPI_IMAGE_FORMAT_NV12_ER_BL   |     |      |  *  |
 *                    | \ref VPI_IMAGE_FORMAT_NV12_ER_BL16 |     |      |  *  |
 *                    | \ref VPI_IMAGE_FORMAT_NV24         |     |      |  *  |
 *                    | \ref VPI_IMAGE_FORMAT_NV24_ER      |     |      |  *  |
 *
 * @param[in] xform Transform to be applied.
 *
 * @param[out] output Output image where warped image is written to.
 *                    Dimensions may be different from \p input.
 *                    + Must not be NULL.
 *                    + Image must have enabled the backends that will execute the algorithm.
 *                    + Must have same format as input image.
 *                    + Dimensions must be even.
 *
 * @param[in] grid Grid on the output to establish performance/quality trade-offs.
 *                 A dense grid will result in best quality, albeit slower performance.
 *                 Grid must be set depending on the perf/quality criteria needed.
 *                 Pass NULL as a shortcut for using a dense grid.
 *                 + CPU and CUDA only accept dense grids.
 *                 + Grid dimensions must match \p output dimensions.
 *
 * @param[in] interp Interpolation mode to be used when source coordinate doesn't fall exactly on pixel center.
 *                   + Valid values:
 *                     - \ref VPI_INTERP_NEAREST
 *                     - \ref VPI_INTERP_LINEAR
 *                     - \ref VPI_INTERP_CATMULL_ROM
 *
 * @param[in] border Border extension to use for samples that fall outsize input's bounds.
 *                   + The accepted border extensions are:
 *                     - \ref VPI_BORDER_ZERO
 *
 * @param[in] flags Flags to modify algorithm behavior.
 *                  + Can be a combination of one or more of the following values:
 *                    - 0 : default, transform maps the input into the output
 *                    - \ref VPI_WARP_INVERSE : transform maps the output back into the input.
 *                    - \ref VPI_PRECISE      : precise, but potentially slower implementation.
 *
 * @retval #VPI_ERROR_INVALID_ARGUMENT      \p stream is NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT      \p input or \p output are NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT      \p xform is NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT      Invalid \p flags.
 * @retval #VPI_ERROR_INVALID_ARGUMENT      \p input and \p output must have the same format.
 * @retval #VPI_ERROR_INVALID_ARGUMENT      \p border not supported.
 * @retval #VPI_ERROR_INVALID_ARGUMENT      \p backend is invalid or unknown.
 * @retval #VPI_ERROR_INVALID_ARGUMENT      \p input and/or \p output dimensions not supported.
 * @retval #VPI_ERROR_INVALID_ARGUMENT      \p grid configuration not supported by backend.
 * @retval #VPI_ERROR_INVALID_ARGUMENT      \p grid dimensions don't match output image's.
 * @retval #VPI_ERROR_INVALID_IMAGE_FORMAT  Image format not supported.
 * @retval #VPI_ERROR_NOT_IMPLEMENTED       Perspective Warp algorithm is not supported by given backend.
 * @retval #VPI_ERROR_INVALID_OPERATION     Backend hardware not available.
 * @retval #VPI_ERROR_INVALID_OPERATION     The needed backends aren't enabled in \p stream, \p input or \p output.
 * @retval #VPI_SUCCESS                     Operation executed successfully.
 */
VPI_PUBLIC VPIStatus vpiSubmitPerspectiveWarp(VPIStream stream, uint64_t backend, VPIImage input,
                                              const VPIPerspectiveTransform xform, VPIImage output,
                                              const VPIWarpGrid *grid, VPIInterpolationType interp,
                                              VPIBorderExtension border, uint64_t flags);

/** @} end of VPI_PerspectiveWarp */

#ifdef __cplusplus
}
#endif

#endif /* NV_VPI_ALGORITHMS_PERSPECTIVE_WARP_H */
