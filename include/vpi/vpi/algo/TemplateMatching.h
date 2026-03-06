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
 * @file TemplateMatching.h
 *
 * Declares functions that implement the template matching algorithm.
 */

#ifndef NV_VPI_ALGORITHMS_TEMPLATE_MATCHING_H
#define NV_VPI_ALGORITHMS_TEMPLATE_MATCHING_H

/**
 * @defgroup VPI_TemplateMatching Template Matching Algorithm
 * @ingroup VPI_API_Algorithms
 * @{
 *
 * Runs the template matching algorithm with template image over the searched image.
 * Refer to \ref algo_template_matching for more details and usage examples.
 */

#include "../Export.h"
#include "../Status.h"
#include "../Types.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Creates payload for \ref vpiSubmitTemplateMatching
 *
 * @attention For this function to succeed, the below libraries must to be installed in the system:
 *            - libnppc.so.11
 *            - libnppial.so.11
 *            - libnppidei.so.11
 *            - libnppist.so.11
 *
 * @param[in] backends VPI backends that are eligible to execute the algorithm.
 *                     + Valid values:
 *                       - \ref VPI_BACKEND_CPU
 *                       - \ref VPI_BACKEND_CUDA
 *                     + Backend must be enabled in current context.
 *
 * @param[in] imageWidth Source image width.
 * 
 * @param[in] imageHeight Source image height.
 * 
 * @param[out] payload Pointer to the payload variable that receives the created handle.
 *
 * @retval #VPI_ERROR_INVALID_ARGUMENT  \p payload is NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT  \p imageWidth and \p imageHeight outside valid range.
 * @retval #VPI_ERROR_INVALID_ARGUMENT  \p backends refers to an invalid backend.
 * @retval #VPI_ERROR_INVALID_OPERATION Backend isn't enabled in current context.
 * @retval #VPI_ERROR_INVALID_OPERATION Required NPP libraries aren't installed in the system.
 * @retval #VPI_ERROR_NOT_IMPLEMENTED   Algorithm is not supported by given backend.
 * @retval #VPI_ERROR_INVALID_CONTEXT   Current context is destroyed.
 * @retval #VPI_ERROR_OUT_OF_MEMORY     Cannot allocate required resources.
 * @retval #VPI_SUCCESS                 Operation executed successfully.
 */
VPI_PUBLIC VPIStatus vpiCreateTemplateMatching(uint64_t backends, int32_t imageWidth, int32_t imageHeight,
                                               VPIPayload *payload);

/**
 * Set the source image
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
 * @param[in] srcImage Source image where the template image will be searching on.
 *                  + Must not be NULL.
 *                  + Image must have enabled the backends that will execute the algorithm.
 *                  + The accepted image formats are:
 *                    | Format                           | CPU | CUDA |
 *                    |----------------------------------|:---:|:----:|
 *                    | \ref VPI_IMAGE_FORMAT_U8         |  *  |   *  | 
 *                    | \ref VPI_IMAGE_FORMAT_Y8_ER      |  *  |   *  |
 *                    | \ref VPI_IMAGE_FORMAT_Y8         |  *  |   *  |
 * 
 * @retval #VPI_ERROR_INVALID_ARGUMENT  \p payload is NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT  \p srcImage is NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT  \p srcImage resolution does not match with the one specified in the \ref vpiCreateTemplateMatching.
 * @retval #VPI_ERROR_INVALID_OPERATION Backend isn't enabled in current context.
 * @retval #VPI_ERROR_INVALID_OPERATION The needed backends aren't enabled in \p srcImage.
 * @retval #VPI_ERROR_INVALID_CONTEXT   Current context is destroyed.
 * @retval #VPI_ERROR_OUT_OF_MEMORY     Cannot allocate required resources.
 * @retval #VPI_SUCCESS                 Operation executed successfully.
 */
VPI_PUBLIC VPIStatus vpiTemplateMatchingSetSourceImage(VPIStream stream, uint64_t backend, VPIPayload payload,
                                                       VPIImage srcImage);

/**
 * Set the template image
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
 * @param[in] templImage Template image.
 *                  + Must not be NULL.
 *                  + Image must have enabled the backends that will execute the algorithm.
 *                  + The accepted image formats are:
 *                    | Format                           | CPU | CUDA |
 *                    |----------------------------------|:---:|:----:|
 *                    | \ref VPI_IMAGE_FORMAT_U8         |  *  |   *  | 
 *                    | \ref VPI_IMAGE_FORMAT_Y8_ER      |  *  |   *  |
 *                    | \ref VPI_IMAGE_FORMAT_Y8         |  *  |   *  |
 *
 * @param[in] mask Mask used when calculating the template matching score.
 *                  + Could be NULL if not needed.
 *                  + Must have same dimension as the template image
 *                  + The accepted image formats are:
 *                    | Format                           | CPU | CUDA |
 *                    |----------------------------------|:---:|:----:|
 *                    | \ref VPI_IMAGE_FORMAT_U8         |  *  |   *  | 
 * 
 * @retval #VPI_ERROR_INVALID_ARGUMENT  \p payload is NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT  \p templImage is NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT  \p templImage resolution is greater than source image.
 * @retval #VPI_ERROR_INVALID_OPERATION Backend isn't enabled in current context.
 * @retval #VPI_ERROR_INVALID_OPERATION The needed backends aren't enabled in \p templImage.
 * @retval #VPI_ERROR_INVALID_CONTEXT   Current context is destroyed.
 * @retval #VPI_ERROR_OUT_OF_MEMORY     Cannot allocate required resources.
 * @retval #VPI_SUCCESS                 Operation executed successfully.
 */
VPI_PUBLIC VPIStatus vpiTemplateMatchingSetTemplateImage(VPIStream stream, uint64_t backend, VPIPayload payload,
                                                         VPIImage templImage, VPIImage mask);

/**
 * Define method to calculate the template matching score.
 */
typedef enum
{
    /** Normalized cross correlation. */
    VPI_TEMPLATE_MATCHING_NCC
} VPITemplateMatchingMethod;

/**
 * Runs the template matching algorithm with provided template.
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
 * @param[out] output Output image where the template matching score is written to.
 *                  + Must not be NULL.
 *                  + Assume the source image is with resolution WxH and template image is with
 *                    resolution wxh, then the output image shall have resolution (W - w + 1) * (H - h + 1)
 *                  + Image format has to be VPI_IMAGE_FORMAT_F32.
 *                  + Image must have enabled the backends that will execute the algorithm.
 *
 * @param[in] method Specify the method when calculating the template matching score.
 *
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p stream is NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p output is NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p backend is invalid or unknown.
 * @retval #VPI_ERROR_INVALID_IMAGE_FORMAT \p output image format not supported.
 * @retval #VPI_ERROR_INVALID_OPERATION    Backend hardware is not available.
 * @retval #VPI_ERROR_INVALID_OPERATION    The needed backends aren't enabled in \p stream and \p output.
 * @retval #VPI_ERROR_NOT_IMPLEMENTED      Algorithm is not supported by given backend.
 * @retval #VPI_SUCCESS                    Operation executed successfully.
 */

VPI_PUBLIC VPIStatus vpiSubmitTemplateMatching(VPIStream stream, uint64_t backend, VPIPayload payload, VPIImage output,
                                               VPITemplateMatchingMethod method);

#ifdef __cplusplus
}
#endif

/** @} end of VPI_TemplateMatching */

#endif /* NV_VPI_ALGORITHMS_TEMPLATE_MATCHING_H */
