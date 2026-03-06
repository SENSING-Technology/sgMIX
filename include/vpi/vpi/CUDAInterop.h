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
 * @file CUDAInterop.h
 *
 * Functions and structures for handling CUDA interoperability with VPI.
 */

#ifndef NV_VPI_CUDAINTEROP_H
#define NV_VPI_CUDAINTEROP_H

#include "Export.h"
#include "Status.h"
#include "Stream.h"
#include "Types.h"
#include "Version.h"

#include <cuda.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Wraps an existing `cudaStream_t` into a VPI stream.
 *
 * CUDA algorithms are submitted for execution in the wrapped `cudaStream_t`.
 * This allows to insert a VPI-driven processing into an existing CUDA pipeline.
 * Algorithms can still be submitted to other backends.
 *
 * The VPIStream doesn't own the `cudaStream_t`. It must be valid during VPIStream lifetime.
 *
 * CUDA kernels can only be submitted directly to cudaStream_t if it's guaranteed that
 * all tasks submitted to VPIStream are finished.
 *
 * @ingroup VPI_Stream
 *
 * @param[in] cudaStream The CUDA stream handle to be wrapped.
 *                       + Mandatory, it can't be NULL.
 * 
 * @param[in] flags Bit field specifying the desired characteristics of the stream.
 *                  \ref VPI_BACKEND_CUDA is always added internally.
 *                  + The field must be a combination of zero or more of the following flags:
 *                    - \ref VPIBackend flags.
 *                      Allows algorithms to be submitted to these backends.
 *                      If no backend flags are passed, it'll consider all
 *                      backends supported by the active \ref VPIContext.
 *                    - \ref stream_flags "Stream-specific flags".
 *                  + If flag \ref VPI_REQUIRE_BACKENDS is given, user must
 *                    pass at least one valid backend that is enabled in current
 *                    context.
 * 
 * @param[out] stream Pointer that will receive the newly created VPIStream.
 * 
 * @retval #VPI_ERROR_INVALID_ARGUMENT  Output \p stream is NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT  Input \p cudaStream is NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT  No backend were given and \ref VPI_REQUIRE_BACKENDS is set.
 * @retval #VPI_ERROR_OUT_OF_MEMORY     Not enough resources to create array.
 * @retval #VPI_ERROR_INVALID_OPERATION Requested backend isn't enabled in current context.
 * @retval #VPI_ERROR_INVALID_CONTEXT   Current context is destroyed.
 * @retval #VPI_SUCCESS                 Operation executed successfully.
 */
VPI_PUBLIC VPIStatus vpiStreamCreateWrapperCUDA(CUstream cudaStream, uint64_t flags, VPIStream *stream);

#ifdef __cplusplus
}
#endif

#endif /* NV_VPI_CUDAINTEROP_H */
