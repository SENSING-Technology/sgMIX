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
 * @file TemporalNoiseReduction.h
 *
 * Declares functions that implement the Temporal Noise Reduction algorithm
 */

#ifndef NV_VPI_ALGORITHMS_TEMPORAL_NOISE_REDUCTION_H
#define NV_VPI_ALGORITHMS_TEMPORAL_NOISE_REDUCTION_H

#include "../Export.h"
#include "../ImageFormat.h"
#include "../Status.h"
#include "../Types.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup VPI_TNR Temporal Noise Reduction
 * @ingroup VPI_API_Algorithms
 * @{
 *
 * Performs temporal noise reduction in a sequence of images.
 * Refer to \ref algo_tnr for more details and usage examples.
 */

/** Defines scene presets for temporal noise reduction.
 * These presets are used by \ref algo_tnr to define its internal parameters suitable
 * for noise reduction of scenes captured with given characteristics.
 */
typedef enum
{
    VPI_TNR_PRESET_DEFAULT,              /**< Default preset, suitable for most scenes. */
    VPI_TNR_PRESET_OUTDOOR_LOW_LIGHT,    /**< Low light outdoor scene. */
    VPI_TNR_PRESET_OUTDOOR_MEDIUM_LIGHT, /**< Medium light outdoor scene. */
    VPI_TNR_PRESET_OUTDOOR_HIGH_LIGHT,   /**< Bright light outdoor scene. */
    VPI_TNR_PRESET_INDOOR_LOW_LIGHT,     /**< Low light indoor scene. */
    VPI_TNR_PRESET_INDOOR_MEDIUM_LIGHT,  /**< Medium light indoor scene. */
    VPI_TNR_PRESET_INDOOR_HIGH_LIGHT     /**< Bright light indoor scene. */
} VPITNRPreset;

/** Defines the version of the \ref algo_tnr algorithm to be used.
 *  Higher version numbers usually achieve better quality.
 */
typedef enum
{
    /** Chooses the version with best quality available in the current device and given backend. */
    VPI_TNR_DEFAULT,

    /** Version 1, without scene control and somewhat poor noise reduction capability specially in dark scenes, but runs fast.
     * + Not supported anymore */
    VPI_TNR_V1,

    /** Version 2, offers noise reduction strength control with decent processing speed.
     * + Not supported anymore */
    VPI_TNR_V2,

    /** Version 3, offers quite good quality overall, specially in dark scenes.
     * + CUDA VIC and backends only supports this one.
     */
    VPI_TNR_V3

} VPITNRVersion;

/**
 * Creates a payload for \ref algo_tnr algorithm.
 * This function allocates all resources needed by the algorithm and ties the
 * returned payload to the given backend.
 *
 * @param[in] backends VPI backends that are eligible to execute the algorithm.
 *                     + Valid values:
 *                       - \ref VPI_BACKEND_CUDA
 *                       - \ref VPI_BACKEND_VIC
 *                     + Backend must be enabled in current context.
 *
 * @param[in] width,height Dimensions of frames to be processed.
 *                         + Must be positive.
 *
 * @param[in] imgFormat Format of the images to be processed. 
 *                      + Supported formats:
 *                        | Formats                            | VIC | CUDA |
 *                        |------------------------------------|:---:|:----:|
 *                        | \ref VPI_IMAGE_FORMAT_NV12_ER      |  *  |  *   |
 *                        | \ref VPI_IMAGE_FORMAT_NV12_ER_BL   |  *  |  *   |
 *                        | \ref VPI_IMAGE_FORMAT_YUYV_ER      |  *  |      |
 *                        | \ref VPI_IMAGE_FORMAT_YUYV_ER_BL   |  *  |      |
 *                        | \ref VPI_IMAGE_FORMAT_UYVY_ER      |  *  |      |
 *                        | \ref VPI_IMAGE_FORMAT_UYVY_ER_BL   |  *  |      |
 *
 * @param[in] version Version of the algorithm to be used.
 *
 * @param[out] payload Pointer to the payload variable that receives the created handle.
 *
 * @retval #VPI_IMAGE_FORMAT_INVALID    \p imgFormat is not supported.
 * @retval #VPI_ERROR_INVALID_ARGUMENT  \p payload is NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT  \p width or \p height outside valid range.
 * @retval #VPI_ERROR_INVALID_ARGUMENT  Invalid TNR \p version.
 * @retval #VPI_ERROR_INVALID_ARGUMENT  \p backends refers to an invalid backend.
 * @retval #VPI_ERROR_INVALID_OPERATION No VIC support is available.
 * @retval #VPI_ERROR_INVALID_OPERATION Backend isn't enabled in current context.
 * @retval #VPI_ERROR_NOT_IMPLEMENTED   Temporal Noise Reduction algorithm version is not supported by given backend.
 * @retval #VPI_ERROR_NOT_IMPLEMENTED   VIC Hardware with TNRv3 support is not available.
 * @retval #VPI_ERROR_INVALID_CONTEXT   Current context is destroyed.
 * @retval #VPI_ERROR_OUT_OF_MEMORY     Cannot allocate required resources.
 * @retval #VPI_SUCCESS                 Operation executed successfully.
 */
VPI_PUBLIC VPIStatus vpiCreateTemporalNoiseReduction(uint64_t backends, int32_t width, int32_t height,
                                                     VPIImageFormat imgFormat, VPITNRVersion version,
                                                     VPIPayload *payload);

/**
 * Structure that defines the parameters for \ref vpiSubmitTemporalNoiseReduction
 */
typedef struct
{
    /** Scene preset to be used. */
    VPITNRPreset preset;

    /** Noise reduction strength.
     *  + Must be >= 0 and <=1. */
    float strength;

} VPITNRParams;

/**
 * Initializes \ref vpiSubmitTemporalNoiseReduction with default values.
 *
 * Defaults:
 * - preset: \ref VPI_TNR_PRESET_DEFAULT
 * - strength: 0.5
 *
 * @param[in] params Structure to be filled with default values.
 *
 * @retval #VPI_ERROR_INVALID_ARGUMENT \p params is NULL.
 * @retval #VPI_SUCCESS Operation executed successfully.
 */
VPI_PUBLIC VPIStatus vpiInitTemporalNoiseReductionParams(VPITNRParams *params);

/**
 * Submits a \ref algo_tnr "Temporal Noise Reduction" operation to the stream.
 *
 * @param[in] stream The stream where the operation will be queued in.
 *                   + Must not be NULL.
 *                   + Stream must have enabled the backends that will execute the algorithm.
 *
 * @param[in] backend Backend that will execute the algorithm.
 *                    + Must be the backend specified during payload creation or 0 as a shorthand to use this backend.
 *
 * @param[in] payload Payload to be submitted along the other parameters.
 *
 * @param[in] prevFrame Result of the previous iteration.
 *                      If the frame passed isn't the result image of the previous iteration, TNR will
 *                      consider it to be NULL, i.e, the internal state will be reset and \p curFrame will
 *                      be considered to be the first frame of a new sequence to be denoised.
 *                      + When processing the first frame of a video sequence, this must be NULL.
 *                      + Must have same format and dimensions as the one specified during payload creation.
 *                      + Input must have enabled the backends that will execute the algorithm.
 *
 * @param[in] curFrame Current (noisy) frame to be processed. 
 *                     + Must not be NULL.
 *                     + Must have same format and dimensions as the one specified during payload creation.
 *                     + Input must have enabled the backends that will execute the algorithm.
 *
 * @param[out] outFrame Output frame, where de-noised image will be written to.
 *                      + Must not be NULL.
 *                      + Must have same format and dimensions as the one specified during payload creation.
 *                      + Input must have enabled the backends that will execute the algorithm.
 *
 * @param[in] params Control parameters for temporal noise reduction
 *                   If NULL, it'll use the defaults given by \ref vpiInitTemporalNoiseReductionParams.
 *
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p stream is NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p curFrame or \p outFrame are NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p payload is not generated using vpiCreateTemporalNoiseReduction family of functions.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p prevFrame, \p curFrame and \p outFrame must have the same format and size
 *                                         configured during \p payload creation.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     Strength in \p params outside valid range.
 * @retval #VPI_ERROR_INVALID_PAYLOAD_TYPE \p payload is invalid.
 * @retval #VPI_ERROR_INVALID_OPERATION    The needed backends aren't enabled in \p stream, \p prevFrame, \p curFrame or \p outFrame.
 * @retval #VPI_SUCCESS                    Operation executed successfully.
 */
VPI_PUBLIC VPIStatus vpiSubmitTemporalNoiseReduction(VPIStream stream, uint64_t backend, VPIPayload payload,
                                                     VPIImage prevFrame, VPIImage curFrame, VPIImage outFrame,
                                                     const VPITNRParams *params);
/** @} end of VPI_TNR */

#ifdef __cplusplus
}
#endif

#endif /* NV_VPI_ALGORITHMS_TEMPORAL_NOISE_REDUCTION_H */
