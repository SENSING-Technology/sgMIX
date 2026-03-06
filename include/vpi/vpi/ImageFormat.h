/*
 * Copyright 2020-2021 NVIDIA Corporation. All rights reserved.
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
 * @file ImageFormat.h
 *
 * Defines types and functions to handle image formats.
 */

#ifndef NV_VPI_IMAGE_FORMAT_H
#define NV_VPI_IMAGE_FORMAT_H

#include "ColorSpec.h"
#include "DataLayout.h"
#include "PixelType.h"

#include <assert.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @ingroup VPI_ImageFormat
 * @{
 */

/**
 * Pre-defined image formats.
 * An image format defines how image pixels are interpreted.
 * Each image format is defined by the following components:
 * - \ref VPIColorModel
 * - \ref VPIColorSpec
 * - \ref VPIChromaSubsampling method (when applicable)
 * - \ref VPIMemLayout
 * - \ref VPIDataType
 * - \ref VPISwizzle
 * - Number of planes
 * - Format packing of each plane.
 *
 * These pre-defined formats are guaranteed to work with algorithms that explicitly support them.
 * Image formats can also be user-defined using the vpiMakeImageFormat family of functions.
 *
 * Using user-defined image formats with algorithms can lead to undefined behavior (segfaults, etc),
 * but usually it works as expected. Result of algorithms using these image formats must be checked
 * for correctness, as it's not guaranteed that they will work.
 */
typedef uint64_t VPIImageFormat;

/** Denotes an invalid image format. */
#define VPI_IMAGE_FORMAT_INVALID ((VPIImageFormat)0)

/** Single plane with one 8-bit unsigned integer channel. */
#define VPI_IMAGE_FORMAT_U8 VPI_DETAIL_MAKE_NONCOLOR_FMT1(PL, UNSIGNED, X000, X8)

/** Single plane with one block-linear 8-bit unsigned integer channel. @{ */
#define VPI_IMAGE_FORMAT_U8_BL VPI_DETAIL_MAKE_NONCOLOR_FMT1(BL, UNSIGNED, X000, X8)
#define VPI_IMAGE_FORMAT_U8_BL16 VPI_DETAIL_MAKE_NONCOLOR_FMT1(BLOCK16_LINEAR, UNSIGNED, X000, X8)
/** @} */

/** Single plane with one 8-bit signed integer channel. */
#define VPI_IMAGE_FORMAT_S8 VPI_DETAIL_MAKE_NONCOLOR_FMT1(PL, SIGNED, X000, X8)

/** Single plane with one 16-bit unsigned integer channel. */
#define VPI_IMAGE_FORMAT_U16 VPI_DETAIL_MAKE_NONCOLOR_FMT1(PL, UNSIGNED, X000, X16)

/** Single plane with one 32-bit unsigned integer channel. */
#define VPI_IMAGE_FORMAT_U32 VPI_DETAIL_MAKE_NONCOLOR_FMT1(PL, UNSIGNED, X000, X32)

/** Single plane with one 32-bit signed integer channel.*/
#define VPI_IMAGE_FORMAT_S32 VPI_DETAIL_MAKE_NONCOLOR_FMT1(PL, SIGNED, X000, X32)

/** Single plane with one 16-bit signed integer channel.*/
#define VPI_IMAGE_FORMAT_S16 VPI_DETAIL_MAKE_NONCOLOR_FMT1(PL, SIGNED, X000, X16)

/** Single plane with one block-linear 16-bit signed integer channel. @{ */
#define VPI_IMAGE_FORMAT_S16_BL VPI_DETAIL_MAKE_NONCOLOR_FMT1(BL, SIGNED, X000, X16)
#define VPI_IMAGE_FORMAT_S16_BL16 VPI_DETAIL_MAKE_NONCOLOR_FMT1(BLOCK16_LINEAR, SIGNED, X000, X16)
/** @} */

/** Single plane with two interleaved 16-bit signed integer channel.*/
#define VPI_IMAGE_FORMAT_2S16 VPI_DETAIL_MAKE_NONCOLOR_FMT1(PL, SIGNED, XY00, X16_Y16)

/** Single plane with two interleaved block-linear 16-bit signed integer channel. @{ */
#define VPI_IMAGE_FORMAT_2S16_BL VPI_DETAIL_MAKE_NONCOLOR_FMT1(BL, SIGNED, XY00, X16_Y16)
#define VPI_IMAGE_FORMAT_2S16_BL16 VPI_DETAIL_MAKE_NONCOLOR_FMT1(BLOCK16_LINEAR, SIGNED, XY00, X16_Y16)
/** @} */

/** Single plane with one 32-bit floating point channel. */
#define VPI_IMAGE_FORMAT_F32 VPI_DETAIL_MAKE_NONCOLOR_FMT1(PL, FLOAT, X000, X32)

/** Single plane with one 64-bit floating point channel. */
#define VPI_IMAGE_FORMAT_F64 VPI_DETAIL_MAKE_NONCOLOR_FMT1(PL, FLOAT, X000, X64)

/** Single plane with two interleaved 32-bit floating point channels. */
#define VPI_IMAGE_FORMAT_2F32 VPI_DETAIL_MAKE_NONCOLOR_FMT1(PL, FLOAT, XY00, X32_Y32)

/** Single plane with one pitch-linear 8-bit unsigned integer channel with limited-range luma (grayscale) information.
 * Values range from 16 to 235. Below this range is considered black, above is considered white.
 */
#define VPI_IMAGE_FORMAT_Y8 VPI_DETAIL_MAKE_YCbCr_FMT1(BT601, NONE, PL, UNSIGNED, X000, X8)

/** Single plane with one block-linear 8-bit unsigned integer channel with limited-range luma (grayscale) information.
 * Values range from 16 to 235. Below this range is considered black, above is considered white.
 * @{ */
#define VPI_IMAGE_FORMAT_Y8_BL VPI_DETAIL_MAKE_YCbCr_FMT1(BT601, NONE, BL, UNSIGNED, X000, X8)
#define VPI_IMAGE_FORMAT_Y8_BL16 VPI_DETAIL_MAKE_YCbCr_FMT1(BT601, NONE, BLOCK16_LINEAR, UNSIGNED, X000, X8)
/** @} */

/** Single plane with one pitch-linear 8-bit unsigned integer channel with full-range luma (grayscale) information.
 * Values range from 0 to 255.
 */
#define VPI_IMAGE_FORMAT_Y8_ER VPI_DETAIL_MAKE_YCbCr_FMT1(BT601_ER, NONE, PL, UNSIGNED, X000, X8)

/** Single plane with one block-linear 8-bit unsigned integer channel with full-range luma (grayscale) information.
 * Values range from 0 to 255.
 * @{ */
#define VPI_IMAGE_FORMAT_Y8_ER_BL VPI_DETAIL_MAKE_YCbCr_FMT1(BT601_ER, NONE, BL, UNSIGNED, X000, X8)
#define VPI_IMAGE_FORMAT_Y8_ER_BL16 VPI_DETAIL_MAKE_YCbCr_FMT1(BT601_ER, NONE, BLOCK16_LINEAR, UNSIGNED, X000, X8)
/** @} */

/** Single plane with one pitch-linear 16-bit unsigned integer channel with limited-range luma (grayscale) information.
 * Values range from 4096 to 60160. Below this range is considered black, above is considered white.
 */
#define VPI_IMAGE_FORMAT_Y16 VPI_DETAIL_MAKE_YCbCr_FMT1(BT601, NONE, PL, UNSIGNED, X000, X16)

/** Single plane with one block-linear 16-bit unsigned integer channel with limited-range luma (grayscale) information.
 * Values range from 4096 to 60160. Below this range is considered black, above is considered white.
 * @{ */
#define VPI_IMAGE_FORMAT_Y16_BL VPI_DETAIL_MAKE_YCbCr_FMT1(BT601, NONE, BL, UNSIGNED, X000, X16)
#define VPI_IMAGE_FORMAT_Y16_BL16 VPI_DETAIL_MAKE_YCbCr_FMT1(BT601, NONE, BLOCK16_LINEAR, UNSIGNED, X000, X16)
/** @} */

/** Single plane with one pitch-linear 16-bit unsigned integer channel with full-range luma (grayscale) information.
 * Values range from 0 to 65535.
 */
#define VPI_IMAGE_FORMAT_Y16_ER VPI_DETAIL_MAKE_YCbCr_FMT1(BT601_ER, NONE, PL, UNSIGNED, X000, X16)

/** Single plane with one block-linear 16-bit unsigned integer channel with full-range luma (grayscale) information.
 * Values range from 0 to 65535.
 * @{ */
#define VPI_IMAGE_FORMAT_Y16_ER_BL VPI_DETAIL_MAKE_YCbCr_FMT1(BT601_ER, NONE, BL, UNSIGNED, X000, X16)
#define VPI_IMAGE_FORMAT_Y16_ER_BL16 VPI_DETAIL_MAKE_YCbCr_FMT1(BT601_ER, NONE, BLOCK16_LINEAR, UNSIGNED, X000, X16)
/** @} */

/** YUV420sp 8-bit pitch-linear format with limited range.
 * Format is composed of two planes:
 * 1. One 8-bit channel with luma (Y'). Values range from 16 to 235.
 * 2. Two interleaved 8-bit channels with chroma (Cb,Cr).
 *    Values range from 16 to 240. Resolution is half of luma plane,
 *    both horizontally and vertically.
 *    For a given pixel, Cb channel has lower memory address than Cr.
 */
#define VPI_IMAGE_FORMAT_NV12 VPI_DETAIL_MAKE_YCbCr_FMT2(BT601, 420, PL, UNSIGNED, XYZ0, X8, X8_Y8)

/** YUV420sp 8-bit block-linear format with limited range.
 * Format is composed of two planes:
 * 1. One 8-bit channel with luma (Y'). Values range from 16 to 235.
 * 2. Two interleaved 8-bit channels with chroma (Cb,Cr).
 *    Values range from 0 to 255. Resolution is half of luma plane,
 *    both horizontally and vertically.
 *    For a given pixel, Cb channel has lower memory address than Cr.
 * @{ */
#define VPI_IMAGE_FORMAT_NV12_BL VPI_DETAIL_MAKE_YCbCr_FMT2(BT601, 420, BL, UNSIGNED, XYZ0, X8, X8_Y8)
#define VPI_IMAGE_FORMAT_NV12_BL16 VPI_DETAIL_MAKE_YCbCr_FMT2(BT601, 420, BLOCK16_LINEAR, UNSIGNED, XYZ0, X8, X8_Y8)
/** @} */

/** YUV420sp 8-bit pitch-linear format with full range.
 * Format is composed of two planes:
 * 1. One 8-bit channel with luma (Y'). Values range from 0 to 255.
 * 2. Two interleaved 8-bit channels with chroma (Cb,Cr).
 *    Values range from 0 to 255. Resolution is half of luma plane,
 *    both horizontally and vertically.
 *    For a given pixel, Cb channel has lower memory address than Cr.
 */
#define VPI_IMAGE_FORMAT_NV12_ER VPI_DETAIL_MAKE_YCbCr_FMT2(BT601_ER, 420, PL, UNSIGNED, XYZ0, X8, X8_Y8)

/** YUV420sp 8-bit block-linear format with full range.
 * Format is composed of two planes:
 * 1. One 8-bit channel with luma (Y'). Values range from 0 to 255.
 * 2. Two interleaved 8-bit channels with chroma (Cb,Cr).
 *    Values range from 0 to 255. Resolution is half of luma plane,
 *    both horizontally and vertically.
 *    For a given pixel, Cb channel has lower memory address than Cr.
 * @{ */
#define VPI_IMAGE_FORMAT_NV12_ER_BL VPI_DETAIL_MAKE_YCbCr_FMT2(BT601_ER, 420, BL, UNSIGNED, XYZ0, X8, X8_Y8)
#define VPI_IMAGE_FORMAT_NV12_ER_BL16 \
    VPI_DETAIL_MAKE_YCbCr_FMT2(BT601_ER, 420, BLOCK16_LINEAR, UNSIGNED, XYZ0, X8, X8_Y8)
/** @} */

/** YUV444sp 8-bit pitch-linear format with limited range.
 * Format is composed of two planes:
 * 1. One 8-bit channel with luma (Y'). Values range from 16 to 235.
 * 2. Two interleaved 8-bit channels with chroma (Cb,Cr).
 *    Values range from 16 to 240. It has the same resolution as luma plane.
 *    For a given pixel, Cb channel has lower memory address than Cr.
 */
#define VPI_IMAGE_FORMAT_NV24 VPI_DETAIL_MAKE_YCbCr_FMT2(BT601, 444, PL, UNSIGNED, XYZ0, X8, X8_Y8)

/** YUV444sp 8-bit block-linear format with limited range.
 * Format is composed of two planes:
 * 1. One 8-bit channel with luma (Y'). Values range from 16 to 235.
 * 2. Two interleaved 8-bit channels with chroma (Cb,Cr).
 *    Values range from 0 to 255. It has the same resolution as luma plane.
 *    For a given pixel, Cb channel has lower memory address than Cr.
 * @{ */
#define VPI_IMAGE_FORMAT_NV24_BL VPI_DETAIL_MAKE_YCbCr_FMT2(BT601, 444, BL, UNSIGNED, XYZ0, X8, X8_Y8)
#define VPI_IMAGE_FORMAT_NV24_BL16 VPI_DETAIL_MAKE_YCbCr_FMT2(BT601, 444, BLOCK16_LINEAR, UNSIGNED, XYZ0, X8, X8_Y8)
/** @} */

/** YUV444sp 8-bit pitch-linear format with full range.
 * Format is composed of two planes:
 * 1. One 8-bit channel with luma (Y'). Values range from 0 to 255.
 * 2. Two interleaved 8-bit channels with chroma (Cb,Cr).
 *    Values range from 0 to 255. It has the same resolution as luma plane.
 *    For a given pixel, Cb channel has lower memory address than Cr.
 */
#define VPI_IMAGE_FORMAT_NV24_ER VPI_DETAIL_MAKE_YCbCr_FMT2(BT601_ER, 444, PL, UNSIGNED, XYZ0, X8, X8_Y8)

/** YUV444sp 8-bit block-linear format with full range.
 * Format is composed of two planes:
 * 1. One 8-bit channel with luma (Y'). Values range from 0 to 255.
 * 2. Two interleaved 8-bit channels with chroma (Cb,Cr).
 *    Values range from 0 to 255. It has the same resolution as luma plane.
 *    For a given pixel, Cb channel has lower memory address than Cr.
 * @{ */
#define VPI_IMAGE_FORMAT_NV24_ER_BL VPI_DETAIL_MAKE_YCbCr_FMT2(BT601_ER, 444, BL, UNSIGNED, XYZ0, X8, X8_Y8)
#define VPI_IMAGE_FORMAT_NV24_ER_BL16 \
    VPI_DETAIL_MAKE_YCbCr_FMT2(BT601_ER, 444, BLOCK16_LINEAR, UNSIGNED, XYZ0, X8, X8_Y8)
/** @} */

/** YUV422 8-bit pitch-linear format in one plane with UYVY ordering and limited range. */
#define VPI_IMAGE_FORMAT_UYVY VPI_DETAIL_MAKE_YCbCr_FMT1(BT601, 422, PL, UNSIGNED, XYZ1, Y8_X8__Z8_X8)

/** YUV422 8-bit block-linear format in one plane with UYVY ordering and limited range. @{ */
#define VPI_IMAGE_FORMAT_UYVY_BL VPI_DETAIL_MAKE_YCbCr_FMT1(BT601, 422, BL, UNSIGNED, XYZ1, Y8_X8__Z8_X8)
#define VPI_IMAGE_FORMAT_UYVY_BL16 VPI_DETAIL_MAKE_YCbCr_FMT1(BT601, 422, BLOCK16_LINEAR, UNSIGNED, XYZ1, Y8_X8__Z8_X8)
/** @} */

/** YUV422 8-bit pitch-linear format in one plane with UYVY ordering and full range. */
#define VPI_IMAGE_FORMAT_UYVY_ER VPI_DETAIL_MAKE_YCbCr_FMT1(BT601_ER, 422, PL, UNSIGNED, XYZ1, Y8_X8__Z8_X8)

/** YUV422 8-bit block-linear format in one plane with UYVY ordering and full range. @{ */
#define VPI_IMAGE_FORMAT_UYVY_ER_BL VPI_DETAIL_MAKE_YCbCr_FMT1(BT601_ER, 422, BL, UNSIGNED, XYZ1, Y8_X8__Z8_X8)
#define VPI_IMAGE_FORMAT_UYVY_ER_BL16 \
    VPI_DETAIL_MAKE_YCbCr_FMT1(BT601_ER, 422, BLOCK16_LINEAR, UNSIGNED, XYZ1, Y8_X8__Z8_X8)
/** @} */

/** YUV422 8-bit pitch-linear format in one plane with YUYV ordering and limited range.
 * Also known as YUY2 format.
 */
#define VPI_IMAGE_FORMAT_YUYV VPI_DETAIL_MAKE_YCbCr_FMT1(BT601, 422, PL, UNSIGNED, XYZ1, X8_Y8__X8_Z8)

/** YUV422 8-bit block-linear format in one plane with YUYV ordering and limited range.
 * Also known as YUY2 format.
 * @{ */
#define VPI_IMAGE_FORMAT_YUYV_BL VPI_DETAIL_MAKE_YCbCr_FMT1(BT601, 422, BL, UNSIGNED, XYZ1, X8_Y8__X8_Z8)
#define VPI_IMAGE_FORMAT_YUYV_BL16 VPI_DETAIL_MAKE_YCbCr_FMT1(BT601, 422, BLOCK16_LINEAR, UNSIGNED, XYZ1, X8_Y8__X8_Z8)
/** @} */

/** YUV422 8-bit pitch-linear format in one plane with YUYV ordering and full range.
 * Also known as YUY2 format.
 */
#define VPI_IMAGE_FORMAT_YUYV_ER VPI_DETAIL_MAKE_YCbCr_FMT1(BT601_ER, 422, PL, UNSIGNED, XYZ1, X8_Y8__X8_Z8)

/** YUV422 8-bit block-linear format in one plane with YUYV ordering and full range.
 * Also known as YUY2 format.
 * @{ */
#define VPI_IMAGE_FORMAT_YUYV_ER_BL VPI_DETAIL_MAKE_YCbCr_FMT1(BT601_ER, 422, BL, UNSIGNED, XYZ1, X8_Y8__X8_Z8)
#define VPI_IMAGE_FORMAT_YUYV_ER_BL16 \
    VPI_DETAIL_MAKE_YCbCr_FMT1(BT601_ER, 422, BLOCK16_LINEAR, UNSIGNED, XYZ1, X8_Y8__X8_Z8)
/** @} */

/** Single plane with interleaved RGB 8-bit channel. */
#define VPI_IMAGE_FORMAT_RGB8 VPI_DETAIL_MAKE_COLOR_FMT1(RGB, UNDEFINED, PL, UNSIGNED, XYZ1, X8_Y8_Z8)

/** Single plane with interleaved BGR 8-bit channel. */
#define VPI_IMAGE_FORMAT_BGR8 VPI_DETAIL_MAKE_COLOR_FMT1(RGB, UNDEFINED, PL, UNSIGNED, ZYX1, X8_Y8_Z8)

/** Single plane with interleaved RGBA 8-bit channel. */
#define VPI_IMAGE_FORMAT_RGBA8 VPI_DETAIL_MAKE_COLOR_FMT1(RGB, UNDEFINED, PL, UNSIGNED, XYZW, X8_Y8_Z8_W8)

/** Single plane with interleaved BGRA 8-bit channel. */
#define VPI_IMAGE_FORMAT_BGRA8 VPI_DETAIL_MAKE_COLOR_FMT1(RGB, UNDEFINED, PL, UNSIGNED, ZYXW, X8_Y8_Z8_W8)

/** Planar RGB with unsigned 8-bit channels. */
#define VPI_IMAGE_FORMAT_RGB8p VPI_DETAIL_MAKE_COLOR_FMT3(RGB, UNDEFINED, PL, UNSIGNED, XYZ1, X8, X8, X8)

/** Planar BGR with unsigned 8-bit channels. */
#define VPI_IMAGE_FORMAT_BGR8p VPI_DETAIL_MAKE_COLOR_FMT3(RGB, UNDEFINED, PL, UNSIGNED, ZYX1, X8, X8, X8)

/** Planar RGBA with unsigned 8-bit channels. */
#define VPI_IMAGE_FORMAT_RGBA8p VPI_DETAIL_MAKE_COLOR_FMT4(RGB, UNDEFINED, PL, UNSIGNED, XYZW, X8, X8, X8, X8)

/** Planar BGRA with unsigned 8-bit channels. */
#define VPI_IMAGE_FORMAT_BGRA8p VPI_DETAIL_MAKE_COLOR_FMT4(RGB, UNDEFINED, PL, UNSIGNED, ZYXW, X8, X8, X8, X8)

/** Creates a user-defined YCbCr color image format constant using abbreviated parameters.
 *
 * This macro allows passing abbreviated format parameters (without the parameter type prefix).
 * Example to create a YUV422 ITU-R BT.709 studio-range block-linear format.
 * \code{.c}
 * VPIImageFormat fmt = VPI_MAKE_YCbCr_IMAGE_FORMAT_ABBREV(BT709, 422, BL, UNSIGNED, XYZ0, 2, X8, X8_Y8);
 * \endcode
 *
 * Fourth plane, packing3, must have at most 64bpp.
 *
 * @param[in] colorSpec     \ref VPIColorSpec to be used, without the VPI_COLOR_SPEC_ prefix.
 * @param[in] chromaSubsamp \ref VPIChromaSubsampling to be used, without the VPI_CSS_ prefix.
 * @param[in] memLayout     \ref VPIMemLayout to be used, without the VPI_MEM_LAYOUT_ prefix.
 * @param[in] dataType      \ref VPIDataType to be used, without the VPI_DATA_TYPE_ prefix.
 * @param[in] swizzle       \ref VPISwizzle operation to be performed on the channels, without the VPI_SWIZZLE_ prefix.
 * @param[in] numPlanes     Number of planes this format has.
 * @param[in] packing0,packing1,... Format packing of each plane, without the VPI_PACKING_ prefix.
 *
 * @returns The user-defined image format.
 */
#ifdef VPI_DOXYGEN
#    define VPI_MAKE_YCbCr_IMAGE_FORMAT_ABBREV(colorSpec, chromaSubsamp, memLayout, dataType, swizzle, numPlanes, \
                                               packing0, packing1, ...)
#else
#    define VPI_MAKE_YCbCr_IMAGE_FORMAT_ABBREV (VPIImageFormat) VPI_DETAIL_MAKE_YCbCr_FMT
#endif

/** Creates a user-defined YCbCr color image format constant.
 *
 * Example to create a YUV422R ITU-R BT.709 full-range with SMPTE240M transfer function, block-linear format.
 * \code{.c}
 * VPIImageFormat fmt = VPI_MAKE_YCbCr_IMAGE_FORMAT(VPI_MAKE_COLOR_SPEC_ABBREV(BT601, SMPTE240M, FULL),
 *                                                  VPI_CSS_422R, VPI_BLOCK_LINEAR, VPI_DATA_TYPE_UNSIGNED, VPI_SWIZZLE_XYZ0,
 *                                                  2, VPI_PACKING_X8, VPI_PACKING_X8_Y8);
 * \endcode
 *
 * Fourth plane (packing3) must have at most 64bpp.
 *
 * @param[in] colorModel    \ref VPIColorModel to be used.
 * @param[in] colorSpec     \ref VPIColorSpec to be used.
 * @param[in] chromaSubsamp \ref VPIChromaSubsampling to be used.
 * @param[in] memLayout     \ref VPIMemLayout to be used.
 * @param[in] dataType      \ref VPIDataType to be used.
 * @param[in] swizzle       \ref VPISwizzle operation to be performed on the channels.
 * @param[in] numPlanes     Number of planes this format has.
 * @param[in] packing0,packing1,... Format packing of each plane.
 *
 * @returns The user-defined image format.
 */
#ifdef VPI_DOXYGEN
#    define VPI_MAKE_YCbCr_IMAGE_FORMAT(colorModel, colorSpec, chromaSubsamp, memLayout, dataType, swizzle, numPlanes, \
                                        packing0, packing1, ...)
#else
#    define VPI_MAKE_YCbCr_IMAGE_FORMAT (VPIImageFormat) VPI_DETAIL_MAKE_YCbCr_FORMAT
#endif

/** Creates a user-defined color image format constant using abbreviated parameters.
 *
 * This macro allows passing abbreviated format parameters (without the parameter type prefix).
 * Example to create a RGB planar ITU-R BT.709 studio-range block-linear format.
 * \code{.c}
 * VPIImageFormat fmt = VPI_MAKE_COLOR_IMAGE_FORMAT_ABBREV(RGB, BT709, BL, UNSIGNED, XYZ0, 3, X8, X8, Y8);
 * \endcode
 *
 * If the color model is \ref VPI_COLOR_MODEL_YCbCr, it's assumed that the chroma subsampling is 4:4:4,
 * i.e, \ref VPI_CSS_444.
 *
 * @param[in] colorModel \ref VPIColorModel to be used, without the VPI_COLOR_MODEL_ prefix.
 * @param[in] colorSpec  \ref VPIColorSpec to be used, without the VPI_COLOR_SPEC_ prefix.
 * @param[in] memLayout  \ref VPIMemLayout to be used, without the VPI_MEM_LAYOUT_ prefix.
 * @param[in] dataType   \ref VPIDataType to be used, without the VPI_DATA_TYPE_ prefix.
 * @param[in] swizzle    \ref VPISwizzle operation to be performed on the channels, without the VPI_SWIZZLE_ prefix.
 * @param[in] numPlanes  Number of planes this format has.
 * @param[in] packing0,packing1,... Format packing of each plane, without the VPI_PACKING_ prefix.
 *                                    + Fourth plane (packing3) must have at most 64bpp.
 *
 * @returns The user-defined image format.
 */
#ifdef VPI_DOXYGEN
#    define VPI_MAKE_COLOR_IMAGE_FORMAT_ABBREV(colorModel, colorSpec, memLayout, dataType, swizzle, numPlanes, \
                                               packing0, packing1, ...)
#else
#    define VPI_MAKE_COLOR_IMAGE_FORMAT_ABBREV (VPIImageFormat) VPI_DETAIL_MAKE_COLOR_FMT
#endif

/** Creates a user-defined color image format constant.
 *
 * Example to create a RGB planar ITU-R BT.709 full-range with SMPTE240M encoding, block-linear format.
 * \code{.c}
 * VPIImageFormat fmt = VPI_MAKE_COLOR_IMAGE_FORMAT(VPI_COLOR_MODEL_RGB, VPI_MAKE_COLOR_SPEC_ABBREV(BT601, SMPTE240M, FULL),
 *                                                  VPI_MEM_LAYOUT_BL, VPI_DATA_TYPE_UNSIGNED, VPI_SWIZZLE_XYZ0,
 *                                                  2, VPI_PACKING_X8, VPI_PACKING_X8, VPI_PACKING_Y8);
 * \endcode
 *
 * If the color model is \ref VPI_COLOR_MODEL_YCbCr, it's assumed that the chroma subsampling is 4:4:4,
 * i.e, \ref VPI_CSS_444.
 *
 * @param[in] colorModel \ref VPIColorModel to be used.
 * @param[in] colorSpec  \ref VPIColorSpec to be used.
 * @param[in] memLayout  \ref VPIMemLayout to be used.
 * @param[in] dataType   \ref VPIDataType to be used.
 * @param[in] swizzle    \ref VPISwizzle operation to be performed on the channels.
 * @param[in] numPlanes  Number of planes this format has.
 * @param[in] packing0,packing1,... Format packing of each plane.
 *                                    + Fourth plane (packing3) must have at most 64bpp.
 *
 * @returns The user-defined image format.
 */
#ifdef VPI_DOXYGEN
#    define VPI_MAKE_COLOR_IMAGE_FORMAT(colorModel, colorSpec, memLayout, dataType, swizzle, numPlanes, packing0, \
                                        packing1, ...)
#else
#    define VPI_MAKE_COLOR_IMAGE_FORMAT (VPIImageFormat) VPI_DETAIL_MAKE_COLOR_FORMAT
#endif

/** Creates a user-defined non-color image format constant using abbreviated parameters.
 *
 * This macro allows passing abbreviated format parameters (without the parameter type prefix).
 *
 * Example to create 3-plane float block-linear image, 1st: 8-bit, 2nd: 16-bit, 3rd: 32-bit
 * \code{.c}
 * VPIImageFormat fmt = VPI_MAKE_NONCOLOR_IMAGE_FORMAT_ABBREV(BL, UNSIGNED, 3, X8, X16, X32);
 * \endcode
 *
 * @param[in] memLayout \ref VPIMemLayout to be used, without the VPI_MEM_LAYOUT_ prefix.
 * @param[in] dataType  \ref VPIDataType to be used, without the VPI_DATA_TYPE_ prefix.
 * @param[in] swizzle   \ref VPISwizzle operation to be performed on the channels.
 * @param[in] numPlanes Number of planes this format has.
 * @param[in] packing0,packing1,... Format packing of each plane, without the VPI_PACKING_ prefix.
 *                                    + Fourth plane (packing3) must have at most 64bpp.
 *
 * @returns The user-defined image format.
 */
#ifdef VPI_DOXYGEN
#    define VPI_MAKE_NONCOLOR_IMAGE_FORMAT_ABBREV(memLayout, dataType, swizzle, numPlanes, packing0, packing1, ...)
#else
#    define VPI_MAKE_NONCOLOR_IMAGE_FORMAT_ABBREV (VPIImageFormat) VPI_DETAIL_MAKE_NONCOLOR_FMT
#endif

/** Creates a user-defined non-color image format constant.
 *
 * Example to create 3-plane float block-linear image, 1st: 8-bit, 2nd: 16-bit, 3rd: 32-bit
 * \code{.c}
 * VPIImageFormat fmt = VPI_MAKE_NONCOLOR_IMAGE_FORMAT(VPI_MEM_LAYOUT_BL, VPI_DATA_TYPE_UNSIGNED,
 *                                                    3, VPI_PACKING_X8, VPI_PACKING_X16, VPI_PACKING_X32);
 * \endcode
 *
 * @param[in] memLayout \ref VPIMemLayout to be used.
 * @param[in] dataType  \ref VPIDataType to be used.
 * @param[in] swizzle   \ref VPISwizzle operation to be performed on the channels.
 * @param[in] numPlanes Number of planes this format has.
 * @param[in] packing0,packing1,... Format packing of each plane.
 *                                    + Fourth plane (packing3) must have at most 64bpp.
 *
 * @returns The user-defined image format.
 */
#ifdef VPI_DOXYGEN
#    define VPI_MAKE_NONCOLOR_IMAGE_FORMAT(memLayout, dataType, swizzle, numPlanes, packing0, packing1, ...)
#else
#    define VPI_MAKE_NONCOLOR_IMAGE_FORMAT (VPIImageFormat) VPI_DETAIL_MAKE_NONCOLOR_FORMAT
#endif

/** Creates a user-defined raw (Bayer pattern) image format constant using abbreviated parameters.
 *
 * This macro allows passing abbreviated format parameters (without the parameter type prefix).
 *
 * Example to create a RGGB Bayer pattern format:
 * \code{.c}
 * VPIImageFormat fmt = VPI_MAKE_RAW_IMAGE_FORMAT_ABBREV(BAYER_RGGB, BL, UNSIGNED, X000, 1, X8);
 * \endcode
 *
 * @param[in] rawPattern \ref VPIRawPattern to be used, without the VPI_RAW_ prefix.
 * @param[in] memLayout  \ref VPIMemLayout to be used, without the VPI_MEM_LAYOUT_ prefix.
 * @param[in] dataType   \ref VPIDataType to be used, without the VPI_DATA_TYPE_ prefix.
 * @param[in] swizzle    \ref VPISwizzle operation to be performed on the channels, without the VPI_SWIZZLE_ prefix.
 * @param[in] numPlanes  Number of planes this format has.
 * @param[in] packing    Format packing of image plane plane, without the VPI_PACKING_ prefix.
 *
 * @returns The user-defined image format.
 */
#ifdef VPI_DOXYGEN
#    define VPI_MAKE_RAW_IMAGE_FORMAT_ABBREV(rawPattern, memLayout, dataType, numPlanes, swizzle, packing)
#else
#    define VPI_MAKE_RAW_IMAGE_FORMAT_ABBREV (VPIImageFormat) VPI_DETAIL_MAKE_RAW_FMT
#endif

/** Creates a user-defined raw (Bayer pattern) image format constant.
 *
 * Example to create a RGGB Bayer pattern format:
 * \code{.c}
 * VPIImageFormat fmt = VPI_MAKE_RAW_IMAGE_FORMAT(VPI_RAW_BAYER_RGGB, VPI_MEM_LAYOUT_BL,
 *                                                VPI_DATA_TYPE_UNSIGNED, VPI_SWIZZLE_X000,
 *                                                1, VPI_PACKING_X8);
 * \endcode
 *
 * @param[in] rawPattern \ref VPIRawPattern to be used.
 * @param[in] memLayout  \ref VPIMemLayout to be used.
 * @param[in] dataType   \ref VPIDataType to be used.
 * @param[in] swizzle    \ref VPISwizzle operation to be performed on the channels.
 * @param[in] numPlanes  Number of planes this format has.
 * @param[in] packing    Format packing of image plane.
 *
 * @returns The user-defined image format.
 */
#ifdef VPI_DOXYGEN
#    define VPI_MAKE_RAW_IMAGE_FORMAT(rawPattern, memLayout, dataType, numPlanes, swizzle, packing)
#else
#    define VPI_MAKE_RAW_IMAGE_FORMAT (VPIImageFormat) VPI_DETAIL_MAKE_RAW_FORMAT
#endif

/** Creates a user-defined YCbCr color image format.
 *
 * When the pre-defined image formats aren't enough, user-defined image formats can be created.
 * @warning It's not guaranteed that algorithms will work correctly with use-defined image formats. It's recommended
 * to check if the results are correct prior deploying the solution in a production environment.
 *
 * Fourth plane (packing3) must have at most 64bpp.
 *
 * @param[in] colorSpec \ref VPIColorSpec to be used.
 * @param[in] chromaSub \ref VPIChromaSubsampling to be used.
 * @param[in] memLayout \ref VPIMemLayout to be used.
 * @param[in] dataType  \ref VPIDataType to be used.
 * @param[in] swizzle   \ref VPISwizzle operation to be performed on the channels.
 * @param[in] packing0,packing1,packing2,packing3 Format packing of each plane. 
 *                                                + When remaining planes aren't needed,
 *                                                  pass \ref VPI_PACKING_0 for them.
 *
 * @returns The user-defined image format.
 * @retval #VPI_IMAGE_FORMAT_INVALID Swizzle packing is invalid.
 * @retval #VPI_IMAGE_FORMAT_INVALID \p colorSpec and \p chromaSub are undefined.
 */
VPI_PUBLIC VPIImageFormat vpiMakeYCbCrImageFormat(VPIColorSpec colorSpec, VPIChromaSubsampling chromaSub,
                                                  VPIMemLayout memLayout, VPIDataType dataType, VPISwizzle swizzle,
                                                  VPIPacking packing0, VPIPacking packing1, VPIPacking packing2,
                                                  VPIPacking packing3);

/** Creates a user-defined color image format.
 *
 * When the pre-defined image formats aren't enough, user-defined image formats can be created.
 * @warning It's not guaranteed that algorithms will work correctly with use-defined image formats. It's recommended
 * to check if the results are correct prior deploying the solution in a production environment.
 *
 * If the color model is \ref VPI_COLOR_MODEL_YCbCr, it's assumed that the chroma subsampling is 4:4:4,
 * i.e, \ref VPI_CSS_444.
 *
 * @param[in] colorModel \ref VPIColorModel to be used.
 * @param[in] colorSpec  \ref VPIColorSpec to be used.
 * @param[in] memLayout  \ref VPIMemLayout to be used.
 * @param[in] dataType   \ref VPIDataType to be used.
 * @param[in] swizzle    \ref VPISwizzle operation to be performed on the channels.
 * @param[in] packing0,packing1,packing2,packing3 Format packing of each plane. 
 *                                                + When remaining planes aren't needed, pass \ref VPI_PACKING_0 for them.
 *                                                + Fourth plane (packing3) must have at most 64bpp.
 *
 * @returns The user-defined image format.
 * @retval #VPI_IMAGE_FORMAT_INVALID \p colorModel is \ref VPI_COLOR_MODEL_UNDEFINED . 
 * @retval #VPI_IMAGE_FORMAT_INVALID \p colorModel is \ref VPI_COLOR_MODEL_RAW .
 * @retval #VPI_IMAGE_FORMAT_INVALID \p colorSpec is \ref VPI_COLOR_SPEC_UNDEFINED .
 * @retval #VPI_IMAGE_FORMAT_INVALID Swizzle packing is invalid.
 * @retval #VPI_IMAGE_FORMAT_INVALID \p colorModel, \p colorSpec and \p chromaSub are undefined.
 * @retval #VPI_IMAGE_FORMAT_INVALID Image format invalid; 4th plane cannot have a 128 bit channel.
 */
VPI_PUBLIC VPIImageFormat vpiMakeColorImageFormat(VPIColorModel colorModel, VPIColorSpec colorSpec,
                                                  VPIMemLayout memLayout, VPIDataType dataType, VPISwizzle swizzle,
                                                  VPIPacking packing0, VPIPacking packing1, VPIPacking packing2,
                                                  VPIPacking packing3);

/** Creates a user-defined non-color image format.
 *
 * When the pre-defined non-color image formats aren't enough, it is possible to define new ones.
 *
 * @warning It's not guaranteed that algorithms will work correctly with use-defined image formats. It's recommended
 * to check if the results are correct prior deploying the solution in a production environment.
 *
 * @param[in] memLayout \ref VPIMemLayout to be used.
 * @param[in] dataType  \ref VPIDataType to be used.
 * @param[in] swizzle   \ref VPISwizzle operation to be performed on the channels.
 * @param[in] packing0,packing1,packing2,packing3 Format packing of each plane. 
 *                                                + When remaining planes aren't needed, pass \ref VPI_PACKING_0 for them.
 *                                                + Fourth plane (packing3) must have at most 64bpp.
 *
 * @returns The user-defined image format.
 *
 * @retval #VPI_IMAGE_FORMAT_INVALID Image format to be queried is invalid.
 * @retval #VPI_IMAGE_FORMAT_INVALID Swizzle packing is invalid. 
 */
VPI_PUBLIC VPIImageFormat vpiMakeNonColorImageFormat(VPIMemLayout memLayout, VPIDataType dataType, VPISwizzle swizzle,
                                                     VPIPacking packing0, VPIPacking packing1, VPIPacking packing2,
                                                     VPIPacking packing3);

/** Creates a user-defined raw image format.
 *
 * When the pre-defined raw image formats aren't enough, it is possible to define new ones.
 * @warning It's not guaranteed that algorithms will work correctly with use-defined image formats. It's recommended
 * to check if the results are correct prior deploying the solution in a production environment.
 *
 * @param[in] rawPattern \ref VPIRawPattern to be used.
 * @param[in] memLayout  \ref VPIMemLayout to be used.
 * @param[in] dataType   \ref VPIDataType to be used.
 * @param[in] swizzle    \ref VPISwizzle operation to be performed on the channels.
 * @param[in] packing0,packing1,packing2,packing3 Format packing of each plane. 
 *                                                + When remaining planes aren't needed, pass \ref VPI_PACKING_0 for them.
 *                                                + Fourth plane (packing3) must have at most 64bpp.
 *
 * @returns The user-defined image format.
 *
 * @retval #VPI_IMAGE_FORMAT_INVALID Image format to be queried is invalid. 
 * @retval #VPI_IMAGE_FORMAT_INVALID \p rawPattern is invalid. 
 * @retval #VPI_IMAGE_FORMAT_INVALID Swizzle packing is invalid. 
 */
VPI_PUBLIC VPIImageFormat vpiMakeRawImageFormat(VPIRawPattern rawPattern, VPIMemLayout memLayout, VPIDataType dataType,
                                                VPISwizzle swizzle, VPIPacking packing0, VPIPacking packing1,
                                                VPIPacking packing2, VPIPacking packing3);

/** Creates a image format from a FourCC code.
 *
 * See https://www.fourcc.org for more information about FourCC.
 *
 * @param[in] fourcc FourCC code.
 * @param[in] colorSpec \ref VPIColorSpec to be used.
 * @param[in] memLayout \ref VPIMemLayout to be used.
 *
 * @returns The image format corresponding to the FourCC code.
 *
 * @retval #VPI_IMAGE_FORMAT_INVALID Invalid \p fourcc. 
 */
VPI_PUBLIC VPIImageFormat vpiMakeImageFormatFromFourCC(uint32_t fourcc, VPIColorSpec colorSpec, VPIMemLayout memLayout);

/** Returns the FourCC code corresponding to an image format.
 *
 * @param[in] fmt Image format to be queried.
 *
 * @returns The FourCC code corresponding to the image format.
 *
 * @retval 0 No FourCC associated with \p fmt, e.g. \ref VPI_IMAGE_FORMAT_2F32.
 */
VPI_PUBLIC uint32_t vpiImageFormatGetFourCC(VPIImageFormat fmt);

/** Get the packing for a given plane of an image format.
 *
 * @param[in] fmt   Image format to be queried.
 * @param[in] plane Which plane whose packing must be returned. 
 *                  + Valid values range from 0 (first plane) to 3 (fourth plane).
 *
 * @returns The plane's format packing.
 *
 * @retval #VPI_PACKING_0 \p fmt to be queried is invalid. 
 * @retval #VPI_PACKING_0 \p plane outside valid range.
 */
VPI_PUBLIC VPIPacking vpiImageFormatGetPlanePacking(VPIImageFormat fmt, int plane);

/** Get the plane width of an image with the given image format and width.
 *
 * @param[in] fmt      Image format to be queried.
 * @param[in] imgWidth Width of the image.
 *                      + Must be >= 1.
 * @param[in] plane    Image plane to be queried.
 *                      + Must be >= 0 and < the number of planes in the image format.
 *
 * @returns The image plane's width.
 *
 * @retval 0 \p fmt is invalid.
 * @retval 0 \p imgWidth is outside valid range.
 * @retval 0 \p plane is outside valid range.
 */
VPI_PUBLIC int32_t vpiImageFormatGetPlaneWidth(VPIImageFormat fmt, int32_t imgWidth, int plane);

/** Get the plane height of an image with the given image format and height.
 *
 * @param[in] fmt      Image format to be queried.
 * @param[in] imgHeight Height of the image.
 *                      + Must be >= 1.
 * @param[in] plane    Image plane to be queried.
 *                      + Must be >= 0 and < the number of planes in the image format.
 *
 * @returns The image plane's height.
 *
 * @retval 0 \p fmt is invalid.
 * @retval 0 \p imgHeight is outside valid range.
 * @retval 0 \p plane is outside valid range.
 */
VPI_PUBLIC int32_t vpiImageFormatGetPlaneHeight(VPIImageFormat fmt, int32_t imgHeight, int plane);

/** Replaces the swizzle and packing of an existing image format.
 *
 * The number of channels represented by the swizzle must be equal to the sum of the number of channels
 * represented by the packings. For instance, XYZ1,X8,X8Y8 is a valid combination with 3 channels.
 * XYZW,X8,X8Y8 isn't as swizzle has 4 channels, and X8,X8Y8 represents in total 3 channels.
 *
 * @param[in] fmt     Image format to have its packing replaced.
 * @param[in] swizzle The new swizzle.
 *                    + Pass \ref VPI_SWIZZLE_INVALID if swizzle must not be updated.
 * 
 * @param[in] packing0,packing1,packing2,packing3 New packing.
 *                                                + Pass \ref VPI_PACKING_INVALID if the packing for
 *                                                  a particular plane must not be updated.
 *                                                + If replacing the fourth packing (packing3),
 *                                                  the packing's bits per pixel must be at most 64.
 *
 * 
 * @returns Updated image format.
 *
 * @retval #VPI_IMAGE_FORMAT_INVALID \p fmt to be queried is invalid. 
 */
VPI_PUBLIC VPIImageFormat vpiImageFormatSetSwizzleAndPacking(VPIImageFormat fmt, VPISwizzle swizzle,
                                                             VPIPacking packing0, VPIPacking packing1,
                                                             VPIPacking packing2, VPIPacking packing3);

/** Get the image format's plane bits per pixel count.
 *
 * @param[in] fmt   Image format to be queried.
 * @param[in] plane Which plane is to be queried.
 *                  + Valid values range from 0 (first plane) to 3 (fourth plane).
 *
 * @returns The number of bits per pixel the given format plane has.
 *
 * @retval 0 \p fmt is invalid.
 * @retval 0 \p plane outside valid range.
 */
VPI_PUBLIC int vpiImageFormatGetPlaneBitsPerPixel(VPIImageFormat fmt, int plane);

/** Set the image format's data type.
 *
 * @param[in] fmt      Image format have its data type replaced.
 * @param[in] dataType The new data type.
 *
 * @returns The new image format based on input's, but with the user-provided data type.
 *
 * @retval #VPI_IMAGE_FORMAT_INVALID \p fmt to be queried is invalid. 
 * @retval #VPI_IMAGE_FORMAT_INVALID \p dataType is invalid.
 */
VPI_PUBLIC VPIImageFormat vpiImageFormatSetDataType(VPIImageFormat fmt, VPIDataType dataType);

/** Get the image format's data type.
 *
 * @param[in] fmt Image format to be queried.
 *
 * @returns The image format's data type.
 *
 * @retval #VPI_DATA_TYPE_INVALID \p fmt to be queried is invalid. 
 */
VPI_PUBLIC VPIDataType vpiImageFormatGetDataType(VPIImageFormat fmt);

/** Get the image format's channel swizzle operation.
 *
 * @param[in] fmt Image format to be queried.
 *
 * @returns The image format's swizzle operation.
 *
 * @retval #VPI_SWIZZLE_0000 \p fmt to be queried is invalid. 
 */
VPI_PUBLIC VPISwizzle vpiImageFormatGetSwizzle(VPIImageFormat fmt);

/** Get the swizzle operation of the given image format's plane.
 *
 * @param[in] fmt   Image format to be queried.
 *                  + Image format should have less packing channels and swizzle channels.
 * @param[in] plane Plane to be queried. 
 *                  + Valid values range from 0 (first) to 3 (fourth and last) plane.
 *
 * @returns The channel swizzle operation performed in the given plane.
 *
 * @retval #VPI_SWIZZLE_0000    \p fmt to be queried is invalid.
 * @retval #VPI_SWIZZLE_INVALID More packing channels than Swizzle channels.
 */
VPI_PUBLIC VPISwizzle vpiImageFormatGetPlaneSwizzle(VPIImageFormat fmt, int plane);

/** Set the image format's memory layout.
 *
 * @param[in] fmt       Image format have its memory layout replaced.
 * @param[in] memLayout The new memory layout.
 *
 * @returns The new image format based on input's, but with the user-provided memory layout.
 *
 * @retval #VPI_IMAGE_FORMAT_INVALID \p fmt to be queried is invalid. 
 * @retval #VPI_IMAGE_FORMAT_INVALID \p memLayout is invalid.
 */
VPI_PUBLIC VPIImageFormat vpiImageFormatSetMemLayout(VPIImageFormat fmt, VPIMemLayout memLayout);

/** Get the image format's memory layout.
 *
 * @param[in] fmt Image format to be queried.
 *
 * @returns The image format's memory layout.
 *
 * @retval #VPI_MEM_LAYOUT_INVALID \p fmt to be queried is invalid. 
 */
VPI_PUBLIC VPIMemLayout vpiImageFormatGetMemLayout(VPIImageFormat fmt);

/** Set the image format's color standard.
 *
 * Only valid for color models that represent image coding systems, such as RGB, Y'CrCb, HSV, etc.
 * For other color models, it'll return VPI_IMAGE_FORMAT_INVALID.
 *
 * @param[in] fmt       Image format have its color spec replaced.
 * @param[in] colorSpec The new color standard.
 *
 * @returns The new image format based on input's, but with the user-provided color spec.
 *
 * @retval #VPI_IMAGE_FORMAT_INVALID \p fmt to be queried is invalid. 
 * @retval #VPI_IMAGE_FORMAT_INVALID \p colorSpec is invalid.
 * @retval #VPI_IMAGE_FORMAT_INVALID Color standard isn't applicable to format's color model..
 */
VPI_PUBLIC VPIImageFormat vpiImageFormatSetColorSpec(VPIImageFormat fmt, VPIColorSpec colorSpec);

/** Get the image format's color standard.
 *
 * @param[in] fmt Image format to be queried.
 *
 * @returns The image format's color standard.
 *
 * @retval #VPI_COLOR_SPEC_INVALID   \p fmt to be queried is invalid. 
 * @retval #VPI_COLOR_SPEC_UNDEFINED It's not applicable, i.e., format model isn't RGB, YCbCr, HSV or HSL.
 */
VPI_PUBLIC VPIColorSpec vpiImageFormatGetColorSpec(VPIImageFormat fmt);

/** Get the image format's color model.
 *
 * @param[in] fmt Image format to be queried.
 *
 * @returns The image format's color model.
 *
 * @retval #VPI_COLOR_MODEL_UNDEFINED \p fmt to be queried is invalid. 
 */
VPI_PUBLIC VPIColorModel vpiImageFormatGetColorModel(VPIImageFormat fmt);

/** Set the image format's chroma subsampling type.
 *
 *
 * @param[in] fmt Image format have its chroma subsampling type replaced.
 *                + It's only applicable if format has YCbCr color model.
 * @param[in] css The new chroma subsampling type.
 *
 * @returns The new image format based on input's, but with the user-provided chroma subsampling type.
 *
 * @retval #VPI_IMAGE_FORMAT_INVALID \p fmt to be queried is invalid. 
 * @retval #VPI_IMAGE_FORMAT_INVALID \p css is invalid.
 * @retval #VPI_CSS_NONE             Format's color model is NOT \ref VPI_COLOR_MODEL_YCbCr.
 */
VPI_PUBLIC VPIImageFormat vpiImageFormatSetChromaSubsampling(VPIImageFormat fmt, VPIChromaSubsampling css);

/** Get the image format's chroma subsampling type.
 *
 * @param[in] fmt Image format to be queried.
 *
 * @returns The image format's chroma subsampling type.
 * @retval #VPI_CSS_INVALID \p fmt is not supported.
 * @retval #VPI_CSS_NONE \p fmt color model is NOT \ref VPI_COLOR_MODEL_YCbCr.
 */
VPI_PUBLIC VPIChromaSubsampling vpiImageFormatGetChromaSubsampling(VPIImageFormat fmt);

/** Get the number of channels in a plane of an image format.
 *
 * @param[in] fmt Image format to be queried.
 * 
 * @param[in] plane Plane to be queried. Valid values range from 0 (first) to 3 (fourth and last) plane.
 *
 * @returns Number of channels in the given plane.
 */
VPI_PUBLIC int vpiImageFormatGetPlaneChannelCount(VPIImageFormat fmt, int plane);

/** Get the number of planes of an image format.
 *
 * @param[in] fmt Image format to be queried.
 *
 * @returns Number of planes defined by the given image format.
 */
VPI_PUBLIC int vpiImageFormatGetPlaneCount(VPIImageFormat fmt);

/** Get the total number of channels of an image format.
 *
 * @param[in] fmt Image format to be queried.
 *
 * @returns The sum of all channel counts in all planes.
 */
VPI_PUBLIC int vpiImageFormatGetChannelCount(VPIImageFormat fmt);

/** Get the image format's bit size for each channel.
 *
 * @param[in] fmt Image format to be queried.
 * 
 * @param[out] bits Pointer to an int32_t array with 4 elements where output will be stored.
 *                  + If it's NULL, the function is a no-op.
 */
VPI_PUBLIC void vpiImageFormatGetBitsPerChannel(VPIImageFormat fmt, int32_t *bits);

/** Get the pixel type of image format's plane.
 *
 * @param[in] fmt   Image format to be queried.
 * @param[in] plane Plane to be queried.
 *                  + Valid values range from 0 (first) to 3 (fourth and last) plane.
 *
 * @returns The pixel type of the given plane.
 *
 * @retval #VPI_PIXEL_TYPE_INVALID \p fmt to be queried is invalid. 
 * @retval #VPI_PIXEL_TYPE_INVALID \p plane is outside range.
 */
VPI_PUBLIC VPIPixelType vpiImageFormatGetPlanePixelType(VPIImageFormat fmt, int plane);

/** Get the plane format of an image format.
 *
 * @param[in] fmt   Image format to be queried.
 * @param[in] plane Plane to be queried. 
 *                  + Valid values range from 0 (first) to 3 (fourth and last) plane.
 *
 * @returns The image format of the given plane.
 *
 * @retval #VPI_IMAGE_FORMAT_INVALID \p fmt packing is invalid. 
 * @retval #VPI_IMAGE_FORMAT_INVALID \p plane is outside range.
 */
VPI_PUBLIC VPIImageFormat vpiImageFormatGetPlaneFormat(VPIImageFormat fmt, int plane);

/** Constructs an image format given the format of each plane.
 *
 * @param[in] plane0,plane1,plane2,plane3 Image format of each plane.
 *                                        + When plane doesn't exist, pass \ref VPI_IMAGE_FORMAT_INVALID.
 *                                        + All plane types must have only 1 plane.
 *                                        + First plane must have a valid packing.
 *                                        + Total number of channels must be at most 4.
 *                                        + Color spec, mem layout and data type of all planes must be the same.
 *                                        + Only one kind of chroma subsampling is allowed.
 *                                        + At least one channel is allowed.
 *                                        + All planes after the first invalid one must be invalid.
 *
 * @returns The image format whose planes has the given formats.
 *
 * @retval #VPI_IMAGE_FORMAT_INVALID Image format to be queried is invalid.
 * @retval #VPI_IMAGE_FORMAT_INVALID All plane types must have only 1 plane.
 * @retval #VPI_IMAGE_FORMAT_INVALID First plane must have a valid packing.
 * @retval #VPI_IMAGE_FORMAT_INVALID Total number of channels must be atmost 4.
 * @retval #VPI_IMAGE_FORMAT_INVALID Color spec, mem layout and data type of all planes must be the same.
 * @retval #VPI_IMAGE_FORMAT_INVALID Only one kind of chroma subsampling is allowed.
 * @retval #VPI_IMAGE_FORMAT_INVALID At least one channel is allowed.
 * @retval #VPI_IMAGE_FORMAT_INVALID All planes after the first invalid one must be invalid.
 */
VPI_PUBLIC VPIImageFormat vpiMakeImageFormatFromPlanes(VPIImageFormat plane0, VPIImageFormat plane1,
                                                       VPIImageFormat plane2, VPIImageFormat plane3);

/** Returns a string representation of the image format.
 *
 * @param[in] fmt Image format whose name is to be returned.
 *
 * @returns The string representation of the image format.
 *          Returned string is valid until next call of this function from the same calling thread.
 *          Returned pointer must not be freed.
 */
VPI_PUBLIC const char *vpiImageFormatGetName(VPIImageFormat fmt);

/** Returns the raw color pattern of the image format.
 *
 * @param[in] fmt Image format to be queried.
 *                + Its color model must be \ref VPI_COLOR_MODEL_RAW .
 * 
 * @returns The raw pattern of given raw image format.
 *
 * @retval #VPI_RAW_INVALID Color model of \p fmt is not \ref VPI_COLOR_MODEL_RAW. 
 */
VPI_PUBLIC VPIRawPattern vpiImageFormatGetRawPattern(VPIImageFormat fmt);

/** Sets the raw color pattern of the image format.
 *
 * @param[in] fmt        Image format to be updated.
 *                       + Its color model must be \ref VPI_COLOR_MODEL_RAW.
 * @param[in] rawPattern The new raw pattern.
 * 
 * @returns The new image format with the updated raw pattern will be returned.
 *
 * @retval #VPI_IMAGE_FORMAT_INVALID \p fmt to be queried is invalid or its color model isn't \ref VPI_COLOR_MODEL_RAW.
 * @retval #VPI_IMAGE_FORMAT_INVALID \p rawPattern is invalid.
 */
VPI_PUBLIC VPIImageFormat vpiImageFormatSetRawPattern(VPIImageFormat fmt, VPIRawPattern rawPattern);

/** Returns whether the image formats have the same data layout.
 *
 * Data layout referts to how pixels are laid out in memory. It doesn't take into account
 * the format's color information.
 *
 * The following characteristics are taken into account:
 * - memory layout (block linear, pitch linear, ...)
 * - data type (signed, unsigned, float, ...)
 * - Swizzle (except for 1/0 in 4th channel)
 * - number of planes
 * - packings (X8_Y8, X16, ...)
 * - chroma subsampling (4:4:4, 4:2:0, ...)
 *
 * @param[in] a, b Image formats to be compared.
 * 
 * @retval 1 Both image formats compares equal with respect to how pixels are laid out in memory.
 * @retval 0 Both image formats compares differently.
 */
VPI_PUBLIC int vpiImageFormatHasSameDataLayout(VPIImageFormat a, VPIImageFormat b);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* NV_VPI_IMAGE_FORMAT_H */
