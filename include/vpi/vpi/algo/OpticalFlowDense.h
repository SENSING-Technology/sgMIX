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
 * @file OpticalFlowDense.h
 *
 * Declares functions that implement the dense optical flow.
 */

#ifndef NV_VPI_ALGORITHMS_OPTICALFLOWDENSE_H
#define NV_VPI_ALGORITHMS_OPTICALFLOWDENSE_H

/**
 * @defgroup VPI_OpticalFlowDense Dense Optical Flow
 * @ingroup VPI_API_Algorithms
 * @{
 *
 * Algorithm used to estimate the motion vectors from previous image to current image.
 * Refer to \ref algo_optflow_dense for more details and usage examples.
 */

#include "../Export.h"
#include "../ImageFormat.h"
#include "../Pyramid.h"
#include "../Status.h"
#include "../Types.h"

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Creates payload for \ref vpiSubmitOpticalFlowDense
 *
 * @param[in] backends VPI backends that are eligible to execute the algorithm.
 *                     + Supported backends:
 *                       - \ref VPI_BACKEND_NVENC
 *                       - \ref VPI_BACKEND_OFA
 *                     + Backend must be enabled in current context.
 *
 * @param[in] width,height Dimensions of current and previous image.
 *                         The output dimensions depend on the \p gridSize
 *                         and the input dimensions.
 *                         + Must be >= 0.
 *
 * @param[in] gridSize Array with grid sizes for each pyramid level.
 *                     If working with input images, not pyramids, \p gridSize
 *                     points to the single grid size used, as if it were for a pyramid with only one level.
 *                     The first value in the array corresponds to the bottom (finer) level, the last
 *                     corresponds to the top (coarse) level.
 *                     Passing 1 allows for a dense grid. With 2 or more, the grid
 *                     will be sparse.
 *                     The output image dimensions depend on the first value in \p gridSize
 *                     and the input dimensions.
 *                     + Valid values:
 *                     | Backend                | Allowed grid sizes per level |
 *                     |------------------------|:----------------------------:|
 *                     | \ref VPI_BACKEND_NVENC |                4             |
 *                     | \ref VPI_BACKEND_OFA   |             1,2,4,8          |
 *
 * @param[in] numLevels Number of pyramid levels to be processed.
 *                      When using pyramids as inputs, this specifies the
 *                      number of levels to be considered. 
 *                      When using images, \p numLevels must be 1.
 *                      + On OFA, the number of levels is limited by the dimensions of the
 *                        smallest pyramid level. It must be >= 32x32.
 *                        Moreover, after dividing this smallest level by \p gridSize, the
 *                        result must be >= 4x4.
 *                      + Valid values:
 *                     | Backend                | minimum | maximum :|
 *                     |------------------------|:-------:|:--------:|
 *                     | \ref VPI_BACKEND_NVENC |    1    |    1     |
 *                     | \ref VPI_BACKEND_OFA   |    1    |    7     |
 *
 * @param[in] inputFmt Format of current and previous image/pyramid.
 *                     + The accepted image formats are:
 *                     | Format                             | NVENC | OFA  |
 *                     |------------------------------------|:-----:|:----:|
 *                     | \ref VPI_IMAGE_FORMAT_NV12_BL      |   *   |  *   |
 *                     | \ref VPI_IMAGE_FORMAT_NV12_BL16    |       |  *   |
 *                     | \ref VPI_IMAGE_FORMAT_NV12_ER_BL   |   *   |  *   |
 *                     | \ref VPI_IMAGE_FORMAT_NV12_ER_BL16 |       |  *   |
 *                     | \ref VPI_IMAGE_FORMAT_U8_BL        |       |  *   |
 *                     | \ref VPI_IMAGE_FORMAT_U8_BL16      |       |  *   |
 *                     | \ref VPI_IMAGE_FORMAT_Y8_BL        |       |  *   |
 *                     | \ref VPI_IMAGE_FORMAT_Y8_BL16      |       |  *   |
 *                     | \ref VPI_IMAGE_FORMAT_Y8_ER_BL     |       |  *   |
 *                     | \ref VPI_IMAGE_FORMAT_Y8_ER_BL16   |       |  *   |
 *                     | \ref VPI_IMAGE_FORMAT_Y16_BL       |       |  *   |
 *                     | \ref VPI_IMAGE_FORMAT_Y16_BL16     |       |  *   |
 *                     | \ref VPI_IMAGE_FORMAT_Y16_ER_BL    |       |  *   |
 *                     | \ref VPI_IMAGE_FORMAT_Y16_ER_BL16  |       |  *   |
 *
 * @param[in] quality Quality of the dense optical flow algorithm.
 *
 * @param[out] payload Pointer to the payload variable that receives the created handle.
 *
 * @retval #VPI_IMAGE_FORMAT_INVALID    \p inputFmt is not supported.
 * @retval #VPI_ERROR_INVALID_ARGUMENT  \p payload is NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT  \p width, \p height, \p numLevels or \p gridSize outside valid range.
 * @retval #VPI_ERROR_INVALID_ARGUMENT  \p backends refers to an invalid backend.
 * @retval #VPI_ERROR_INVALID_OPERATION Backend hardware not available.
 * @retval #VPI_ERROR_INVALID_OPERATION Backend isn't enabled in current context.
 * @retval #VPI_ERROR_NOT_IMPLEMENTED   Dense Optical Flow algorithm is not supported by given backends.
 * @retval #VPI_ERROR_INVALID_CONTEXT   Current context is destroyed.
 * @retval #VPI_ERROR_OUT_OF_MEMORY     Cannot allocate required resources.
 * @retval #VPI_SUCCESS                 Operation executed successfully.
 */
VPI_PUBLIC VPIStatus vpiCreateOpticalFlowDense(uint64_t backends, int32_t width, int32_t height,
                                               VPIImageFormat inputFmt, const int32_t *gridSize, int32_t numLevels,
                                               VPIOpticalFlowQuality quality, VPIPayload *payload);
/**
 * Runs dense Optical Flow on two frames, outputting motion vectors.
 *
 * @param[in] stream The stream where the operation will be queued in.
 *                   + Must not be NULL.
 *                   + Stream must have enabled the backends that will execute the algorithm.
 *
 * @param[in] backend Backend that will execute the algorithm.
 *                    + Must be the backend specified during payload creation or 0 as a shorthand to use this backend.
 *
 * @param[in] payload Payload created by \ref vpiCreateOpticalFlowDense. 
 *                    + The number of pyramid levels specified in the payload must be 1.
 *
 * @param[in] prevImg Previous frame.
 *                    + Must not be NULL.
 *                    + Must have same format and dimensions as the one specified during payload creation.
 *                    + Image must have enabled the backends that will execute the algorithm.
 *
 * @param[in] curImg Current frame.
 *                   + Must not be NULL.
 *                   + Must have same format and dimensions as \p prevImg.
 *                   + Image must have enabled the backends that will execute the algorithm.
 *
 * @param[out] mvImg Motion vectors output.
 *                   + Must not be NULL.
 *                   + Image must have enabled the backends that will execute the algorithm.
 *                   + The width and height depend on the input dimensions and grid size specified on the payload.
 *                     It's calculated by:
 *                     - output_width  = (input_width  + gridSize-1)/gridSize
 *                     - output_height = (input_height + gridSize-1)/gridSize
 *                   + Supported formats:
 *                     - \ref VPI_IMAGE_FORMAT_2S16_BL is supported. Motion vector is in fixed point S10.5 format.
 * 
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p stream is NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p prevImg, \p curImg or \p mvImg are NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p payload is not generated using vpiCreateOpticalFlowDense.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     number of pyramid levels specified in \p payload is not 1.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p prevImg and \p curImg dimensions does not match the one associated with the \p payload. 
 * @retval #VPI_ERROR_INVALID_IMAGE_FORMAT \p prevImg and \p curImg format does not match the one associated with the \p payload. 
 * @retval #VPI_ERROR_INVALID_PAYLOAD_TYPE \p payload is invalid.
 * @retval #VPI_ERROR_INVALID_OPERATION    The needed backends aren't enabled in \p stream, \p prevImg, \p curImg or \p mvImg.
 * @retval #VPI_SUCCESS                    Operation executed successfully.
 */
VPI_PUBLIC VPIStatus vpiSubmitOpticalFlowDense(VPIStream stream, uint64_t backend, VPIPayload payload, VPIImage prevImg,
                                               VPIImage curImg, VPIImage mvImg);

/**
 * Runs dense Optical Flow on two frames, outputting motion vectors.
 *
 * @param[in] stream The stream where the operation will be queued in.
 *                   + Must not be NULL.
 *                   + Stream must have enabled the backends that will execute the algorithm.
 *
 * @param[in] backend Backend that will execute the algorithm.
 *                    + Must be the backend specified during payload creation or 0 as a shorthand to use this backend.
 *
 * @param[in] payload Payload created by \ref vpiCreateOpticalFlowDense. 
 *
 * @param[in] prevPyr Previous pyramid frame.
 *                    + Must not be NULL.
 *                    + Must have same format and dimensions as the one specified during payload creation.
 *                    + Pyramid height must be >= the number of pyramid levels specified in the payload.
 *                    + Pyramid must have enabled the backends that will execute the algorithm.
 *
 * @param[in] curPyr Current pyramid frame.
 *                   + Must not be NULL.
 *                   + Must have same format and dimensions as \p prevImg.
 *                   + Number of pyramid levels must be the same as specified in the payload.
 *                   + Pyramid must have enabled the backends that will execute the algorithm.
 *
 * @param[out] mvImg Motion vectors output.
 *                   + Must not be NULL.
 *                   + Image must have enabled the backends that will execute the algorithm.
 *                   + The width and height depend on the input dimensions and grid size of the bottom pyramid level
 *                     specified in the payload.
 *                     It's calculated by:
 *                     - output_width  = (input_width  + gridSize-1)/gridSize
 *                     - output_height = (input_height + gridSize-1)/gridSize
 *                   + Supported formats:
 *                     - \ref VPI_IMAGE_FORMAT_2S16_BL is supported. Motion vector is in fixed point S10.5 format.
 * 
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p stream is NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p prevPyr, \p cuurPyr or \p mvImg are NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p payload is not generated using vpiCreateOpticalFlowDense.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p prevPyr and \p cuurPyr dimensions does not match the one associated with the \p payload. 
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p prevPyr and \p curPyr number of Levels out of valid range.
 * @retval #VPI_ERROR_INVALID_IMAGE_FORMAT \p prevPyr and \p curPyr format does not match the one associated with the \p payload. 
 * @retval #VPI_ERROR_INVALID_PAYLOAD_TYPE \p payload is invalid.
 * @retval #VPI_ERROR_INVALID_OPERATION    The needed backends aren't enabled in \p stream, \p prevPyr, \p cuurPyr or \p mvImg.
 * @retval #VPI_SUCCESS                    Operation executed successfully.
 */
VPI_PUBLIC VPIStatus vpiSubmitOpticalFlowDensePyramid(VPIStream stream, uint64_t backend, VPIPayload payload,
                                                      VPIPyramid prevPyr, VPIPyramid curPyr, VPIImage mvImg);

/** Dense optical flow parameters for semi-global matching pass.
 * Each pyramid level can be associated with different parameters.
 * The first value in the arrays corresponds to the bottom (finer) level, the last
 * corresponds to the top (coarse) level. */
typedef struct
{
    /** P1 penalty value of semi-global matching algorithm used.
     * Only used by OFA backend and ignored by others. */
    int32_t p1[VPI_MAX_PYRAMID_LEVEL_COUNT];

    /** P2 penalty value of semi-global matching algorithm used.
     * Only used by OFA backend and ignored by others. */
    int32_t p2[VPI_MAX_PYRAMID_LEVEL_COUNT];

    /** Alpha value for adaptative P2 used in semi-global matching.
     * - Valid values are 0 (disable adaptative P2), 1, 2, 4 or 8.
     * Only used by OFA backend and ignored by others. */
    int32_t p2Alpha[VPI_MAX_PYRAMID_LEVEL_COUNT];

    /** Number of semi-global matching passes.
     * - Valid range is from 1 to 3, inclusive.
     * Only used by OFA backend and ignored by others. */
    int32_t numPasses[VPI_MAX_PYRAMID_LEVEL_COUNT];

    /** Enable/disable diagonal directions in semi-global matching. *
     * To disable, set it to 0, any other values will enable it.
     * Only used by OFA backend and ignored by others. */
    int8_t includeDiagonals[VPI_MAX_PYRAMID_LEVEL_COUNT];
} VPIOpticalFlowDenseSGMParams;

/** Retrieves the semi-global matching parameters set up in the Dense Optical Flow payload.
 *
 * @param [in] payload Payload created by \ref vpiCreateOpticalFlowDense.
 *                     + Payload must be using the OFA backend.
 * 
 * @param [out] sgmParams Structure where parameters will be written to.
 *                        + Must not be NULL.
 *
 * @retval #VPI_ERROR_INVALID_PAYLOAD_TYPE \p payload is invalid.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p sgmParams is NULL.
 * @retval #VPI_ERROR_INVALID_OPERATION    \p payload was not created for OFA dense optical flow.
 * @retval #VPI_SUCCESS                    Operation executed successfully.
 */
VPI_PUBLIC VPIStatus vpiOpticalFlowDenseGetSGMParams(VPIPayload payload, VPIOpticalFlowDenseSGMParams *sgmParams);

/** Sets the semi-global matching parameters to be used by the Dense Optical Flow operations with the given payload.
 *
 * The parameters will be only effective on future dense optical flow operations on the given payload.
 * Already submitted operations won't be affected.
 *
 * @param [in] payload Payload created by \ref vpiCreateOpticalFlowDense.
 *                     + Payload must be using the OFA backend.
 * 
 * @param [in] sgmParams SGM parameters to be used by future dense optical flow submissions with given payload.
 *                        + Must not be NULL.
 *
 * @retval #VPI_ERROR_INVALID_PAYLOAD_TYPE \p payload is invalid.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p sgmParams is NULL.
 * @retval #VPI_ERROR_INVALID_OPERATION    \p payload was not created for OFA dense optical flow.
 * @retval #VPI_SUCCESS                    Operation executed successfully.
 */
VPI_PUBLIC VPIStatus vpiOpticalFlowDenseSetSGMParams(VPIPayload payload, const VPIOpticalFlowDenseSGMParams *sgmParams);

#ifdef __cplusplus
}
#endif

/** @} */ // end of VPI_OpticalFlowDense

#endif // NV_VPI_ALGORITHMS_OPTICALFLOWDENSE_H
