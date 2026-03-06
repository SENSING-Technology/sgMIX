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
 * @file FFT.h
 *
 * Declares functions that implement the Fast Fourier Transform algorithm and its inverse.
 */

#ifndef NV_VPI_ALGORITHMS_FFT_H
#define NV_VPI_ALGORITHMS_FFT_H

#include "../AlgoFlags.h"
#include "../Export.h"
#include "../Status.h"
#include "../Types.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup VPI_FFT Fast Fourier Transform
 * @ingroup VPI_API_Algorithms
 * @{
 *
 * Operations that applies the Fast Fourier Transform and its inverse to 2D images.
 *
 * Refer to \ref algo_fft for more details and usage examples regarding FFT.
 *
 * Refer to \ref algo_ifft for more details and usage examples regarding IFFT.
 *
 * Both FFT and inverse FFT need a payload created during application
 * initialization phase, where image dimensions, input and output formats are
 * defined. The payload then can be used to submit operations on different
 * images, as long as their dimensions and formats match what was defined during
 * payload creation.
 *
 */

/**
 * Creates payload for direct Fast Fourier Transform algorithm.
 * The created payload can then be used with \ref vpiSubmitFFT.
 *
 * @attention For this function to succeed, it is required that the library
 *            libcufft.so.10 is installed in the system.
 *
 * @param[in] backends VPI backend that will execute the algorithm.
 *                     + Valid values:
 *                       - \ref VPI_BACKEND_CPU
 *                       - \ref VPI_BACKEND_CUDA 
 *                     + Backend must be enabled in current context.
 *
 * @param[in] inputWidth, inputHeight Dimensions of the input/output images to be used.
 *                                    + Must be >= 1x1.
 *                                    + With CPU backend, image width must be even.
 *
 * @param[in] inFormat Input image format.
 *                     + Supported formats:
 *                       - \ref VPI_IMAGE_FORMAT_F32, real input.
 *                       - \ref VPI_IMAGE_FORMAT_2F32, complex input.
 *
 * @param[in] outFormat Output image format.
 *                      + Supported format:
 *                        - \ref VPI_IMAGE_FORMAT_2F32, complex output.
 *
 * @param[out] payload Pointer to the payload variable that receives the created handle.
 *
 * @retval #VPI_ERROR_INVALID_ARGUMENT  \p payload is NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT  \p inputWidth or \p inputHeight outside valid range.
 * @retval #VPI_ERROR_INVALID_ARGUMENT  \p backend refers to an invalid backend.
 * @retval #VPI_ERROR_NOT_IMPLEMENTED   FFT algorithm is not supported by given backend.
 * @retval #VPI_ERROR_INVALID_CONTEXT   Current context is destroyed.
 * @retval #VPI_IMAGE_FORMAT_INVALID    \p outFormat is not supported.
 * @retval #VPI_ERROR_OUT_OF_MEMORY     Cannot allocate required resources.
 * @retval #VPI_ERROR_INVALID_OPERATION Backend isn't enabled in current context.
 * @retval #VPI_ERROR_INVALID_OPERATION Library libcufft.so.10 isn't installed in the system.
 * @retval #VPI_SUCCESS                 Operation executed successfully.
 */
VPI_PUBLIC VPIStatus vpiCreateFFT(uint64_t backends, int32_t inputWidth, int32_t inputHeight,
                                  const VPIImageFormat inFormat, const VPIImageFormat outFormat, VPIPayload *payload);

/**
 * Runs the direct Fast Fourier Transform on single image.
 *
 * @param[in] stream The stream where the operation will be queued in.
 *                   + Must not be NULL.
 *                   + Stream must have enabled the backends that will execute the algorithm.
 *
 * @param[in] backend Backend that will execute the algorithm.
 *                    + Must be the backend specified during payload creation or 0 as a shorthand to use this backend.
 *
 * @param[in] payload Payload created with \ref vpiCreateFFT
 *
 * @param[in] input Input image in space domain.
 *                  + Must not be NULL.
 *                  + Must have same format and dimensions as the one specified during payload creation.
 *                  + Image must have enabled the backends that will execute the algorithm.
 *                  + When using the CPU backend, input rows must be aligned to 4 bytes.
 *
 * @param[out] output Image where the result in frequency domain will be written to.
 *                    The top left pixel of the output represents the DC (0 Hz) component.
 *                    + Must not be NULL.
 *                    + Must have same format as the one specified during payload creation.
 *                    + Image must have enabled the backends that will execute the algorithm.
 *                    + When using the CPU backend, output rows must be aligned to 4 bytes.
 *                    + Output dimensions depends on input's based on input's format, as shown below:
 *                      | Input Format               | Output Size      |
 *                      |----------------------------|------------------|
 *                      | \ref VPI_IMAGE_FORMAT_2F32 | W x H            |
 *                      | \ref VPI_IMAGE_FORMAT_F32  | floor(W/2)+1 x H |
 *                    + When input is real, output contains only the left half of the full Hermitian (symmetric-conjugate).
 *
 * @param[in] flags Control flags.
 *                  + Currently it must be 0.
 * 
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p stream is NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p input or \p output are NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p payload is not created using vpiCreateFFT.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p input image dimension and format does not match the ones associated with \p payload.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p output image format does not match the one associated with \p payload.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     Non-supported \p flags.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p output dimensions are non-conformant with input's.
 * @retval #VPI_ERROR_INVALID_PAYLOAD_TYPE \p payload is invalid.
 * @retval #VPI_ERROR_INVALID_OPERATION    The needed backends aren't enabled in \p stream, \p input or \p output.
 * @retval #VPI_SUCCESS                    Operation executed successfully.
 */
VPI_PUBLIC VPIStatus vpiSubmitFFT(VPIStream stream, uint64_t backend, VPIPayload payload, VPIImage input,
                                  VPIImage output, uint64_t flags);

/**
 * Creates payload for inverse Fast Fourier Transform algorithm.
 * The created payload can then be used with \ref vpiSubmitIFFT.
 *
 * @attention For this function to succeed, it is required that the library
 *            libcufft.so.10 is installed in the system.
 *
 * @param[in] backends VPI backend that will execute the algorithm.
 *                     + Valid values:
 *                       - \ref VPI_BACKEND_CPU
 *                       - \ref VPI_BACKEND_CUDA 
 *                     + Backend must be enabled in current context.
 *
 * @param[in] outputWidth, outputHeight Output image dimensions.
 *                                      + Must be >= 1.
 *
 * @param[in] inFormat Input image format.
 *                     + Supported format:
 *                       - \ref VPI_IMAGE_FORMAT_2F32, complex input.
 *
 * @param[in] outFormat Output image format.
 *                      + Must have same format as the one specified during payload creation.
 *                      + With CPU backend, output width must be even.
 *                      + Supported formats:
 *                        - \ref VPI_IMAGE_FORMAT_F32, real output.
 *                        - \ref VPI_IMAGE_FORMAT_2F32, complex output.
 *
 * @param[out] payload Pointer to the payload variable that receives the created handle.
 *
 * @retval #VPI_ERROR_INVALID_ARGUMENT  \p payload is NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT  \p outputWidth or \p outputHeight outside valid range.
 * @retval #VPI_ERROR_INVALID_ARGUMENT  \p backends refers to an invalid backend.
 * @retval #VPI_ERROR_NOT_IMPLEMENTED   IFFT algorithm is not supported by given backend.
 * @retval #VPI_ERROR_INVALID_CONTEXT   Current context is destroyed.
 * @retval #VPI_IMAGE_FORMAT_INVALID    \p outFormat is not supported.
 * @retval #VPI_ERROR_OUT_OF_MEMORY     Cannot allocate required resources.
 * @retval #VPI_ERROR_INVALID_OPERATION Backend isn't enabled in current context.
 * @retval #VPI_ERROR_INVALID_OPERATION Library libcufft.so.10 isn't installed in the system.
 * @retval #VPI_SUCCESS                 Operation executed successfully.
 */
VPI_PUBLIC VPIStatus vpiCreateIFFT(uint64_t backends, int32_t outputWidth, int32_t outputHeight,
                                   const VPIImageFormat inFormat, const VPIImageFormat outFormat, VPIPayload *payload);

/**
 * Runs the inverse Fast Fourier Transform on single image.
 *
 * @param[in] stream A stream handle where the operation will be queued in.
 *                   + Must not be NULL.
 *                   + Stream must have enabled the backends that will execute the algorithm.
 *
 * @param[in] backend Backend that will execute the algorithm.
 *                    + Must be the backend specified during payload creation or 0 as a shorthand to use this backend.
 *
 * @param[in] payload Payload created with \ref vpiCreateIFFT
 *
 * @param[in] input Input image in frequency domain.
 *                  The top left pixel of the input represents the DC (0 Hz) component.
 *                  + Must not be NULL.
 *                  + Must have same format and dimensions as the one specified during payload creation.
 *                  + Image must have enabled the backends that will execute the algorithm.
 *                  + When using the CPU backend, input rows must be aligned to 4 bytes.
 *                  + When output is real, input contains only the left half of the full Hermitian (symmetric-conjugate).
 *                  + Input dimensions depends on output's based on output's format, as shown below:
 *                    | Output Format              | Input Size       |
 *                    |----------------------------|------------------|
 *                    | \ref VPI_IMAGE_FORMAT_2F32 | W x H            |
 *                    | \ref VPI_IMAGE_FORMAT_F32  | floor(W/2)+1 x H |
 *
 * @param[out] output Image where the result in space domain will be written to.
 *                    + Must not be NULL.
 *                    + Must have same format as the one specified during payload creation.
 *                    + Image must have enabled the backends that will execute the algorithm.
 *                    + When using the CPU backend, output rows must be aligned to 4 bytes.
 *
 * @param[in] flags Control flags.
 *                  + Valid values are a combination of one or more of the following flags:
 *                    - 0 for default processing, where output scaled. 
 *                    - \ref VPI_DENORMALIZED_OUTPUT : Output is left denormalized. This leads to
 *                                                     faster execution as normalization isn't usually needed.
 *                                                     Absence of this flag will scale the output, dividing it by the
 *                                                     total pixel count. This makes the output the exact inverse of
 *                                                     direct Fast Fourier Transform.
 * 
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p stream is NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p input or \p output are NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p payload is not created using vpiCreateIFFT.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p input image dimension and format does not match the ones associated with \p payload.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p output image format does not match the one associated with \p payload.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p flags includes a non-supported flag.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p output dimensions are non-conformant with input's.
 * @retval #VPI_ERROR_INVALID_PAYLOAD_TYPE \p payload is invalid.
 * @retval #VPI_ERROR_INVALID_OPERATION    The needed backends aren't enabled in \p stream, \p input or \p output.
 * @retval #VPI_SUCCESS                    Operation executed successfully.
 */
VPI_PUBLIC VPIStatus vpiSubmitIFFT(VPIStream stream, uint64_t backend, VPIPayload payload, VPIImage input,
                                   VPIImage output, uint64_t flags);

/** @} end of VPI_FFT */

#ifdef __cplusplus
}
#endif

#endif /* NV_VPI_ALGORITHMS_FFT_H */
