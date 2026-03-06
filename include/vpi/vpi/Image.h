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
 * @file Image.h
 *
 * Functions and structures for dealing with VPI images.
 */

#ifndef NV_VPI_IMAGE_H
#define NV_VPI_IMAGE_H

#include "Export.h"
#include "ImageFormat.h"
#include "PixelType.h"
#include "Status.h"
#include "Types.h"

#include <stdint.h>

/**
 * An abstract representation of a 2D image.
 *
 * There are two ways of creating 2D image containers with the API. The most
 * basic one is to use \ref vpiImageCreate to allocate and initialize an empty
 * (zeroed) \ref VPIImage object. The memory for the image data is allocated
 * and managed by VPI. Parameters such as width, height and pixel type are
 * immutable and specified at the construction time. The internal memory layout
 * is also backend-specific. More importantly, efficient exchange of image data
 * between different hardware backends might force the implementation to
 * allocate the memory in multiple memory pools (e.g. dGPU and system DRAM). In
 * some scenarios (to optimize performance and memory use), it might be
 * beneficial to constrain the internal allocation policy to support only a
 * particular set of backends. 
 *
 * To enable interop with existing memory buffers, the user can also
 * create an image object that wraps a user-allocated (and managed) image
 * data using \ref vpiImageCreateWrapper. Similarly to \ref vpiImageCreate,
 * image parameters passed to it are fixed. 
 *
 * The wrapped memory can be redefined by calling \ref vpiImageSetWrapper
 * corresponding to the image wrapper creation function used, as long as the
 * new wrapped memory has the same capacity and type as the one originally
 * wrapped. It's more efficient to create the VPIImage wrapper once and reuse
 * it later then creating and destroying it all the time.
 *
 * The set of \ref vpiImageLockData / \ref vpiImageUnlock allows the user to read
 * from/write to the image data from the host. These functions are non-blocking
 * and oblivious to the stream command queue, so it's up to the user to make
 * sure that all pending operations using this image as input or output are
 * finished. Also, depending on which device the memory is allocated,
 * lock/unlock operation might be time-consuming and, for example, involve
 * copying data over PCIe bus for dGPUs.
 *
 * @defgroup VPI_Image Image
 * @ingroup VPI_API_Core
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/** Represents one image plane in pitch-linear layout. */
typedef struct VPIImagePlanePitchLinearRec
{
    /** Type of each pixel within this plane.
     *  If it is \ref VPI_PIXEL_TYPE_INVALID, it will be inferred from \ref VPIImageBufferPitchLinear::format. */
    VPIPixelType pixelType;

    /** Width of this plane in pixels.
     *  + It must be >= 1. */
    int32_t width;

    /** Height of this plane in pixels.
     *  + It must be >= 1. */
    int32_t height;

    /** Difference in bytes of beginning of one row and the beginning of the previous.
         This is used to address every row (and ultimately every pixel) in the plane.
         @code
            T *pix_addr = (T *)((uint8_t *)data + pitchBytes*height)+width;
         @endcode
         where T is the C type related to pixelType.

         + It must be at least `(width * \ref vpiPixelTypeGetBitsPerPixel(pixelType) + 7)/8`.
    */
    int32_t pitchBytes;

    /** Pointer to the first row of this plane.
         This points to the actual data represented by this plane.
         Depending on how the plane is used, the pointer might be
         addressing a GPU memory or host memory. Care is needed to
         know when it is allowed to dereference this memory. */
    void *data;

} VPIImagePlanePitchLinear;

/** Maximum number of data planes an image can have. */
#define VPI_MAX_PLANE_COUNT (6)

/** Stores the image plane contents. */
typedef struct VPIImageBufferPitchLinearRec
{
    /** Image format. */
    VPIImageFormat format;

    /** Number of planes.
     *  + Must be >= 1. */
    int32_t numPlanes;

    /** Data of all image planes in pitch-linear layout.
     *  + Only the first \ref numPlanes elements must have valid data. */
    VPIImagePlanePitchLinear planes[VPI_MAX_PLANE_COUNT];

} VPIImageBufferPitchLinear;

/** Represents how the image data is stored. */
typedef enum
{
    /** Invalid buffer type.
     *  This is commonly used to inform that no buffer type was selected. */
    VPI_IMAGE_BUFFER_INVALID,

    /** Host-accessible with planes in pitch-linear memory layout. */
    VPI_IMAGE_BUFFER_HOST_PITCH_LINEAR,

    /** CUDA-accessible with planes in pitch-linear memory layout. */
    VPI_IMAGE_BUFFER_CUDA_PITCH_LINEAR,

    /** Buffer stored in a cudaArray_t.
     * Please consult <a href="https://docs.nvidia.com/cuda/cuda-c-programming-guide/index.html#cuda-arrays">cudaArray_t</a>
     * for more information. */
    VPI_IMAGE_BUFFER_CUDA_ARRAY,

    /** EGLImage.
     * Please consult <a href="https://www.khronos.org/registry/EGL/extensions/KHR/EGL_KHR_image_base.txt">EGLImageKHR</a>
     * for more information. */
    VPI_IMAGE_BUFFER_EGLIMAGE,

    /** NvBuffer.
     * Please consult <a href="https://docs.nvidia.com/metropolis/deepstream/sdk-api/group__ds__nvbuf__api.html">NvBuffer</a>
     * for more information.
     * @note Interop with NvBuffer objects created with older
     *       <a href="https://docs.nvidia.com/jetson/l4t-multimedia/nvbuf__utils_8h.html">nvbuf_utils</a> isn't supported anymore.
     */

    VPI_IMAGE_BUFFER_NVBUFFER,

} VPIImageBufferType;

typedef void *EGLImageKHR;
typedef struct cudaArray *cudaArray_t;

/** Represents the available methods to access image contents.
 * The correct method depends on \ref VPIImageData::bufferType. */
typedef union VPIImageBufferRec
{
    /** Image stored in pitch-linear layout.
     * To be used when \ref VPIImageData::bufferType is:
     * - \ref VPI_IMAGE_BUFFER_HOST_PITCH_LINEAR
     * - \ref VPI_IMAGE_BUFFER_CUDA_PITCH_LINEAR
     */
    VPIImageBufferPitchLinear pitch;

    /** Image stored in a `cudaArray_t`.
     * To be used when \ref VPIImageData::bufferType is:
     * - \ref VPI_IMAGE_BUFFER_CUDA_ARRAY
     */
    cudaArray_t cudaarray;

    /** Image stored as an EGLImageKHR.
     * To be used when \ref VPIImageData::bufferType is:
     * - \ref VPI_IMAGE_BUFFER_EGLIMAGE
     */
    EGLImageKHR egl;

    /** Image stored as an NvBuffer file descriptor.
     * To be used when \ref VPIImageData::bufferType is:
     * - \ref VPI_IMAGE_BUFFER_NVBUFFER
     */
    int fd;

} VPIImageBuffer;

/** Stores information about image characteristics and content. */
typedef struct VPIImageDataRec
{
    /** Type of image buffer.
     *  It defines which member of the \ref VPIImageBuffer tagged union that
     *  must be used to access the image contents. */
    VPIImageBufferType bufferType;

    /** Stores the image contents. */
    VPIImageBuffer buffer;

} VPIImageData;

/**
 * Create an empty image instance with the specified flags. Image data is zeroed.
 *
 * The following flags affect the behavior of the allocated image
 *
 * @param[in] width,height Dimensions of the created image.
 *                         + Width and height must be > 0.
 * 
 * @param[in] fmt Format of the created image.
 *                + Must be a valid format.
 * 
 * @param[in] flags Bit field specifying the desired characteristics of the image.
 *                  + The field must be a combination of zero or more of the following flags:
 *                    - \ref VPIBackend flags.
 *                      This image can be used in algorithms running in these backends.
 *                      If no backend flags are given and \ref VPI_REQUIRE_BACKENDS is not set,
 *                      it'll consider all backends supported by the active \ref VPIContext,
 *                      but disable the backends that are incompatible with the given image parameters.
 *                    - \ref VPI_RESTRICT_MEM_USAGE
 *                      For block-linear formats, this flag will avoid an internal pitch-linear buffer alocation.
 *                      The downside is that \ref vpiImageLockData will fail for these formats.
 *                      Pass this flag if system is under memory restrictions, and no image locking is required.
 *                    - \ref common_flags "Common object flags".
 *                  + If flag \ref VPI_REQUIRE_BACKENDS is given, user must pass at least one valid backend that is 
 *                    enabled in current context *AND* is compatible with the given image parameters.
 * 
 * @param[out] img Pointer to memory that will receive the created image handle.
 *
 * @retval #VPI_ERROR_INVALID_ARGUMENT  Output \p img handle is NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT  \p width or \p height outside valid range.
 * @retval #VPI_ERROR_INVALID_ARGUMENT  \p fmt is invalid or not supported.
 * @retval #VPI_ERROR_INVALID_ARGUMENT  \p flags is invalid.
 * @retval #VPI_ERROR_INVALID_ARGUMENT  No backend were given and \ref VPI_REQUIRE_BACKENDS is set.
 * @retval #VPI_ERROR_OUT_OF_MEMORY     Not enough resources to allocate image.
 * @retval #VPI_ERROR_INVALID_CONTEXT   Current context was destroyed.
 * @retval #VPI_ERROR_INVALID_OPERATION Some requested backend isn't enabled in current context.
 * @retval #VPI_ERROR_INVALID_OPERATION Some requested backend can't be created with given image parameters.
 * @retval #VPI_SUCCESS                 Operation executed successfully.
 */
VPI_PUBLIC VPIStatus vpiImageCreate(int32_t width, int32_t height, VPIImageFormat fmt, uint64_t flags, VPIImage *img);

/** Parameters for customizing image wrapping.
 *
 * These parameters are used to customize how image wrapping will be made.
 * Make sure to call \ref vpiInitImageWrapperParams to initialize this
 * structure before updating its attributes. This guarantees that new attributes
 * added in future versions will have a suitable default value assigned.
 */
typedef struct
{
    /** Color spec to override the one defined by the \ref VPIImageData wrapper.
     * If set to \ref VPI_COLOR_SPEC_DEFAULT, infer the color spec from \ref VPIImageData,
     * i.e., no overriding will be done. */
    VPIColorSpec colorSpec;
} VPIImageWrapperParams;

/** Initialize \ref VPIImageWrapperParams with default values.
 *
 * Default values are:
 * - colorSpec: \ref VPI_COLOR_SPEC_DEFAULT, color spec won't be overriden.
 *
 * @param[in] params Structure to be initialized with default values.
 *                   - Mandatory, can't be NULL.
 *
 * @retval #VPI_ERROR_INVALID_ARGUMENT \p params is NULL.
 * @retval #VPI_SUCCESS                Operation executed successfully.
 */
VPI_PUBLIC VPIStatus vpiInitImageWrapperParams(VPIImageWrapperParams *params);

/**
 * Create an image that wraps an axis-aligned rectangular sub-region of an existing image.
 *
 * Create an image instance that is a view of the input image with specific position (x, y), dimensions (width, height) and flags.
 * The resulting image shares the underlying data with the input image, i.e. there is no copy involved in the creation of a view.
 * Similarly to creating image instances that wrap around external buffers, the resulting image view instances do not own the data.
 * It is legal to create a view from another image view instance.
 *
 * @param[in] imgParent Handle to the input parent image.
 *                  + Mandatory, it can't be NULL.
 *                  + On Tegra devices: parent image must be either wrapping CPU or CUDA buffer,
 *                    or have only CPU or CUDA backends enabled.
 *                  + It must not be destroyed while there are image views associated with it.
 *
 * @param[in] clipBounds Rectangle defining the clip bounds of the created image view.
 *                       + The rectangle must be inside input parent image.
 *
 * @param[in] flags Flags with the characteristics of the created image view.
 *                  + They must be combination of zero or more of the following flags:
 *                    - \ref VPIBackend flags.
 *                      This image view will work with streams with these backends enabled.
 *                      The enable backends must have some common backend with the input image flags.
 *                      If no backend flags are passed, it'll consider all backends supported by the active \ref VPIContext.
 *
 * @param[out] imgView Pointer to memory that will receive the created image view handle.
 *                     + Mandatory, it can't be NULL.
 *
 * @retval #VPI_ERROR_INVALID_OPERATION \p imgView cannot be created with the given \p imgParent.
 * @retval #VPI_ERROR_INVALID_OPERATION The backends in \p imgParent and \p flags are not compatible.
 * @retval #VPI_ERROR_INVALID_ARGUMENT  Input parent image \p imgParent is NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT  \p imgParent has no backends.
 * @retval #VPI_ERROR_INVALID_ARGUMENT  \p output handle is NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT  \p clipBounds is NULL or outside the valid range.
 * @retval #VPI_ERROR_INVALID_ARGUMENT  \p flags are invalid.
 * @retval #VPI_ERROR_NOT_IMPLEMENTED   Image view is not implemented for the given input parent image.
 * @retval #VPI_ERROR_INVALID_CONTEXT   \p imgParent and \p imgView contexts are different.
 * @retval #VPI_ERROR_INVALID_CONTEXT   Current context was destroyed.
 * @retval #VPI_SUCCESS                 Operation executed successfully.
 */
VPI_PUBLIC VPIStatus vpiImageCreateView(VPIImage imgParent, const VPIRectangleI *clipBounds, uint64_t flags,
                                        VPIImage *imgView);

/**
 * Redefines the image view position inside a parent image.
 *
 * This operation is efficient and does not allocate memory. The view memory will be
 * accessible to the same backends specified during view creation.  The parent image
 * may be the original parent of the view or a new parent.
 *
 * @param[in] view Handle to image view.
 *                 + It must have been created by \ref vpiImageCreateView.
 *
 * @param[in] parent Handle to a potentially new input parent image.
 *                   + The parent image format must match the view's.
 *                   + Mandatory, it can't be NULL.
 *                   + On Tegra devices: parent image must be either wrapping CPU or CUDA buffer,
 *                     or have only CPU or CUDA backends enabled.
 *                   + It must not be destroyed while there are image views associated with it.
 *
 * @param[in] clipBounds Rectangle defining the clip bounds of the redefined image view.
 *                       + The rectangle must be inside chosen parent image.
 *                       + The width and height of the rectangle must be the same as the image view.
 *
 * @retval #VPI_ERROR_INVALID_ARGUMENT  Input \p view is NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT  Input \p parent is NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT  \p clipBounds is NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT  \p clipBounds is outside valid range.
 * @retval #VPI_ERROR_INVALID_ARGUMENT  \p clipBounds has different size than image view.
 * @retval #VPI_ERROR_INVALID_ARGUMENT  \p parent image format does not match \p view image format.
 * @retval #VPI_ERROR_INVALID_ARGUMENT  \p parent has no backends.
 * @retval #VPI_ERROR_INVALID_OPERATION \p view is not created using \ref vpiImageCreateView.
 * @retval #VPI_ERROR_INVALID_OPERATION \p view cannot be redefined with the given \p parent.
 * @retval #VPI_SUCCESS                 Operation executed successfully.
 */
VPI_PUBLIC VPIStatus vpiImageSetView(VPIImage view, VPIImage parent, const VPIRectangleI *clipBounds);

/**
 * Create an image object by wrapping an existing memory block.
 *  
 * The underlying image object does not own/claim the memory block.
 *
 * @param[in] data Pointer to structure with memory to be wrapped.
 *                     + Mandatory, it can't be NULL.
 *                     + The following buffer types are supported:
 *                       - \ref VPI_IMAGE_BUFFER_HOST_PITCH_LINEAR
 *                       - \ref VPI_IMAGE_BUFFER_CUDA_PITCH_LINEAR
 *                       - \ref VPI_IMAGE_BUFFER_EGLIMAGE (on Tegra platforms only)
 *                       - \ref VPI_IMAGE_BUFFER_NVBUFFER (on Tegra platforms only)
 *
 * @param[in] params If not NULL, use the parameters to modify how \p data is wrapped.
 *                   Otherwise, it'll use defaults given by \ref vpiInitImageWrapperParams.
 * 
 * @param[in] flags Bit field specifying the desired characteristics of the image.
 *                  Depending on some buffer types, the following flags will be added automatically:
 *                  | Buffer type                             | Added flag             |
 *                  |-----------------------------------------|------------------------|
 *                  | \ref VPI_IMAGE_BUFFER_HOST_PITCH_LINEAR | \ref VPI_BACKEND_CPU   | 
 *                  | \ref VPI_IMAGE_BUFFER_CUDA_PITCH_LINEAR | \ref VPI_BACKEND_CUDA  |
 *
 *                  + The field must be a combination of zero or more of the following flags:
 *                    - \ref VPIBackend flags.
 *                      This image can be used in algorithms running in these backends.
 *                      If no backend flags are given and \ref VPI_REQUIRE_BACKENDS is not set,
 *                      it'll consider all backends supported by the active \ref VPIContext,
 *                      but disable the backends that are incompatible with the given image parameters.
 *                    - \ref VPI_RESTRICT_MEM_USAGE
 *                      For block-linear formats, this flag will avoid an internal pitch-linear buffer alocation.
 *                      The downside is that \ref vpiImageLockData will fail for these formats.
 *                      Pass this flag if system is under memory restrictions, and no image locking is required.
 *                    - \ref common_flags "Common object flags".
 *                  + If backends are given and \ref VPI_REQUIRE_BACKENDS is
 *                    set, they all must enabled in current context.
 *                  + If backends are automatically added, they must be enabled
 *                    in current context.
 * 
 * @param[out] img Pointer to memory that will receive the created image handle.
 *
 * @retval #VPI_ERROR_INVALID_ARGUMENT  \p img is NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT  \p data is NULL or contains invalid/unsupported values.
 * @retval #VPI_ERROR_INVALID_ARGUMENT  Buffer type in \p data isn't supported.
 * @retval #VPI_ERROR_INVALID_ARGUMENT  EGLImage handle is NULL or invalid (e.g. EGL_NO_IMAGE).
 * @retval #VPI_ERROR_INVALID_ARGUMENT  NvBuffer file descriptor is invalid.
 * @retval #VPI_ERROR_INVALID_ARGUMENT  No backend were given and \ref VPI_REQUIRE_BACKENDS is set.
 * @retval #VPI_ERROR_OUT_OF_MEMORY     Not enough resources to create image.
 * @retval #VPI_ERROR_INTERNAL          Can't retrieve default EGLDisplay when wrapping EGLImage, or some unspecified internal error.
 * @retval #VPI_ERROR_INVALID_CONTEXT   Current context is destroyed.
 * @retval #VPI_ERROR_INVALID_OPERATION Requested backend isn't enabled in current context.
 * @retval #VPI_ERROR_INVALID_OPERATION A requested backend can't be created with given image parameters.
 * @retval #VPI_SUCCESS                 Operation executed successfully.
 */
VPI_PUBLIC VPIStatus vpiImageCreateWrapper(const VPIImageData *data, const VPIImageWrapperParams *params,
                                           uint64_t flags, VPIImage *img);

/**
 * Redefines the wrapped memory in an existing \ref VPIImage wrapper.
 *
 * This operation is efficient and does not allocate memory. The wrapped memory will be
 * accessible to the same backends specified during wrapper creation.
 *
 * The wrapped memory must not be deallocated while it's still being wrapped.
 * 
 * @param[in] img Handle to image.
 *                + It must have been created by \ref vpiImageCreateWrapper
 *                + Image must not be locked.
 * 
 * @param[in] data Pointer to structure with memory buffer to be wrapped.
 *                     + Mandatory, it can't be NULL.
 *                     + The existing wrapped image and the new one must have same dimensions and format.
 *                     + The old and new buffer types must match.
 *                     + The wrapped memory must point to a buffer that corresponds to the given buffer type.
 *
 * @retval #VPI_ERROR_INVALID_ARGUMENT  \p data is NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT  New and old buffer types don't match.
 * @retval #VPI_ERROR_INVALID_ARGUMENT  \p data dimensions and/or format don't match \p img.
 * @retval #VPI_ERROR_INVALID_ARGUMENT  Wrapped EGLImage is invalid (e.g. EGL_NO_IMAGE).
 * @retval #VPI_ERROR_INTERNAL          Cannot retrieve EGLDisplay.
 * @retval #VPI_ERROR_INVALID_OPERATION \p img is not created using \ref vpiImageCreateWrapper.
 * @retval #VPI_ERROR_INVALID_OPERATION \p img is locked.
 * @retval #VPI_ERROR_INVALID_OPERATION EGL/NvBuffer wrapping not supported on this platform.
 * @retval #VPI_SUCCESS                 Operation executed successfully.
 */
VPI_PUBLIC VPIStatus vpiImageSetWrapper(VPIImage img, const VPIImageData *data);

/**
 * Destroy an image instance.
 *
 * This function deallocates all resources allocated by the image creation function.
 * When destroying an VPIImage wrapper, the wrapped memory itself isn't deallocated.
 *
 * @param[in] img \p img to be destroyed. 
 *                Passing NULL is allowed, to which the function simply does nothing.
 *                + Image must not be in use by any stream, or else undefined behavior might ensue.
 */
VPI_PUBLIC void vpiImageDestroy(VPIImage img);

/**
 * Get the image dimensions in pixels.
 *
 * @param[in] img Image to be queried.
 *                + Mandatory, it can't be NULL.
 * 
 * @param[out] width, height Pointers to buffers where the image width and height (respectively) will be written to.
 *
 * @retval #VPI_ERROR_INVALID_ARGUMENT \p img is NULL or doesn't represent a \ref VPIImage instance.
 * @retval #VPI_ERROR_INVALID_ARGUMENT Output \p width or \p height pointers are NULL.
 * @retval #VPI_SUCCESS                Operation executed successfully.
 */
VPI_PUBLIC VPIStatus vpiImageGetSize(VPIImage img, int32_t *width, int32_t *height);

/**
 * Get the image format.
 *
 * @param[in] img Image to be queried.
 *                + Mandatory, it can't be NULL.
 * 
 * @param[out] format A pointer to where the mage format will be written to.
 *                    + Mandatory, it can't be NULL.
 *
 * @retval #VPI_ERROR_INVALID_ARGUMENT \p img is NULL or doesn't represent a \ref VPIImage instance.
 * @retval #VPI_ERROR_INVALID_ARGUMENT \p format pointer is NULL.
 * @retval #VPI_SUCCESS                Operation executed successfully
 */
VPI_PUBLIC VPIStatus vpiImageGetFormat(VPIImage img, VPIImageFormat *format);

/**
 * Get the image flags.
 *
 * @param[in] img Image to be queried.
 *                + Mandatory, it can't be NULL.
 * 
 * @param[out] flags A pointer to where the flags will be written to.
 *                   + Mandatory, it can't be NULL.
 *
 * @retval #VPI_ERROR_INVALID_ARGUMENT \p flags pointer is NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT \p img is NULL or doesn't represent a \ref VPIImage instance.
 * @retval #VPI_SUCCESS                Operation executed successfully.
 */
VPI_PUBLIC VPIStatus vpiImageGetFlags(VPIImage img, uint64_t *flags);

/**
 * Acquires the lock on an image object.
 *
 * Image locking is required when the image object wraps externally-accessible buffers,
 * and these buffers will be accessed outside VPI. As long as the lock is held, any
 * attempt of VPI to access the image in a mode not compatible with the lock mode will
 * result in asynchronous stream errors, \ref VPI_ERROR_BUFFER_LOCKED.
 *
 * The image can be locked multiple times. Each lock operation increments a
 * counter and must be matched by a corresponding \ref vpiImageUnlock
 * call.
 *
 * @param[in] img Image to be locked.
 *                + Mandatory, it can't be NULL.
 *                + Image must not be locked in a mode that is incompatible with given \p mode.
 * 
 * @param[in] mode Lock mode, depending on whether the memory will be written to and/or read from.
 *                 + Valid values: 
 *                   - \ref VPI_LOCK_READ
 *                   - \ref VPI_LOCK_WRITE
 *                   - \ref VPI_LOCK_READ_WRITE
 *
 * @retval #VPI_ERROR_INVALID_ARGUMENT  \p img is NULL or doesn't represent a \ref VPIImage instance.
 * @retval #VPI_ERROR_BUFFER_LOCKED     Image is already locked by either a stream or the user.
 * @retval #VPI_SUCCESS                 Operation executed successfully.
 */
VPI_PUBLIC VPIStatus vpiImageLock(VPIImage img, VPILockMode mode);

/**
 * Acquires the lock on an image object and returns the image contents.
 *
 * Depending on the internal image representation, as well as the actual location in memory, this
 * function might have a significant performance overhead due to format conversion, layout conversion,
 * device-to-host memory copy, etc.
 *
 * The image can be locked multiple times. Each lock operation increments a
 * counter and must be matched by a corresponding \ref vpiImageUnlock
 * call.
 *
 * @param[in] img Image to be locked.
 *                + Mandatory, it can't be NULL.
 *                + Image must not be locked in a mode that is incompatible with given \p mode.
 *                + For some buffer types, the image must have the following backend enabled:
 *                  | Buffer type                             | Required backend flags |
 *                  |-----------------------------------------|------------------------|
 *                  | \ref VPI_IMAGE_BUFFER_HOST_PITCH_LINEAR | \ref VPI_BACKEND_CPU   |
 *                  | \ref VPI_IMAGE_BUFFER_CUDA_PITCH_LINEAR | \ref VPI_BACKEND_CUDA  |
 * 
 * @param[in] mode Lock mode, depending on whether the memory will be written to and/or read from.
 *                 + Valid values: 
 *                   - \ref VPI_LOCK_READ
 *                   - \ref VPI_LOCK_WRITE
 *                   - \ref VPI_LOCK_READ_WRITE
 *
 * @param[in] bufType The type of buffer returned in \p data.
 *                    It defines how the image contents can be accessed by the caller.
 *                    Valid types are:
 *                    - \ref VPI_IMAGE_BUFFER_HOST_PITCH_LINEAR
 *                    - \ref VPI_IMAGE_BUFFER_CUDA_PITCH_LINEAR
 * 
 * @param[out] data A pointer to a structure that will be filled with image memory contents information.
 *                  + Must not be NULL.
 *                  + The buffers it points to are valid until the image is unlocked.
 *
 * @retval #VPI_ERROR_INVALID_ARGUMENT  \p img is NULL or doesn't represent a \ref VPIImage instance.
 * @retval #VPI_ERROR_INVALID_ARGUMENT  \p data must not be NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT  \p bufType isn't supported.
 * @retval #VPI_ERROR_INVALID_OPERATION \p img doesn't have required backends enabled.
 * @retval #VPI_ERROR_BUFFER_LOCKED     Image is already locked by either a stream or the user.
 * @retval #VPI_SUCCESS                 Operation executed successfully.
 */
VPI_PUBLIC VPIStatus vpiImageLockData(VPIImage img, VPILockMode mode, VPIImageBufferType bufType, VPIImageData *data);

/**
 * Releases the lock on an image object.
 *
 * The image is effectively unlocked when the internal lock counter reaches 0.
 *
 * @param[in] img Image to be unlocked.
 *                + Mandatory, it can't be NULL.
 *                + Image must have CPU backend enabled.
 *
 * @retval #VPI_ERROR_INVALID_ARGUMENT  \p img is NULL or doesn't represent a \ref VPIImage instance.
 * @retval #VPI_ERROR_INVALID_OPERATION \p img isn't locked.
 * @retval #VPI_SUCCESS                 Operation executed successfully.
 */
VPI_PUBLIC VPIStatus vpiImageUnlock(VPIImage img);

#ifdef __cplusplus
}
#endif

/** @} end of VPI_Image */

#endif /* NV_VPI_IMAGE_H */
