/*
 * Copyright 2020-2021 NVIDIA Corporation. All rights reserved.
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
 * @file Remap.h
 *
 * Declares functions that implement the \ref algo_remap algorithm.
 *
 * Remap warps an image using a given a VPIWarpMap. This operation is used, among other
 * things, for lens distortion correction.
 *
 */

#ifndef NV_VPI_ALGORITHMS_REMAP_IMAGE_H
#define NV_VPI_ALGORITHMS_REMAP_IMAGE_H

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
 * @defgroup VPI_Remap Remap
 * @ingroup VPI_API_Algorithms
 * @{
 *
 * Remaps the input image, effectively warping it using a user-provided mapping.
 * Refer to \ref algo_remap for more details and usage examples.
 */

/** Create a payload for \ref algo_remap algorithm.
 * This function allocates all resources needed by the Remap algorithm
 * and ties the returned payload to the given stream.
 *
 * @param[in] backends VPI backends that are eligible to execute the algorithm.
 *                     + Valid values:
 *                       - \ref VPI_BACKEND_CPU
 *                       - \ref VPI_BACKEND_CUDA
 *                       - \ref VPI_BACKEND_VIC
 *                     + Backend must be enabled in current context.
 *
 * @param[in] warpMap Mapping of output back into input. 
 *                    + It must define a grid with the same size as output image.
 *
 * @param[out] payload Pointer to the payload variable that receives the created handle.
 *
 * @retval #VPI_ERROR_INVALID_ARGUMENT  \p payload is NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT  \p warpMap is NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT  \p backends refers to an invalid backend.
 * @retval #VPI_ERROR_NOT_IMPLEMENTED   Remap algorithm is not supported by given backends.
 * @retval #VPI_ERROR_NOT_IMPLEMENTED   VIC hardware with Remap support is not available.
 * @retval #VPI_ERROR_INVALID_CONTEXT   Current context is destroyed.
 * @retval #VPI_ERROR_OUT_OF_MEMORY     Cannot allocate required resources.
 * @retval #VPI_ERROR_INVALID_OPERATION Backend isn't enabled in current context.
 * @retval #VPI_SUCCESS                 Operation executed successfully.
 */
VPI_PUBLIC VPIStatus vpiCreateRemap(uint64_t backends, const VPIWarpMap *warpMap, VPIPayload *payload);

/** Submits a \ref algo_remap "Remap" operation to the stream.
 *
 * @param[in] stream The stream where the operation will be queued in.
 *                   + Must not be NULL.
 *                   + Stream must have enabled the backends that will execute the algorithm.
 *
 * @param[in] backend Backend that will execute the algorithm.
 *                    + Must be the backend specified during payload creation or 0 as a shorthand to use this backend.
 *
 * @param[in] payload Payload created by \ref vpiCreateRemap.
 *
 * @param[in] input Image to be remapped.
 *                  In a lens distortion correction context, this would correspond to the distorted image.
 *                  + Must not be NULL.
 *                  + Image must have enabled the backends that will execute the algorithm.
 *                  + On VIC, maximum input dimensions is 16384x16384.
 *                  + Supported image formats are:
 *                    | Formats                          | CPU | CUDA | VIC |
 *                    |----------------------------------|:---:|:----:|:---:|
 *                    | \ref VPI_IMAGE_FORMAT_U8         |  *  |   *  |  *  |
 *                    | \ref VPI_IMAGE_FORMAT_U16        |  *  |   *  |  *  |
 *                    | \ref VPI_IMAGE_FORMAT_Y8         |  *  |   *  |  *  |
 *                    | \ref VPI_IMAGE_FORMAT_Y8_BL      |     |      |  *  |
 *                    | \ref VPI_IMAGE_FORMAT_Y8_ER      |  *  |   *  |  *  |
 *                    | \ref VPI_IMAGE_FORMAT_Y8_ER_BL   |     |      |  *  |
 *                    | \ref VPI_IMAGE_FORMAT_Y16        |  *  |   *  |  *  |
 *                    | \ref VPI_IMAGE_FORMAT_Y16_BL     |     |      |  *  |
 *                    | \ref VPI_IMAGE_FORMAT_Y16_ER     |  *  |   *  |     |
 *                    | \ref VPI_IMAGE_FORMAT_NV12       |  *  |   *  |  *  |
 *                    | \ref VPI_IMAGE_FORMAT_NV12_BL    |     |      |  *  |
 *                    | \ref VPI_IMAGE_FORMAT_NV12_ER    |  *  |   *  |  *  |
 *                    | \ref VPI_IMAGE_FORMAT_NV12_ER_BL |     |      |  *  |
 *                    | \ref VPI_IMAGE_FORMAT_NV24       |     |      |  *  |
 *                    | \ref VPI_IMAGE_FORMAT_NV24_BL    |     |      |  *  |
 *                    | \ref VPI_IMAGE_FORMAT_NV24_ER    |     |      |  *  |
 *                    | \ref VPI_IMAGE_FORMAT_NV24_ER_BL |     |      |  *  |
 *                    | \ref VPI_IMAGE_FORMAT_RGB8       |  *  |   *  |     |
 *                    | \ref VPI_IMAGE_FORMAT_BGR8       |  *  |   *  |     |
 *                    | \ref VPI_IMAGE_FORMAT_RGBA8      |  *  |   *  |     |
 *                    | \ref VPI_IMAGE_FORMAT_BGRA8      |  *  |   *  |     |
 *
 * @param[out] output Stores the remapped image.
 *                    In a lens distortion correction context, this would correspond to the corrected (distortion-free).
 *                    + Must not be NULL.
 *                    + Must have same format as input image.
 *                    + Must have the same type as input
 *                    + Must have same dimensions as the warp map specified during payload creation.
 *                    + Image must have enabled the backends that will execute the algorithm.
 *                    + On VIC, maximum input dimensions is 16384x16384.
 *
 * @param[in] interp Interpolation method to be used. Valid values:
 *                    - \ref VPI_INTERP_NEAREST
 *                    - \ref VPI_INTERP_LINEAR
 *                    - \ref VPI_INTERP_CATMULL_ROM
 *
 * @param[in] border What value to pick if remapped coordinated falls outside input image.
 *                   + The accepted border extensions are:
 *                     - \ref VPI_BORDER_ZERO (VIC only supports this one)
 *                     - \ref VPI_BORDER_CLAMP
 *                     - \ref VPI_BORDER_MIRROR
 *                     - \ref VPI_BORDER_REFLECT 
 *
 * @param[in] flags Control flags.
 *                  + Valid values are a combination of one or more of the flags:
 *                    - 0 : default, negation of all other flags.
 *                    - \ref VPI_PRECISE : precise, but potentially slower implementation.
 *
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p stream is NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p input or \p output are NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p payload is not generated using vpiCreateRemap.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     Invalid \p flags.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p input and \p output must have the same format.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p output must have same size corresponding to the warp map passed during payload creation.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p border not supported.
 * @retval #VPI_ERROR_INVALID_PAYLOAD_TYPE \p payload is invalid.
 * @retval #VPI_ERROR_INVALID_OPERATION    The needed backends aren't enabled in \p stream, \p input or \p output.
 * @retval #VPI_SUCCESS                    Operation executed successfully.
 */
VPI_PUBLIC VPIStatus vpiSubmitRemap(VPIStream stream, uint64_t backend, VPIPayload payload, VPIImage input,
                                    VPIImage output, VPIInterpolationType interp, VPIBorderExtension border,
                                    uint64_t flags);

#ifdef __cplusplus
}
#endif

/** @} end of VPI_Remap */

#endif /* NV_VPI_ALGORITHMS_REMAP_IMAGE_H */
