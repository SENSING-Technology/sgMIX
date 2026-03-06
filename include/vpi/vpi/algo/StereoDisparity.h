/*
 * Copyright 2019-2024 NVIDIA Corporation. All rights reserved.
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

#ifndef NV_VPI_ALGORITHMS_STEREO_DISPARITY_H
#define NV_VPI_ALGORITHMS_STEREO_DISPARITY_H

/**
 * @file StereoDisparity.h
 *
 * Declares functions that implement stereo disparity estimation algorithms.
 */

#include "../Export.h"
#include "../ImageFormat.h"
#include "../Status.h"
#include "../Types.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup VPI_StereoDisparityEstimator Stereo Disparity Estimator
 * @ingroup VPI_API_Algorithms
 * @{
 *
 * Estimates disparity from a stereo pair.
 * Refer to \ref algo_stereo_disparity for more details and usage examples.
 */

/**
 * Structure that defines the parameters for \ref vpiCreateStereoDisparityEstimator
 */
typedef struct
{
    /** Maximum disparity for matching search.
     *  + Must be positive.
     *  + On OFA or OFA+PVA+VIC backend, maxDisparity must be 128 or 256.
     *  + On CUDA backends, maxDisparity must be >= 0 and <= 256.
     */
    int32_t maxDisparity;

    /** Output's downscale factor with respect to the input's resolution.
     *  + For OFA and OFA+PVA+VIC backend, the allowed values are 1, 2, 4 or 8.
     *  + For other backends, only 1 is supported, i.e. output dimension equals input's. */
    int32_t downscaleFactor;

    /** Include diagonals or oblique paths in semi-global matching (SGM) computation.
     * Including diagonal paths makes the stereo disparity estimator slower, but increases the result quality.
     * Including diagonal paths also increases memory usage in the CUDA backend.
     * If zero means not to include diagonal paths, i.e. horizontal and vertical paths only.
     * If not zero means to include diagonal paths, i.e. horizontal, vertical and oblique paths.
     * It's only applicable when using CUDA and OFA backends. */
    int8_t includeDiagonals;

} VPIStereoDisparityEstimatorCreationParams;

/**
 * Initializes \ref VPIStereoDisparityEstimatorCreationParams with default values.
 *
 * Defaults:
 * - maxDisparity     = 64
 * - downscaleFactor  = 1
 * - includeDiagonals = 1
 *
 * @param[in] params Structure to be filled with default values.
 *
 * @retval #VPI_ERROR_INVALID_ARGUMENT \p params is NULL.
 * @retval #VPI_SUCCESS Operation executed successfully.
 */
VPI_PUBLIC VPIStatus vpiInitStereoDisparityEstimatorCreationParams(VPIStereoDisparityEstimatorCreationParams *params);

/**
 * Creates payload for \ref vpiSubmitStereoDisparityEstimator
 *
 * @param[in] backends VPI backends that are eligible to execute the algorithm.
 *                     + Valid values:
 *                       - \ref VPI_BACKEND_CUDA
 *                       - \ref VPI_BACKEND_OFA
 *                       - \ref VPI_BACKEND_PVA | \ref VPI_BACKEND_OFA | \ref VPI_BACKEND_VIC
 *                     + Backends must be enabled in current context.
 *
 * @param[in] imageWidth, imageHeight Input image dimensions.
 *                                    + Must be >= 0.
 *                                    + On OFA, input width and height must be between 16 and 16384 after applying downscaleFactor.
 *                                    + On OFA+PVA+VIC, input width must be at least max(64, maxDisparity/downscaleFactor) * downscaleFactor.
 *
 * @param[in] inputFormat Input image format.
 *                        + Supported image formats are:
 *                          | Formats                            | CUDA | OFA | OFA+PVA+VIC |
 *                          |------------------------------------|:----:|:---:|:-----------:|
 *                          | \ref VPI_IMAGE_FORMAT_U8           |   *  |     |             |
 *                          | \ref VPI_IMAGE_FORMAT_U16          |   *  |     |             |
 *                          | \ref VPI_IMAGE_FORMAT_Y8           |   *  |     |             |
 *                          | \ref VPI_IMAGE_FORMAT_Y8_BL        |      |  *  |      *      |
 *                          | \ref VPI_IMAGE_FORMAT_Y8_BL16      |      |  *  |      *      |
 *                          | \ref VPI_IMAGE_FORMAT_Y8_ER        |   *  |     |             |
 *                          | \ref VPI_IMAGE_FORMAT_Y8_ER_BL     |      |  *  |      *      |
 *                          | \ref VPI_IMAGE_FORMAT_Y8_ER_BL16   |      |  *  |      *      |
 *                          | \ref VPI_IMAGE_FORMAT_Y16          |   *  |     |             |
 *                          | \ref VPI_IMAGE_FORMAT_Y16_BL       |      |  *  |      *      |
 *                          | \ref VPI_IMAGE_FORMAT_Y16_BL16     |      |  *  |      *      |
 *                          | \ref VPI_IMAGE_FORMAT_Y16_ER       |   *  |     |             |
 *                          | \ref VPI_IMAGE_FORMAT_Y16_ER_BL    |      |  *  |      *      |
 *                          | \ref VPI_IMAGE_FORMAT_Y16_ER_BL16  |      |  *  |      *      |
 *                          | \ref VPI_IMAGE_FORMAT_NV12         |   *  |     |             |
 *                          | \ref VPI_IMAGE_FORMAT_NV12_BL      |      |  *  |      *      |
 *                          | \ref VPI_IMAGE_FORMAT_NV12_BL16    |      |  *  |      *      |
 *                          | \ref VPI_IMAGE_FORMAT_NV12_ER      |   *  |     |             |
 *                          | \ref VPI_IMAGE_FORMAT_NV12_ER_BL   |      |  *  |      *      |
 *                          | \ref VPI_IMAGE_FORMAT_NV12_ER_BL16 |      |  *  |      *      |
 *                          | \ref VPI_IMAGE_FORMAT_NV24         |   *  |     |             |
 *                          | \ref VPI_IMAGE_FORMAT_NV24_BL      |      |  *  |      *      |
 *                          | \ref VPI_IMAGE_FORMAT_NV24_BL16    |      |  *  |      *      |
 *                          | \ref VPI_IMAGE_FORMAT_NV24_ER      |   *  |     |             |
 *                          | \ref VPI_IMAGE_FORMAT_NV24_ER_BL   |      |  *  |      *      |
 *                          | \ref VPI_IMAGE_FORMAT_NV24_ER_BL16 |      |  *  |      *      |
 *
 * @param[in] params Creation parameters.
 *                   Pass NULL to use defaults given by \ref vpiInitStereoDisparityEstimatorCreationParams.
 *
 * @param[out] payload Pointer to the payload variable that receives the created handle.
 *
 * @retval #VPI_ERROR_INVALID_IMAGE_FORMAT \p inputFormat is not supported.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p payload is NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p imageWidth or \p imageHeight outside valid range.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p backends refers to an invalid backend.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     Maximum disparity in \p params outside valid range.
 * @retval #VPI_ERROR_NOT_IMPLEMENTED      Stereo Disparity Estimator algorithm is not supported by given backends.
 * @retval #VPI_ERROR_INVALID_OPERATION    Backend hardware not available.
 * @retval #VPI_ERROR_INVALID_CONTEXT      Current context is destroyed.
 * @retval #VPI_ERROR_OUT_OF_MEMORY        Cannot allocate required resources.
 * @retval #VPI_ERROR_INVALID_OPERATION    Backend isn't enabled in current context.
 * @retval #VPI_SUCCESS                    Operation executed successfully.
 */
VPI_PUBLIC VPIStatus vpiCreateStereoDisparityEstimator(uint64_t backends, int32_t imageWidth, int32_t imageHeight,
                                                       VPIImageFormat inputFormat,
                                                       const VPIStereoDisparityEstimatorCreationParams *params,
                                                       VPIPayload *payload);

/** Defines the way the confidence values are computed.
 *
 *  VPI_STEREO_CONFIDENCE_[ABSOLUTE|RELATIVE] is based on the disparity difference from left to right (D_lr)
 *  and from right to left (D_rl), and potentially the maximum disparity (MAX_DISP). See equations for details.
 *
 *  VPI_STEREO_CONFIDENCE_INFERENCE uses a convolutional neural network (CNN) to determine confidence. It is only
 *  available for the combination of OFA+VIC+PVA backend.
 *
 *  Lower confidence values (than the given confidence threshold parameter) make the output disparity invalid.
 */
typedef enum
{
    /** The U16 confidence value of a pixel is given by:
     *  [ 1 - abs(D_lr - D_rl) / MAX_DISP ] * 0xFFFF
     */
    VPI_STEREO_CONFIDENCE_ABSOLUTE = 0,

    /** The U16 confidence value of a pixel is given by:
     *  [ 1 - abs(D_lr - D_rl) / D_lr ] * 0xFFFF
     */
    VPI_STEREO_CONFIDENCE_RELATIVE = 1,

    /** The confidence value of a pixel is on a 0:UINT16_MAX scale, mapping from 0% to 100%.
     */
    VPI_STEREO_CONFIDENCE_INFERENCE = 2

} VPIStereoDisparityConfidenceType;

/**
 * Structure that defines the parameters for \ref vpiSubmitStereoDisparityEstimator
 */
typedef struct
{
    /** Represents the median filter size on OFA+PVA+VIC backend
        or census transform window size (other backends) used in the algorithm.
     *  + On CUDA backend this is ignored. A 9x7 window is used instead.
     *  + On OFA backend it is ignored.
     *  + On OFA+PVA+VIC backend, valid values are 1, 3, 5 or 7. */
    int32_t windowSize;

    /** Maximum disparity for matching search.
     *  + Maximum disparity must be 0 (use from payload), or positive and less or equal to what's configured in payload.
     *  + For CUDA, it must be equal to the configured in the payload (or 0) */
    int32_t maxDisparity;

    /** Confidence threshold above which disparity values are considered valid.
     * Only used in CUDA, and OFA+PVA+VIC backends.
     * + Must be a value in U16 ranges, i.e. from 0 to 65535. */
    int32_t confidenceThreshold;

    /** Ignored, do not use.
     * + Must be a value between 1 and 8. */
    int32_t quality;

    /** Computation type to produce the confidence output.
     * + Use \ref VPI_STEREO_CONFIDENCE_ABSOLUTE for absolute difference between disparities computation.
     * + Use \ref VPI_STEREO_CONFIDENCE_RELATIVE for relative difference between disparities computation.
     * + Use \ref VPI_STEREO_CONFIDENCE_INFERENCE for deep-learning based confidence (OFA+PVA+VIC only) */
    VPIStereoDisparityConfidenceType confidenceType;

    /** Minimum possible disparity value.
     * It's only applicable when using CUDA backend.
     * Normally it is zero, but it may be adjusted depending on input images.
     * + Must be between 0 and maximum disparity. */
    int32_t minDisparity;

    /** Penalty on disparity changes of +/- 1 between neighbor pixels.
     * It is normally a small value, in (min, max) disparity range, used to penalize small changes in disparity, i.e. one.
     * Using a lower P1 penalty allows for an adaptation to slanted or curver surfaces.
     * (Please refer to \ref paper_sgm on how penalties affect disparity values.)
     * It's only applicable when using CUDA and OFA backends.
     * + Must be a positive value.
     * + Must be smaller than or equal to P2. */
    int32_t p1;

    /** Penalty on disparity changes of more than 1 between neighbor pixels.
     * It is normally a large value, in (min, max) disparity range, used to penalize large changes in disparity.
     * Using a higher P2 penalty avoids abrupt disparity changes, i.e. discontinuities.
     * (Please refer to \ref paper_sgm on how penalties affect disparity values.)
     * It's only applicable when using CUDA and OFA backends.
     * + Must be a positive value.
     * + Must be equal to or greater than P1.
     * + in CUDA backend, must be less than 256.
     * + In OFA backends, must be less than or equal to 217 - P1.
     * + In OFA+PVA+VIC backend with \ref VPI_STEREO_CONFIDENCE_INFERENCE , must be less than or equal to 89 - P1. */
    int32_t p2;

    /** Alpha is used to enable adaptive large penalty (adaptive P2) feature.
     * It's used to better preserve object boundaries and thin objects.
     * Higher alpha indicates lower adaptation of P2 in accordance to pixel intensities.
     * It's the alpha value in: P2' = - (1 / alpha) * abs(I_cur - I_prev) + P2;
     * where P2' is the adaptive version of the given P2 penalty.
     * The abs(I_cur - I_prev) is the absolute difference in intensity between the current and previous pixel
     * location along the direction of path cost evaluation.  The intuition behind this adaptation is that large
     * disparity differences should be penalized less if they coincide with large intensity changes.
     * The alpha value can be 0, 1, 2, 4 or 8.  Using an alpha value of 0 disables the adaptive P2 feature.
     * It's only applicable when using OFA backend.
     * + Must be 0, 1, 2, 4 or 8. */
    int32_t p2Alpha;

    /** Uniqueness ratio, in [0, 1] range, is a margin by which best cost value should win over the second best.
     * A value of 1 means less pixels are marked invalid, as the full cost is considered when comparing against the current best cost.
     * Values less than 1 mean only a ratio of the cost is considered for the best cost.
     * A value of 0 means more pixels are marked invalid, as only disparity changes of +/- 1 pixels are considered.
     * Use it to improve certainty of computed disparity.
     * Use value outside [0, 1], such as -1, to disregard the uniqueness computation.
     * It's only applicable when using CUDA backend. */
    float uniqueness;

    /** Number of passes in memory-efficient semi-global matching (eSGM) computation.
     * A value of 1 or 2 means to do one forward or one forward and one backward passes per direction.
     * A value of 3 means to do another forward pass after forward and backward passes.
     * Setting number of passes to 3 makes the stereo disparity estimator slower, but increases the result quality.
     * It's only applicable when using OFA backends.
     * + Must be 1, 2 or 3. */
    int8_t numPasses;

} VPIStereoDisparityEstimatorParams;

/**
 * Initializes \ref VPIStereoDisparityEstimatorParams with default values.
 *
 * Defaults:
 * - windowSize          = 5
 * - maxDisparity        = 0 (uses disparity set during payload construction)
 * - confidenceThreshold = 32767
 * - confidenceType      = \ref VPI_STEREO_CONFIDENCE_ABSOLUTE
 * - minDisparity        = 0
 * - p1                  = 3
 * - p2                  = 48
 * - p2Alpha             = 0
 * - uniqueness          = -1
 * - numPasses           = 3
 *
 * @param[in] params Structure to be filled with default values.
 *
 * @retval #VPI_ERROR_INVALID_ARGUMENT \p params is NULL.
 * @retval #VPI_SUCCESS Operation executed successfully.
 */
VPI_PUBLIC VPIStatus vpiInitStereoDisparityEstimatorParams(VPIStereoDisparityEstimatorParams *params);

/**
 * Runs stereo processing on a pair of images and outputs a disparity map.
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
 * @param[in] left Left stereo input image.
 *                 + Must not be NULL.
 *                 + Must have same format and dimensions as the one specified during payload creation.
 *                 + Image must have enabled the backends that will execute the algorithm.
 *
 * @param[in] right Right stereo input image.
 *                  + Must not be NULL.
 *                  + Must have same format and dimensions as \p left.
 *                  + Image must have enabled the backends that will execute the algorithm.
 *
 * @param[out] disparity Image where the disparity values will be written to.
 *                       Returned values are in Q10.5 format, i.e., signed fixed point with 5 fractional bits.
 *                       Divide it by 32.0f to convert it to floating point.
 *                       + Must not be NULL.
 *                       + On OFA backend, image format must be \ref VPI_IMAGE_FORMAT_S16_BL
 *                       + On other backends, image format must be \ref VPI_IMAGE_FORMAT_S16.
 *                       + On OFA+PVA+VIC backend, and if windowSize==1, confidenceMap==NULL and confidenceThreshold==0,
 *                         the image format can be either \ref VPI_IMAGE_FORMAT_S16 or \ref VPI_IMAGE_FORMAT_S16_BL.
 *                       + Dimensions must be equal to \p input,
 *                         but downscaled by \ref VPIStereoDisparityEstimatorCreationParams::downscaleFactor
 *                         passed in \p params.
 *                       + Image must have enabled the backends that will execute the algorithm.
 *
 * @param[out] confidenceMap Image containing a confidence score telling how accurate the disparity is.
 *                           Pass NULL if it isn't needed.
 *                           Returned confidence range from 0 to 65535.
 *                           The higher the confidence, more accurate is the corresponding disparity.
 *                           + OFA backend doesn't support confidence map output, it must be NULL.
 *                           + Image format must be \ref VPI_IMAGE_FORMAT_U16
 *                           + Must be same dimensions as \p disparity.
 *                           + Image must have enabled the backends that will execute the algorithm.
 *
 * @param[in] params Pointer to algorithm control parameters. If NULL, it'll use the
 *                   defaults given by \ref vpiInitStereoDisparityEstimatorParams.
 *
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p stream is NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p left, \p right or \p disparity are NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     Maximum disparity in \p params outside valid range.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     Window size in \p params outside valid range.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p left and \p right images must have the same dimensions.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     Input image dimensions must be equal to what's configured in \p payload.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p confidenceMap and \p disparity must have same dimensions.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     Backend doesn't output confidence map.
 * @retval #VPI_ERROR_INVALID_IMAGE_FORMAT \p left and \p right format don't match payload image format.
 * @retval #VPI_ERROR_INVALID_IMAGE_FORMAT \p disparity format is invalid.
 * @retval #VPI_ERROR_INVALID_PAYLOAD_TYPE \p payload is invalid.
 * @retval #VPI_ERROR_INVALID_OPERATION    The needed backends aren't enabled in \p stream, \p left, \p right,
 *                                         \p disparity or \p confidenceMap.
 * @retval #VPI_SUCCESS                    Operation executed successfully.
 */
VPI_PUBLIC VPIStatus vpiSubmitStereoDisparityEstimator(VPIStream stream, uint64_t backend, VPIPayload payload,
                                                       VPIImage left, VPIImage right, VPIImage disparity,
                                                       VPIImage confidenceMap,
                                                       const VPIStereoDisparityEstimatorParams *params);
/** @} end of VPI_StereoDisparityEstimator */

#ifdef __cplusplus
}
#endif

#endif /* NV_VPI_ALGORITHMS_STEREO_DISPARITY_H */
