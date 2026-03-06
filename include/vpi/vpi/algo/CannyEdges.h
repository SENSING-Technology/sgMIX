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
 * @file CannyEdges.h
 *
 * Declares functions that implement the canny edge detector algorithm.
 */

#ifndef NV_VPI_ALGORITHMS_CANNY_EDGE_DETECTOR_H
#define NV_VPI_ALGORITHMS_CANNY_EDGE_DETECTOR_H

/**
 * @defgroup VPI_CannyEdgeDetector Canny Edge Detector
 * @ingroup VPI_API_Algorithms
 * @{
 *
 * Runs the canny edge detector algorithm over the input image.
 * Refer to \ref algo_canny_edge_detector for more details and usage examples.
 */

#include "../Export.h"
#include "../Status.h"
#include "../Types.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Creates payload for \ref vpiSubmitCannyEdgeDetector
 *
 * @param[in] backends VPI backends that are eligible to execute the algorithm.
 *                     + Valid values:
 *                       - \ref VPI_BACKEND_CPU
 *                       - \ref VPI_BACKEND_CUDA
 *                     + Backend must be enabled in current context.
 *
 * @param[in] imageWidth, imageHeight Input image dimensions.
 *                                    + Must be >= 1x1.
 *
 * @param[out] payload Pointer to the payload variable that receives the created handle.
 *
 * @retval #VPI_ERROR_INVALID_ARGUMENT  \p payload is NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT  \p imageWidth and \p imageHeight outside valid range.
 * @retval #VPI_ERROR_INVALID_ARGUMENT  \p backends refers to an invalid backend.
 * @retval #VPI_ERROR_INVALID_OPERATION Backend isn't enabled in current context.
 * @retval #VPI_ERROR_NOT_IMPLEMENTED   Canny edge detector algorithm is not supported by given backend.
 * @retval #VPI_ERROR_INVALID_CONTEXT   Current context is destroyed.
 * @retval #VPI_ERROR_OUT_OF_MEMORY     Cannot allocate required resources.
 * @retval #VPI_SUCCESS                 Operation executed successfully.
 */
VPI_PUBLIC VPIStatus vpiCreateCannyEdgeDetector(uint64_t backends, int32_t imageWidth, int32_t imageHeight,
                                                VPIPayload *payload);

/**
 * Structure that defines the parameters for \ref vpiSubmitCannyEdgeDetector
 */
typedef struct
{
    /** Determine the norm to calculate the gradient intensity. */
    VPINormType normType;

    /** Specify the method used to generate gradient from the input image. */
    VPIGradientMethod gradMethod;

    /** Gradient window size.
     *  + Must be 3, 5 or 7. */
    int32_t gradientSize;
} VPICannyEdgeDetectorParams;

/** Initialize \ref vpiInitCannyEdgeDetectorParams with default values.
 *
 * Default values:
 *   - normType: \ref VPI_NORM_L2
 *   - gradMethod: GEN_GRADIENT_SOBEL
 *   - gradientSize: 3
 *
 * @param[out] params Structure to be filled with default values.
 *
 * @returns an error code on failure else \ref VPI_SUCCESS.
 */
VPI_PUBLIC VPIStatus vpiInitCannyEdgeDetectorParams(VPICannyEdgeDetectorParams *params);

/**
 * Runs the canny edge detector algorithm over an image.
 *
 * @param[in] stream A stream handle where the operation will be queued into.
 *                   + Must not be NULL.
 *                   + Stream must have enabled the backends that will execute the algorithm.
 *
 * @param[in] backend Backend that will execute the algorithm.
 *                    + Valid values:
 *                      - \ref VPI_BACKEND_CPU
 *                      - \ref VPI_BACKEND_CUDA
 *
 * @param[in] payload Pointer to the payload variable that receives the created handle.
 *
 * @param[in] input Input image.
 *                  + Must not be NULL.
 *                  + Image must have enabled the backends that will execute the algorithm.
 *                  + The accepted image formats are:
 *                    | Format                           | CPU | CUDA |
 *                    |----------------------------------|:---:|:----:|
 *                    | \ref VPI_IMAGE_FORMAT_U8         |  *  |   *  |
 *                    | \ref VPI_IMAGE_FORMAT_S8         |  *  |   *  |
 *                    | \ref VPI_IMAGE_FORMAT_U16        |  *  |   *  |
 *                    | \ref VPI_IMAGE_FORMAT_S16        |  *  |   *  |
 *                    | \ref VPI_IMAGE_FORMAT_F32        |  *  |   *  |
 *                    | \ref VPI_IMAGE_FORMAT_Y8_ER      |  *  |   *  |
 *                    | \ref VPI_IMAGE_FORMAT_Y8         |  *  |   *  |
 *                    | \ref VPI_IMAGE_FORMAT_Y16_ER     |  *  |   *  |
 *                    | \ref VPI_IMAGE_FORMAT_Y16        |  *  |   *  |
 *
 * @param[out] output Output image where the result is written to.
 *                  + Must not be NULL.
 *                  + It must have same dimensions as input image.
 *                  + Image must have enabled the backends that will execute the algorithm.
 *                  + The accepted formats are the same as \p input
 *
 * @param[in] thresholdStrong Strong threshold for the hysteresis procedure.
 * @param[in] thresholdWeak Weak threshold for the hysteresis procedure.
 * @param[in] edgeValue Value used to mark edge.
 *                  + This value should be different than nonEdgeValue.
 *                  + This value will be clamped if it exceeds the value range of the output image.
 *
 * @param[in] nonEdgeValue Value used to mark non-edge.
 *                  + This value should be different than edgeValue.
 *                  + This value will be clamped if it exceeds the value range of the output image.
 *
 * @param[in] params Control parameters of the canny edge detector algorithm.
 *
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p stream is NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p input or \p output are NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p input or \p output image dimensions ouside valid range.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p backend is invalid or unknown.
 * @retval #VPI_ERROR_INVALID_IMAGE_FORMAT \p input or \p output image format not supported.
 * @retval #VPI_ERROR_INVALID_OPERATION    Backend hardware is not available.
 * @retval #VPI_ERROR_INVALID_OPERATION    The needed backends aren't enabled in \p stream, \p input or \p output.
 * @retval #VPI_ERROR_NOT_IMPLEMENTED      Canny edge detector algorithm is not supported by given backend.
 * @retval #VPI_SUCCESS                    Operation executed successfully.
 */

VPI_PUBLIC VPIStatus vpiSubmitCannyEdgeDetector(VPIStream stream, uint64_t backend, VPIPayload payload, VPIImage input,
                                                VPIImage output, float thresholdStrong, float thresholdWeak,
                                                float edgeValue, float nonEdgeValue,
                                                const VPICannyEdgeDetectorParams *params);

#ifdef __cplusplus
}
#endif

/** @} end of VPI_CannyEdgeDetector */

#endif /* NV_VPI_ALGORITHMS_CANNY_EDGE_DETECTOR_H */
