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
 * @file KLTFeatureTracker.h
 *
 * Declares functions that implement the KLT Feature Tracker algorithm.
 */

#ifndef NV_VPI_ALGORITHMS_KLT_BOUNDING_BOX_TRACKER_H
#define NV_VPI_ALGORITHMS_KLT_BOUNDING_BOX_TRACKER_H

/**
 * @defgroup VPI_KLTFeatureTracker KLT Feature Tracker
 * @ingroup VPI_API_Algorithms
 * @{
 *
 * Runs KLT Feature tracking on a sequence of frames.
 * Refer to \ref algo_klt_tracker for more details and usage examples.
 */

#include "../Export.h"
#include "../ImageFormat.h"
#include "../Status.h"
#include "../Types.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Creation parameters of KLT Feature Tracker. */
typedef struct
{
    /** Maximum number of templates to be tracked.
     *  + Must be >= 1 and <= 128. */
    int32_t maxTemplateCount;

    /** Maximum width of each tracked template.
     *  + It must be >= 4 and <= 128 (on PVA, <= 64). */
    int32_t maxTemplateWidth;

    /** Maximum height of each tracked templates.
     *  + It must be >= 4 and <= 128 (on PVA, <= 64). */
    int32_t maxTemplateHeight;

} VPIKLTFeatureTrackerCreationParams;

/** Initialize \ref VPIKLTFeatureTrackerCreationParams with default values.
 *
 * Default values:
 *   - maxTemplateCount:  128
 *   - maxTemplateWidth:  64
 *   - maxTemplateHeight: 64
 *
 * @param[out] params Structure to be filled with default values.
 *
 * @retval #VPI_ERROR_INVALID_ARGUMENT \p params is NULL.
 * @retval #VPI_SUCCESS Operation executed successfully.
 */
VPI_PUBLIC VPIStatus vpiInitKLTFeatureTrackerCreationParams(VPIKLTFeatureTrackerCreationParams *params);

/**
 * Creates payload for \ref vpiSubmitKLTFeatureTracker
 *
 * @param[in] backends VPI backends that are eligible to execute the algorithm.
 *                     + Valid values:
 *                       - \ref VPI_BACKEND_CPU
 *                       - \ref VPI_BACKEND_CUDA
 *                       - \ref VPI_BACKEND_PVA
 *                     + Backend must be enabled in current context.
 *
 * @param[in] imageWidth, imageHeight Input image dimensions.
 *                                    + On PVA, dimensions must be between 65x65 and 3264x2448.
 *                                    + Must be >= 0 (other backends).
 *
 * @param[in] imageFormat Input image format.
 *                        + The accepted image formats are:
 *                          - \ref VPI_IMAGE_FORMAT_U8
 *                          - \ref VPI_IMAGE_FORMAT_S8
 *                          - \ref VPI_IMAGE_FORMAT_U16 (PVA only supports this format, pixel values' range must be between 0 and 255)
 *                          - \ref VPI_IMAGE_FORMAT_S16
 *
 * @param[in] params params 
 *
 * @param[out] payload Pointer to the payload variable that receives the created handle.
 *
 * @retval #VPI_ERROR_INVALID_ARGUMENT  \p payload is NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT  \p imageWidth and \p imageHeight outside valid range.
 * @retval #VPI_ERROR_INVALID_ARGUMENT  \p maxTemplateWidth and \p maxTemplateHeight in \p params outside valid range.
 * @retval #VPI_ERROR_INVALID_ARGUMENT  \p maxTemplateCount in \p params outside valid range.
 * @retval #VPI_ERROR_INVALID_ARGUMENT  \p backends refers to an invalid backend.
 * @retval #VPI_IMAGE_FORMAT_INVALID    \p imageFormat is not supported.
 * @retval #VPI_ERROR_INVALID_OPERATION PVA hardware is not available.
 * @retval #VPI_ERROR_INVALID_OPERATION Backend isn't enabled in current context.
 * @retval #VPI_ERROR_NOT_IMPLEMENTED   KLT Feature Tracker algorithm is not supported by given backend.
 * @retval #VPI_ERROR_INVALID_CONTEXT   Current context is destroyed.
 * @retval #VPI_ERROR_OUT_OF_MEMORY     Cannot allocate required resources.
 * @retval #VPI_SUCCESS                 Operation executed successfully.
 */

VPI_PUBLIC VPIStatus vpiCreateKLTFeatureTracker(uint64_t backends, int32_t imageWidth, int32_t imageHeight,
                                                VPIImageFormat imageFormat,
                                                const VPIKLTFeatureTrackerCreationParams *params, VPIPayload *payload);

/**
 * KLT Feature Tracker algorithm type.
 */
typedef enum
{
    /** Inverse compositional algorithm for KLT tracker.
        The inverse compositional algorithm is a reformulation of the classic
        Lucas-Kanade algorithm to make the steepest-descent images and Hessian
        constant.<br>
        <table border=0px margin=0px>
        <tr><td valign=top>Ref:</td><td>Simon Baker, Iain Matthew, <a href="http://www.ncorr.com/download/publications/bakerunify.pdf">"Lucas-Kanade 20 Years On: A Unified Framework"</a>.<br>
             International Journal of Computer Vision, February 2004, Volume 56, issue 3, pp. 228-231.</td></tr>
        </table>
    */
    VPI_KLT_INVERSE_COMPOSITIONAL
} VPIKLTFeatureTrackerType;

/**
 * Structure that defines the parameters for \ref vpiCreateKLTFeatureTracker
 */
typedef struct
{
    /** Number of Inverse compositional iterations of scale estimations.
     * + On PVA the maximum allowed is 20. */
    int32_t numberOfIterationsScaling;

    /** Threshold for requiring template update.
     *  + Must be a value between 0 and 1.
     *  + Must be greater than nccThresholdKill. */
    float nccThresholdUpdate;

    /** Threshold to consider template tracking was lost.
     *  + Must be a value between 0 and 1. */
    float nccThresholdKill;

    /** Threshold to early stop iteration.
     *  + Must be a value between 0 and 1.
        + Must be greater than nccThresholdUpdate. */
    float nccThresholdStop;

    /** Maximum relative scale change.
     *  Scale changes larger than this will make KLT consider that tracking was lost.
     *  + Must be >= 0.
     *  + On PVA, maximum scale change is 0.2. */
    float maxScaleChange;

    /** Maximum relative translation change.
     *  + Must be less >= 0.
     *  + Translation changes larger than this will make KLT consider that tracking was lost. */
    float maxTranslationChange;

    /** Type of KLT tracking that will be performed. */
    VPIKLTFeatureTrackerType trackingType;
} VPIKLTFeatureTrackerParams;

/** Initialize \ref VPIKLTFeatureTrackerParams with default values.
 *
 * Default values:
 *   - numberOfIterationsScaling: 20
 *   - nccThresholdUpdate: 0.8
 *   - nccThresholdKill: 0.6
 *   - nccThresholdStop: 1.0
 *   - maxScaleChange: 0.2
 *   - maxTranslationChange: 1.5
 *   - trackingType: VPI_KLT_INVERSE_COMPOSITIONAL
 *
 * @param[out] params Structure to be filled with default values.
 *
 * @returns an error code on failure else \ref VPI_SUCCESS.
 */
VPI_PUBLIC VPIStatus vpiInitKLTFeatureTrackerParams(VPIKLTFeatureTrackerParams *params);

/**
 * Runs KLT Feature Tracker on two frames. Outputs tracked bounding boxes and estimated transform array.
 *
 * @param[in] stream The stream where the operation will be queued in.
 *                   + Must not be NULL.
 *                   + Stream must have enabled the backends that will execute the algorithm.
 *
 * @param[in] backend Backend that will execute the algorithm.
 *                    + Must be the backend specified during payload creation or 0 as a shorthand to use this backend.
 *
 * @param[in] payload Payload created by \ref vpiCreateKLTFeatureTracker.
 *
 * @param[in] referenceImage Reference image.
 *                           + Must not be NULL.
 *                           + Must have same dimensions and format as template image. 
 *                           + Image must have enabled the backends that will execute the algorithm.
 *
 * @param[in] inputBoxList Input bounding box array.
 *                         + Must not be NULL.
 *                         + Must have type \ref VPI_ARRAY_TYPE_KLT_TRACKED_BOUNDING_BOX.
 *                         + Array must have enabled the backends that will execute the algorithm.
 *                         + On PVA, array capacity must be >= 128.
 *
 * @param[in] inputPredictionList Input predicted transform array.
 *                                + Must not be NULL.
 *                                + Must have type \ref VPI_ARRAY_TYPE_HOMOGRAPHY_TRANSFORM_2D.
 *                                + Array must have enabled the backends that will execute the algorithm.
 *                                + On PVA, array capacity must be >= 128.
 *
 * @param[in] templateImage Template image.
 *                          + Must not be NULL.
 *                          + Must have same dimensions and format as \p referenceImage.
 *                          + Image must have enabled the backends that will execute the algorithm.
 *
 * @param[out] outputBoxList Output Bounding box array.
 *                           + Must not be NULL.
 *                           + Must have type \ref VPI_ARRAY_TYPE_KLT_TRACKED_BOUNDING_BOX.
 *                           + Array must have enabled the backends that will execute the algorithm.
 *                           + On PVA, array capacity must be >= 128.
 *
 * @param[out] outputEstimationList Estimated transform array.
 *                                  + Must not be NULL.
 *                                  + Must have type \ref VPI_ARRAY_TYPE_HOMOGRAPHY_TRANSFORM_2D.
 *                                  + Array must have enabled the backends that will execute the algorithm.
 *                                  + On PVA, array capacity must be >= 128.
 *
 * @param[in] params Control parameters of the KLT feature tracker algorithm.
 * 
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p stream is NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p referenceImage, \p templateImage, \p inputBoxList, \p inputPredictionList,
 *                                         \p outputBoxList or \p outputEstimationList are NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p params is NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p payload not created by vpiCreateKLTFeatureTracker.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p templateImage and \p referenceImage dimensions and format do not match.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     Input and parameter image dimension do not match.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     Image format and \p payload image format do not match.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     Unsupported array type for \p inputBoxList or \p outputBoxList.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     Unsupported array type for \p inputPredictionList or \p outputEstimationList.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     Parameter(s) in \p params outside valid range.
 * @retval #VPI_ERROR_INVALID_PAYLOAD_TYPE \p payload is invalid.
 * @retval #VPI_ERROR_INVALID_OPERATION    The needed backends aren't enabled in \p stream, \p referenceImage, \p templateImage,
 *                                         \p inputBoxList, \p inputPredictionList, \p outputBoxList or \p outputEstimationList.
 * @retval #VPI_SUCCESS                    Operation executed successfully.
 */
VPI_PUBLIC VPIStatus vpiSubmitKLTFeatureTracker(VPIStream stream, uint64_t backend, VPIPayload payload,
                                                VPIImage templateImage, VPIArray inputBoxList,
                                                VPIArray inputPredictionList, VPIImage referenceImage,
                                                VPIArray outputBoxList, VPIArray outputEstimationList,
                                                const VPIKLTFeatureTrackerParams *params);
/** @} end of VPI_KLTFeatureTracker */

#ifdef __cplusplus
}
#endif

#endif /* NV_VPI_ALGORITHMS_KLT_BOUNDING_BOX_TRACKER_H */
