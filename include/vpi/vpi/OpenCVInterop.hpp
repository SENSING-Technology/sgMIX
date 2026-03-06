/*
 * Copyright 2020 NVIDIA Corporation. All rights reserved.
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
 * @file OpenCVInterop.hpp
 *
 * Functions for handling OpenCV interoperability with VPI.
 */

#ifndef NV_VPI_OPENCV_INTEROP_HPP
#define NV_VPI_OPENCV_INTEROP_HPP

#include "detail/OpenCVUtils.hpp"

#include "Image.h"

#include <opencv2/core/core.hpp>

#include <vector>

/**
 * Interoperability with OpenCV library.
 *
 * VPI allows easy interoperability with OpenCV. This makes it easy to wrap
 * existing `cv::Mat` object into \ref VPIImage to be used in VPI pipelines.
 *
 * It's also possible to create a `cv::Mat` out of a \ref VPIImageData. 
 *
 * @defgroup VPI_OpenCVInterop OpenCV Interoperability
 * @{
 */

/**
 * Wraps a `cv::Mat` in an \ref VPIImage with the given image format.
 *
 * It also wraps YUV images in 4:2:2 (packed and semi-planar) and 4:2:0 (semi-planar) formats
 * that are understood by `cv::cvtColor`.
 *
 * The `cv::Mat` must not be destroyed while there's a \ref VPIImage still
 * wrapping it. Failure to do so leads to undefined behavior.
 *
 * @param[in] mat The OpenCV matrix to be wrapped.
 *                + Must not be empty.
 *                + Number of dimensions must be 2.
 *
 * @param[in] fmt The image format associated with \p mat.
 *                + Format must be consistent with \p mat dimensions and type.
 *
 * @param[in] flags Bit field specifying the desired characteristics of the image.
 *                  \ref VPI_BACKEND_CPU flag is always added internally.
 *                  + The field must be a combination of zero or more of the following flags:
 *                    - \ref VPIBackend flags.
 *                      This image can be used in algorithms running in these backends.
 *                      If no backend flags are passed, it'll consider all
 *                      backends supported by the active \ref VPIContext, but
 *                      disable the backends that are incompatible with the given
 *                      image parameters.
 *                    - \ref common_flags "Common object flags".
 *                  + If backends are given, they must enabled in current context.
 *
 * @param[out] img Pointer to memory that will receive the created image handle.
 *
 * @retval #VPI_ERROR_INVALID_IMAGE_FORMAT \p fmt is invalid or inconsistent with \p mat.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p mat is empty or has wrong number of dimensions.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     Output \p img is NULL.
 * @retval #VPI_ERROR_OUT_OF_MEMORY        Not enough resources to create image.
 * @retval #VPI_ERROR_INVALID_CONTEXT      Current context is destroyed.
 * @retval #VPI_ERROR_INVALID_OPERATION    Requested backend isn't enabled in current context.
 * @retval #VPI_SUCCESS                    Operation executed successfully.
 */
inline VPIStatus vpiImageCreateWrapperOpenCVMat(const cv::Mat &mat, VPIImageFormat fmt, uint64_t flags, VPIImage *img)
{
    VPIImageData imgData = {};

    VPIStatus status = ::nv::vpi::detail::FillImageData(mat, fmt, &imgData);
    if (status != VPI_SUCCESS)
    {
        return status;
    }

    return vpiImageCreateWrapper(&imgData, nullptr, flags, img);
}

/**
 * Wraps a `cv::Mat` in a \ref VPIImage and infers the image format from its type.
 *
 * The rules to infer the image type are:
 * - unsigned and float 3-channel images are taken to be BGR
 * - unsigned and float 4-channel images are taken to be BGRA
 * - other types are mapped to non-color vpi image formats with the same pixel layout.
 *
 * The `cv::Mat` must not be destroyed while there's a \ref VPIImage still
 * wrapping it. Failure to do so leads to undefined behavior.
 *
 * @param[in] mat The OpenCV matrix to be wrapped.
 *                + Number of dimensions must be 2.
 *
 * @param[in] flags Bit field specifying the desired characteristics of the image.
 *                  \ref VPI_BACKEND_CPU flag is always added internally.
 *                  + The field must be a combination of zero or more of the following flags:
 *                    - \ref VPIBackend flags.
 *                      This image can be used in algorithms running in these backends.
 *                      If no backend flags are passed, it'll consider all
 *                      backends supported by the active \ref VPIContext, but
 *                      disable the backends that are incompatible with the given
 *                      image parameters.
 *                    - \ref common_flags "Common object flags"
 *                  + If backends are given, they must enabled in current context.
 *
 * @param[out] img Pointer to memory that will receive the created image handle.
 *
 * @retval #VPI_ERROR_INVALID_IMAGE_FORMAT \p mat type is not supported.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p mat is empty or has wrong number of dimensions.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     Output \p img is NULL.
 * @retval #VPI_ERROR_OUT_OF_MEMORY        Not enough resources to create image.
 * @retval #VPI_ERROR_INVALID_CONTEXT      Current context is destroyed.
 * @retval #VPI_ERROR_INVALID_OPERATION    Requested backend isn't enabled in current context.
 * @retval #VPI_SUCCESS                    Operation executed successfully.
 */
inline VPIStatus vpiImageCreateWrapperOpenCVMat(const cv::Mat &mat, uint64_t flags, VPIImage *img)
{
    VPIImageData imgData = {};
    VPIStatus status     = ::nv::vpi::detail::FillImageData(mat, &imgData);
    if (status != VPI_SUCCESS)
    {
        return status;
    }

    return vpiImageCreateWrapper(&imgData, nullptr, flags, img);
}

/**
 * Redefines the wrapped `cv::Mat` of an existing \ref VPIImage wrapper.
 *
 * The new `cv::Mat` must have the same characteristics (type and dimensions) of the existing
 * `cv::Mat` being wrapped.
 *
 * This operation is efficient and does not allocate memory. The wrapped memory will be
 * accessible to the same backends specified during wrapper creation.
 *
 * The wrapped `cv::Mat` must be valid while it's still being wrapped.
 *
 * @param[in] img Handle to an image.
 *                + Must have been created by \ref vpiImageCreateWrapperOpenCVMat or
 *                  \ref vpiImageCreateWrapper and buffer type \ref VPI_IMAGE_BUFFER_HOST_PITCH_LINEAR.
 *                + Image must not be locked.
 *
 * @param[in] mat `cv::Mat` object to be wrapped.
 *                + Number of dimensions must be 2.
 *                + The existing wrapped image and \p mat must have same dimensions and compatible formats.
 *
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p mat is empty or has wrong number of dimensions.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     Output \p img is NULL.
 * @retval #VPI_ERROR_INVALID_IMAGE_FORMAT \p mat type is not supported.
 * @retval #VPI_ERROR_INVALID_OPERATION    \p mat dimensions and/or format don't match \p img.
 * @retval #VPI_ERROR_INVALID_OPERATION    \p img is locked.
 * @retval #VPI_ERROR_INVALID_OPERATION    \p img is not created by \ref vpiImageCreateWrapperOpenCVMat
 *                                         or \ref vpiImageCreateWrapper with correct buffer type.
 * @retval #VPI_SUCCESS                    Operation executed successfully.
 */
inline VPIStatus vpiImageSetWrappedOpenCVMat(VPIImage img, const cv::Mat &mat)
{
    VPIImageFormat fmt;
    VPIStatus status = vpiImageGetFormat(img, &fmt);
    if (status != VPI_SUCCESS)
    {
        return status;
    }

    VPIImageData imgData = {};
    status               = ::nv::vpi::detail::FillImageData(mat, fmt, &imgData);
    if (status != VPI_SUCCESS)
    {
        return status;
    }

    return vpiImageSetWrapper(img, &imgData);
}

/**
 * Wraps a vector of `cv::Mat`s in a \ref VPIImage, each `cv::Mat` representing one image plane.
 *
 * The wrapped `cv::Mat`s must not be destroyed while there's a \ref VPIImage still
 * wrapping it. The `std::vector` object itself isn't used so it can be destroyed, or even be temporary.
 * Failure to keep `cv::Mat`s allocated leads to undefined behavior.
 *
 * @param[in] matPlanes Vector of `cv::Mat`, each one representing one image plane.
 *                      + Vector can't be empty.
 *                      + `cv::Mat` in each plane must not be empty.
 *                      + `cv::Mat` dimensions must be 2.
 *
 * @param[in] fmt The image format associated with \p matPlanes.
 *                + Format must be consistent with \p matPlanes. Namely, the number of planes in \p fmt
 *                  must match \p matPlanes size, and each plane must be consistent with corresponding
 *                  `cv::Mat` type.
 *
 * @param[in] flags Bit field specifying the desired characteristics of the image.
 *                  \ref VPI_BACKEND_CPU flag is always added internally.
 *                  + The field must be a combination of zero or more of the following flags:
 *                    - \ref VPIBackend flags.
 *                      This image can be used in algorithms running in these backends.
 *                      If no backend flags are passed, it'll consider all
 *                      backends supported by the active \ref VPIContext, but
 *                      disable the backends that are incompatible with the given
 *                      image parameters.
 *                    - \ref common_flags "Common object flags"
 *                  + If backends are given, they must enabled in current context.
 * 
 * @param[out] img Pointer to memory that will receive the created image handle.
 *
 * @retval #VPI_ERROR_INVALID_IMAGE_FORMAT \p fmt is not supported.
 * @retval #VPI_ERROR_INVALID_IMAGE_FORMAT \p matPlanes type not consistent with \p fmt.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     Some `cv::Mat` in \p matPlanes is empty, or matPlanes is empty.
 * @retval #VPI_ERROR_INVALID_OPERATION    Requested backend isn't enabled in current context.
 * @retval #VPI_SUCCESS                    Operation executed successfully.
 */
inline VPIStatus vpiImageCreateWrapperOpenCVMatPlanes(const std::vector<cv::Mat> &matPlanes, VPIImageFormat fmt,
                                                      uint64_t flags, VPIImage *img)
{
    VPIImageData imgData = {};
    VPIStatus status     = ::nv::vpi::detail::FillImageData(matPlanes, fmt, &imgData);
    if (status != VPI_SUCCESS)
    {
        return status;
    }

    return vpiImageCreateWrapper(&imgData, nullptr, flags, img);
}

/**
 * Replaces the wrapped `cv::Mat` planes with new ones.
 *
 * The new `cv::Mat` vector must have the same size, and contained `cv::Mat`s
 * must have the same characteristics (type and dimensions) of the existing
 * `cv::Mat`s being wrapped.
 *
 * The \ref VPIImage must have been created by \ref vpiImageCreateWrapperOpenCVMatPlanes.
 *
 * This operation is efficient and does not allocate memory. The wrapped memory will be
 * accessible to the same backends specified during wrapper creation.
 *
 * The wrapped `cv::Mat`s must be valid while it's still being wrapped. The `std::vector` object
 * itself can be destroyed, or even be temporary.
 *
 * @param[in] img Handle to an image.
 *                + Must have been created by \ref vpiImageCreateWrapperOpenCVMat or
 *                  \ref vpiImageCreateWrapper with buffer type \ref VPI_IMAGE_BUFFER_HOST_PITCH_LINEAR.
 *
 * @param[in] matPlanes Vector of `cv::Mat`, each one representing one image plane.
 *
 * @retval #VPI_ERROR_INVALID_IMAGE_FORMAT Image format is not supported.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     Input \p matPlanes is NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     Input \p imgData is NULL.
 * @retval #VPI_ERROR_INVALID_OPERATION    \p img is not created by \ref vpiImageCreateWrapperOpenCVMat
 *                                         or \ref vpiImageCreateWrapper with correct buffer type.
 * @retval #VPI_SUCCESS                    Operation executed successfully.
 */
inline VPIStatus vpiImageSetWrappedOpenCVMatPlanes(VPIImage img, const std::vector<cv::Mat> &matPlanes)
{
    VPIImageFormat fmt;
    VPIStatus status = vpiImageGetFormat(img, &fmt);
    if (status != VPI_SUCCESS)
    {
        return status;
    }

    VPIImageData imgData = {};
    status               = ::nv::vpi::detail::FillImageData(matPlanes, fmt, &imgData);
    if (status != VPI_SUCCESS)
    {
        return status;
    }

    return vpiImageSetWrapper(img, &imgData);
}

/**
 * Fills an existing `cv::Mat` with data from \ref VPIImageData coming from a locked \ref VPIImage.
 *
 * No image contents will be copied, they're just referenced by `cv::Mat`. The result can be used
 * in OpenCV operations.
 *
 * If imgData has 2 or 3 planes that are contiguous in memory and represent some multi-planar
 * formats as understood by `cv::cvtColor` such as NV12 or YUV422, the resulting mat will be filled
 * so that `cv::cvtColor` understand it as having these types. In other works, they can be used
 * with `cv::COLOR_YUV2RGB_NV12` or `COLOR_YUV2RGB_UYVY`.
 *
 * @param[in] imgData Reference to \ref VPIImageData returned by \ref vpiImageLockData.
 *                    If the pixel type of a plane is \ref VPI_PIXEL_TYPE_DEFAULT, it'll be inferred from
 *                    the image format given.
 *                    + Image format must be representable by a `cv::Mat`.
 *
 * @param[out] mat `cv::Mat` that will refer to imgData contents.
 *                 + Mandatory, can't be NULL.
 *
 * @retval #VPI_ERROR_INVALID_ARGUMENT Output \p mat is NULL.
 * @retval #VPI_ERROR_INVALID_IMAGE_FORMAT \p imgData is invalid.
 * @retval #VPI_SUCCESS Operation executed successfully.
 */
inline VPIStatus vpiImageDataExportOpenCVMat(const VPIImageData &imgData, cv::Mat *mat)
{
    if (mat == NULL)
    {
        return VPI_ERROR_INVALID_ARGUMENT;
    }

    return ::nv::vpi::detail::ExportOpenCVMat(imgData, *mat);
}

/**
 * Fills a vector of `cv::Mat` with all planes from \ref VPIImageData.
 *
 * No image contents will be copied, they're just referenced by cv::Mat. The result can be used
 * in OpenCV operations.
 *
 * @param[in] imgData Reference to \ref VPIImageData returned by \ref vpiImageLockData.
 *                    If the pixel type of a plane is \ref VPI_PIXEL_TYPE_DEFAULT, it'll be inferred from
 *                    the image format given.
 *                    + Image format must be representable by a list of `cv::Mat`s, one per plane.
 *                    + The buffer type must be \ref VPI_IMAGE_BUFFER_HOST_PITCH_LINEAR .
 *
 * @param[out] matPlanes Vector of `cv::Mat`, each one wrapping one image plane in imgData.
 *                       + Mandatory, can't be NULL.
 *
 * @retval #VPI_ERROR_INVALID_ARGUMENT Output \p matPlanes is NULL.
 * @retval #VPI_ERROR_INVALID_IMAGE_FORMAT \p imgData is invalid.
 * @retval #VPI_SUCCESS Operation executed successfully.
 */
inline VPIStatus vpiImageDataExportOpenCVMatPlanes(const VPIImageData &imgData, std::vector<cv::Mat> *matPlanes)
{
    if (matPlanes == NULL)
    {
        return VPI_ERROR_INVALID_ARGUMENT;
    }

    return ::nv::vpi::detail::ExportOpenCVMatPlanes(imgData, *matPlanes);
}

/**
 * Fills an existing \ref VPIImageData with data from given `cv::Mat`.
 *
 * It performs the inverse operation of \ref vpiImageDataExportOpenCVMat. Please
 * refer to this function's description on what is the criteria used for image
 * format deduction.
 *
 * @param[in] mat Reference to `cv::Mat` with information to be filled in output
 *                + It must not be empty.
 *                + `cv::Mat` number of dimensions equal to 2.
 *
 * @param[out] imgData \ref VPIImageData that will be filled.
 *                     Existing contents are discarded.
 *                     + Mandatory, must not be NULL.
 *
 * @retval #VPI_ERROR_INVALID_ARGUMENT Input \p mat is empty.
 * @retval #VPI_ERROR_INVALID_ARGUMENT \p mat has wrong number of dimensions.
 * @retval #VPI_ERROR_INVALID_ARGUMENT \p imgData is NULL.
 * @retval #VPI_SUCCESS Operation executed successfully.
 */
inline VPIStatus vpiImageDataImportOpenCVMat(const cv::Mat &mat, VPIImageData *imgData)
{
    if (imgData == NULL)
    {
        return VPI_ERROR_INVALID_ARGUMENT;
    }

    return ::nv::vpi::detail::FillImageData(mat, imgData);
}

/**
 * Fills an existing \ref VPIImageData with data from given `cv::Mat`,
 * forcing the given image format.
 *
 * It performs the inverse operation of \ref vpiImageDataExportOpenCVMat. Please
 * refer to this function's description on what is the criteria used for image
 * format deduction.
 *
 * @param[in] mat Reference to `cv::Mat` with information to be filled in output
 *                + It must not be empty.
 *                + `cv::Mat` number of dimensions equal to 2.
 *
 * @param[in] fmt The image format associated with mat.
 *                + It must be consistent with \p mat dimesions and type.
 *
 * @param[out] imgData \ref VPIImageData that will be filled.
 *                     Existing contents are discarded.
 *                     + Mandatory, must not be NULL.
 *
 * @retval #VPI_ERROR_INVALID_ARGUMENT Input \p mat is empty.
 * @retval #VPI_ERROR_INVALID_ARGUMENT \p imgData is NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT \p mat has wrong number of dimensions.
 * @retval #VPI_ERROR_INVALID_IMAGE_FORMAT \p fmt not consistent with \p mat.
 * @retval #VPI_SUCCESS Operation executed successfully.
 */
inline VPIStatus vpiImageDataImportOpenCVMat(const cv::Mat &mat, VPIImageFormat fmt, VPIImageData *imgData)
{
    if (imgData == NULL)
    {
        return VPI_ERROR_INVALID_ARGUMENT;
    }

    return ::nv::vpi::detail::FillImageData(mat, fmt, imgData);
}

/**
 * Fills a vector of `cv::Mat` describing all planes from given \ref VPIImageData
 * forcing the given image format.
 *
 * It performs the inverse operation of \ref vpiImageDataExportOpenCVMatPlanes. Please
 * refer to this function's description on what is the criteria used for image
 * format deduction.
 *
 * The image format isn't deduced from the input `cv::Mat`, but is given by the user.
 * If the format isn't compatible with the `cv::Mat`, an error is returned.
 *
 * @param[in] matPlanes Vector of `cv::Mat`, each describing one plane.
 *                      + It must not be empty.
 *                      + `cv::Mat` in each plane must not be empty.
 *                      + `cv::Mat` in each plane must have number of dimensions equal to 2.
 *
 * @param[in] fmt The image format associated with matPlanes.
 *                + It must be consistent with the \p matPlanes size and the contained
 *                  `cv::Mat`'s dimensions and type.
 *
 * @param[out] imgData \ref VPIImageData that will be filled.
 *                     Existing contents are discarded.
 *                     + Mandatory, must not be NULL.
 *
 * @retval #VPI_ERROR_INVALID_ARGUMENT \p imgData is NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT \p matPlanes is empty.
 * @retval #VPI_ERROR_INVALID_ARGUMENT Some `cv::Mat` in \p matPlanes is empty or has wrong number of dimensions.
 * @retval #VPI_ERROR_INVALID_IMAGE_FORMAT \p fmt not consistent with \p matPlanes.
 * @retval #VPI_SUCCESS Operation executed successfully.
 */
inline VPIStatus vpiImageDataImportOpenCVMatPlanes(const std::vector<cv::Mat> &matPlanes, VPIImageFormat fmt,
                                                   VPIImageData *imgData)
{
    if (imgData == NULL)
    {
        return VPI_ERROR_INVALID_ARGUMENT;
    }

    return ::nv::vpi::detail::FillImageData(matPlanes, fmt, imgData);
}

/** @} */

#endif // NV_VPI_OPENCV_INTEROP_HPP
