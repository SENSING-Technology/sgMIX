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
 * @file Histogram.h
 *
 * Declares functions that compute image histogram.
 */

#ifndef NV_VPI_ALGORITHMS_HISTOGRAM_H
#define NV_VPI_ALGORITHMS_HISTOGRAM_H

#include "../AlgoFlags.h"
#include "../Export.h"
#include "../Status.h"
#include "../Types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup VPI_Histogram Image Histogram
 * @ingroup VPI_API_Algorithms
 * @{
 *
 * Computes image histogram from the input image.
 * Refer to \ref algo_histogram for more details and usage examples.
 */

/**
 * Creates payload for Image Histogram Even algorithm.
 * It calculates the image histogram assuming uniform (even) bins.
 *
 * @param[in] backends VPI backends that are eligible to execute the algorithm.
 *                     + Valid values: 
 *                       - \ref VPI_BACKEND_CPU
 *                       - \ref VPI_BACKEND_CUDA
 *                     + Backend must be enabled in current context.
 *
 * @param[in] fmt Format of input image.
 *                + The accepted image formats are:
 *                  - \ref VPI_IMAGE_FORMAT_U8
 *                  - \ref VPI_IMAGE_FORMAT_U16
 *
 * @param[in] start Specifies the low end of the histogram range (inclusive).
 *                  + Must be <= \p end.
 *
 * @param[in] end Specifies the high end of the histogram range (exclusive).
 *                + Must be >= \p start.
 *
 * @param[in] numBins Specifies the number of bins in the output array.
 *                    + Maximum number of bins depends on input image format:
 *                      - \ref VPI_IMAGE_FORMAT_U8  : 256
 *                      - \ref VPI_IMAGE_FORMAT_U16 : 65536
 *
 * @param[out] payload Pointer to the payload variable that receives the created handle.
 *
 * @retval #VPI_ERROR_INVALID_ARGUMENT  \p payload is NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT  Maximum allowed \p numBins for given \p fmt outside valid range.
 * @retval #VPI_ERROR_INVALID_ARGUMENT  Invalid \p start and \p end values.
 * @retval #VPI_ERROR_INVALID_ARGUMENT  \p backends refers to an invalid backend.
 * @retval #VPI_ERROR_NOT_IMPLEMENTED   Histogram Even algorithm is not supported by given backends.
 * @retval #VPI_ERROR_INVALID_CONTEXT   Current context was destroyed.
 * @retval #VPI_IMAGE_FORMAT_INVALID    \p fmt is not supported.
 * @retval #VPI_ERROR_OUT_OF_MEMORY     Cannot allocate required resources.
 * @retval #VPI_ERROR_INVALID_OPERATION Backend isn't enabled in current context.
 * @retval #VPI_SUCCESS                 Operation executed successfully.
 */
VPI_PUBLIC VPIStatus vpiCreateHistogramEven(uint64_t backends, VPIImageFormat fmt, float start, float end,
                                            int32_t numBins, VPIPayload *payload);

/**
 * Computes the image histogram.
 *
 * @param[in] stream The stream where the operation will be queued in.
 *                   + Must not be NULL.
 *                   + Stream must have enabled the backends that will execute the algorithm.
 *
 * @param[in] backend Backend that will execute the algorithm.
 *                    + Must be the backend specified during payload creation or 0 as a shorthand to use this backend.
 *
 * @param[in] payload Payload created by \ref vpiCreateHistogramEven.
 *
 * @param[in] input Input image.
 *                  + Must not be NULL.
 *                  + Must have same format as the one specified during payload creation.
 *                  + Image must have enabled the backends that will execute the algorithm.
 *
 * @param[out] output Where the resulting image histogram will be written to.
 *                    + Must not be NULL.
 *                    + Array's capacity must be greater or equal than the number of bins configured in the payload.
 *                    + Array must have enabled the backends that will execute the algorithm.
 *
 * @param[in] flags Control flags.
 *                  + Valid values are a combination of one or more of the following flags:
 *                    - 0: Output will be zeroed out first.
 *                    - \ref VPI_ACCUMULATE_OUTPUT : accumulate on existing output values.
 * 
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p stream is NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p input or \p output are NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p payload not created by vpiCreateHistogramEven.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p output array's capacity outside valid range.
 * @retval #VPI_ERROR_INVALID_ARRAY_TYPE   \p output array type not supported.
 * @retval #VPI_ERROR_INVALID_IMAGE_FORMAT \p input image format does not match with one associated with \p payload.
 * @retval #VPI_ERROR_INVALID_PAYLOAD_TYPE \p payload is invalid.
 * @retval #VPI_ERROR_INVALID_OPERATION    The needed backends aren't enabled in \p stream, \p input or \p output.
 * @retval #VPI_SUCCESS                    Operation executed successfully.
 */
VPI_PUBLIC VPIStatus vpiSubmitHistogram(VPIStream stream, uint64_t backend, VPIPayload payload, VPIImage input,
                                        VPIArray output, uint64_t flags);

#ifdef __cplusplus
}
#endif

/** @} */

#endif /* NV_VPI_ALGORITHMS_HISTOGRAM_H */
