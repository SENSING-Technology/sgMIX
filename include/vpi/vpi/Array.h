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
 * @file Array.h
 *
 * Functions and structures for dealing with VPI arrays.
 */

#ifndef NV_VPI_ARRAY_H
#define NV_VPI_ARRAY_H

#include "Export.h"
#include "Status.h"
#include "Types.h"

#include <stdint.h>

/**
 * An abstract representation of a generic 1D array.
 *
 * There are two ways of creating 1D array containers with the API. The most
 * basic one is to use \ref vpiArrayCreate to allocate and initialize an empty
 * (zeroed) \ref VPIArray object. The memory for the array data is allocated
 * and managed by the backend implementation. Parameters such as capacity and
 * type are immutable and specified at the construction time. The internal
 * memory layout is also backend-specific. More importantly, efficient exchange
 * of array data between different hardware blocks might force the
 * implementation to allocate the memory in multiple memory pools (e.g. dGPU
 * and system DRAM). In some scenarios (to optimize performance and memory use)
 * it might be beneficial to constrain the internal allocation policy to
 * support only a particular set of backends.
 *
 * To enable interop with existing host- or gpu-side code, the user can also
 * create an array object that wraps a user-allocated (and managed) array
 * data. Similarly to \ref vpiArrayCreate, array parameters passed to
 * \ref vpiArrayCreateWrapper are fixed.
 *
 * The wrapped memory can be redefined by calling \ref vpiArraySetWrapper
 * as long as the new wrapped memory has the same buffer type, capacity, type
 * as the one originally wrapped. It's more efficient to create the \ref VPIArray
 * wrapper once and reuse it later then creating and destroying it all the
 * time.
 *
 * The set of \ref vpiArrayLockData / \ref vpiArrayUnlock allows to read from/write
 * to the array data from host. These functions are non-blocking and oblivious
 * to the stream command queue so it's up to the user to make sure that all
 * pending operations using this array as input or output are finished. Also,
 * depending on which device the memory is allocated, lock/unlock operation
 * might be time-consuming and, for example, involve copying data over PCIe bus
 * for dGPUs.
 *
 * @defgroup VPI_Array Array
 * @ingroup VPI_API_Core
 * @{
 *
 */

#include "ArrayType.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Stores information about array characteristics and content.
 */

/** Represents array information as an array of structures (AOS). */
typedef struct VPIArrayBufferAOS
{
    /** Type of each array element. */
    VPIArrayType type;

    /** Points to the number of elements in the array.
     *  + Must be >= 0. */
    int32_t *sizePointer;

    /** Maximum number of elements that the array can hold.
     *  + Must be >= 0. */
    int32_t capacity;

    /** Size in bytes of each array element.
     *  + Must be >= 0. 
     *  + Must be at least as large as the element size.
     *  + Must be a multiple of the element size. */
    int32_t strideBytes;

    /** Points to the first element of the array. */
    void *data;
} VPIArrayBufferAOS;

/** Represents how the array data is stored. */
typedef enum
{
    /** Invalid buffer type.
     *  This is commonly used to inform that no buffer type was selected. */
    VPI_ARRAY_BUFFER_INVALID,

    /** Host-accessible array-of-structures. */
    VPI_ARRAY_BUFFER_HOST_AOS,

    /** CUDA-accessible array-of-structures. */
    VPI_ARRAY_BUFFER_CUDA_AOS,

} VPIArrayBufferType;

/** Represents the availablemethods to access array contents.
 * The correct method depends on \ref VPIArrayData::bufferType . */
typedef struct VPIArrayBufferRec
{
    /** Array stored in array-of-structures layout.
     * To be used when \ref VPIArrayData::bufferType is:
     * - \ref VPI_ARRAY_BUFFER_HOST_AOS
     * - \ref VPI_ARRAY_BUFFER_CUDA_AOS
     */
    VPIArrayBufferAOS aos;

} VPIArrayBuffer;

/** Stores information about array characteristics and contents. */
typedef struct VPIArrayDataRec
{
    /** Type of array buffer.
     *  It defines which member of the \ref VPIArrayBuffer tagged union that
     *  must be used to access the array contents. */
    VPIArrayBufferType bufferType;

    /** Stores the array contents. */
    VPIArrayBuffer buffer;

} VPIArrayData;

/**
 * Create an empty array instance.
 *
 * Array data is zeroed. Maximum capacity of the array is fixed and defined at the
 * construction-time. Array size is set to zero. 
 * The VPIArray object owns the allocated memory.
 *
 * @param[in] capacity Array capacity in elements. 
 *                     + Must be >= 0.
 * 
 * @param[in] type Type of each array element.
 *                 + Can't be \ref VPI_ARRAY_TYPE_INVALID.
 * 
 * @param[in] flags Bit field specifying the desired characteristics of the array.
 *                  + The field must be a combination of zero or more of the following flags:
 *                    - \ref VPIBackend flags.
 *                      This array can be used in algorithms running in these backends.
 *                      If no backend flags are passed and \ref VPI_REQUIRE_BACKENDS flag isn't present,
 *                      it'll consider all backends supported by the active context.
 *                    - \ref common_flags "Common object flags".
 *                  + If flag \ref VPI_REQUIRE_BACKENDS is given, user must pass at least one valid backend, and
 *                    they all must be enabled in current context.
 * 
 * @param[out] array Pointer to memory that will receive the created array handle.
 *
 * @retval #VPI_ERROR_INVALID_ARRAY_TYPE Invalid \p type.
 * @retval #VPI_ERROR_INVALID_ARGUMENT   Invalid \p flags.
 * @retval #VPI_ERROR_INVALID_ARGUMENT   \p capacity outside valid range.
 * @retval #VPI_ERROR_INVALID_ARGUMENT   \p array is NULL or doesn't represent a \ref VPIArray instance.
 * @retval #VPI_ERROR_INVALID_ARGUMENT   No backend were given and \ref VPI_REQUIRE_BACKENDS is set.
 * @retval #VPI_ERROR_OUT_OF_MEMORY      Not enough resources to allocate array.
 * @retval #VPI_ERROR_INVALID_CONTEXT    Current context is destroyed.
 * @retval #VPI_ERROR_INVALID_OPERATION  Requested backend isn't enabled in current context.
 * @retval #VPI_SUCCESS                  Operation executed successfully.
 */
VPI_PUBLIC VPIStatus vpiArrayCreate(int32_t capacity, VPIArrayType type, uint64_t flags, VPIArray *array);

/**
 * Create an array object by wrapping an existing host memory block.
 *
 * The returned handle must be destroyed when not being used anymore by calling \ref vpiArrayDestroy.
 *
 * The object doesn't own the wrapped memory. The user is still responsible for wrapped memory lifetime, which
 * must be valid until the array object is destroyed.
 *
 * @param[in] data Pointer to structure with memory buffer to be wrapped.
 *                      + Mandatory, it can't be NULL.
 * 
 * @param[in] flags Bit field specifying the desired characteristics of the array.
 *                  Depending on some buffer types, the following flags will be added automatically:
 *                  | Buffer type                    | Added flag             |
 *                  |--------------------------------|------------------------|
 *                  | \ref VPI_ARRAY_BUFFER_HOST_AOS | \ref VPI_BACKEND_CPU   | 
 *                  | \ref VPI_ARRAY_BUFFER_CUDA_AOS | \ref VPI_BACKEND_CUDA  |
 *
 *                  + The field must be a combination of zero or more of the following flags:
 *                    - \ref VPIBackend flags.
 *                      This array can be used in algorithms running in these backends.
 *                      If no backend flags are passed and \ref VPI_REQUIRE_BACKENDS flag isn't present,
 *                      it'll consider all backends supported by the active context.
 *                    - \ref common_flags "Common object flags".
 *                  + If backends are given and \ref VPI_REQUIRE_BACKENDS is
 *                    set, they all must enabled in current context.
 *                  + If backends are automatically added, they must be enabled
 *                    in current context.
 * 
 * @param[out] array Pointer to memory that will receive the created array handle.
 *
 * @retval #VPI_ERROR_INVALID_ARGUMENT  \p array is NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT  \p data is NULL or contains invalid/unsupported values.
 * @retval #VPI_ERROR_INVALID_ARGUMENT  Buffer type in \p data isn't supported.
 * @retval #VPI_ERROR_INVALID_ARGUMENT  No backend were given and \ref VPI_REQUIRE_BACKENDS is set.
 * @retval #VPI_ERROR_OUT_OF_MEMORY     Not enough resources to create array.
 * @retval #VPI_ERROR_INVALID_CONTEXT   Current context is destroyed.
 * @retval #VPI_ERROR_INVALID_OPERATION Requested backend isn't enabled in current context.
 * @retval #VPI_ERROR_INVALID_OPERATION Automatically added backend flags aren't enabled in current context.
 * @retval #VPI_SUCCESS                 Operation executed successfully.
 */
VPI_PUBLIC VPIStatus vpiArrayCreateWrapper(const VPIArrayData *data, uint64_t flags, VPIArray *array);

/**
 * Redefines the wrapped memory buffer in an existing \ref VPIArray wrapper.
 *
 * This operation is efficient and does not allocate memory. The wrapped memory will be
 * accessible to the same backends specified during wrapper creation.
 *
 * The wrapped memory must not be deallocated while it's still being wrapped.
 *
 * @param[in] array Handle to array.
 *                  + It must have been created by \ref vpiArrayCreateWrapper.
 *                  + Array must not be locked.
 * 
 * @param[in] data Pointer to structure with host memory to be wrapped.
 *                 + Mandatory, it can't be NULL.
 *                 + The existing wrapped array and the new one must have same capacity and element type.
 *                 + The old and new buffer types must match.
 *                 + The wrapped memory must point to a buffer that corresponds to the given buffer type.
 *
 * @retval #VPI_ERROR_INVALID_ARGUMENT  \p data is NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT  \p data has an unsupported buffer type.
 * @retval #VPI_ERROR_INVALID_ARGUMENT  New and old buffer types don't match.
 * @retval #VPI_ERROR_INVALID_OPERATION \p array is not created suing vpiArrayCreateWrapper.
 * @retval #VPI_ERROR_INVALID_OPERATION \p data capacity and/or format don't match \p array.
 * @retval #VPI_ERROR_INVALID_OPERATION \p array is locked.
 * @retval #VPI_SUCCESS                 Operation executed successfully.
 */
VPI_PUBLIC VPIStatus vpiArraySetWrapper(VPIArray array, const VPIArrayData *data);

/**
 * Destroy an array instance.
 *
 * This function deallocates all resources allocated by the array creation function.
 * When destroying an VPIArray wrapper, the wrapped memory itself isn't deallocated.
 *
 * @param[in] array Array handle to be destroyed.
 *                  Passing NULL is allowed, to which the function simply does nothing.
 *                  + Array must not be in use by any stream, or else undefined behavior will ensue.
 */
VPI_PUBLIC void vpiArrayDestroy(VPIArray array);

/**
 * Returns the array size in elements.
 *
 * @param[in] array A valid array handle.
 *                  + Mandatory, it can't be NULL.
 * 
 * @param[out] size A pointer to a variable which will be set to the size of the array.
 *                  + Mandatory, it can't be NULL.
 *
 * @retval #VPI_ERROR_INVALID_ARGUMENT \p array is NULL or doesn't represent a \ref VPIArray instance.
 * @retval #VPI_ERROR_INVALID_ARGUMENT Output \p size pointer is NULL.
 * @retval #VPI_SUCCESS                Operation executed successfully.
 */
VPI_PUBLIC VPIStatus vpiArrayGetSize(VPIArray array, int32_t *size);

/**
 * Set the array size in elements.
 *
 * @param[in] array A valid array handle.
 *                  + Mandatory, it can't be NULL.
 * 
 * @param[in] size The new size of the array. 
 *                 + Must be less than or equal to array's capacity.
 *                 + Must be >= 0.
 *
 * @retval #VPI_ERROR_INVALID_ARGUMENT \p array is NULL or doesn't represent a \ref VPIArray instance.
 * @retval #VPI_ERROR_INVALID_ARGUMENT Input \p size outside valid range.
 * @retval #VPI_SUCCESS                Operation executed successfully.
 */
VPI_PUBLIC VPIStatus vpiArraySetSize(VPIArray array, int32_t size);

/**
 * Returns the array capacity in elements.
 *
 * @param[in] array A valid array handle.
 *                  + Mandatory, it can't be NULL.
 * 
 * @param[out] capacity A pointer to a variable which will be set to the capacity of the array.
 *                      + Mandatory, it can't be NULL.
 *
 * @retval #VPI_ERROR_INVALID_ARGUMENT \p array is NULL or doesn't represent a \ref VPIArray instance.
 * @retval #VPI_ERROR_INVALID_ARGUMENT Output \p capacity pointer is NULL.
 * @retval #VPI_SUCCESS                Operation executed successfully
 */
VPI_PUBLIC VPIStatus vpiArrayGetCapacity(VPIArray array, int32_t *capacity);

/**
 * Returns the array stride (distance between two consecutive elements) in bytes.
 *
 * @param[in] array A valid array handle.
 *                  + Mandatory, it can't be NULL.
 * 
 * @param[out] strideBytes A pointer to a variable which will be set to the stride of the array element, in bytes.
 *                         + Mandatory, it can't be NULL.
 *
 * @retval #VPI_ERROR_INVALID_ARGUMENT \p array is NULL or doesn't represent a \ref VPIArray instance.
 * @retval #VPI_ERROR_INVALID_ARGUMENT Output \p strideBytes pointer is NULL.
 * @retval #VPI_SUCCESS                Operation executed successfully.
 */
VPI_PUBLIC VPIStatus vpiArrayGetStrideBytes(VPIArray array, int32_t *strideBytes);

/**
 * Returns the array flags.
 *
 * @param[in] array A valid array handle.
 *                  + Mandatory, it can't be NULL.
 * 
 * @param[out] flags A pointer where the flags will be written to.
 *                   + Mandatory, it can't be NULL.
 *
 * @retval #VPI_ERROR_INVALID_ARGUMENT \p array is NULL or doesn't represent a \ref VPIArray instance.
 * @retval #VPI_ERROR_INVALID_ARGUMENT Output \p flags pointer is NULL.
 * @retval #VPI_SUCCESS                Operation executed successfully.
 */
VPI_PUBLIC VPIStatus vpiArrayGetFlags(VPIArray array, uint64_t *flags);

/**
 * Returns the array type.
 *
 * @param[in] array A valid array handle.
 *                  + Mandatory, it can't be NULL.
 * 
 * @param[out] type A pointer where the array type will be written to.
 *                  + Mandatory, it can't be NULL.
 *
 * @retval #VPI_ERROR_INVALID_ARGUMENT \p array is NULL or doesn't represent a \ref VPIArray instance.
 * @retval #VPI_ERROR_INVALID_ARGUMENT Output \p type pointer is NULL.
 * @retval #VPI_SUCCESS                Operation executed successfully.
 */
VPI_PUBLIC VPIStatus vpiArrayGetType(VPIArray array, VPIArrayType *type);

/**
 * Acquires the lock on an array object.
 *
 * Array locking is required when the array object wraps externally-accessible buffers,
 * and these buffers will be accessed outside VPI. As long as the lock is held, any
 * attempt of VPI to access the array in a mode not compatible with the lock mode will
 * result in asynchronous stream errors, \ref VPI_ERROR_BUFFER_LOCKED.
 *
 * The array can be locked multiple times. Each lock operation increments a
 * counter and must be matched by a corresponding \ref vpiArrayUnlock
 * call. Lock will fail if the array is being used by a stream.
 *
 * @param[in] array Array to be locked.
 *                  + Mandatory, it can't be NULL.
 *                  + Array must not be locked in a mode that is incompatible with given \p mode.
 * 
 * @param[in] mode Lock mode, depending on whether the memory will be written to and/or read from.
 *                 + Valid values are: 
 *                   - \ref VPI_LOCK_READ
 *                   - \ref VPI_LOCK_WRITE
 *                   - \ref VPI_LOCK_READ_WRITE
 * 
 * @retval #VPI_ERROR_INVALID_ARGUMENT  \p array is NULL or doesn't represent a \ref VPIArray instance.
 * @retval #VPI_ERROR_BUFFER_LOCKED     Array is already locked by either a stream or the user.
 * @retval #VPI_SUCCESS                 Operation executed successfully
 */
VPI_PUBLIC VPIStatus vpiArrayLock(VPIArray array, VPILockMode mode);

/**
 * Acquires the lock on an array object and returns the array contents.
 *
 * Depending on the internal array representation, as well as the actual location in memory, this
 * function might have a significant performance overhead due to type conversion, memory copies, etc.
 *
 * The array can be locked multiple times. Each lock operation increments a
 * counter and must be matched by a corresponding \ref vpiArrayUnlock
 * call. Lock will fail if the array is being used by a stream.
 *
 * @param[in] array Array to be locked.
 *                  + Mandatory, it can't be NULL.
 *                  + Array must not be locked in a mode that is incompatible with given \p mode.
 *                  + For some buffer types, the image must have the following backend enabled:
 *                  | Buffer type                    | Required backend flags |
 *                  |--------------------------------|------------------------|
 *                  | \ref VPI_ARRAY_BUFFER_HOST_AOS | \ref VPI_BACKEND_CPU   | 
 *                  | \ref VPI_ARRAY_BUFFER_CUDA_AOS | \ref VPI_BACKEND_CUDA  |
 * 
 * @param[in] bufType The type of buffer returned in \p data.
 *                    It defines how the array contents can be accessed by the caller.
 *                    Valid types are:
 *                    - \ref VPI_ARRAY_BUFFER_HOST_AOS
 *                    - \ref VPI_ARRAY_BUFFER_CUDA_AOS
 * 
 * @param[in] mode Lock mode, depending on whether the memory will be written to and/or read from.
 *                 + Valid values are: 
 *                   - \ref VPI_LOCK_READ
 *                   - \ref VPI_LOCK_WRITE
 *                   - \ref VPI_LOCK_READ_WRITE
 * 
 * @param[out] data A pointer to a structure that will be filled with array memory information..
 *                  + The buffer it points to are valid until the array is unlocked.
 *
 * @retval #VPI_ERROR_INVALID_ARGUMENT  \p array is NULL or doesn't represent a \ref VPIArray instance.
 * @retval #VPI_ERROR_INVALID_ARGUMENT  \p data is NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT  \p bufType isn't supported.
 * @retval #VPI_ERROR_INVALID_OPERATION \p array doesn't have required backends enabled.
 * @retval #VPI_ERROR_BUFFER_LOCKED     Array is already locked by either a stream or the user.
 * @retval #VPI_SUCCESS                 Operation executed successfully
 */
VPI_PUBLIC VPIStatus vpiArrayLockData(VPIArray array, VPILockMode mode, VPIArrayBufferType bufType, VPIArrayData *data);

/**
 * Releases the lock on array object.
 *
 * This function might have a significant performance overhead (type conversion, layout
 * conversion, host-to-device memory copy).
 *
 * The array is effectively unlocked when the internal lock counter reaches 0.
 *
 * @param[in] array A valid array handle.
 *                  + Mandatory, it can't be NULL.
 *
 * @retval #VPI_ERROR_INVALID_ARGUMENT  \p array is NULL or doesn't represent a \ref VPIArray instance.
 * @retval #VPI_ERROR_INVALID_OPERATION \p array isn't locked.
 * @retval #VPI_SUCCESS                 Operation executed successfully
 */
VPI_PUBLIC VPIStatus vpiArrayUnlock(VPIArray array);

#ifdef __cplusplus
}
#endif

/** @} end of VPI_Array */

#endif /* NV_VPI_ARRAY_H */
