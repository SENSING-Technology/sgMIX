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
 * @file ImageFlip.h
 *
 * Declares functions that implement Image flip algorithms.
 */

#ifndef NV_VPI_ALGORITHMS_IMAGEFLIP_H
#define NV_VPI_ALGORITHMS_IMAGEFLIP_H

#include "../Export.h"
#include "../Status.h"
#include "../Types.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup VPI_ImageFlip Image Flip
 * @ingroup VPI_API_Algorithms
 * @{
 *
 * Flips a 2D image either horizontally, vertically or both.
 * Refer to \ref algo_image_flip for more details and usage examples.
 */

/**
 * Flips a 2D image either horizontally, vertically or both.
 * 
 *
 * @param[in] stream A stream handle where the operation will be queued into.
 *
 * @param[in] backend Backend that will execute the algorithm.
 *                    + Valid values: 
 *                      - \ref VPI_BACKEND_CPU
 *                      - \ref VPI_BACKEND_CUDA
 *                      - \ref VPI_BACKEND_VIC
 *
 * @param[in] input Input image to be flipped.
 *                  + CPU and CUDA accept all pitch-linear image formats, with at most 4 planes.
 *                  + Valid Image formats for VIC backend:
 *                     - \ref VPI_IMAGE_FORMAT_Y8
 *                     - \ref VPI_IMAGE_FORMAT_Y8_ER
 *                     - \ref VPI_IMAGE_FORMAT_RGBA8
 *                     - \ref VPI_IMAGE_FORMAT_BGRA8
 *                     - \ref VPI_IMAGE_FORMAT_NV12
 *                     - \ref VPI_IMAGE_FORMAT_NV12_ER
 *                     - \ref VPI_IMAGE_FORMAT_NV24
 *                     - \ref VPI_IMAGE_FORMAT_NV24_ER
 *                     - \ref VPI_IMAGE_FORMAT_UYVY
 *                     - \ref VPI_IMAGE_FORMAT_UYVY_ER
 *                     - \ref VPI_IMAGE_FORMAT_YUYV
 *                     - \ref VPI_IMAGE_FORMAT_YUYV_ER
 *                     - \ref VPI_IMAGE_FORMAT_Y8_BL
 *                     - \ref VPI_IMAGE_FORMAT_Y8_ER_BL
 *                     - \ref VPI_IMAGE_FORMAT_NV12_BL
 *                     - \ref VPI_IMAGE_FORMAT_NV12_ER_BL
 *                     - \ref VPI_IMAGE_FORMAT_NV24_BL
 *                     - \ref VPI_IMAGE_FORMAT_NV24_ER_BL
 *                     - \ref VPI_IMAGE_FORMAT_UYVY_BL
 *                     - \ref VPI_IMAGE_FORMAT_UYVY_ER_BL
 *                     - \ref VPI_IMAGE_FORMAT_YUYV_BL
 *                     - \ref VPI_IMAGE_FORMAT_YUYV_ER_BL
 *                     - \ref VPI_IMAGE_FORMAT_Y8_BL16
 *                     - \ref VPI_IMAGE_FORMAT_Y8_ER_BL16
 *                     - \ref VPI_IMAGE_FORMAT_NV12_BL16
 *                     - \ref VPI_IMAGE_FORMAT_NV12_ER_BL16
 *                     - \ref VPI_IMAGE_FORMAT_NV24_BL16
 *                     - \ref VPI_IMAGE_FORMAT_NV24_ER_BL16
 *                     - \ref VPI_IMAGE_FORMAT_UYVY_BL16
 *                     - \ref VPI_IMAGE_FORMAT_UYVY_ER_BL16
 *                     - \ref VPI_IMAGE_FORMAT_YUYV_BL16
 *                     - \ref VPI_IMAGE_FORMAT_YUYV_ER_BL16
 *                  + Images whose format has either 4:2:0 or 4:2:2 chroma subsampling must have even dimensions.
 * 
 * @param[out] output Output image where the result will be written to.
 *                    + It must have the same format and dimensions as input.
 * 
 * @param[in] flipMode Direction the image needs to be flipped in.
 *                     + Accepted inputs:
 *                       - \ref VPI_FLIP_HORIZ
 *                       - \ref VPI_FLIP_VERT
 *                       - \ref VPI_FLIP_BOTH
 * 
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p stream is NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p input or \p output are NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p input and \p output image dimensions and formats do not match.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     Image dimensions aren't supported for given format.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p flipMode is not valid.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p backend is invalid or unknown.
 * @retval #VPI_ERROR_INVALID_IMAGE_FORMAT \p input and \p output formats aren't supported
 * @retval #VPI_ERROR_NOT_IMPLEMENTED      Image Flip algorithm is not supported by given backend.
 * @retval #VPI_ERROR_INVALID_OPERATION    Backend hardware not available.
 * @retval #VPI_ERROR_INVALID_OPERATION    The needed backends aren't enabled in \p stream, \p input or \p output.
 * @retval #VPI_SUCCESS                    Operation executed successfully.
 */
VPI_PUBLIC VPIStatus vpiSubmitImageFlip(VPIStream stream, uint64_t backend, VPIImage input, VPIImage output,
                                        VPIFlipMode flipMode);
/** @} */

#ifdef __cplusplus
}
#endif

#endif /* NV_VPI_ALGORITHMS_IMAGEFLIP_H */
