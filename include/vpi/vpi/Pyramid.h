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
 * @file Pyramid.h
 *
 * Functions and structures for dealing with VPI pyramids.
 */

#ifndef NV_VPI_PYRAMID_H
#define NV_VPI_PYRAMID_H

#include "Export.h"
#include "Image.h"
#include "Status.h"
#include "Types.h"
#include "Version.h"

#include <stdint.h>

/**
 * An abstract representation of a 2D image pyramid.
 *
 * 2D image pyramid containers are created by calling \ref vpiPyramidCreate to
 * allocate and initialize an empty (zeroed) \ref VPIPyramid object. The memory
 * for the image pyramid data is allocated and managed by VPI.
 *
 * Image formats match the ones supported by image container. The pyramid is
 * not necessarily dyadic. The scale between levels is defined in the
 * constructor.
 *
 * Parameters such as levels, scale, width, height and image format are
 * immutable and specified at the construction time. The internal memory layout
 * is also backend-specific. More importantly, efficient exchange of image
 * pyramid data between different hardware blocks might force the
 * implementation to allocate the memory in multiple memory pools (e.g. dGPU
 * and system DRAM). In some scenarios (to optimize performance and memory
 * use), it might be beneficial to constrain the internal allocation policy to
 * support only a particular set of backends. 
 *
 * The set of \ref vpiPyramidLockData / \ref vpiPyramidUnlock calls allows the user
 * to read from/write to the image data from the host. These functions are
 * non-blocking and oblivious to the device command queue, so it's up to the
 * user to make sure that all pending operations using this image pyramid as
 * input or output are finished. Also, depending on the enabled backends
 * lock/unlock operation might be time-consuming and, for example, involve
 * copying data over PCIe bus for dGPUs.
 *
 * @defgroup VPI_Pyramid Pyramid
 * @ingroup VPI_API_Core
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/** Maximum number of pyramid levels */
#define VPI_MAX_PYRAMID_LEVEL_COUNT (10)

/** Stores the pyramid contents.
 * Each level is represented by an entire \ref VPIImageData. There are `numLevels` levels, and
 * they can be accessed from `levels[0]` to `levels[numLevels-1]`.
 */
typedef struct VPIPyramidDataRec
{
    /** Number of levels (i.e. height) of the pyramid. */
    int32_t numLevels;

    /** Scale factor of resolution between two adjecent levels. */
    float scale;

    /** Contents of every pyramid level.
     *  Only the first numLevels levels has valid data. */
    VPIImageData levels[VPI_MAX_PYRAMID_LEVEL_COUNT];

} VPIPyramidData;

/**
 * Create an empty image pyramid instance with the specified flags. Pyramid data is zeroed.
 *
 * @param[in] width, height Dimensions of the finest pyramid level.
 *                          + Width and height must be > 0.
 * 
 * @param[in] numLevels Number of levels.
 *                      + Must be >= 1 and <= \ref VPI_MAX_PYRAMID_LEVEL_COUNT.
 * 
 * @param[in] scale Scale factor from one level and the next.
 *                  + Must be > 0 and <= 1.
 * 
 * @param[in] fmt Image format of each level.
 *                + Accepts non-YUV or YUV 4:4:4 formats (no chroma subsampling allowed)
 *                + CPU and CUDA backends only support pitch-linear formats.
 *                + Can't be \ref VPI_IMAGE_FORMAT_INVALID.
 * 
 * @param[in] flags Bit field specifying the desired characteristics of the pyramid.
 *                  + The field must be a combination of zero or more of the following flags:
 *                    - \ref VPIBackend flags.
 *                      This pyramid can be used in algorithms running in these backends.
 *                      If no backend flags are given and \ref VPI_REQUIRE_BACKENDS is not set,
 *                      it'll consider all backends supported by the active \ref VPIContext,
 *                      but disable the backends that are incompatible with the given image parameters.
 *                    - \ref common_flags "Common object flags"
 *                  + If flag \ref VPI_REQUIRE_BACKENDS is given, user must pass at least one valid backend that is 
 *                    enabled in current context *AND* is compatible with the given image parameters.
 * 
 * @param[out] pyr Pointer to memory that will receive the created pyramid handle.
 *
 * @retval #VPI_ERROR_INVALID_ARGUMENT     Output \p pyr is NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p numLevels outside valid range.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p width or \p height outside valid range.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p scale outside valid range.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     No backend were given and \ref VPI_REQUIRE_BACKENDS is set.
 * @retval #VPI_ERROR_INVALID_IMAGE_FORMAT \p fmt is not accepted.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     Invalid \p flags.
 * @retval #VPI_ERROR_OUT_OF_MEMORY        Not enough resources to create image.
 * @retval #VPI_ERROR_INVALID_CONTEXT      Current context it destroyed.
 * @retval #VPI_ERROR_INVALID_OPERATION    Requested backend isn't enabled in current context.
 * @retval #VPI_SUCCESS                    Operation executed successfully.
 */
VPI_PUBLIC VPIStatus vpiPyramidCreate(int32_t width, int32_t height, VPIImageFormat fmt, int32_t numLevels, float scale,
                                      uint64_t flags, VPIPyramid *pyr);

/**
 * Destroy an image pyramid instance as well as all resources it owns.
 *
 * @param[in] pyr Pyramid handle. 
 *                Passing NULL is allowed, to which the function simply does nothing.
 *                + Pyramid must not be in use by any stream, or else undefined behavior will ensue.
 */
VPI_PUBLIC void vpiPyramidDestroy(VPIPyramid pyr);

/**
 * Returns the flags associated with the pyramid.
 *
 * @param[in] pyr Pyramid handle.
 *                + Mandatory, it can't be NULL.
 * 
 * @param[out] flags Pointer to where the flags will be written.
 *                   + Mandatory, it can't be NULL.
 *
 * @retval #VPI_ERROR_INVALID_ARGUMENT Output \p flags pointer is NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT \p pyr is NULL or doesn't represent a \ref VPIPyramid instance.
 * @retval #VPI_SUCCESS                Operation executed successfully.
 */
VPI_PUBLIC VPIStatus vpiPyramidGetFlags(VPIPyramid pyr, uint64_t *flags);

/**
 * Returns the image format of the pyramid levels.
 *
 * @param[in] pyr Pyramid handle.
 *                + Mandatory, it can't be NULL.
 * 
 * @param[out] fmt Pointer to where the image format will be written.
 *                 + Mandatory, it can't be NULL.
 *
 * @retval #VPI_ERROR_INVALID_ARGUMENT Output \p fmt is NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT \p pyr is NULL or doesn't represent a \ref VPIPyramid instance.
 * @retval #VPI_SUCCESS                Operation executed successfully.
 */
VPI_PUBLIC VPIStatus vpiPyramidGetFormat(VPIPyramid pyr, VPIImageFormat *fmt);

/**
 * Get the image pyramid level count.
 *
 * @param[in] pyr Pyramid handle.
 *                + Mandatory, it can't be NULL.
 * 
 * @param[out] numLevels A pointer to a variable which will be set to the number of levels of the image pyramid.
 *                       + Mandatory, it can't be NULL.
 *
 * @retval #VPI_ERROR_INVALID_ARGUMENT Output \p numLevels pointer is NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT \p pyr is NULL or doesn't represent a \ref VPIPyramid instance.
 * @retval #VPI_SUCCESS                Operation executed successfully.
 */
VPI_PUBLIC VPIStatus vpiPyramidGetNumLevels(VPIPyramid pyr, int32_t *numLevels);

/**
 * Get the image width and height in pixels (*for all levels at once*).
 *
 * @param[in] pyr Pyramid handle.
 *                + Mandatory, it can't be NULL.
 * 
 * @param[in] outSize Size of the output arrays, in elements.
 *                    + Must be >= 1.
 * 
 * @param[out] outWidth, outHeight Pointers to an array which will be filled
 *                                 with widths and heights (respectively) of all image pyramid levels.
 *                                 If any of them is NULL, the corresponding data isn't returned.
 *
 * @retval #VPI_ERROR_INVALID_ARGUMENT \p outSize outside valid range.
 * @retval #VPI_ERROR_INVALID_ARGUMENT \p pyr is NULL or doesn't represent a \ref VPIPyramid instance.
 * @retval #VPI_ERROR_INVALID_ARGUMENT \p outWidth and \p outHeight can't be NULL.
 * @retval #VPI_SUCCESS                Operation executed successfully.
 */
VPI_PUBLIC VPIStatus vpiPyramidGetSize(VPIPyramid pyr, int32_t outSize, int32_t *outWidth, int32_t *outHeight);

/**
 * Returns the scale factor of the pyramid levels.
 *
 * @param[in] pyr Pyramid handle.
 *                + Mandatory, it can't be NULL.
 * 
 * @param[out] scale Pointer to where the scale will be written.
 *                + Mandatory, it can't be NULL.
 *
 * @retval #VPI_ERROR_INVALID_ARGUMENT Output \p scale pointer is NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT \p pyr is NULL or doesn't represent a \ref VPIPyramid instance.
 * @retval #VPI_SUCCESS                Operation executed successfully.
 */
VPI_PUBLIC VPIStatus vpiPyramidGetScale(VPIPyramid pyr, float *scale);

/**
 * Acquires the lock on an pyramid object.
 *
 * As long as the lock is held, any attempt of VPI to access the image in a
 * mode not compatible with the lock mode will result in asynchronous stream
 * errors, \ref VPI_ERROR_BUFFER_LOCKED.
 *
 * @param[in] pyr Pyramid handle.
 *                + Mandatory, it can't be NULL.
 *                + Pyramid must not be locked in a mode that is incompatible with given \p mode.
 * 
 * @param[in] lock Lock mode.
 *                 + Valid values are: 
 *                   - \ref VPI_LOCK_READ
 *                   - \ref VPI_LOCK_WRITE
 *                   - \ref VPI_LOCK_READ_WRITE
 *
 * @retval #VPI_ERROR_INVALID_ARGUMENT  \p pyr is NULL or doesn't represent a \ref VPIPyramid instance.
 * @retval #VPI_ERROR_BUFFER_LOCKED     Pyramid is already locked by either a stream or the user.
 * @retval #VPI_ERROR_INVALID_ARGUMENT  \p out is NULL.
 * @retval #VPI_SUCCESS                 Operation executed successfully
 */
VPI_PUBLIC VPIStatus vpiPyramidLock(VPIPyramid pyr, VPILockMode lock);

/**
 * Acquires the lock on a pyramid object and returns host-accessible pointers to each level of the pyramid.
 * Depending on the internal image representation, as well as the actual location in memory, this
 * function might have a significant performance overhead (format conversion, layout conversion,
 * device-to-host memory copy).
 *
 * @param[in] pyr Pyramid handle.
 *                + Mandatory, it can't be NULL.
 *                + Pyramid must not be locked in a mode that is incompatible with given \p mode.
 *                + For some buffer types, the pyramid must have the following backend enabled:
 *                  | Buffer type                             | Required backend flags |
 *                  |-----------------------------------------|------------------------|
 *                  | \ref VPI_IMAGE_BUFFER_HOST_PITCH_LINEAR | \ref VPI_BACKEND_CPU   |
 *                  | \ref VPI_IMAGE_BUFFER_CUDA_PITCH_LINEAR | \ref VPI_BACKEND_CUDA  |
 * 
 * @param[in] lock Lock mode.
 *                 + Valid values are: 
 *                   - \ref VPI_LOCK_READ
 *                   - \ref VPI_LOCK_WRITE
 *                   - \ref VPI_LOCK_READ_WRITE
 *
 * @param[in] bufType The type of buffer returned in \p data.
 *                    It defines how the image contents can be accessed by the caller.
 *                    Valid types are:
 *                    - \ref VPI_IMAGE_BUFFER_HOST_PITCH_LINEAR
 * 
 * @param[out] out A pointer to a structure that will receive the pyramid data to be accessed from host.
 *                 Pass NULL if you're only interested in making sure that the wrapped image is
 *                 updated with the most recent contents from VPI. The image will still be locked.
 *                 + The buffers it points to are valid until the pyramid is unlocked.
 *
 * @retval #VPI_ERROR_INVALID_ARGUMENT  \p pyr is NULL or doesn't represent a \ref VPIPyramid instance.
 * @retval #VPI_ERROR_INVALID_OPERATION \p img doesn't have required backends enabled.
 * @retval #VPI_ERROR_INVALID_ARGUMENT  \p out is NULL.
 * @retval #VPI_ERROR_BUFFER_LOCKED     Pyramid is already locked by either a stream or the user.
 * @retval #VPI_SUCCESS                 Operation executed successfully
 */
VPI_PUBLIC VPIStatus vpiPyramidLockData(VPIPyramid pyr, VPILockMode lock, VPIImageBufferType bufType,
                                        VPIPyramidData *out);

/**
 * Releases the lock on a image pyramid object.
 * This function might have a significant performance overhead (format conversion, layout
 * conversion, host-to-device memory copy).
 *
 * @param[in] pyr Pyramid handle.
 *                + Mandatory, it can't be NULL.
 *
 * @retval #VPI_ERROR_INVALID_ARGUMENT  \p pyr is NULL or doesn't represent a \ref VPIPyramid instance.
 * @retval #VPI_ERROR_INVALID_OPERATION \p pyr doesn't have CPU backend enabled.
 * @retval #VPI_ERROR_INVALID_OPERATION \p pyr isn't locked.
 * @retval #VPI_SUCCESS                 Operation executed successfully.
 */
VPI_PUBLIC VPIStatus vpiPyramidUnlock(VPIPyramid pyr);

/**
 * Creates an image that wraps one pyramid level.
 *
 * The created image doesn't own its contents. Destroying the pyramid while there
 * are images wrapping its levels leads to undefined behavior. If image wraps the
 * base pyramid level, locking the pyramid will also lock the image. Once the image
 * isn't needed anymore, call \ref vpiImageDestroy to free resources.
 *
 * The created image inherits the flags of the wrapped pyramid, including the
 * enabled backends.
 *
 * @param[in] pyr The pyramid whose level will be wrapped.
 *                + Mandatory, it can't be NULL.
 * 
 * @param[in] level Pyramid level to wrap.
 *                  + Must be >= 0.
 * 
 * @param[out] img Pointer to an image handle that will hold the created image.
 *                 + Mandatory, it can't be NULL.
 * 
 * @retval #VPI_ERROR_INVALID_ARGUMENT  Output \p img is NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT  \p pyr is NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT  \p level outside valid range.
 * @retval #VPI_ERROR_INVALID_ARGUMENT  \p pyr is NULL or doesn't represent a \ref VPIPyramid instance.
 * @retval #VPI_ERROR_INVALID_OPERATION Wrapped image is not created in current context.
 * @retval #VPI_ERROR_INVALID_CONTEXT   Current context was destroyed.
 * @retval #VPI_ERROR_OUT_OF_MEMORY     Not enough resources to create image.
 * @retval #VPI_SUCCESS                 Operation executed successfully.
 */
VPI_PUBLIC VPIStatus vpiImageCreateWrapperPyramidLevel(VPIPyramid pyr, int32_t level, VPIImage *img);

#ifdef __cplusplus
}
#endif

/** @} end of VPI_Pyramid */

#endif /* NV_VPI_PYRAMID_H */
