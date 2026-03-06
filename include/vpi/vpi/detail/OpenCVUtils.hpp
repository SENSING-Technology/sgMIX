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

#ifndef NV_VPI_DETAIL_OPENCV_UTILS_HPP
#define NV_VPI_DETAIL_OPENCV_UTILS_HPP

#include "../Image.h"
#include "../ImageFormat.h"
#include "../Status.h"

#include <opencv2/core/core.hpp>

#include <stdint.h>

#include <cassert>
#include <cstring>
#include <vector>

namespace nv { namespace vpi { namespace detail {

inline VPIImageFormat ToImageFormatFromOpenCVType(int cvType)
{
    VPIImageFormat imgFormat = VPI_IMAGE_FORMAT_INVALID;

    switch (CV_MAT_DEPTH(cvType))
    {
    case CV_8U:
        switch (CV_MAT_CN(cvType))
        {
        case 1:
            imgFormat = VPI_IMAGE_FORMAT_U8;
            break;

        case 2:
            imgFormat = VPI_MAKE_NONCOLOR_IMAGE_FORMAT_ABBREV(PL, UNSIGNED, XY00, 1, X8_Y8);
            break;

        case 3:
            imgFormat = VPI_IMAGE_FORMAT_BGR8;
            break;

        case 4:
            imgFormat = VPI_IMAGE_FORMAT_BGRA8;
            break;
        }
        break;

    case CV_8S:
        switch (CV_MAT_CN(cvType))
        {
        case 1:
            imgFormat = VPI_IMAGE_FORMAT_S8;
            break;

        case 2:
            imgFormat = VPI_MAKE_NONCOLOR_IMAGE_FORMAT_ABBREV(PL, SIGNED, XY00, 1, X8_Y8);
            break;

        case 3:
            imgFormat = VPI_MAKE_NONCOLOR_IMAGE_FORMAT_ABBREV(PL, SIGNED, XYZ0, 1, X8_Y8_Z8);
            break;

        case 4:
            imgFormat = VPI_MAKE_NONCOLOR_IMAGE_FORMAT_ABBREV(PL, SIGNED, XYZW, 1, X8_Y8_Z8_W8);
            break;
        }
        break;

    case CV_16U:
        switch (CV_MAT_CN(cvType))
        {
        case 1:
            imgFormat = VPI_IMAGE_FORMAT_U16;
            break;

        case 2:
            imgFormat = VPI_MAKE_NONCOLOR_IMAGE_FORMAT_ABBREV(PL, UNSIGNED, XY00, 1, X16_Y16);
            break;

        case 3:
            // BGR
            imgFormat = VPI_MAKE_COLOR_IMAGE_FORMAT_ABBREV(RGB, UNDEFINED, PL, UNSIGNED, ZYX1, 1, X16_Y16_Z16);
            break;

        case 4:
            // ABGR
            imgFormat = VPI_MAKE_COLOR_IMAGE_FORMAT_ABBREV(RGB, UNDEFINED, PL, UNSIGNED, WZYX, 1, X16_Y16_Z16_W16);
            break;
        }
        break;

    case CV_16S:
        switch (CV_MAT_CN(cvType))
        {
        case 1:
            imgFormat = VPI_IMAGE_FORMAT_S16;
            break;

        case 2:
            imgFormat = VPI_MAKE_NONCOLOR_IMAGE_FORMAT_ABBREV(PL, SIGNED, XY00, 1, X16_Y16);
            break;

        case 3:
            imgFormat = VPI_MAKE_NONCOLOR_IMAGE_FORMAT_ABBREV(PL, SIGNED, XYZ0, 1, X16_Y16_Z16);
            break;

        case 4:
            imgFormat = VPI_MAKE_NONCOLOR_IMAGE_FORMAT_ABBREV(PL, SIGNED, XYZW, 1, X16_Y16_Z16_W16);
            break;
        }
        break;

    case CV_32S:
        switch (CV_MAT_CN(cvType))
        {
        case 1:
            imgFormat = VPI_MAKE_NONCOLOR_IMAGE_FORMAT_ABBREV(PL, SIGNED, X000, 1, X32);
            break;

        case 2:
            imgFormat = VPI_MAKE_NONCOLOR_IMAGE_FORMAT_ABBREV(PL, SIGNED, XY00, 1, X32_Y32);
            break;

        case 3:
            imgFormat = VPI_MAKE_NONCOLOR_IMAGE_FORMAT_ABBREV(PL, SIGNED, XYZ0, 1, X32_Y32_Z32);
            break;

        case 4:
            imgFormat = VPI_MAKE_NONCOLOR_IMAGE_FORMAT_ABBREV(PL, SIGNED, XYZW, 1, X32_Y32_Z32_W32);
            break;
        }
        break;

    case CV_32F:
        switch (CV_MAT_CN(cvType))
        {
        case 1:
            imgFormat = VPI_IMAGE_FORMAT_F32;
            break;

        case 2:
            imgFormat = VPI_IMAGE_FORMAT_2F32;
            break;

        case 3:
            imgFormat = VPI_MAKE_COLOR_IMAGE_FORMAT_ABBREV(RGB, UNDEFINED, PL, FLOAT, ZYX1, 1, X32_Y32_Z32);
            break;

        case 4:
            imgFormat = VPI_MAKE_COLOR_IMAGE_FORMAT_ABBREV(RGB, UNDEFINED, PL, FLOAT, WZYX, 1, X32_Y32_Z32_W32);
            break;
        }
        break;

    case CV_64F:
        switch (CV_MAT_CN(cvType))
        {
        case 1:
            imgFormat = VPI_MAKE_NONCOLOR_IMAGE_FORMAT_ABBREV(PL, FLOAT, X000, 1, X64);
            break;

        case 2:
            imgFormat = VPI_MAKE_NONCOLOR_IMAGE_FORMAT_ABBREV(PL, FLOAT, XY00, 1, X64_Y64);
            break;

        case 3:
            imgFormat = VPI_MAKE_COLOR_IMAGE_FORMAT_ABBREV(RGB, UNDEFINED, PL, FLOAT, ZYX1, 1, X64_Y64_Z64);
            break;

        case 4:
            imgFormat = VPI_MAKE_COLOR_IMAGE_FORMAT_ABBREV(RGB, UNDEFINED, PL, FLOAT, WZYX, 1, X64_Y64_Z64_W64);
            break;
        }
        break;
    }

    return imgFormat;
}

inline int ToOpenCVType(VPIPixelType pixType)
{
    if (vpiPixelTypeGetMemLayout(pixType) != VPI_MEM_LAYOUT_PITCH_LINEAR)
    {
        return -1;
    }

    int depth            = -1;
    VPIDataType dataType = vpiPixelTypeGetDataType(pixType);
    int elemSize         = 0;

    if (dataType == VPI_DATA_TYPE_UNSIGNED)
    {
        switch (vpiPixelTypeGetPacking(pixType))
        {
        case VPI_PACKING_X8:
        case VPI_PACKING_X8_Y8:
        case VPI_PACKING_X8_Y8_Z8:
        case VPI_PACKING_X8_Y8_Z8_W8:
        case VPI_PACKING_X8_Y8__X8_Z8:
        case VPI_PACKING_Y8_X8__Z8_X8:
            depth    = CV_8U;
            elemSize = 8;
            break;

        case VPI_PACKING_X16:
        case VPI_PACKING_X16_Y16:
        case VPI_PACKING_X16_Y16_Z16:
        case VPI_PACKING_X16_Y16_Z16_W16:
            depth    = CV_16U;
            elemSize = 16;
            break;

        default:
            return -1;
        }
    }
    else if (dataType == VPI_DATA_TYPE_SIGNED)
    {
        switch (vpiPixelTypeGetPacking(pixType))
        {
        case VPI_PACKING_X8:
        case VPI_PACKING_X8_Y8:
        case VPI_PACKING_X8_Y8_Z8:
        case VPI_PACKING_X8_Y8_Z8_W8:
        case VPI_PACKING_X8_Y8__X8_Z8:
        case VPI_PACKING_Y8_X8__Z8_X8:
            depth    = CV_8S;
            elemSize = 8;
            break;

        case VPI_PACKING_X16:
        case VPI_PACKING_X16_Y16:
        case VPI_PACKING_X16_Y16_Z16:
        case VPI_PACKING_X16_Y16_Z16_W16:
            depth    = CV_16S;
            elemSize = 16;
            break;

        case VPI_PACKING_X32:
        case VPI_PACKING_X32_Y32:
        case VPI_PACKING_X32_Y32_Z32:
        case VPI_PACKING_X32_Y32_Z32_W32:
            depth    = CV_32S;
            elemSize = 32;
            break;

        default:
            return -1;
        }
    }
    else if (dataType == VPI_DATA_TYPE_FLOAT)
    {
        switch (vpiPixelTypeGetPacking(pixType))
        {
        case VPI_PACKING_X32:
        case VPI_PACKING_X32_Y32:
        case VPI_PACKING_X32_Y32_Z32:
        case VPI_PACKING_X32_Y32_Z32_W32:
            depth    = CV_32F;
            elemSize = 32;
            break;

        case VPI_PACKING_X64:
        case VPI_PACKING_X64_Y64:
        case VPI_PACKING_X64_Y64_Z64:
        case VPI_PACKING_X64_Y64_Z64_W64:
            depth    = CV_64F;
            elemSize = 64;
            break;

        default:
            return -1;
        }
    }
    else
    {
        return -1;
    }

    assert(elemSize != 0);

    return CV_MAKE_TYPE(depth, vpiPixelTypeGetBitsPerPixel(pixType) / elemSize);
}

inline VPIStatus FillImageDataOnePlaneImpl(const cv::Mat &mat, VPIImageFormat fmt, VPIImageBufferPitchLinear *imgData)
{
    VPIPixelType pixType = vpiImageFormatGetPlanePixelType(fmt, 0);

    if (ToOpenCVType(pixType) != mat.type())
    {
        return VPI_ERROR_INVALID_IMAGE_FORMAT;
    }

    assert(imgData != nullptr);

    VPIImagePlanePitchLinear &imgPlane = imgData->planes[0];

    imgPlane.width      = mat.cols;
    imgPlane.height     = mat.rows;
    imgPlane.pitchBytes = mat.step[0];
    imgPlane.data       = mat.data;
    imgPlane.pixelType  = pixType;
    return VPI_SUCCESS;
}

inline VPIStatus FillImageDataTwoPlanesImpl(const cv::Mat &mat, VPIImageFormat fmt, VPIImageBufferPitchLinear *imgData)
{
    VPIChromaSubsampling css = vpiImageFormatGetChromaSubsampling(fmt);

    if (vpiChromaSubsamplingGetSamplesHoriz(css) != 2)
    {
        return VPI_ERROR_INVALID_IMAGE_FORMAT;
    }

    if (mat.type() == CV_8UC1)
    {
        if (vpiImageFormatGetPlanePacking(fmt, 0) != VPI_PACKING_X8 ||
            vpiImageFormatGetPlanePacking(fmt, 1) != VPI_PACKING_X8_Y8)
        {
            return VPI_ERROR_INVALID_IMAGE_FORMAT;
        }
    }
    else if (mat.type() == CV_16UC1)
    {
        if (vpiImageFormatGetPlanePacking(fmt, 0) != VPI_PACKING_X16 ||
            vpiImageFormatGetPlanePacking(fmt, 1) != VPI_PACKING_X16_Y16)
        {
            return VPI_ERROR_INVALID_IMAGE_FORMAT;
        }
    }
    else
    {
        return VPI_ERROR_INVALID_IMAGE_FORMAT;
    }

    assert(imgData != nullptr);

    VPIImagePlanePitchLinear *imgPlanes = imgData->planes;

    imgPlanes[0].pixelType = vpiImageFormatGetPlanePixelType(fmt, 0);
    imgPlanes[1].pixelType = vpiImageFormatGetPlanePixelType(fmt, 1);

    imgPlanes[0].width = mat.cols;
    imgPlanes[1].width = mat.cols / 2;

    switch (vpiChromaSubsamplingGetSamplesVert(css))
    {
    case 4:
        imgPlanes[0].height = mat.rows / 2;
        break;

    case 2:
        imgPlanes[0].height = mat.rows * 2 / 3;
        break;

    case 1:
        imgPlanes[0].height = mat.rows * 4 / 5;
        break;

    default:
        return VPI_ERROR_INVALID_IMAGE_FORMAT;
    }

    imgPlanes[1].height = mat.rows - imgPlanes[0].height;

    imgPlanes[0].pitchBytes = mat.step[0];
    imgPlanes[1].pitchBytes = mat.step[0];

    imgPlanes[0].data = mat.data;
    imgPlanes[1].data = const_cast<uint8_t *>(mat.ptr(imgPlanes[0].height, 0));

    return VPI_SUCCESS;
}

inline VPIStatus FillImageDataThreePlanesImpl(const cv::Mat &mat, VPIImageFormat fmt,
                                              VPIImageBufferPitchLinear *imgData)
{
    VPIChromaSubsampling css = vpiImageFormatGetChromaSubsampling(fmt);

    if (vpiChromaSubsamplingGetSamplesHoriz(css) != 2)
    {
        return VPI_ERROR_INVALID_IMAGE_FORMAT;
    }

    // column count must be even
    if (mat.cols & 1)
    {
        return VPI_ERROR_INVALID_IMAGE_FORMAT;
    }

    switch (mat.type())
    {
    case CV_8UC1:
        // must have packed rows
        if (mat.step[0] != (size_t)mat.cols)
        {
            return VPI_ERROR_INVALID_IMAGE_FORMAT;
        }

        for (int p = 0; p < 3; ++p)
        {
            if (vpiImageFormatGetPlanePacking(fmt, p) != VPI_PACKING_X8)
            {
                return VPI_ERROR_INVALID_IMAGE_FORMAT;
            }
        }
        break;

    case CV_16UC1:
        // must have packed rows
        if (mat.step[0] != (size_t)mat.cols * 2)
        {
            return VPI_ERROR_INVALID_IMAGE_FORMAT;
        }

        for (int p = 0; p < 3; ++p)
        {
            if (vpiImageFormatGetPlanePacking(fmt, p) != VPI_PACKING_X16)
            {
                return VPI_ERROR_INVALID_IMAGE_FORMAT;
            }
        }
        break;

    default:
        return VPI_ERROR_INVALID_IMAGE_FORMAT;
    }

    assert(imgData != nullptr);

    VPIImagePlanePitchLinear *imgPlanes = imgData->planes;

    imgPlanes[0].pixelType = vpiImageFormatGetPlanePixelType(fmt, 0);
    imgPlanes[1].pixelType = vpiImageFormatGetPlanePixelType(fmt, 1);
    imgPlanes[2].pixelType = vpiImageFormatGetPlanePixelType(fmt, 2);

    imgPlanes[0].width = mat.cols;
    imgPlanes[1].width = mat.cols / 2;
    imgPlanes[2].width = mat.cols / 2;

    switch (vpiChromaSubsamplingGetSamplesVert(css))
    {
    case 4:
        imgPlanes[0].height = mat.rows / 2;
        break;
    case 2:
        imgPlanes[0].height = mat.rows * 2 / 3;
        break;
    case 1:
        imgPlanes[0].height = mat.rows * 4 / 5;
        break;
    default:
        return VPI_ERROR_INVALID_IMAGE_FORMAT;
    }

    imgPlanes[1].height = mat.rows - imgPlanes[0].height;
    imgPlanes[2].height = imgPlanes[1].height;

    imgPlanes[0].pitchBytes = mat.step[0];
    imgPlanes[1].pitchBytes = mat.step[0] / 2;
    imgPlanes[2].pitchBytes = mat.step[0] / 2;

    imgPlanes[0].data = mat.data;
    imgPlanes[1].data = mat.data + mat.step[0] * imgPlanes[0].height;
    imgPlanes[2].data = reinterpret_cast<uint8_t *>(imgPlanes[1].data) + imgPlanes[1].height * imgPlanes[1].pitchBytes;

    return VPI_SUCCESS;
}

inline VPIStatus FillImageData(const cv::Mat &mat, VPIImageFormat fmt, VPIImageData *imgData)
{
    assert(imgData != NULL);

    if (mat.data == NULL)
    {
        return VPI_ERROR_INVALID_ARGUMENT;
    }

    if (mat.dims != 2)
    {
        return VPI_ERROR_INVALID_ARGUMENT;
    }

    if (vpiImageFormatGetMemLayout(fmt) != VPI_MEM_LAYOUT_PITCH_LINEAR)
    {
        return VPI_ERROR_INVALID_IMAGE_FORMAT;
    }

    int numPlanes = vpiImageFormatGetPlaneCount(fmt);

    memset(imgData, 0, sizeof(*imgData));

    imgData->bufferType             = VPI_IMAGE_BUFFER_HOST_PITCH_LINEAR;
    imgData->buffer.pitch.format    = fmt;
    imgData->buffer.pitch.numPlanes = numPlanes;

    VPIStatus status;
    switch (numPlanes)
    {
    case 1:
        status = FillImageDataOnePlaneImpl(mat, fmt, &imgData->buffer.pitch);
        break;

    case 2:
        status = FillImageDataTwoPlanesImpl(mat, fmt, &imgData->buffer.pitch);
        break;

    case 3:
        status = FillImageDataThreePlanesImpl(mat, fmt, &imgData->buffer.pitch);
        break;

    default:
        status = VPI_ERROR_INTERNAL;
        break;
    }

    return status;
}

inline VPIStatus FillImageData(const std::vector<cv::Mat> &matPlanes, VPIImageFormat fmt, VPIImageData *imgData)
{
    assert(imgData != NULL);

    if (matPlanes.empty())
    {
        return VPI_ERROR_INVALID_ARGUMENT;
    }

    if (matPlanes.size() != (size_t)vpiImageFormatGetPlaneCount(fmt))
    {
        return VPI_ERROR_INVALID_IMAGE_FORMAT;
    }

    memset(imgData, 0, sizeof(*imgData));

    imgData->bufferType             = VPI_IMAGE_BUFFER_HOST_PITCH_LINEAR;
    imgData->buffer.pitch.format    = fmt;
    imgData->buffer.pitch.numPlanes = matPlanes.size();

    for (size_t p = 0; p < matPlanes.size(); ++p)
    {
        const cv::Mat &mat = matPlanes[p];

        if (mat.data == NULL)
        {
            return VPI_ERROR_INVALID_ARGUMENT;
        }

        if (mat.dims != 2)
        {
            return VPI_ERROR_INVALID_ARGUMENT;
        }

        VPIPixelType pixType = vpiImageFormatGetPlanePixelType(fmt, p);

        if (ToOpenCVType(pixType) != mat.type())
        {
            return VPI_ERROR_INVALID_IMAGE_FORMAT;
        }

        VPIImagePlanePitchLinear &plane = imgData->buffer.pitch.planes[p];

        plane.width      = mat.cols;
        plane.height     = mat.rows;
        plane.pitchBytes = mat.step[0];
        plane.data       = mat.data;
        plane.pixelType  = pixType;
    }

    return VPI_SUCCESS;
}

inline VPIStatus FillImageData(const cv::Mat &mat, VPIImageData *imgData)
{
    assert(imgData != NULL);

    if (mat.data == NULL)
    {
        return VPI_ERROR_INVALID_ARGUMENT;
    }

    if (mat.dims != 2)
    {
        return VPI_ERROR_INVALID_ARGUMENT;
    }

    VPIImageFormat fmt = ToImageFormatFromOpenCVType(mat.type());
    if (fmt == VPI_IMAGE_FORMAT_INVALID)
    {
        return VPI_ERROR_INVALID_ARGUMENT;
    }

    assert(imgData != nullptr);

    imgData->bufferType                 = VPI_IMAGE_BUFFER_HOST_PITCH_LINEAR;
    VPIImageBufferPitchLinear &imgPitch = imgData->buffer.pitch;

    imgPitch.format               = fmt;
    imgPitch.numPlanes            = 1;
    imgPitch.planes[0].width      = mat.cols;
    imgPitch.planes[0].height     = mat.rows;
    imgPitch.planes[0].pitchBytes = mat.step[0];
    imgPitch.planes[0].data       = mat.data;
    imgPitch.planes[0].pixelType  = vpiImageFormatGetPlanePixelType(fmt, 0);

    return VPI_SUCCESS;
}

inline VPIStatus CalcHeightOpenCVMatOnePlane(const VPIImageBufferPitchLinear &imgData, int &height)
{
    height = imgData.planes[0].height;
    return VPI_SUCCESS;
}

inline VPIStatus CalcHeightOpenCVMatTwoPlanes(const VPIImageBufferPitchLinear &imgData, int &height)
{
    VPIChromaSubsampling css = vpiImageFormatGetChromaSubsampling(imgData.format);

    if (vpiChromaSubsamplingGetSamplesHoriz(css) != 2)
    {
        return VPI_ERROR_INVALID_IMAGE_FORMAT;
    }

    if (imgData.planes[0].pitchBytes != imgData.planes[1].pitchBytes)
    {
        return VPI_ERROR_INVALID_IMAGE_FORMAT;
    }

    if (imgData.planes[1].width * 2 != imgData.planes[0].width)
    {
        return VPI_ERROR_INVALID_IMAGE_FORMAT;
    }

    if (imgData.planes[1].data !=
        reinterpret_cast<uint8_t *>(imgData.planes[0].data) + imgData.planes[0].height * imgData.planes[0].pitchBytes)
    {
        return VPI_ERROR_INVALID_IMAGE_FORMAT;
    }

    switch (vpiChromaSubsamplingGetSamplesVert(css))
    {
    case 1:
    case 2:
    case 4:
        height = imgData.planes[0].height + imgData.planes[1].height;
        break;

    default:
        return VPI_ERROR_INVALID_IMAGE_FORMAT;
    }

    return VPI_SUCCESS;
}

inline VPIStatus CalcHeightOpenCVMatThreePlanes(const VPIImageBufferPitchLinear &imgData, int &height)
{
    VPIChromaSubsampling css = vpiImageFormatGetChromaSubsampling(imgData.format);

    // All planes must have packed rows
    for (int i = 0; i < 3; ++i)
    {
        if (imgData.planes[i].pitchBytes !=
            imgData.planes[i].width * vpiPixelTypeGetBitsPerPixel(imgData.planes[i].pixelType) / 8)
        {
            return VPI_ERROR_INVALID_IMAGE_FORMAT;
        }
    }

    if (imgData.planes[0].pitchBytes != imgData.planes[1].pitchBytes * 2 ||
        imgData.planes[1].pitchBytes != imgData.planes[2].pitchBytes)
    {
        return VPI_ERROR_INVALID_IMAGE_FORMAT;
    }

    int samplesVert = vpiChromaSubsamplingGetSamplesVert(css);

    if ((imgData.planes[1].height * 4 + samplesVert - 1) / samplesVert != imgData.planes[0].height)
    {
        return VPI_ERROR_INVALID_IMAGE_FORMAT;
    }

    if (imgData.planes[2].height != imgData.planes[1].height)
    {
        return VPI_ERROR_INVALID_IMAGE_FORMAT;
    }

    for (int i = 0; i < 2; ++i)
    {
        if (imgData.planes[i + 1].data != reinterpret_cast<uint8_t *>(imgData.planes[i].data) +
                                              imgData.planes[i].height * imgData.planes[i].pitchBytes)
        {
            return VPI_ERROR_INVALID_IMAGE_FORMAT;
        }
    }

    switch (vpiChromaSubsamplingGetSamplesVert(css))
    {
    case 1:
    case 2:
    case 4:
        height = imgData.planes[0].height + imgData.planes[1].height;
        break;

    default:
        return VPI_ERROR_INVALID_IMAGE_FORMAT;
    }

    return VPI_SUCCESS;
}

inline VPIStatus ExportOpenCVMat(const VPIImageData &imgData, cv::Mat &mat)
{
    if (imgData.bufferType != VPI_IMAGE_BUFFER_HOST_PITCH_LINEAR)
    {
        return VPI_ERROR_INVALID_ARGUMENT;
    }

    const VPIImageBufferPitchLinear &imgPitch = imgData.buffer.pitch;

    VPIStatus status;
    int height;
    switch (imgPitch.numPlanes)
    {
    case 1:
        status = CalcHeightOpenCVMatOnePlane(imgPitch, height);
        break;

    case 2:
        status = CalcHeightOpenCVMatTwoPlanes(imgPitch, height);
        break;

    case 3:
        status = CalcHeightOpenCVMatThreePlanes(imgPitch, height);
        break;

    default:
        status = VPI_ERROR_INVALID_IMAGE_FORMAT;
    }

    if (status == VPI_SUCCESS)
    {
        VPIPixelType pixType = imgPitch.planes[0].pixelType;

        if (pixType == VPI_PIXEL_TYPE_DEFAULT)
        {
            pixType = vpiImageFormatGetPlanePixelType(imgPitch.format, 0);
            if (pixType == VPI_PIXEL_TYPE_INVALID)
            {
                return VPI_ERROR_INVALID_IMAGE_FORMAT;
            }
        }

        int cvType = ToOpenCVType(pixType);
        if (cvType == -1)
        {
            return VPI_ERROR_INVALID_IMAGE_FORMAT;
        }

        mat = cv::Mat(height, imgPitch.planes[0].width, cvType, imgPitch.planes[0].data, imgPitch.planes[0].pitchBytes);
    }

    return status;
}

inline VPIStatus ExportOpenCVMatPlanes(const VPIImageData &imgData, std::vector<cv::Mat> &mat)
{
    if (imgData.bufferType != VPI_IMAGE_BUFFER_HOST_PITCH_LINEAR)
    {
        return VPI_ERROR_INVALID_ARGUMENT;
    }

    const VPIImageBufferPitchLinear &imgPitch = imgData.buffer.pitch;

    if (imgPitch.numPlanes > VPI_MAX_PLANE_COUNT)
    {
        return VPI_ERROR_INVALID_ARGUMENT;
    }

    int cvTypes[VPI_MAX_PLANE_COUNT];
    for (int i = 0; i < imgPitch.numPlanes; ++i)
    {
        VPIPixelType pixType = imgPitch.planes[i].pixelType;

        if (pixType == VPI_PIXEL_TYPE_DEFAULT)
        {
            pixType = vpiImageFormatGetPlanePixelType(imgPitch.format, i);
            if (pixType == VPI_PIXEL_TYPE_INVALID)
            {
                return VPI_ERROR_INVALID_IMAGE_FORMAT;
            }
        }

        cvTypes[i] = ToOpenCVType(pixType);

        if (cvTypes[i] == -1)
        {
            return VPI_ERROR_INVALID_IMAGE_FORMAT;
        }
    }

    mat.resize(imgPitch.numPlanes);

    for (int i = 0; i < imgPitch.numPlanes; ++i)
    {
        mat[i] = cv::Mat(imgPitch.planes[i].height, imgPitch.planes[i].width, cvTypes[i], imgPitch.planes[i].data,
                         imgPitch.planes[i].pitchBytes);
    }

    return VPI_SUCCESS;
}

}}} // namespace nv::vpi::detail

#endif // NV_VPI_DETAIL_OPENCV_UTILS_HPP
