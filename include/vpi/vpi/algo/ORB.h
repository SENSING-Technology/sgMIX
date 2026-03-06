/*
 * Copyright 2022-2024 NVIDIA Corporation. All rights reserved.
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
 * @file ORB.h
 *
 * Declares functions that implement support for ORB.
 */

#ifndef NV_VPI_ALGORITHMS_ORB_H
#define NV_VPI_ALGORITHMS_ORB_H

#include "../Export.h"
#include "../Status.h"
#include "../Types.h"
#include "FASTCorners.h"

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup VPI_OrbFeatureDetector ORB features
 * @ingroup VPI_API_Algorithms
 * @{
 *
 * Runs the ORB algorithm over the input image to detect features and extract descriptors.
 * Refer to \ref algo_orb_feature_detector for more details and usage examples.
 */

/**
 * Use non-rotationally-invariant BRIEF in ORB.
 */
#define VPI_DISABLE_RBRIEF (0x01)

/**
 * Structure that defines the parameters for \ref vpiSubmitORBFeatureDetector.
 */
typedef struct
{
    /**
     * Parameters for the FAST corner detector, see \ref algo_fast_corners_detector for more details.
     * The FAST algorithm is used by ORB to detect corners or features per level of the input pyramid.
     */
    VPIFASTCornerDetectorParams fastParams;

    /**
     * The maximum number N of features per level of the input pyramid to be used by ORB.
     * The FAST algorithm may find a large number C of corners per level prior to filtering the N top corners.
     * The number C is the capacity input argument for the create ORB payload function:
     * \ref vpiCreateORBFeatureDetector.
     * The maximum number of features for all levels is defined by the capacity of the output arrays passed as
     * arguments to the submit ORB function: \ref vpiSubmitORBFeatureDetector.
     * The filtering is done depending on the \ref VPIORBParams.scoreType parameter.
     * + The number N (i.e. this parameter) must be lower or equal to the capacity C (see above definitions).
     */
    int32_t maxFeaturesPerLevel;

    /**
     * Maximum number of levels in the input pyramid to utilize.
     */
    int32_t maxPyramidLevels;

    /**
     * The score type allows to define how scores are assigned to corners.
     * The cornerness score is used to sort all corners detected by FAST from highest to lowest score value.
     * After score assignment, ORB filters the top N corners, where N is determined by the parameter:
     * \ref VPIORBParams.maxFeaturesPerLevel.
     * Use \ref VPI_CORNER_SCORE_HARRIS to assign cornerness scores based on Harris response score.
     * Use \ref VPI_CORNER_SCORE_FAST to skip cornerness score assignment and sorting.
     * By using FAST score type the performance of ORB is improved but the quality of output features is reduced.
     */
    VPICornerScore scoreType;

    /**
     * Control flags.
     * + Valid values are a combination of one or more of the following flags:
     *   -  0: default, negation of all other flags.
     *   - \ref VPI_DISABLE_RBRIEF : Disable rotationally-invariant BRIEF.
     */
    uint32_t flags;
} VPIORBParams;

/** Initializes \ref VPIORBParams with default values.
 *
 * Default values are:
 *  - params.fastParams                      See \ref vpiInitFASTCornerDetectorParams
 *  - params.maxFeaturesPerLevel:            100
 *  - params.maxPyramidLevels:               4
 *  - params.scoreType:                      VPI_CORNER_SCORE_HARRIS;
 *  - params.flags:                          0
 *
 * @retval #VPI_ERROR_INVALID_ARGUMENT \p params is NULL.
 * @retval #VPI_SUCCESS Operation executed successfully.
 */
VPI_PUBLIC VPIStatus vpiInitORBParams(VPIORBParams *params);

/**
 * Creates an \ref algo_orb_feature_detector "ORB feature detector" payload.
 * This function allocates all temporary memory needed by the algorithm.
 *
 * @param[in] backends VPI backends that are eligible to execute the algorithm.
 *                     + Valid values:
 *                       - \ref VPI_BACKEND_CPU
 *                       - \ref VPI_BACKEND_CUDA
 *                     + Backend must be enabled in current context.
 *
 * @param[in] capacity Capacity of internal buffers used to store FAST corners and scores per input pyramid level.
 *                     It determines the maximum number of features per level detected by FAST prior to ORB filtering.
 *                     The ORB algorithm assigns scores to these features, cf. \ref VPIORBParams.scoreType, and
 *                     sort them to filter the top \em N best features in accordance to these scores, where \em N is the
 *                     \ref VPIORBParams.maxFeaturesPerLevel parameter.
 *
 * @param[out] payload Pointer to the payload variable that receives the created handle.
 *
 * @retval #VPI_ERROR_INVALID_ARGUMENT  \p payload is NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT  \p backends refers to an invalid backend.
 * @retval #VPI_ERROR_INVALID_OPERATION Backend hardware not available.
 * @retval #VPI_ERROR_NOT_IMPLEMENTED   ORB algorithm is not supported by given backend.
 * @retval #VPI_ERROR_INVALID_CONTEXT   Current context is destroyed.
 * @retval #VPI_ERROR_OUT_OF_MEMORY     Cannot allocate required resources.
 * @retval #VPI_ERROR_INVALID_OPERATION Backend isn't enabled in current context.
 * @retval #VPI_SUCCESS                 Operation executed successfully.
 */
VPI_PUBLIC VPIStatus vpiCreateORBFeatureDetector(uint64_t backends, int32_t capacity, VPIPayload *payload);

/**
 * Submits an \ref algo_orb_feature_detector "ORB feature detector" operation to the stream.
 *
 * @note This operation detects features and extracts descriptors at the same time.  In case only the feature
 * detection is necessary, use this operation passing NULL to the descriptor output array for it to be ignored.
 *
 * @param[in] stream The stream where the operation will be queued in.
 *                   + Must not be NULL.
 *                   + Stream must have enabled the backends that will execute the algorithm.
 *
 * @param[in] backend Backend that will execute the algorithm.
 *                    + Valid values:
 *                      - \ref VPI_BACKEND_CPU
 *                      - \ref VPI_BACKEND_CUDA
 *
 * @param[in] payload Payload to be submitted along the other parameters.
 *
 * @param[in] input Input Input pyramid on which ORB will be executed.
 *                  + Must not be NULL.
 *                  + Pyramid must have enabled the backends that will execute the algorithm.
 *                  + The input pyramid must have scale == 0.5.
 *                  + The accepted image formats are:
 *                    - \ref VPI_IMAGE_FORMAT_Y8
 *                    - \ref VPI_IMAGE_FORMAT_Y16
 *                    - \ref VPI_IMAGE_FORMAT_Y8_ER
 *                    - \ref VPI_IMAGE_FORMAT_Y16_ER
 *                    - \ref VPI_IMAGE_FORMAT_U8
 *                    - \ref VPI_IMAGE_FORMAT_S8
 *                    - \ref VPI_IMAGE_FORMAT_U16
 *                    - \ref VPI_IMAGE_FORMAT_S16
 *
 * @param[out] outCorners Array that will receive the detected corners.
 *                        Array size is updated with the number of corners found.
 *                        Array capacity defines the maximum number of corners to be found for all levels.
 *                        The maximum possible number of features in all levels \em F is defined as the
 *                        \p VPIORBParams.maxFeaturesPerLevel times the number of levels.
 *                        + Must not be NULL.
 *                        + It must have type VPI_ARRAY_TYPE_PYRAMIDAL_KEYPOINT_F32 (see note).
 *                        + It must have the same capacity as outDescriptors if less than \em F (see above).
 *                        + Array must have enabled the backends that will execute the algorithm.
 *
 * @param[out] outDescriptors Array that will receive the descriptors for the corners
 *                            Array size is updated with the number of corners found.
 *                            Array capacity defines the maximum number of descriptors to be found for all levels.
 *                            The maximum possible number of features in all levels \em F is defined as the
 *                            \p VPIORBParams.maxFeaturesPerLevel times the number of levels.
 *                            + It may be NULL to do feature detection only.
 *                            + It must have type \ref VPI_ARRAY_TYPE_BRIEF_DESCRIPTOR.
 *                            + It must have the same capacity as outCorners if less than \em F (see above).
 *                            + Array must have enabled the backends that will execute the algorithm.
 *
 * @param[in] params Pointer to a VPIORBParams.
 *                   It defines the parameters for this algorithm invocation.
 *                   These parameters can vary in every call and will be copied internally.
 *                   Thus there is no need to keep the parameters object around.
 *                   - If NULL, use the defaults given by \ref vpiInitORBParams.
 *
 * @param[in] border How to handle pixels outside image boundaries.
 *                   It affects FAST corner detection per level, rf. \ref algo_fast_corners_detector.
 *                   + The accepted border extensions are:
 *                     - \ref VPI_BORDER_ZERO
 *                     - \ref VPI_BORDER_CLAMP
 *                     - \ref VPI_BORDER_REFLECT
 *                     - \ref VPI_BORDER_MIRROR
 *                     - \ref VPI_BORDER_LIMITED (ignore pixels with circle going outside image boundaries)
 *
 * @note For backward compatibility with VPI 3.0, \ref VPI_ARRAY_TYPE_KEYPOINT_F32 is also supported as an array type for
 * the \p outCorners parameter. With the VPI 3.0 legacy ORB behavior, keypoint coordinates are scaled to the resolution
 * of the first pyramid level and descriptors are computed within the first image of the pyramid.
 *
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p stream is NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p input or \p outCorners or \p outDescriptors are NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p backend is invalid or unknown.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     A parameter in \p params is outside valid range.
 * @retval #VPI_ERROR_INVALID_ARRAY_TYPE   Invalid \p outCorners or \p outDescriptors array type.
 * @retval #VPI_ERROR_INVALID_IMAGE_FORMAT Unsupported input format.
 * @retval #VPI_ERROR_INVALID_OPERATION    The needed backends aren't enabled in \p stream or \p input.
 * @retval #VPI_ERROR_INVALID_OPERATION    The needed backends aren't enabled in \p outCorners or \p outDescriptors.
 * @retval #VPI_SUCCESS                    Operation executed successfully.
 */
VPI_PUBLIC VPIStatus vpiSubmitORBFeatureDetector(VPIStream stream, uint64_t backend, VPIPayload payload,
                                                 VPIPyramid input, VPIArray outCorners, VPIArray outDescriptors,
                                                 const VPIORBParams *params, VPIBorderExtension border);

/**
 * Creates an \ref algo_orb_feature_detector "ORB descriptor extractor" payload.
 * This function allocates all temporary memory needed by the algorithm.
 *
 * @param[in] backends VPI backends that are eligible to execute the algorithm.
 *                     + Valid values:
 *                       - \ref VPI_BACKEND_CPU
 *                       - \ref VPI_BACKEND_CUDA
 *                       - \ref VPI_BACKEND_PVA
 *                     + Backend must be enabled in current context.
 *
 * @param[out] payload Pointer to the payload variable that receives the created handle.
 *
 * @retval #VPI_ERROR_INVALID_ARGUMENT  \p payload is NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT  \p backends refers to an invalid backend.
 * @retval #VPI_ERROR_INVALID_OPERATION Backend hardware not available.
 * @retval #VPI_ERROR_NOT_IMPLEMENTED   ORB algorithm is not supported by given backend.
 * @retval #VPI_ERROR_INVALID_CONTEXT   Current context is destroyed.
 * @retval #VPI_ERROR_OUT_OF_MEMORY     Cannot allocate required resources.
 * @retval #VPI_ERROR_INVALID_OPERATION Backend isn't enabled in current context.
 * @retval #VPI_SUCCESS                 Operation executed successfully.
 */
VPI_PUBLIC VPIStatus vpiCreateORBDescriptorExtractor(uint64_t backends, VPIPayload *payload);

/**
 * Submits an \ref algo_orb_feature_detector "ORB descriptor extractor" operation to the stream.
 *
 * @note This operation is only useful in a scenario where decoupled ORB feature detection and extraction is
 * necessary.  For cases where both detection and extraction is executed at the same time, use \ref
 * vpiSubmitORBFeatureDetector instead.
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
 * @param[in] payload Payload reserved for future use.  It may be NULL.
 *
 * @param[in] input Input Input on which ORB descriptor extractor will be executed.
 *                  + Must not be NULL.
 *                  + Input must have enabled the backends that will execute the algorithm.
 *                  + Input must be a VPIPyramid (see note).
 *                  + The input pyramid must have scale == 0.5.
 *                  + The accepted image formats are:
 *                    - \ref VPI_IMAGE_FORMAT_Y8
 *                    - \ref VPI_IMAGE_FORMAT_Y16
 *                    - \ref VPI_IMAGE_FORMAT_Y8_ER
 *                    - \ref VPI_IMAGE_FORMAT_Y16_ER
 *                    - \ref VPI_IMAGE_FORMAT_U8
 *                    - \ref VPI_IMAGE_FORMAT_S8
 *                    - \ref VPI_IMAGE_FORMAT_U16
 *                    - \ref VPI_IMAGE_FORMAT_S16
 *                  + With \ref VPI_BACKEND_PVA :
 *                    - Signed image formats are not supported.
 *                    - The resolution of every pyramid level must be greater than 45x45
 *
 * @param[in] inCorners Array with corners to compute descriptors from.
 *                      Each item in this array, i.e. a corner, is used to compute one item in the output array,
 *                      i.e. a descriptor for the corresponding corner.
 *                      The size of this input array determines the number of corners to extract descriptors from.
 *                      + Must not be NULL.
 *                      + It must have type \ref VPI_ARRAY_TYPE_PYRAMIDAL_KEYPOINT_F32 (see note).
 *                      + Array must have enabled the backends that will execute the algorithm.
 *                      + With \ref VPI_BACKEND_PVA :
 *                        - \ref VPIPyramidalKeypointF32.layer is ignored and the descriptors are computed as if
 *                          layer == 0 always.
 *
 * @param[out] outDescriptors Array that will receive the descriptors for the corners.
 *                            Each position in this output array corresponds to the corner descriptor for the same
 *                            position in the input array.
 *                            The output array capacity defines the maximum number of descriptors to be extracted.
 *                            The output array capacity must be bigger than the size of the input array to extract
 *                            descriptors for all corners.
 *                            + Must not be NULL.
 *                            + It must have type \ref VPI_ARRAY_TYPE_BRIEF_DESCRIPTOR .
 *                            + Array must have enabled the backends that will execute the algorithm.
 *                            + With \ref VPI_BACKEND_PVA :
 *                              - Descriptors generated by PVA are not typically compatible with descriptors generated
 *                                by other backends.
 *
 * @param[in] flags Control flags.
 *                  + Valid values are a combination of one or more of the following flags:
 *                    -  0: default, negation of all other flags.
 *                    - \ref VPI_DISABLE_RBRIEF : Disable rotationally-invariant BRIEF
 *
 * @note For backward compatibility with VPI 3.0 an alternative API signature is available where \p input must be a
 * VPIImage and \p inCorners must have type \ref VPI_ARRAY_TYPE_KEYPOINT_F32 . To select the VPI 3.0 interface, define
 * \ref NV_VPI_VERSION_API as NV_VPI_MAKE_VERSION(3,0) prior to including the \ref ORB.h header file. PVA backend is
 * only supported for API level 3.1 and up, while using VPI library version 3.2 and up.
 *
 * @retval #VPI_ERROR_INVALID_ARGUMENT     Either \p stream or \p input or \p inCorners or \p outDescriptors is NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     The \p backend is invalid or unknown.
 * @retval #VPI_ERROR_INVALID_ARRAY_TYPE   Invalid \p inCorners or \p outDescriptors array type.
 * @retval #VPI_ERROR_INVALID_IMAGE_FORMAT Unsupported input format.
 * @retval #VPI_ERROR_INVALID_OPERATION    The needed backends aren't enabled in \p stream or \p input.
 * @retval #VPI_ERROR_INVALID_OPERATION    The needed backends aren't enabled in \p inCorners or \p outDescriptors.
 * @retval #VPI_SUCCESS                    Operation executed successfully.
 */
#if NV_VPI_VERSION_API_AT_LEAST(3, 1)
VPI_PUBLIC VPIStatus vpiSubmitORBDescriptorExtractor(VPIStream stream, uint64_t backend, VPIPayload payload,
                                                     VPIPyramid input, VPIArray inCorners, VPIArray outDescriptors,
                                                     uint32_t flags);
#else
__asm__(".symver vpiSubmitORBDescriptorExtractor,vpiSubmitORBDescriptorExtractor@VPI_3.0");
VPI_PUBLIC VPIStatus vpiSubmitORBDescriptorExtractor(VPIStream stream, uint64_t backend, VPIPayload payload,
                                                     VPIImage input, VPIArray inCorners, VPIArray outDescriptors,
                                                     uint32_t flags);
#endif

#ifdef __cplusplus
}
#endif

/** @} */ // end of VPI_OrbFeatureDetector

#endif /* NV_VPI_ALGORITHMS_ORB_H */
