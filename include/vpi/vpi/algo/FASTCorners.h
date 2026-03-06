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
 * @file FASTCorners.h
 *
 * Declares functions that implement support for FAST Corners.
 */

#ifndef NV_VPI_ALGORITHMS_FAST_CORNERS_H
#define NV_VPI_ALGORITHMS_FAST_CORNERS_H

#include "../Export.h"
#include "../Status.h"
#include "../Types.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup VPI_FASTCorners FAST Corners
 * @ingroup VPI_API_Algorithms
 * @{
 *
 * Detect FAST corners in an image, usually used in keypoint tracking.
 * Refer to \ref algo_fast_corners_detector for more details and usage examples.
 */

/**
 * Structure that defines the parameters for \ref vpiSubmitFASTCornerDetector
 */
typedef struct
{
    /** Circle radius around a pixel to check if it is a corner.
     *  The following circle radii are supported, correlating each radius with the
     *  number of pixels in the circle:
     *  | Circle radius | Number of pixels in the circle |
     *  |:-------------:|:------------------------------:|
     *  |       1       |      8                         |
     *  |       2       |     12                         |
     *  |       3       |     16                         |
     *  + Must be 1, 2 or 3. */
    int32_t circleRadius;

    /** Arc length in pixels over the circle to check central pixel is a corner.
     *  Arc length is the minimum number of contiguous pixels in the circle that are either
     *  brighter (have higher intensity values) or darker (have lower intensity values) than
     *  the central pixel, plus or minus a threshold, respectively, to mark it as a corner.
     *  + For VPI_BACKEND_CPU, it must be in the valid range from 1 to the number of pixels in the circle.
     *  + For VPI_BACKEND_CUDA, it must be either 5, 7 or 9 according to the circle radius as:
     *  | Circle radius | Arc length |
     *  |:-------------:|:----------:|
     *  |       1       |      5     |
     *  |       2       |      7     |
     *  |       3       |      9     |
     */
    int32_t arcLength;

    /** Threshold to select a pixel as being part of the arc in circle around a keypoint candidate.
     *  Intensity threshold is used to check if a pixel in the circle is either brighter
     *  (has higher intensity value) or darker (has lower intensity value) than the central pixel.
     *  This check is used in the arc calculation to determine if a pixel is in the arc or not. */
    float intensityThreshold;

    /** Whether to apply non-maximum suppression to remove corners too close together.
     *  If not '0', it activates non-maximum suppression (NMS) for the FAST corner detector algorithm.
     *  The NMS for FAST suppresses pixels in a 1-ring neighborhood by increasing the intensity
     *  threshold value and removing corner pixels that fail the arc length calculation,
     *  keeping the single pixel in the neighborhood that survives.
     *  If '0', it deactivates the non-maximum suppression. */
    int8_t nonMaxSuppression;
} VPIFASTCornerDetectorParams;

/** Initializes \ref VPIFASTCornerDetectorParams with default values.
 *
 * Default values are:
 * - circleRadius:        3
 * - arcLength:           9
 * - intensityThreshold: 10
 * - nonMaxSuppression:   1
 * 
 * @retval #VPI_ERROR_INVALID_ARGUMENT \p params is NULL.
 * @retval #VPI_SUCCESS Operation executed successfully.
 */
VPI_PUBLIC VPIStatus vpiInitFASTCornerDetectorParams(VPIFASTCornerDetectorParams *params);

/**
 * Submits a \ref algo_fast_corners_detector "FAST Corner Detector" operation to the stream.
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
 * @param[in] input Input image from where the FAST corners will be extracted.
 *                  + Must not be NULL.
 *                  + Image must have enabled the backends that will execute the algorithm.
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
 *                        + Must not be NULL.
 *                        + It must have type \ref VPI_ARRAY_TYPE_KEYPOINT_F32.
 *                        + Array must have enabled the backends that will execute the algorithm.
 *
 * @param[in] params Pointer to a \ref VPIFASTCornerDetectorParams.
 *                   It defines the parameters for this algorithm invocation.
 *                   These parameters can vary in every call and will be copied internally.
 *                   Thus there is no need to keep the parameters object around.
 *                   - If NULL, use the defaults given by \ref vpiInitFASTCornerDetectorParams.
 *
 * @param[in] border How to handle pixels outside image boundaries.
 *                   + The accepted border extensions are:
 *                     - \ref VPI_BORDER_ZERO
 *                     - \ref VPI_BORDER_CLAMP
 *                     - \ref VPI_BORDER_REFLECT
 *                     - \ref VPI_BORDER_MIRROR
 *                     - \ref VPI_BORDER_LIMITED (ignore pixels with circle going outside image boundaries)
 *  
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p stream is NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p input or \p outCorners are NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     Circle radius in \p params outside valid range.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     Arc length in \p params outside valid range.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     Intensity threshold in \p params outside valid range.
 * @retval #VPI_ERROR_INVALID_ARRAY_TYPE   Invalid \p outCorners array type.
 * @retval #VPI_ERROR_INVALID_IMAGE_FORMAT Unsupported input format.
 * @retval #VPI_ERROR_INVALID_OPERATION    The needed backends aren't enabled in \p stream, \p input or \p outCorners.
 * @retval #VPI_SUCCESS                    Operation executed successfully.
 */
VPI_PUBLIC VPIStatus vpiSubmitFASTCornerDetector(VPIStream stream, uint64_t backend, VPIImage input,
                                                 VPIArray outCorners, const VPIFASTCornerDetectorParams *params,
                                                 VPIBorderExtension border);

/** @} end of VPI_FASTCorners */

#ifdef __cplusplus
}
#endif

#endif /* NV_VPI_ALGORITHMS_FAST_CORNERS_H */
