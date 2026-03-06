/*
 * Copyright 2024 NVIDIA Corporation. All rights reserved.
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
 * @file DynamicRemap.h
 *
 * Declares functions that implement the \ref algo_dynamic_remap algorithm.
 *
 * Dynamic remap warps an image using a given map image.
 *
 */

#ifndef NV_VPI_ALGORITHMS_DYNAMIC_REMAP_IMAGE_H
#define NV_VPI_ALGORITHMS_DYNAMIC_REMAP_IMAGE_H

#include "../AlgoFlags.h"
#include "../Export.h"
#include "../Status.h"
#include "../Types.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup VPI_DynamicRemap Dynamic Remap
 * @ingroup VPI_API_Algorithms
 * @{
 *
 * Dynamically remaps pixels from an input image to an output image via a user-provided map image.  It is dynamic
 * since it allows for dynamic update of the map image, as a regular async stream ordered operation.
 *
 * Refer to \ref algo_dynamic_remap for more details and usage examples.
 */

/** Create a payload for \ref algo_dynamic_remap algorithm.
 * This function allocates all resources needed by the Dynamic Remap algorithm
 * and ties the returned payload to the given stream.
 *
 * @param[in] backends VPI backends that are eligible to execute the algorithm.
 *                     + Valid values:
 *                       - \ref VPI_BACKEND_CUDA
 *                     + Backend must be enabled in current context.
 *
 * @param[out] payload Pointer to the payload variable that receives the created handle.
 *                     + It must not be NULL.
 *
 * @retval #VPI_ERROR_INVALID_ARGUMENT  \p payload is NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT  \p warpMap is NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT  \p backends refers to an invalid backend.
 * @retval #VPI_ERROR_NOT_IMPLEMENTED   Algorithm is not supported by given backends.
 * @retval #VPI_ERROR_INVALID_CONTEXT   Current context is destroyed.
 * @retval #VPI_ERROR_OUT_OF_MEMORY     Cannot allocate required resources.
 * @retval #VPI_ERROR_INVALID_OPERATION Backend isn't enabled in current context.
 * @retval #VPI_SUCCESS                 Operation executed successfully.
 */
VPI_PUBLIC VPIStatus vpiCreateDynamicRemap(uint64_t backends, VPIPayload *payload);

/** Submits a \ref algo_dynamic_remap "Dynamic Remap" operation to the stream.
 *
 * The dynamic remap copies values from input to output via a map, using two sampling steps:
 *   (1) a control-point value is sampled in the map;
 *   (2) a pixel value is sampled in the input.
 * The following formula summarizes the two sampling steps:
 *   output(i, j) = input(map(i, j));
 * where each sampling step has its own interpolation.
 *
 * @param[in] stream The stream where the operation will be queued in.
 *                   + It must not be NULL.
 *                   + Stream must have enabled the backends that will execute the algorithm.
 *
 * @param[in] backend Backend that will execute the algorithm.
 *                    + It must be the backend specified during payload creation or 0 as a shorthand to use its backend.
 *
 * @param[in] payload Payload created by \ref vpiCreateDynamicRemap.
 *
 * @param[in] input Image to be remapped.
 *                  + It must not be NULL.
 *                  + It must have enabled the backends that will execute the algorithm.
 *                  + Supported image formats are:
 *                    - \ref VPI_IMAGE_FORMAT_U8
 *                    - \ref VPI_IMAGE_FORMAT_Y8
 *                    - \ref VPI_IMAGE_FORMAT_Y8_ER
 *                    - \ref VPI_IMAGE_FORMAT_F32
 *                    - \ref VPI_IMAGE_FORMAT_RGB8
 *                    - \ref VPI_IMAGE_FORMAT_BGR8
 *                    - \ref VPI_IMAGE_FORMAT_RGBA8
 *                    - \ref VPI_IMAGE_FORMAT_BGRA8
 *
 * @param[out] output Stores the remapped image.
 *                    + It must not be NULL.
 *                    + It must have enabled the backends that will execute the algorithm.
 *                    + It must have the same format as input image.
 *                    + It can have dimensions different than input and map.
 *
 * @param[out] map Stores the mapping of output coordinates into the input image.
 *                 + It must not be NULL.
 *                 + It must have enabled the backends that will execute the algorithm.
 *                 + It must have format: \ref VPI_IMAGE_FORMAT_2F32
 *                 + It can have dimensions different than input and output.
 *
 * @param[in] inputInterpolation Interpolation method to be used when fetching values from input. Valid values:
 *                               - \ref VPI_INTERP_NEAREST
 *                               - \ref VPI_INTERP_LINEAR
 *                               - \ref VPI_INTERP_CATMULL_ROM
 *
 * @param[in] mapInterpolation Interpolation method to be used when fetching values from map. Valid values:
 *                             - \ref VPI_INTERP_NEAREST
 *                             - \ref VPI_INTERP_LINEAR
 *                             - \ref VPI_INTERP_CATMULL_ROM
 *
 * @param[in] border What value to pick if remapped coordinates fall outside input image.
 *                   + The accepted border extensions are:
 *                     - \ref VPI_BORDER_ZERO
 *                     - \ref VPI_BORDER_CLAMP
 *                     - \ref VPI_BORDER_MIRROR
 *                     - \ref VPI_BORDER_REFLECT
 *
 * @param[in] flags Control flags.
 *                  + Valid values are a combination of one or more of the flags:
 *                    - 0 : default operator mode, i.e. sampling aligned to pixel center, map values are absolute,
 *                      denormalized positions in the input.
 *                    - \ref VPI_SAMPLING_ALIGNED_TO_CORNER : The sampling of pixels in continuous spatial domain
 *                      may be either at top-left corner or at the center.  If given, sampling occurs at the
 *                      top-left corner, or else (default) sampling at the pixel center.
 *                    - \ref VPI_RELATIVE_MAP : If given, map values as relative offsets to be applied to each
 *                      output position to fetch values from the input, or else (default) values as absolute
 *                      positions in the input.
 *                    - \ref VPI_NORMALIZED_MAP : If given, map values as normalized in [-1, 1] range for either
 *                      absolute positions or relative offsets, or else (default) map values as denormalized.
 *
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p stream is NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p input or \p map or \p output are NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p payload is not generated using vpiCreateDynamicRemap.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p input and \p output must have the same format.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p border not supported.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p flags has an invalid combination.
 * @retval #VPI_ERROR_INVALID_PAYLOAD_TYPE \p payload is invalid.
 * @retval #VPI_ERROR_INVALID_OPERATION    The needed backends aren't enabled in \p stream, \p input or \p output or \p map.
 * @retval #VPI_SUCCESS                    Operation executed successfully.
 */
VPI_PUBLIC VPIStatus vpiSubmitDynamicRemap(VPIStream stream, uint64_t backend, VPIPayload payload, VPIImage input,
                                           VPIImage output, VPIImage map, VPIInterpolationType inputInterpolation,
                                           VPIInterpolationType mapInterpolation, VPIBorderExtension border,
                                           uint64_t flags);

#ifdef __cplusplus
}
#endif

/** @} end of VPI_DynamicRemap */

#endif /* NV_VPI_ALGORITHMS_DYNAMIC_REMAP_IMAGE_H */
