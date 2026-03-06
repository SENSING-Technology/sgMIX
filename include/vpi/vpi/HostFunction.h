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
 * @file HostFunction.h
 *
 * Declares functions that implement the User Function feature.
 */

#ifndef NV_VPI_HOSTFUNCTION_H
#define NV_VPI_HOSTFUNCTION_H

#include "Export.h"
#include "Status.h"
#include "Types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup VPI_HostFunction User-Defined Function
 * @ingroup VPI_API_Algorithms
 * @{
 *
 * Submits a user-defined function to be executed on the stream.
 */

/** Pointer to a host function.
 *
 * @ingroup VPI_HostFunction
 *
 * @param[in] hostData Callback data passed to submission function.
 */
typedef void (*VPIHostFunction)(void *hostData);

/**
 * Pushes a command that will invoke a custom host-side function after all prior commands have been
 * processed.
 *
 * The execution of commands submitted after this call is suspended until the callback
 * is finished. The function will be executed at most once. It won't be executed if a previous stream task
 * failed.
 *
 * This operation will implicitly flush the stream.
 *
 * @param[in] stream Stream that will execute the host function call.
 *                   + Mandatory, cannot be NULL.
 * 
 * @param[in] hostFunc Host function to be executed.
 *                   + Mandatory, cannot be NULL.
 * 
 * @param[in] hostData Pointer that will be passed unchanged to the user function when it's called.
 *
 * @retval #VPI_ERROR_INVALID_ARGUMENT \p hostFunc is NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT \p hostData is NULL.
 * @retval #VPI_SUCCESS                Operation executed successfully.
 */
VPI_PUBLIC VPIStatus vpiSubmitHostFunction(VPIStream stream, VPIHostFunction hostFunc, void *hostData);

/** Pointer to a host function that handles stream status.
 *
 * Most often the function will return the same \p status passed.
 * In some scenarios some other status values can be returned. Specifically, if
 * \p status is \ref VPI_SUCCESS and an error status is returned, the stream
 * will be set to error state. Conversely, if \p status is not \ref VPI_SUCCESS
 * and \ref VPI_SUCCESS is returned, the stream's error state will be reset.
 * Further tasks in it will be executed.
 *
 * @ingroup VPI_HostFunction
 *
 * @param[in] stream Stream that is executing the host function.
 *
 * @param[in] status Status of the last operation executed by the stream.
 *
 * @param[in] hostData Callback data passed to submission function.
 *
 * @returns The new stream status. Mostly often \p status is returned.
 */
typedef VPIStatus (*VPIHostFunctionEx)(VPIStream stream, VPIStatus status, void *hostData);

/**
 * Pushes a command that will invoke a custom host-side function after all prior commands have been
 * processed.
 *
 * The execution of commands submitted after this call is suspended until the
 * callback is finished. The function is guaranteed to be executed exactly
 * once. In case the stream is in error state, the corresponding error status
 * will be passed to hostFunc. 
 *
 * This operation will implicitly flush the stream.
 *
 * @param[in] stream Stream that will execute the host function call.
 *                   + Mandatory, cannot be NULL.
 * 
 * @param[in] hostFunc Host function to be executed.
 *                   + Mandatory, cannot be NULL.
 * 
 * @param[in] hostData Pointer that will be passed unchanged to the user function.
 *
 * @retval #VPI_ERROR_INVALID_ARGUMENT \p stream is NULL or doesn't correspond to a \ref VPIStream instance.
 * @retval #VPI_ERROR_INVALID_ARGUMENT \p hostFunc is NULL. 
 * @retval #VPI_SUCCESS                Operation executed successfully.
 */
VPI_PUBLIC VPIStatus vpiSubmitHostFunctionEx(VPIStream stream, VPIHostFunctionEx hostFunc, void *hostData);

/** @}  end of VPI_HostFunction */

#ifdef __cplusplus
}
#endif

#endif /* NV_VPI_HOSTFUNCTION_H */
