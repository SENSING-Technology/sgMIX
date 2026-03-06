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
 * @file OpticalFlowPyrLK.h
 *
 * Declares functions that implement the Pyramidal LK Optical Flow algorithm.
 */

#ifndef NV_VPI_ALGORITHMS_OPTICALFLOWPYRLK_H
#define NV_VPI_ALGORITHMS_OPTICALFLOWPYRLK_H

/**
 * @defgroup VPI_OpticalFlowPyrLK Pyramidal LK Optical Flow
 * @ingroup VPI_API_Algorithms
 * @{
 *
 * Algorithm used to track points from one frame to the next.
 * Refer to \ref algo_optflow_lk for more details and usage examples.
 */

#include "../Export.h"
#include "../ImageFormat.h"
#include "../Status.h"
#include "../Types.h"

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Defines the error measurement types
 */
typedef enum
{
    /** L1 distance between previous feature and a next feature. */
    VPI_LK_ERROR_L1
} VPIEpsilonType;

/**
 * Structure that defines the parameters for \ref vpiSubmitOpticalFlowPyrLK.
 */
typedef struct
{
    /** Uses initial estimations stored in current frame keypoints array when this flag is not 0,
     *  otherwise previous frame keypoints are copied to current frame keypoints array and is considered the initial estimate.
     */
    uint32_t useInitialFlow;

    /** Specifies the termination criteria.
     *  + It must be bitwise combination of one or more of the following values:
     *    - \ref VPI_TERMINATION_CRITERIA_ITERATIONS
     *    - \ref VPI_TERMINATION_CRITERIA_EPSILON
     */
    uint32_t termination;

    /** Specifies the tracking error type. */
    VPIEpsilonType epsilonType;

    /** Specifies minimum error threshold for terminating the algorithm.
     *  Only applicable if termination flags includes \ref VPI_TERMINATION_CRITERIA_ITERATIONS. */
    float epsilon;

    /** Specifies the maximum number of iterations.
     *  Only applicable if termination flags includes \ref VPI_TERMINATION_CRITERIA_ITERATIONS.
     *  + Must be >= 1 and <= 32. */
    int32_t numIterations;

    /** Specifies the size of the window on which to perform the algorithm.
     *  + Must be >= 6 and <= 32.
     *  + For PVA Backend: Must be 7, 9 or 11.*/
    int32_t windowDimension;

} VPIOpticalFlowPyrLKParams;

/**
 * Initializes \ref VPIOpticalFlowPyrLKParams with default values.
 * Returns the default Pyramidal LK Optical Flow parameters that are compatible with all the given backends.
 * 
 * Defaults:
 * - useInitialFlow  = 0
 * - termination     = VPI_TERMINATION_CRITERIA_ITERATIONS | VPI_TERMINATION_CRITERIA_EPSILON
 * - epsilonType     = VPI_LK_ERROR_L1
 * - epsilon         = 0
 * - windowDimension = 11 if backends include \ref VPI_BACKEND_PVA, otherwise 15 
 * - numIterations   = 6
 *
 * @param[in] backends Specifies the backend to get default values for.
 *                     + Valid values: 
 *                       - \ref VPI_BACKEND_CPU
 *                       - \ref VPI_BACKEND_CUDA
 *                       - \ref VPI_BACKEND_PVA
 *                     + Can be \ref VPI_BACKEND_ALL or a combination of multiple \ref VPIBackend.
 * 
 * @param[in] params Structure to be filled with default values.
 * 
 * @retval #VPI_ERROR_INVALID_ARGUMENT \p params is NULL.
 * @retval #VPI_SUCCESS Operation executed successfully.
 */
#if NV_VPI_VERSION_API_AT_MOST(3, 1)
__asm__(".symver vpiInitOpticalFlowPyrLKParams,vpiInitOpticalFlowPyrLKParams@VPI_3.0");
VPI_PUBLIC VPIStatus vpiInitOpticalFlowPyrLKParams(VPIOpticalFlowPyrLKParams *params);
#else
VPI_PUBLIC VPIStatus vpiInitOpticalFlowPyrLKParams(uint64_t backends, VPIOpticalFlowPyrLKParams *params);
#endif

/**
 * Creates payload for \ref vpiSubmitOpticalFlowPyrLK
 *
 * @param[in] backends VPI backends that are eligible to execute the algorithm.
 *                     + Valid values: 
 *                       - \ref VPI_BACKEND_CPU
 *                       - \ref VPI_BACKEND_CUDA
 *                       - \ref VPI_BACKEND_PVA
 *                     + Backend must be enabled in current context.
 *
 * @param[in] fmt Specifies the format for the pyramid.
 *                + The accepted image formats are:
 *                  - \ref VPI_IMAGE_FORMAT_U8
 *                  - \ref VPI_IMAGE_FORMAT_U16
 *
 * @param[in] width, height Specifies the dimensions of the pyramid on the finest resolution.
 *                          + Must be >= 0.
 *
 * @param[in] levels Number of levels of the pyramid to be used.
 *
 * @param[in] scale Scale of the pyramid to be used.
 *
 * @param[out] payload Pointer to the payload variable that receives the created handle.
 *
 * @retval #VPI_IMAGE_FORMAT_INVALID    \p fmt is not supported.
 * @retval #VPI_ERROR_INVALID_ARGUMENT  \p payload is NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT  \p width or \p height outside valid range.
 * @retval #VPI_ERROR_INVALID_ARGUMENT  \p fmt is not supported.
 * @retval #VPI_ERROR_INVALID_ARGUMENT  \p backends refers to an invalid backend.
 * @retval #VPI_ERROR_NOT_IMPLEMENTED   Pyramidal LK Optical Flow algorithm is not supported by given backend.
 * @retval #VPI_ERROR_INVALID_CONTEXT   Current context is destroyed.
 * @retval #VPI_ERROR_OUT_OF_MEMORY     Cannot allocate required resources.
 * @retval #VPI_ERROR_INVALID_OPERATION Backend isn't enabled in current context.
 * @retval #VPI_SUCCESS                 Operation executed successfully.
 */
VPI_PUBLIC VPIStatus vpiCreateOpticalFlowPyrLK(uint64_t backends, int32_t width, int32_t height, VPIImageFormat fmt,
                                               int32_t levels, float scale, VPIPayload *payload);

/**
 * Runs Pyramidal LK Optical Flow on two frames.
 * Outputs estimated feature points and tracking status.
 *
 * @param[in] stream The stream where the operation will be queued in.
 *                   + Must not be NULL.
 *                   + Stream must have enabled the backends that will execute the algorithm.
 *
 * @param[in] backend Backend that will execute the algorithm.
 *                    + Must be the backend specified during payload creation or 0 as a shorthand to use this backend.
 *
 * @param[in] payload Payload created with \ref vpiCreateOpticalFlowPyrLK.
 *
 * @param[in] prevPyr Previous frame, represented as a gaussian pyramid.
 *                    + Must not be NULL.
 *                    + Must have same format, scale and number of levels as the ones specified during payload creation.
 *                    + Pyramid must have enabled the backends that will execute the algorithm.
 *
 * @param[in] curPyr  Current frame, represented as a gaussian pyramid.
 *                    + Must not be NULL.
 *                    + Must have same number of levels, size, scale and format as \p prevPyr.
 *                    + Pyramid must have enabled the backends that will execute the algorithm.
 *
 * @param[in] prevPts Array of keypoints in the \p prevPyr high resolution pyramid.
 *                    + Must not be NULL.
 *                    + The type of array must be \ref VPI_ARRAY_TYPE_KEYPOINT_F32.
 *                    + If useInitialFlow in \p params is not zero, \p prevPts must have same size as \p curPts.
 *                    + Array must have enabled the backends that will execute the algorithm.
 *
 * @param[in,out] curPts Array of keypoints in first level of \p curPyr.
 *                       It is used as starting search point in curPyr if useInitialFlow in \p params is not 0.
 *                       + Must not be NULL.
 *                       + The type of array must be \ref VPI_ARRAY_TYPE_KEYPOINT_F32.
 *                       + If useInitialFlow in \p params is not zero, \p curPts must have same size as \p prevPts.
 *                       + Array capacity must be greater or equal than \p prevPts array size.
 *                       + Array must have enabled the backends that will execute the algorithm.
 *
 * @param[in,out] trackingStatus Status array for tracking status for each input in prevPts.
 *                               If element is 0, the corresponding keypoint is being tracked, otherwide, tracking is lost.
 *                               + Must not be NULL.
 *                               + The type of the array must be \ref VPI_ARRAY_TYPE_U8.
 *                               + Array capacity must be greater or equal than \p prevPts array size.
 *                               + Array must have enabled the backends that will execute the algorithm.
 *
 * @param[in] params Parameters for the LK tracker.
 *                   If NULL, it uses default parameters are returned by \ref vpiInitOpticalFlowPyrLKParams.
 * 
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p stream is NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p prevPyr, \p curPyr, \p prevPts, \p curPts or \p trackingStatus are NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p payload is not generated using vpiCreateKLTFeatureTracker.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p prevPyr and \p curPyr should have same number of levels, size, scale and format.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p prevPyr and \p curPyr format should match with format specified in the \p payload creation.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     Invalid termination criteria provided in \p params.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     Number of iterations in \p params outside valid range.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     Tracking window dimension in \p params is invalid.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p prevPts or \p curPts have unsupported array type.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p prevPts or \p curPts's size outside valid range.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p prevPts or \p trackingStatus's capacity outside valid range.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p trackingStatus has unsupported array type.
 * @retval #VPI_ERROR_INVALID_PAYLOAD_TYPE \p payload is invalid.
 * @retval #VPI_ERROR_INVALID_OPERATION    The needed backends aren't enabled in \p stream, \p prevPyr, \p curPyr,
 *                                         \p prevPts, \p curPts or \p trackingStatus.
 * @retval #VPI_SUCCESS                    Operation executed successfully.
 */
VPI_PUBLIC VPIStatus vpiSubmitOpticalFlowPyrLK(VPIStream stream, uint64_t backend, VPIPayload payload,
                                               VPIPyramid prevPyr, VPIPyramid curPyr, VPIArray prevPts, VPIArray curPts,
                                               VPIArray trackingStatus, const VPIOpticalFlowPyrLKParams *params);

#ifdef __cplusplus
}
#endif

/** @} */ // end of VPI_OpticalFlowPyrLK

#endif // NV_VPI_ALGORITHMS_OPTICALFLOWPYRLK_H
