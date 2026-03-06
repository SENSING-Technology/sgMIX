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
 * @file ColorNames.h
 *
 * Declares functions that implement the Color Names algorithm.
 */

#ifndef NV_VPI_ALGORITHMS_COLORNAMES_H
#define NV_VPI_ALGORITHMS_COLORNAMES_H

#include "../Export.h"
#include "../Status.h"
#include "../Types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup VPI_Colornames Color Names Features Extractor
 * @ingroup VPI_API_Algorithms
 * @{
 *
 * Extracts the Color Names features of the input image.
 */

/**
 * Creates the payload for the Color Names algorithm.
 * @param[in] backends VPI backends that are eligible to execute the algorithm.
 *                     + Valid values:
 *                       - \ref VPI_BACKEND_CPU
 *                       - \ref VPI_BACKEND_CUDA
 *                       - \ref VPI_BACKEND_PVA
 *
 * @param[in] outFormat Format of the output images.
 *                      16-bit types will return more precise values, in exchange of some minor performance hit. 
 *                      8-bit types will be less precise, but typically lead to faster execution time. *                      
 *                      + Valid Image formats:
 *                        | Formats                      | CPU | CUDA | PVA |
 *                        |------------------------------|:---:|:----:|:---:|
 *                        | \ref VPI_IMAGE_FORMAT_U8     |  *  |   *  |  *  |
 *                        | \ref VPI_IMAGE_FORMAT_S8     |  *  |   *  |     |
 *                        | \ref VPI_IMAGE_FORMAT_U16    |  *  |   *  |     |
 *                        | \ref VPI_IMAGE_FORMAT_S16    |  *  |   *  |     |
 *                    
 *
 * @param[out] payload Pointer to a handle that will receive the created payload.
 * 
 * @retval #VPI_IMAGE_FORMAT_INVALID   \p outType is not supported.
 * @retval #VPI_ERROR_INVALID_ARGUMENT \p payload is NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT \p backends refers to an invalid backend.
 * @retval #VPI_ERROR_NOT_IMPLEMENTED  ColorNames is not supported by given backend.
 * @retval #VPI_ERROR_INVALID_CONTEXT  Current context is destroyed.
 * @retval #VPI_ERROR_OUT_OF_MEMORY    Cannot allocate required resources.
 * @retval #VPI_SUCCESS                Operation executed successfully.
 */
VPI_PUBLIC VPIStatus vpiCreateExtractColorNameFeatures(uint64_t backends, VPIImageFormat outFormat,
                                                       VPIPayload *payload);

/** Submits the Color Names algorithm to the stream.
 * 
 * @param[in] stream The stream where the operation will be queued in.
 *                   + Must not be NULL.
 *                   + Stream must have enabled the backends that will execute the algorithm.
 *
 * @param[in] backend Backend that will execute the algorithm.
 *                    Must be the backend specified during payload creation or 0 as a shorthand to use this backend.
 *
 * @param[in] payload Payload to be submitted along the other parameters.
 *
 * @param[in] input Input image.
 *                  + Must not be NULL.
 *                  + Image must have enabled the backends that will execute the algorithm.
 *                  + Supported formats:
 *                        |         Formats                 | CPU | CUDA | PVA |
 *                        |---------------------------------|:---:|:----:|:---:|
 *                        | \ref VPI_IMAGE_FORMAT_RGB8      |  *  |   *  |     |
 *                        | \ref VPI_IMAGE_FORMAT_RGBA8     |  *  |   *  |     |
 *                        | \ref VPI_IMAGE_FORMAT_NV12_ER   |  *  |   *  |     |
 *                        | \ref VPI_IMAGE_FORMAT_RGB8p     |     |      |  *  |
 *                        
 *
 * @param[out] output Pointer to an array of \ref VPIImage where the features will be written to.
 *                    Returned features have \p numOutputs dimensions,
 *                    each dimension will be written to each image in sequence.
 *                    + Must not be NULL.
 *                    + The images must have the same format as specified during payload creation.
 *                    + All images must have same format.
 *                    + CPU and CUDA: All images must have same dimensions as input image.
 *                    + PVA: Dimensions of all images must be input's width/4 and height/4.
 *                    + All Image must have enabled the backends that will execute the algorithm.
 *
 * @param[in] numOutputs Number of images in output array.
 *                       It specifies the number of dimensions each feature has.
 *                       + It must be equal to 10.
 *
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p stream is NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p input or \p output are NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p numOutputs outside valid range.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p payload is not generated using vpiCreateExtractColorNameFeatures.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     Number of \p output images outside valid range.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     All \p output images must have same format.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     All \p output images must have same dimensions as input.
 * @retval #VPI_ERROR_INVALID_PAYLOAD_TYPE \p payload is invalid.
 * @retval #VPI_ERROR_INVALID_OPERATION    The needed backends aren't enabled in \p stream, \p input or \p output.
 * @retval #VPI_SUCCESS                    Operation executed successfully.
 */
VPI_PUBLIC VPIStatus vpiSubmitExtractColorNameFeatures(VPIStream stream, uint64_t backend, VPIPayload payload,
                                                       VPIImage input, VPIImage *output, int32_t numOutputs);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* NV_VPI_ALGORITHMS_COLORNAMES_H */
