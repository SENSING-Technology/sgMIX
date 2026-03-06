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
 * @file Context.h
 *
 * Functions and structures for dealing with VPI contexts.
 */

#ifndef NV_VPI_CONTEXT_H
#define NV_VPI_CONTEXT_H

#include "Export.h"
#include "Status.h"
#include "Types.h"
#include "Version.h"

#include <stdint.h>

/**
 * Context is the implicit owner of objects created when it's active and their properties.
 *
 * This is a top-level class that manages the lifetime and properties of all
 * other API objects.
 * Specifically, it has the following properties:
 * - Thread-specific - every thread within an application has one current \ref VPIContext instance set.
 *   If no instance has been explicitly set for a particular thread, all API calls from this thread
 *   will use a process-specific default context instance instead.
 * - Configurable - the user can specify during construction which kinds of backends are enabled
 *   for this context. This effectively allows to mask support for a particular hardware. For
 *   example, creating a \ref VPIStream instance for a CUDA backend will fail
 *   if the current context doesn't have the \ref VPI_BACKEND_CUDA flag set.
 *   When no backends are passed to context creation, all backends supported by
 *   the running platform are enabled in it. For example, when passing '0' as
 *   context flags (no backends) and only CPU and CUDA backends are available,
 *   \ref vpiContextGetFlags will return `\ref VPI_BACKEND_CUDA|\ref VPI_BACKEND_CPU` flags.
 * - Allows for association with a specific CUDA driver API `CUcontext` instance - this should permit
 *   multi-GPU processing.
 * - Sharing buffers between different contexts is not permitted.
 *
 * @defgroup VPI_Context Context
 * @ingroup VPI_API_Core
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @private
 * Forward declaration of CUDA context handle
 */
typedef struct CUctx_st *CUcontext;

/** @anchor context_flags @name Special contexts */
/**@{*/
#define VPI_GLOBAL_CONTEXT ((VPIContext)0x610BA1C1D) /**< Global context identifier */
/**@}*/

/**
 * Represents a context was destroyed or is invalid for some other reason.
 */
#define VPI_INVALID_CONTEXT ((VPIContext)-1)

/**
 * Create a context instance.
 *
 * The context owns the following objects that are created when it is active:
 * - \ref VPIImage
 * - \ref VPIArray
 * - \ref VPIPyramid
 * - \ref VPIStream
 * - \ref VPIEvent
 * - \ref VPIPayload
 *
 * The backends enabled in the context restrict the allowed backends that can be
 * enabled when creating the objects when context is active.
 *
 * The context is created and internal resources are allocated. It isn't
 * assigned to the calling thread, though. For that, use either
 * \ref vpiContextSetCurrent or \ref vpiContextPush.
 *
 * @param[in] flags Bit field specifying the desired characteristics of the context.
 *                  + The field must be a combination of zero or more of the following flags:
 *                    - \ref VPIBackend flags.
 *                      Defines the set of backends that can be enabled during object
 *                      creation when this context is active.
 *                      If no backend flags are given, VPI will enable all
 *                      backends supported by the running platform.
 *                    - \ref context_flags "Context-specific flags".
 *                    - \ref common_flags "Common object flags".
 *                  + If backends are given, their corresponding hardware must be available.
 * 
 * @param[out] ctx Pointer to memory that will receive the new context handle.
 *
 * @retval #VPI_ERROR_INVALID_ARGUMENT  Output \p ctx is NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT  \p flags is invalid.
 * @retval #VPI_ERROR_INVALID_ARGUMENT  No backend were given and \ref VPI_REQUIRE_BACKENDS is set.
 * @retval #VPI_ERROR_INVALID_OPERATION Requested backend isn't available in current system.
 * @retval #VPI_SUCCESS                 Operation executed successfully.
 */
VPI_PUBLIC VPIStatus vpiContextCreate(uint64_t flags, VPIContext *ctx);

/**
 * Create a context instance that wraps a CUDA context.
 *
 * CUDA operations will all under the wrapped context whenever the created
 * context is active.
 *
 * \note Currently VPI doesn't do anything with the CUDA context, so currently the
 *       call performs exactly like \ref vpiContextCreate.
 *
 * Behavior regarding objects created when this context is active is same as
 * \ref vpiContextCreate.
 *
 * @param[in] flags Bit field specifying the desired characteristics of the context.
 *                  + The field must be a combination of zero or more of the following flags:
 *                    - \ref VPIBackend flags.
 *                      Defines the set of backends that can be enabled during object
 *                      creation when this context is active.
 *                      If no backend flags are given, VPI will enable all
 *                      backends supported by the running platform.
 *                    - \ref context_flags "Context-specific flags".
 *                    - \ref common_flags "Common object flags".
 *                  + If backends are given, their corresponding hardware must be available.
 * 
 * @param[in] cudaCtx CUDA context handle to be wrapped.
 *                    + Mandatory, it can't be NULL.
 *                    + It must be a valid CUDA context.
 * 
 * @param[out] ctx Pointer to memory that will receive the created context handle.
 *
 * @retval #VPI_ERROR_INVALID_ARGUMENT  Output \p ctx is NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT  \p flags is invalid.
 * @retval #VPI_ERROR_INVALID_ARGUMENT  \p cudaCtx is NULL or doesn't represent a valid CUDA context.
 * @retval #VPI_ERROR_OUT_OF_MEMORY     Not enough resources to create context.
 * @retval #VPI_ERROR_INVALID_OPERATION Requested backend isn't available in current system.
 * @retval #VPI_SUCCESS                 Operation executed successfully.
 */
VPI_PUBLIC VPIStatus vpiContextCreateWrapperCUDA(uint64_t flags, CUcontext cudaCtx, VPIContext *ctx);

/**
 * Destroy a context instance as well as all resources it owns.
 *
 * The context will be destroyed regardless of how many threads it is active in. It is the
 * responsibility of the calling function to ensure that no API call issues using ctx while
 * \ref vpiContextDestroy is executing.
 *
 * Upon context destruction, all streams associated with it will be synced, then all objects
 * associated with it will be destroyed.
 *
 * If context is current to the calling thread, then it will also be popped from the current thread's
 * context stack as though \ref vpiContextPop was called. If the context is current to other threads, then
 * it'll will remain current to those threads, and attempting to access it from those threads will
 * result in the error \ref VPI_ERROR_INVALID_CONTEXT.
 *
 * @param[in] ctx Context handle.
 *                Passing NULL is allowed, to which the function simply does nothing.
 *                Passing \ref VPI_GLOBAL_CONTEXT also makes it do nothing as it's
 *                impossible to destroy the global context.
 */
VPI_PUBLIC void vpiContextDestroy(VPIContext ctx);

/**
 * Controls low-level task parallelism of CPU devices owned by the context.
 *
 * This function allows the user to overload the parallel_for implementation
 * used by the CPU backend. Changing this parallel_for implementation on a context
 * that is performing CPU processing is undefined and might lead to application
 * crashes.
 *
 * When a context is created, the parallel_for implementation used is the default.
 * It allows for each CPU task to equaly use all cores available.
 *
 * @param[in] ctx Context handle.
 *                + Mandatory, it can't be NULL or invalid.
 * 
 * @param[in] config A pointer to parallel_for configuration.
 *                   Passing NULL will make the context to fallback to its default
 *                   internal implementation.
 *
 * @retval #VPI_ERROR_INVALID_ARGUMENT \p ctx is NULL or invalid.
 * @retval #VPI_SUCCESS                Operation executed successfully .
 */
VPI_PUBLIC VPIStatus vpiContextSetParallelFor(VPIContext ctx, const VPIParallelForConfig *config);

/**
 * Returns parameters set by \ref vpiContextSetParallelFor.
 *
 * @param[in] ctx Context handle.
 *                + Mandatory, it can't be NULL or invalid.
 * 
 * @param[out] config A pointer to parallel_for configuration.
 *                    + Mandatory, it can't be NULL.
 *
 * @retval #VPI_ERROR_INVALID_ARGUMENT Output \p config pointer is NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT \p ctx is NULL or invalid.
 * @retval #VPI_SUCCESS                Operation executed successfully.
 */
VPI_PUBLIC VPIStatus vpiContextGetParallelFor(VPIContext ctx, VPIParallelForConfig *config);

/**
 * Gets the context for the calling thread.
 *
 * @param[out] ctx Pointer to a context handle.
 *                 + Mandatory, it can't be NULL or invalid.
 *
 * @retval #VPI_ERROR_INVALID_ARGUMENT Output \p ctx is NULL.
 * @retval #VPI_ERROR_INVALID_CONTEXT  Current context is destroyed.
 * @retval #VPI_SUCCESS                Operation executed successfully
 */
VPI_PUBLIC VPIStatus vpiContextGetCurrent(VPIContext *ctx);

/**
 * Sets the context for the calling thread.
 *
 * If the context stack isn't empty, this function will replace the top of the stack (current context)
 * of thread.
 *
 * @param[in] ctx Context handle.
 *                + Mandatory, it can't be NULL or invalid;
 *
 * @retval #VPI_ERROR_INVALID_ARGUMENT \p ctx is NULL or invalid value.
 * @retval #VPI_SUCCESS                Operation executed successfully.
 */
VPI_PUBLIC VPIStatus vpiContextSetCurrent(VPIContext ctx);

/**
 * Pushes the context to a per-thread context stack and sets this context as the current context
 * for the calling thread.
 * 
 * + At most 8 contexts can be pushed to the stack.
 *
 * @param[in] ctx Context handle.
 *                + Mandatory, it can't be NULL.
 *                + It must be a valid context.
 *
 * @retval #VPI_ERROR_INVALID_CONTEXT   \p ctx is invalid.
 * @retval #VPI_ERROR_INVALID_ARGUMENT  \p ctx is NULL.
 * @retval #VPI_ERROR_INVALID_OPERATION Stack size outside valid range.
 * @retval #VPI_ERROR_INVALID_CONTEXT   Current context is destroyed.
 * @retval #VPI_SUCCESS                 Operation executed successfully
 */
VPI_PUBLIC VPIStatus vpiContextPush(VPIContext ctx);

/**
 * Pops a context from a per-thread context stack and saves it to the ctx variable.
 *
 * After the pop operation, the context on top of context stack is current (active) for the calling thread.
 *
 * + The context stack cannot be empty.
 * 
 * @param[out] ctx Pointer to a context handle that receives the popped context.
 *                 If NULL, it'll still pop the context, but won't return it.
 *
 * @retval #VPI_ERROR_INVALID_OPERATION Cannot pop from empty stack.
 * @retval #VPI_SUCCESS                 Operation executed successfully.
 */
VPI_PUBLIC VPIStatus vpiContextPop(VPIContext *ctx);

/**
 * Get the current context flags.
 *
 * This function can be used to verify underlying backend support of current context.
 *
 * @param[in] ctx Context handle.
 *                + Mandatory, it can't be NULL or invalid;
 * 
 * @param[out] flags Pointer to a variable which will be set to the current context flags.
 *                   + Mandatory, it can't be NULL.
 *
 * @retval #VPI_ERROR_INVALID_ARGUMENT  Output \p flags is NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT  \p ctx is NULL.
 * @retval #VPI_ERROR_INVALID_CONTEXT   \p ctx is invalid.
 * @retval #VPI_ERROR_INVALID_CONTEXT   Current context is destroyed.
 * @retval #VPI_SUCCESS                 Operation executed successfully.
 */
VPI_PUBLIC VPIStatus vpiContextGetFlags(VPIContext ctx, uint64_t *flags);

#ifdef __cplusplus
}
#endif

/** @} end of VPI_Context */

#endif /* NV_VPI_CONTEXT_H */
