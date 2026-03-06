/*
 * Copyright 2022 NVIDIA Corporation. All rights reserved.
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
 * @file ImageStats.h
 *
 * Declares functions that implement image statistics algorithms.
 */

#ifndef NV_VPI_ALGORITHMS_IMAGE_STATS_H
#define NV_VPI_ALGORITHMS_IMAGE_STATS_H

#include "../Export.h"
#include "../Status.h"
#include "../Types.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup VPI_ImageStats Image Statistics
 * @ingroup VPI_API_Algorithms
 * @{
 *
 * Returns various image statistics of the input image.
 *
 */

/**
 * Returns various image statistics of the input image.
 *
 * @param[in] stream A stream handle where the operation will be queued into.
 *
 * @param[in] backend Backend that will execute the algorithm.
 *                    + Valid values: 
 *                      - \ref VPI_BACKEND_CPU
 *                      - \ref VPI_BACKEND_CUDA
 *
 * @param[in] input Input image.
 *                  + Valid Image formats:
 *                    | Formats                       | CPU | CUDA |
 *                    |-------------------------------|:---:|:----:|
 *                    | \ref VPI_IMAGE_FORMAT_U8      |  *  |   *  |
 *                    | \ref VPI_IMAGE_FORMAT_S8      |  *  |   *  |
 *                    | \ref VPI_IMAGE_FORMAT_U16     |  *  |   *  |
 *                    | \ref VPI_IMAGE_FORMAT_S16     |  *  |   *  |
 *                    | \ref VPI_IMAGE_FORMAT_Y8      |  *  |   *  |
 *                    | \ref VPI_IMAGE_FORMAT_Y8_ER   |  *  |   *  |
 *                    | \ref VPI_IMAGE_FORMAT_Y16     |  *  |   *  |
 *                    | \ref VPI_IMAGE_FORMAT_Y16_ER  |  *  |   *  |
 *                    | \ref VPI_IMAGE_FORMAT_U32     |  *  |   *  |
 *                    | \ref VPI_IMAGE_FORMAT_S32     |  *  |   *  |
 *                    | \ref VPI_IMAGE_FORMAT_RGB8    |  *  |   *  |
 *                    | \ref VPI_IMAGE_FORMAT_RGBA8   |  *  |   *  |
 *                    | \ref VPI_IMAGE_FORMAT_BGR8    |  *  |   *  |
 *                    | \ref VPI_IMAGE_FORMAT_BGRA8   |  *  |   *  |
 *                    | \ref VPI_IMAGE_FORMAT_NV12    |  *  |   *  |
 *                    | \ref VPI_IMAGE_FORMAT_NV12_ER |  *  |   *  |
 *                    | \ref VPI_IMAGE_FORMAT_NV24    |  *  |   *  |
 *                    | \ref VPI_IMAGE_FORMAT_NV24_ER |  *  |   *  |
 *                    | \ref VPI_IMAGE_FORMAT_2S16    |  *  |   *  |
 *                    | \ref VPI_IMAGE_FORMAT_F32     |  *  |   *  |
 *                    | \ref VPI_IMAGE_FORMAT_F64     |  *  |   *  |
 *                    | \ref VPI_IMAGE_FORMAT_2F32    |  *  |   *  |
 *                    | \ref VPI_IMAGE_FORMAT_RGB8p   |  *  |      |
 *                    | \ref VPI_IMAGE_FORMAT_RGBA8p  |  *  |      |
 *                    | \ref VPI_IMAGE_FORMAT_BGR8p   |  *  |      |
 *                    | \ref VPI_IMAGE_FORMAT_BGRA8p  |  *  |      |
 * 
 * @param[out] statistics Output array.
 *                        + Must be of type \ref VPI_ARRAY_TYPE_STATISTICS
 *                        + Must not be NULL.
 * 
 * @param[in] mask Image statistics kernel binary image mask.
 *                 It defines the pixels that will be used for the image statistics calculation.
 *                 The pixel under a non-zero mask element will be used in image statistics calculation.
 *                 Use NULL for all elements in statistics calculation, i.e. all mask pixels considered to be non-zero.
 *                 + If not NULL, it must be of image format \ref VPI_IMAGE_FORMAT_U8 and same size as \p input image.
 *
 * @param[in] flags Statistics calculation flags.
 *                  Flags determine which image statistics will be calculated.
 *                  + The accepted flags are:
 *                    - \ref VPI_STAT_PIXEL_COUNT
 *                    - \ref VPI_STAT_SUM
 *                    - \ref VPI_STAT_MEAN (also triggers the \ref VPI_STAT_PIXEL_COUNT and \ref VPI_STAT_SUM flags)
 *                    - \ref VPI_STAT_VARIANCE (also triggers the \ref VPI_STAT_MEAN flag)
 *                    - \ref VPI_STAT_COVARIANCE (also triggers the \ref VPI_STAT_VARIANCE flag)
 *                  + \p flags can be a combination of the accepted flags.
 * 
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p stream is NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p input or \p statistics are NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p input and \p mask image dimensions do not match.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p flags not supported.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p backend is invalid or unknown.
 * @retval #VPI_ERROR_INVALID_IMAGE_FORMAT \p input and \p mask formats aren't supported
 * @retval #VPI_ERROR_NOT_IMPLEMENTED      Image Stats algorithm is not supported by given backend.
 * @retval #VPI_ERROR_INVALID_OPERATION    Backend hardware not available.
 * @retval #VPI_ERROR_INVALID_OPERATION    The needed backends aren't enabled in \p stream, \p input or \p statistics.
 * @retval #VPI_SUCCESS                    Operation executed successfully.
 */
VPI_PUBLIC VPIStatus vpiSubmitImageStats(VPIStream stream, uint64_t backend, VPIImage input, VPIArray statistics,
                                         VPIImage mask, uint32_t flags);
/** @} */

#ifdef __cplusplus
}
#endif

#endif /* NV_VPI_ALGORITHMS_IMAGE_STATS_H */
