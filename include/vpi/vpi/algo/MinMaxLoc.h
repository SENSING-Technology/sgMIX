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
 * @file MinMaxLoc.h
 *
 * Declares functions to perform minimum and maximum location finding in images.
 *
 */

#ifndef NV_VPI_ALGORITHMS_MINMAXLOC_H
#define NV_VPI_ALGORITHMS_MINMAXLOC_H

#include "../Export.h"
#include "../Status.h"
#include "../Types.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup VPI_MinMaxLoc MinMaxLoc
 * @ingroup VPI_API_Algorithms
 * @{
 *
 * Find in an image the minimum and maximum value locations.
 * The function \ref vpiCreateMinMaxLoc is used to create the payload for the algorithm.
 * The function \ref vpiSubmitMinMaxLoc is used to find minimum and maximum locations in an image.
 *
 */

/**
 * Creates payload for \ref vpiSubmitMinMaxLoc
 *
 * @param[in] backends VPI backend that will execute the algorithm.
 *                     + Valid values:
 *                       - \ref VPI_BACKEND_CPU
 *                       - \ref VPI_BACKEND_CUDA 
 *                     + Backend must be enabled in current context.
 *
 * @param[in] imageWidth, imageHeight Input image dimensions.
 *                                    + Must be >= 1x1.
 *
 * @param[in] imageFormat Input image format.
 *                        + Supported formats:
 *                          - \ref VPI_IMAGE_FORMAT_U8
 *                          - \ref VPI_IMAGE_FORMAT_S8
 *                          - \ref VPI_IMAGE_FORMAT_U16
 *                          - \ref VPI_IMAGE_FORMAT_S16
 *                          - \ref VPI_IMAGE_FORMAT_U32
 *                          - \ref VPI_IMAGE_FORMAT_S32
 *                          - \ref VPI_IMAGE_FORMAT_F32
 *                          - \ref VPI_IMAGE_FORMAT_F64
 *
 * @param[out] payload Pointer to the payload variable that receives the created handle.
 *
 * @retval #VPI_ERROR_INVALID_ARGUMENT  \p payload is NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT  \p imageWidth or \p imageHeight outside valid range.
 * @retval #VPI_ERROR_INVALID_ARGUMENT  \p backend refers to an invalid backend.
 * @retval #VPI_ERROR_NOT_IMPLEMENTED   MinMaxLoc algorithm is not supported by given backend.
 * @retval #VPI_ERROR_INVALID_CONTEXT   Current context is destroyed.
 * @retval #VPI_IMAGE_FORMAT_INVALID    \p imageFormat is not supported.
 * @retval #VPI_ERROR_OUT_OF_MEMORY     Cannot allocate required resources.
 * @retval #VPI_ERROR_INVALID_OPERATION Backend isn't enabled in current context.
 * @retval #VPI_SUCCESS                 Operation executed successfully.
 */
VPI_PUBLIC VPIStatus vpiCreateMinMaxLoc(uint64_t backends, int32_t imageWidth, int32_t imageHeight,
                                        VPIImageFormat imageFormat, VPIPayload *payload);

/**
 * Finds minimum and maximum value locations in an image.
 *
 * @note The MinMaxLoc algorithm does not guarantee deterministic output.
 *       Each array capacity (**minCoords** and **maxCoords** in
 *       \ref vpiSubmitMinMaxLoc) limit the number of locations found by the
 *       algorithm, that is the total number may be greater than this
 *       limitation and the set of locations returned might differ from one
 *       backend to another and in different runs on the same backend.
 *       Additionally, there is no strict ordering imposed to each array of
 *       locations and might also differ on different backends and runs.
 *
 * @param[in] stream The stream handle where the operation will be queued in.
 *                   + Must not be NULL.
 *                   + Stream must have enabled the backends that will execute the algorithm.
 *
 * @param[in] backend Backend that will execute the algorithm.
 *                    + Must be the backend specified during payload creation or 0 as a shorthand to use this backend.
 *
 * @param[in] payload Payload as created by \ref vpiCreateMinMaxLoc.
 *
 * @param[in] input Input image where minimum and/or maximum are to be found.
 *                  + Must not be NULL.
 *                  + Input image size and format must match the ones defined in \ref vpiCreateMinMaxLoc.
 *                  + Input image pitch must be a multiple of its pixels if using \ref VPI_BACKEND_CUDA.
 *                  + Image must have enabled the backends that will execute the algorithm.
 *
 * @param[out] minCoords Output array that stores pixel coordinates with minimum value.
 *                       If not needed, pass NULL. Number of coordinates returned is limited by array capacity.
 *                       + Array type must be \ref VPI_ARRAY_TYPE_KEYPOINT_F32.
 *                       + Array must have enabled the backends that will execute the algorithm.
 *
 * @param[out] maxCoords Output array that stores pixel coordinates with maximum value.
 *                       If not needed, pass NULL. Number of coordinates returned is limited by array capacity.
 *                       + Array type must be \ref VPI_ARRAY_TYPE_KEYPOINT_F32.
 *                       + Array must have enabled the backends that will execute the algorithm.
 *
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p stream is NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p input is NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p payload is not created using vpiCreateMinMaxLoc.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p input image dimension and format does not match the ones associated with \p payload.
 * @retval #VPI_ERROR_INVALID_ARRAY_TYPE   \p minCoords or \p maxCoords array type not accepted.
 * @retval #VPI_ERROR_INVALID_PAYLOAD_TYPE \p payload is invalid.
 * @retval #VPI_ERROR_INVALID_OPERATION    The given backend isn't enabled in \p stream.
 * @retval #VPI_ERROR_INVALID_OPERATION    The needed backends aren't enabled in \p stream, \p input, \p minCoords or \p maxCoords.
 * @retval #VPI_SUCCESS                    Operation executed successfully.
 */
VPI_PUBLIC VPIStatus vpiSubmitMinMaxLoc(VPIStream stream, uint64_t backend, VPIPayload payload, VPIImage input,
                                        VPIArray minCoords, VPIArray maxCoords);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* NV_VPI_ALGORITHMS_MINMAXLOC_H */
