/*
 * Copyright 2023 NVIDIA Corporation. All rights reserved.
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
 * @file DCFTracker.h
 *
 * Declares functions that implement the DCF Tracker algorithm.
 */

#ifndef NV_VPI_ALGORITHMS_DCF_TRACKER_H
#define NV_VPI_ALGORITHMS_DCF_TRACKER_H

/**
 * @defgroup VPI_DCFTracker DCF Tracker
 * @ingroup VPI_API_Algorithms
 * @{
 *
 * Tracks object bounding boxes on a sequence of frames using the
 * Discriminative Correlation Filter (DCF) algorithm.
 *
 * The processing flow allows for external customization of the tracking algorithm,
 * including tracking termination criteria and external refinement of estimated
 * of the tracked object's bounding box.
 *
 * The high-level pseudo-code of the processing flow is as follows:
 * @code

   VPIDCFTrackerCreationParams creatParams;
   vpiInitDCFTrackerCreationParams(&creatParams);
   // customize creatParams as needed

   VPIPayload dcfPayload;
   vpiCreateDCFTracker(..., NUM_SEQUENCES, MAX_OBJECTS, &creatParams, &dcfPayload);

   VPIPayload cropPayload;
   vpiCreateCropScaler(..., , NUM_SEQUENCES, MAX_OBJECTS, &cropPayload);

   // Optionally user can retrieve the internal array that stores the channel weights and
   // the maximum correlation response for each tracked object. These can be used, together
   // with the correlation map, to decide whether tracking was lost of not.
   VPIArray channelWeights;
   int32_t numFeatureChannels
   vpiDCFTrackerGetChannelWeights(dcfPayload, &channelWeights, &numFeatureChannels);

   // Array storing maximum correlation responses per object (optional)
   VPIArray outMaxCorrelationResponses = // ...

   // Stores the current (first) input frame per input sequence
   VPIImage frames[NUM_SEQUENCES] = // ...
   // Set element to 1 if objects in given sequence must be tracked, otherwise
   // 0 to ignore them.
   int32_t enabledSequences[NUM_SEQUENCES] = // ...

   // Array of objects detected in input frames. It must be populated with
   // objects detected in all input frame sequence
   // The state of the objects must be \ref VPI_TRACKING_STATE_NEW.
   VPIArray inObjects = // ...

   // Array of objects with estimated bounding boxes.
   // Initially it's an empty array with same capacity and format as inObjects.
   VPIArray outObjects = // ...

   // Image that stores the image patches of all objects.
   // Dimensions must be:
   //    width:  createParams.featurePatchSize*createParams.hogCellSize
   //    height: width * (inObjects' capacity)
   VPIImage objPatches = // ...

   VPIDCFTrackerParams params;
   vpiInitDCFTrackerParams(&params);
   // customize params as needed

   // Extract from the first frame the image patches of each object given its current bounding box.
   vpiSubmitCropScalerBatch(stream, cropPayload, frames, NUM_SEQUENCES,
                            inObjects, objPatches);

   // Update internal state with the new objects detected in first frame.
   vpiSubmitDCFTrackerUpdateBatch(stream, dcfPayload, enabledSequences, NUM_SEQUENCES,
                                  NULL, objPatches, inObjects, &params);
   while(HasMoreFrames())
   {
       // Load next set of frames from the input sequences.
       LoadNextFramesFromSequence(frames, NUM_SEQUENCES);

       for(int i=0; i<NUM_SEQUENCES; ++i) enabledSequences[i] = frames[i] != NULL;

       // Extract from the current frame the image patches of each object given
       // its current bounding box.
       vpiSubmitCropScalerBatch(stream, cropPayload, frames, NUM_SEQUENCES,
                                NULL, inObjects, objPatches);

       // Localize the tracked objects in the new image patches.
       vpiSubmitDCFTrackerLocalizeBatch(stream, dcfPayload, enabledSequences, NUM_SEQUENCES,
                                        NULL, objPatches, inObjects, outObjects, outCorrelationResponses,
                                        outMaxCorrelationResponses, &params);
       // object's bounding box will reflect its position in input frame(s)

       // By using 'outMaxCorrelationResponses', 'outCorrelationResponses' and 'channelWeights',
       // the following processing can be done at this point:
       // 1. Object's bounding boxes can be refined by some external method.
       // 2. Object's state can be updated, e.g. whether tracking was lost or not. If tracking
       //    was lost, object's state must be set to \ref VPI_TRACKING_STATE_LOST.
       // 3. New objects can be added to outObjects, their state must be \ref VPI_TRACKING_STATE_NEW.
       //    Objects in the array whose state is \ref VPI_TRACKING_STATE_LOST can be reused for new objects.

       // Extract from the current frame the image patches of each object given its refined bounding box.
       vpiSubmitCropScalerBatch(stream, cropPayload, frames, NUM_SEQUENCES, inObjects, objPatches);

       // Update tracking information
       vpiSubmitDCFTrackerUpdateBatch(stream, dcfPayload, enabledSequences, NUM_SEQUENCES,
                                      NULL, objPatches, outObjects, &params);

       // Do a ping-pong with the in and out objects.
       // Output object array will be the input in next iteration, and current input will be the output.
       std::swap(inObjects, outObjects);
  }
 * @endcode
 */

#include "../Export.h"
#include "../ImageFormat.h"
#include "../Status.h"
#include "../Types.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Flags to customize DCF Tracker algorithm */
typedef enum
{
    /** Use ColorNames feature. */
    VPI_DCFTRACKER_USE_COLORNAMES = 1,
    /** Use Histogram of Oriented Gradients feature. */
    VPI_DCFTRACKER_USE_HOG = 2,
    /** Use 16-bit high precision features. If not set, use 8-bit precision features. */
    VPI_DCFTRACKER_USE_HIGH_PRECISION_FEATURES = 4,
    /** Use deterministic mode. Determinism can only be guaranteed per architecture. */
    VPI_DCFTRACKER_DETERMINISTIC = 8
} VPIDCFTrackerCreationFlag;

/** Creation parameters of DCF Tracker. */
typedef struct
{
    /** Flags to customize DCF Tracker algorithm.
     * + Must be one or more of the \ref VPIDCFTrackerCreationFlag enums, or-ed together.
     * + Must select either ColorName or HoG or both features.
     */
    uint32_t flags;

    /** Size of an object feature patch.
     * The patch image is always square, thus the value passed correspond
     * to one dimension.
     * - For \ref VPI_BACKEND_CUDA, the valid sizes are: 12, 18, 24, 30, 32, 36.
     * - For \ref VPI_BACKEND_PVA, the valid sizes are: 24. */
    int32_t featurePatchSize;

    /** Cell size for features from Histogram of Oriented Gradients.
     *  + Must be between 2 and 32, inclusive.
     *  + Must be a power of two. */
    int32_t hogCellSize;

    /** Offset for the center of the Hanning window relative to the patch height.
     * + Must be between -0.5 to 0.5, inclusive */
    float featureFocusVertOffsetFactor;

    /** Standard deviation for gaussian for desired response.
     * + Must be > 0.0. */
    float gaussianSigma;
} VPIDCFTrackerCreationParams;

/** Initialize \ref VPIDCFTrackerCreationParams with default values.
 *
 * Default values:
 *   - flags:     \ref VPI_DCFTRACKER_USE_COLORNAMES | \ref VPI_DCFTRACKER_USE_HOG
 *   - featurePatchSize: 24
 *   - hogCellSize: 4
 *   - featureFocusVertOffsetFactor: 0.0
 *   - gaussianSigma: 0.75
 *
 * @param[out] params Structure to be filled with default values.
 *
 * @retval #VPI_ERROR_INVALID_ARGUMENT \p params is NULL.
 * @retval #VPI_SUCCESS Operation executed successfully.
 */
VPI_PUBLIC VPIStatus vpiInitDCFTrackerCreationParams(VPIDCFTrackerCreationParams *params);

/**
 * Creates payload for DCF Tracker. 
 *
 * @param[in] backends VPI backends that are eligible to execute the algorithm.
 *                     + Valid values:
 *                       - \ref VPI_BACKEND_PVA
 *                       - \ref VPI_BACKEND_CUDA
 *                     + Backend must be enabled in current context.
 *
 * @param[in] maxNumSequences Maximum number of sequences that contains objects.
 *                            + Must be >= 1.
 *                            + For \ref VPI_BACKEND_PVA, must be <= 33
 *                            + For \ref VPI_BACKEND_CUDA, must be <= 1024
 *
 * @param[in] maxNumObjects Maximum number of objects to track.
 *                          + Must be >= 1.
 *                          + For \ref VPI_BACKEND_PVA, must be <= 512
 *
 * @param[in] params Configuration parameters.
 *                   Pass NULL to use defaults specified by \ref vpiInitDCFTrackerCreationParams.
 *
 * @param[out] payload Pointer to the payload variable that receives the created handle.
 *
 * @retval #VPI_ERROR_INVALID_ARGUMENT  \p payload is NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT  \p backends refers to an invalid backend.
 * @retval #VPI_ERROR_INVALID_ARGUMENT  \p maxTrackedObjects is outside valid range.
 * @retval #VPI_ERROR_INVALID_ARGUMENT  One or more configuration parameters are invalid.
 * @retval #VPI_ERROR_INVALID_OPERATION PVA hardware is not available.
 * @retval #VPI_ERROR_INVALID_OPERATION Backend isn't enabled in current context.
 * @retval #VPI_ERROR_NOT_IMPLEMENTED   DCF Tracker algorithm is not supported by given backend.
 * @retval #VPI_ERROR_INVALID_CONTEXT   Current context is destroyed.
 * @retval #VPI_ERROR_OUT_OF_MEMORY     Cannot allocate required resources.
 * @retval #VPI_SUCCESS                 Operation executed successfully.
 */

VPI_PUBLIC VPIStatus vpiCreateDCFTracker(uint64_t backends, int32_t maxNumSequences, int32_t maxNumObjects,
                                         const VPIDCFTrackerCreationParams *params, VPIPayload *payload);

/**
 * Structure that defines the parameters for \ref vpiCreateDCFTracker */
typedef struct
{
    /** Regularization factor used in DCF filter creation. */
    float dcfRegFactor;
} VPIDCFTrackerParams;

/** Initialize \ref VPIDCFTrackerParams with default values.
 *
 * Default values:
 *   - dcfRegFactor: 5.f/100
 *
 * @param[out] params Structure to be filled with default values.
 *
 * @returns an error code on failure else \ref VPI_SUCCESS.
 */
VPI_PUBLIC VPIStatus vpiInitDCFTrackerParams(VPIDCFTrackerParams *params);

/**
 * Localizes each tracked object in the input image patches using the Discriminative Correlation Filter method.
 *
 * The bounding box of each tracked object will be updated with the object's
 * estimated position in its corresponding input patch.
 *
 * @param[in] stream The stream where the operation will be queued in.
 *                   + Must not be NULL.
 *                   + Stream must have enabled the backends that will execute the algorithm.
 *
 * @param[in] backend Backend that will execute the algorithm.
 *                    + Must be the backend specified during payload creation or 0 as a shorthand to use this backend.
 *
 * @param[in] payload Payload created by \ref vpiCreateDCFTracker.
 *                    + Must not be NULL.
 *
 * @param[in] enabledSequences Defines the sequences whose objects are to be processed, irrespective of their state.
 *                             If an element in the array is 0, the objects that refer the sequence with given index
 *                             won't be processed, otherwise the algorithm will use the object's state to tell if objects
 *                             is to be processed or not.
 *                             - If NULL, all sequences will be enabled.
 *
 * @param[in] numSequences Number of elements in \p enabledSequences.
 *                         If an object refer to a sequence whose index is >= \p numSequences, this sequence will
 *                         be assumed to be enabled.
 *                         Or else if the sequence index refered is < 0, this sequence will be assumed to be disabled.
 *                         + Must be >= 0 and <= max sequences specified in payload creation.
 *
 * @param[in] featureMaskingWindow Image used as window applied to each object feature channel, commonly used to reduce border effects.
 *                                 If NULL, a standard Hanning window will be used in all objects being tracked.
 *                                 + Image format must be \ref VPI_IMAGE_FORMAT_F32.
 *                                 + Image size must be equal to feature size as specified in featurePatchSize configuration.
 *
 * @param[in] inPatches Image containing the image patches that tentatively contain the tracked objects.
 *                      Patches are usually generated by the CropScaler algorithm.
 *                      + Must not be NULL.
 *                      + Image width must be equal to featurePatchSize * hogCellSize (see \ref VPIDCFTrackerCreationParams).
 *                        If HOG isn't being used, consider hogCellSize==1.
 *                      + Image height must be a multiple of featurePatchSize * hogCellSize, and
 *                        at be at least featurePatchSize * hogCellSize * (number of objects in \p inObjects).
 *                        If HOG isn't being used, consider hogCellSize==1.
 *                      + Valid Image formats:
 *                        |         Formats                 | CUDA |  PVA |
 *                        |---------------------------------|:----:|:----:|
 *                        | \ref VPI_IMAGE_FORMAT_RGBA8     |   *  |      |
 *                        | \ref VPI_IMAGE_FORMAT_RGB8p     |      |   *  |
 *
 * @param[in] inObjects Input array with objects being tracked.
 *                      + Must not be NULL.
 *                      + Must have type \ref VPI_ARRAY_TYPE_DCF_TRACKED_BOUNDING_BOX.
 *                      + Array must have enabled the backends that will execute the algorithm.
 *
 * @param[out] outObjects Output objects' array with their bounding box updated with the estimated position on input sequence.
 *                        Only objects whose state is \ref VPI_TRACKING_STATE_TRACKED are updated. Remaining objects
 *                        are just copied over.
 *                        + Must not be NULL.
 *                        + Must have type \ref VPI_ARRAY_TYPE_DCF_TRACKED_BOUNDING_BOX.
 *                        + Its capacity must be equal or greater than the number of elements in \p inObjects.
 *                        + Array must have enabled the backends that will execute the algorithm.
 *
 * @param[out] outCorrelationResponses Optional output with the correlation response map for each object being tracked.
 *                                     If NULL, correlation response won't be generated.
 *                                     The correlation map for an object 'o' will start at row `o * featurePatchSize`
 *                                     and have height 'featurePatchSize'.
 *                                     + Must be an image with format \ref VPI_IMAGE_FORMAT_F32.
 *                                     + Image width must be equal to patch width (see \ref VPIDCFTrackerCreationParams::featurePatchSize)
 *                                     + Image height must be equal to patch height times the number of objects in \p inObjects.
 *
 * @param[out] outMaxCorrelationResponses Option output with the maximum correlation response for each tracked object.
 *                                        The maximum correlation response is only valid for objects
 *                                        whose state is \ref VPI_TRACKING_STATE_TRACKED
 *                                        and \ref VPI_TRACKING_STATE_SHADOW_TRACKED.
 *                                        + Must be an array with type \ref VPI_ARRAY_TYPE_F32.
 *                                        + Array capacity must be >= the number of objects in \p inObjets array.
 *
 * @param[in] params Control parameters of the DCF tracker algorithm.
 *                   If NULL, use defaults as specified by \ref vpiInitDCFTrackerParams.
 *
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p stream is NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p inPatches, \p inObjects or outObjects are NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p payload not created by vpiCreateDCFTracker.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     Unsupported array type in \p inObjects or \p outObjects.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p outObjects capacity too small.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     Parameter(s) in \p params outside valid range.
 * @retval #VPI_ERROR_INVALID_PAYLOAD_TYPE \p payload is invalid.
 * @retval #VPI_ERROR_INVALID_OPERATION    The needed backends aren't enabled in \p stream, \p inPatches, \p inObjects or outObjects.
 * @retval #VPI_SUCCESS                    Operation executed successfully.
 */
VPI_PUBLIC VPIStatus vpiSubmitDCFTrackerLocalizeBatch(
    VPIStream stream, uint64_t backend, VPIPayload payload, const int32_t *enabledSequences, int32_t numSequences,
    VPIImage featureMaskingWindow, VPIImage inPatches, VPIArray inObjects, VPIArray outObjects,
    VPIImage outCorrelationResponses, VPIArray outMaxCorrelationResponses, const VPIDCFTrackerParams *params);

/**
 * Update internal object tracking information based on its state and its corresponding input image patch.
 *
 * The operation performed on each object depends on the object's state:
 * - \ref VPI_TRACKING_STATE_NEW : object will be set up for tracking starting from the given input image patch.
 *                                 Upon function execution in the given stream, its state will transition
 *                                 to \ref VPI_TRACKING_STATE_TRACKED if object can be tracked, or
 *                                 \ref VPI_TRACKING_STATE_LOST otherwise.
 * - \ref VPI_TRACKING_STATE_TRACKED : updates internal information about the object using the input image patch
 *                                     that must correspond to its bounding box in the current sequence frame,
 *                                     which could have been modified between localize and update calls.
 *
 *
 * @param[in] stream The stream where the operation will be queued in.
 *                   + Must not be NULL.
 *                   + Stream must have enabled the backends that will execute the algorithm.
 *
 * @param[in] backend Backend that will execute the algorithm.
 *                    + Must be the backend specified during payload creation or 0 as a shorthand to use this backend.
 *
 * @param[in] payload Payload created by \ref vpiCreateDCFTracker.
 *                    + Must not be NULL.
 *
 * @param[in] enabledSequences Defines the sequences whose objects are to be processed, irrespective of their state.
 *                             If an element in the array is 0, the objects that refer the sequence with given index
 *                             won't be processed, otherwise the algorithm will use the object's state to tell if objects
 *                             is to be processed or not.
 *                             - If NULL, all sequences will be enabled.
 *
 * @param[in] numSequences Number of elements in \p enabledSequences.
 *                         If an object refer to a sequence whose index is >= \p numSequences, this sequence will
 *                         be assumed to be enabled.
 *                         Or else if the sequence index refered is < 0, this sequence will be assumed to be disabled.
 *                         + Must be >= 0 and <= max sequences specified in payload creation.
 *
 * @param[in] featureMaskingWindow Image used as window applied to each object feature channel, commonly used to reduce border effects.
 *                                 If NULL, a standard Hanning window will be used for all patches being tracked.
 *                                 + Image format must be \ref VPI_IMAGE_FORMAT_F32.
 *                                 + Image size must be equal to patch size as specified in featurePatchSize configuration.
 *
 * @param[in] modelMaskingWindow Image used as window applied to the internal object model.
 *                               If NULL, the feature masking window will be used.
 *                               + Image format must be \ref VPI_IMAGE_FORMAT_F32.
 *                               + Image size must be equal to patch size as specified in featurePatchSize configuration.
 *
 * @param[in] inPatches Image containing the patches of each object in \p trackedObjects that corresponds to its bounding box.
 *                      Patches are usually generated by the CropScaler algorithm.
 *                      + Must not be NULL.
 *                      + Image width must be equal to featurePatchSize * hogCellSize (see \ref VPIDCFTrackerCreationParams).
 *                        If HOG isn't being used, consider hogCellSize==1.
 *                      + Image height must be a multiple of featurePatchSize * hogCellSize, and
 *                        at be at least featurePatchSize * hogCellSize * (number of objects in \p trackedObjects).
 *                        If HOG isn't being used, consider hogCellSize==1.
 *                      + Valid Image formats:
 *                        |         Formats                 | CUDA |  PVA |
 *                        |---------------------------------|:----:|:----:|
 *                        | \ref VPI_IMAGE_FORMAT_RGBA8     |   *  |      |
 *                        | \ref VPI_IMAGE_FORMAT_RGB8p     |      |   *  |
 *
 * @param[in,out] trackedObjects Array with tracked objects.
 *                               + Must not be NULL.
 *                               + Must have type \ref VPI_ARRAY_TYPE_DCF_TRACKED_BOUNDING_BOX.
 *                               + Array must have enabled the backends that will execute the algorithm.
 *
 * @param[in] params Control parameters of the DCF tracker algorithm.
 *                   If NULL, use defaults as specified by \ref vpiInitDCFTrackerParams.
 *
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p stream is NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p inPatches or trackedObjects are NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p payload not created by vpiCreateDCFTracker.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p inPatches dimensions outside valid range.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     Unsupported array type in \p trackedObjects.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     Parameter(s) in \p params outside valid range.
 * @retval #VPI_ERROR_INVALID_PAYLOAD_TYPE \p payload is invalid.
 * @retval #VPI_ERROR_INVALID_OPERATION    The needed backends aren't enabled in \p stream, \p inPatches or \p trackedObjects.
 * @retval #VPI_SUCCESS                    Operation executed successfully.
 */
VPI_PUBLIC VPIStatus vpiSubmitDCFTrackerUpdateBatch(VPIStream stream, uint64_t backend, VPIPayload payload,
                                                    const int32_t *enabledSequences, int32_t numSequences,
                                                    VPIImage featureMaskingWindow, VPIImage modelMaskingWindow,
                                                    VPIImage inPatches, VPIArray trackedObjects,
                                                    const VPIDCFTrackerParams *params);

/** Returns the array with channel weight information for each tracked object.
 *
 * @param[in] payload Payload created by \ref vpiCreateDCFTracker.
 *                    + Must not be NULL.
 *
 * @param[out] channelWeights Array of channel weights for each tracked object.
 *                            The returned \ref VPIArray has type \ref VPI_ARRAY_TYPE_F32 and size equal to
 *                            \p numFeatureChannels times the number of tracked objects.
 *                            The weight 'c' of object with index 'o' in array buffer 'd' is given by:
 *                            \f[
 *                              w_d(c,o) = (float)d + c * numFeatureChannels
 *                            \f]
 *                            The weight information is only valid for objects
 *                            whose state is \ref VPI_TRACKING_STATE_TRACKED.
 *                            + Must not be NULL.
 *                            @note This array must not be destroyed via \ref vpiArrayDestroy.
 * @param[out] numFeatureChannels Number of feature channels per object.
 *
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p channelWeights or numFeatureChannels is NULL.
 * @retval #VPI_SUCCESS                    Operation executed successfully.
 */
VPI_PUBLIC VPIStatus vpiDCFTrackerGetChannelWeights(VPIPayload payload, VPIArray *channelWeights,
                                                    int32_t *numFeatureChannels);

/** @} end of VPI_DCFTracker */

#ifdef __cplusplus
}
#endif

#endif /* NV_VPI_ALGORITHMS_DCF_TRACKER_H */
