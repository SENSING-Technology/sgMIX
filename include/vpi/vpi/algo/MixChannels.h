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
 * @file MixChannels.h
 *
 * Declares functions that implement support for Mix Channels.
 */

#ifndef NV_VPI_ALGORITHMS_MIX_CHANNELS_H
#define NV_VPI_ALGORITHMS_MIX_CHANNELS_H

#include "../Export.h"
#include "../Status.h"
#include "../Types.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup VPI_MixChannels Mix Channels
 * @ingroup VPI_API_Algorithms
 * @{
 *
 * Mix channels from one or more input image channels to one or more output image channels.
 * The operation is a copy operation without increasing or decreasing resolution, the input
 * and output sizes must match, and without any color conversion.
 * Refer to \ref algo_mix_channels for more details and usage examples.
 */

/** Maximum number of images that can be passed to \ref algo_mix_channels as either input or output.
 */
#define VPI_MIX_CHANNELS_MAX_IMAGES 4

/**
 * Submits a \ref algo_mix_channels "Mix Channels" operation to the stream.
 *
 * @param[in] stream The stream where the operation will be queued in.
 *                   + Must not be NULL.
 *                   + Stream must have enabled the backends that will execute the algorithm.
 *
 * @param[in] backend Backend that will execute the algorithm.
 *                    + Valid values: 
 *                      - \ref VPI_BACKEND_CPU
 *                      - \ref VPI_BACKEND_CUDA
 *                      - \ref VPI_BACKEND_PVA   
 *
 * @param[in] inputs One or more input images from where to draw channels from.
 *                   + Must not be NULL.
 *                   + All input images must be already allocated.
 *                   + Each input image plane size must match the mapped output image plane size.
 *                   + Images must have enabled the backends that will execute the algorithm.
 *                   + The accepted image formats for CUDA and CPU backends are:
 *                     - \ref VPI_IMAGE_FORMAT_U8
 *                     - \ref VPI_IMAGE_FORMAT_S8
 *                     - \ref VPI_IMAGE_FORMAT_U16
 *                     - \ref VPI_IMAGE_FORMAT_U32
 *                     - \ref VPI_IMAGE_FORMAT_S32
 *                     - \ref VPI_IMAGE_FORMAT_S16
 *                     - \ref VPI_IMAGE_FORMAT_2S16
 *                     - \ref VPI_IMAGE_FORMAT_F32
 *                     - \ref VPI_IMAGE_FORMAT_F64
 *                     - \ref VPI_IMAGE_FORMAT_2F32
 *                     - \ref VPI_IMAGE_FORMAT_Y8
 *                     - \ref VPI_IMAGE_FORMAT_Y8_ER
 *                     - \ref VPI_IMAGE_FORMAT_Y16
 *                     - \ref VPI_IMAGE_FORMAT_Y16_ER
 *                     - \ref VPI_IMAGE_FORMAT_NV12
 *                     - \ref VPI_IMAGE_FORMAT_NV12_ER
 *                     - \ref VPI_IMAGE_FORMAT_NV24
 *                     - \ref VPI_IMAGE_FORMAT_NV24_ER
 *                     - \ref VPI_IMAGE_FORMAT_RGB8
 *                     - \ref VPI_IMAGE_FORMAT_BGR8
 *                     - \ref VPI_IMAGE_FORMAT_RGBA8
 *                     - \ref VPI_IMAGE_FORMAT_BGRA8
 *                     - \ref VPI_IMAGE_FORMAT_RGB8p
 *                     - \ref VPI_IMAGE_FORMAT_BGR8p
 *                     - \ref VPI_IMAGE_FORMAT_RGBA8p
 *                     - \ref VPI_IMAGE_FORMAT_BGRA8p
 *                   + For PVA backend, 2 use cases are supported:
 *                     - Conversion of 4 planar U8 images to 1 interleaved RGBA8 image.
 *                     - Conversion of 1 interleaved RGBA8 image to 4 planar U8 images.
 *                  + For PVA backend, minimum input image plane size is 128x96 pixels.
 *
 * @param[in] numInputs Number of inputs in the array of inputs pointer argument.
 *                      + Must be between 1 and \ref VPI_MIX_CHANNELS_MAX_IMAGES .
 *
 * @param[out] outputs One or more output images from where to insert channels to.
 *                     + Must not be NULL.
 *                     + All output images must be already allocated.
 *                     + Each output image plane size must match the mapped input image plane size.
 *                     + Images must have enabled the backends that will execute the algorithm.
 *                     + The accepted image formats are the same as in \p inputs.
 *
 * @param[in] numOutputs Number of outputs in the array of outputs pointer argument.
  *                      + Must be between 1 and \ref VPI_MIX_CHANNELS_MAX_IMAGES .
 *
 * @param[in] inMapping Array with indices to channels of input images
 *                    The channels are considered linearized throughout images, that is each input is
 *                    viewed as a sequence of channels and all inputs are concatenated in its array order.
 *                    The index of an input channel gives the position on the whole inputs
 *                    channels concatenated sequence.
 *                    + Must contain existing input channel indices.
 *                    + Array size must be equal to \p numMapping.
 *
 * @param[in] outMapping Array with indices to channels of output images
 *                    The channels are considered linearized throughout images, that is each output is
 *                    viewed as a sequence of channels and all outputs are concatenated in its array order.
 *                    The index of an output channel gives the position on the whole outputs
 *                    channels concatenated sequence.
 *                    + Must contain existing output channel indices.
 *                    + Array size must be equal to \p numMapping.
 *
 * @param[in] numMapping Number of mappings in the mapping array pointer argument.
 *                       + Must be >= 1 and <= \ref VPI_MIX_CHANNELS_MAX_IMAGES * \ref VPI_MAX_PLANE_COUNT.
 *  
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p stream or \p inputs or \p outputs or \p mappings are NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p numInputs or \p numOutputs or \p numMapping are outside valid range.
 * @retval #VPI_ERROR_INVALID_IMAGE_FORMAT Unsupported input or output format.
 * @retval #VPI_ERROR_INVALID_OPERATION    The needed backends aren't enabled in \p stream, \p inputs or \p outputs.
 * @retval #VPI_SUCCESS                    Operation executed successfully.
 */
VPI_PUBLIC VPIStatus vpiSubmitMixChannels(VPIStream stream, uint64_t backend, VPIImage *inputs, int numInputs,
                                          VPIImage *outputs, int numOutputs, const int *inMapping,
                                          const int *outMapping, int numMapping);

/** @} end of VPI_MixChannels */

#ifdef __cplusplus
}
#endif

#endif /* NV_VPI_ALGORITHMS_MIX_CHANNELS_H */
