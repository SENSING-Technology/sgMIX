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
 * @file PixelType.h
 *
 * Defines types and functions to handle pixel types.
 */

#ifndef NV_VPI_PIXEL_TYPE_H
#define NV_VPI_PIXEL_TYPE_H

#include "DataLayout.h"
#include "detail/FormatUtils.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @ingroup VPI_ImageFormat
 * @{
 */

/**
 * Pre-defined pixel types.
 * Pixel types defines the geometry of pixels in a image plane without taking into account what the value represents.
 * For example, a \ref VPI_IMAGE_FORMAT_NV12 is composed of 2 planes, each one with the following pixel types:
 * + \ref VPI_PIXEL_TYPE_U8 representing pixels as 8-bit unsigned values.
 * + \ref VPI_PIXEL_TYPE_2U8 representing pixels as two interleaved 32-bit floating-point values.
 */
typedef uint64_t VPIPixelType;

/* clang-format off */

/** Used to signal that the pixel type must be inferred from image format. */
#define VPI_PIXEL_TYPE_DEFAULT ((VPIPixelType)0)
/** Signal format conversion errors. */
#define VPI_PIXEL_TYPE_INVALID VPI_PIXEL_TYPE_DEFAULT

/** One channel of unsigned 8-bit value. */
#define VPI_PIXEL_TYPE_U8   VPI_DETAIL_MAKE_PIX_TYPE(PL, UNSIGNED, X8)
/** Two interleaved channels of unsigned 8-bit values. */
#define VPI_PIXEL_TYPE_2U8  VPI_DETAIL_MAKE_PIX_TYPE(PL, UNSIGNED, X8_Y8)
/** Three interleaved channels of unsigned 8-bit values. */
#define VPI_PIXEL_TYPE_3U8  VPI_DETAIL_MAKE_PIX_TYPE(PL, UNSIGNED, X8_Y8_Z8)
/** Four interleaved channels of unsigned 8-bit values. */
#define VPI_PIXEL_TYPE_4U8  VPI_DETAIL_MAKE_PIX_TYPE(PL, UNSIGNED, X8_Y8_Z8_W8)

/** One channel of signed 8-bit value. */
#define VPI_PIXEL_TYPE_S8   VPI_DETAIL_MAKE_PIX_TYPE(PL, SIGNED, X8)
/** Two interleaved channels of signed 8-bit values. */
#define VPI_PIXEL_TYPE_2S8  VPI_DETAIL_MAKE_PIX_TYPE(PL, SIGNED, X8_Y8)
/** Three interleaved channels of signed 8-bit values. */
#define VPI_PIXEL_TYPE_3S8  VPI_DETAIL_MAKE_PIX_TYPE(PL, SIGNED, X8_Y8_Z8)
/** Four interleaved channels of signed 8-bit values. */
#define VPI_PIXEL_TYPE_4S8  VPI_DETAIL_MAKE_PIX_TYPE(PL, SIGNED, X8_Y8_Z8_W8)

/** One channel of unsigned 16-bit value. */
#define VPI_PIXEL_TYPE_U16  VPI_DETAIL_MAKE_PIX_TYPE(PL, UNSIGNED, X16)
/** Two interleaved channels of unsigned 16-bit values. */
#define VPI_PIXEL_TYPE_2U16 VPI_DETAIL_MAKE_PIX_TYPE(PL, UNSIGNED, X16_Y16)
/** Three interleaved channels of unsigned 16-bit values. */
#define VPI_PIXEL_TYPE_3U16 VPI_DETAIL_MAKE_PIX_TYPE(PL, UNSIGNED, X16_Y16_Z16)
/** Four interleaved channels of unsigned 16-bit values. */
#define VPI_PIXEL_TYPE_4U16 VPI_DETAIL_MAKE_PIX_TYPE(PL, UNSIGNED, X16_Y16_Z16_W16)

/** One channel of signed 16-bit value. */
#define VPI_PIXEL_TYPE_S16  VPI_DETAIL_MAKE_PIX_TYPE(PL, SIGNED, X16)
/** Two interleaved channels of signed 16-bit values. */
#define VPI_PIXEL_TYPE_2S16 VPI_DETAIL_MAKE_PIX_TYPE(PL, SIGNED, X16_Y16)
/** Three interleaved channels of signed 16-bit values. */
#define VPI_PIXEL_TYPE_3S16 VPI_DETAIL_MAKE_PIX_TYPE(PL, SIGNED, X16_Y16_Z16)
/** Four interleaved channels of signed 16-bit values. */
#define VPI_PIXEL_TYPE_4S16 VPI_DETAIL_MAKE_PIX_TYPE(PL, SIGNED, X16_Y16_Z16_W16)

/** One channel of unsigned 32-bit value. */
#define VPI_PIXEL_TYPE_U32  VPI_DETAIL_MAKE_PIX_TYPE(PL, UNSIGNED, X32)
/** Two interleaved channels of unsigned 32-bit values. */
#define VPI_PIXEL_TYPE_2U32 VPI_DETAIL_MAKE_PIX_TYPE(PL, UNSIGNED, X32_Y32)
/** Three interleaved channels of unsigned 32-bit values. */
#define VPI_PIXEL_TYPE_3U32 VPI_DETAIL_MAKE_PIX_TYPE(PL, UNSIGNED, X32_Y32_Z32)
/** Four interleaved channels of unsigned 32-bit values. */
#define VPI_PIXEL_TYPE_4U32 VPI_DETAIL_MAKE_PIX_TYPE(PL, UNSIGNED, X32_Y32_Z32_W32)

/** One channel of signed 32-bit value. */
#define VPI_PIXEL_TYPE_S32  VPI_DETAIL_MAKE_PIX_TYPE(PL, SIGNED, X32)
/** Two interleaved channels of signed 32-bit values. */
#define VPI_PIXEL_TYPE_2S32 VPI_DETAIL_MAKE_PIX_TYPE(PL, SIGNED, X32_Y32)
/** Three interleaved channels of signed 32-bit values. */
#define VPI_PIXEL_TYPE_3S32 VPI_DETAIL_MAKE_PIX_TYPE(PL, SIGNED, X32_Y32_Z32)
/** Four interleaved channels of signed 32-bit values. */
#define VPI_PIXEL_TYPE_4S32 VPI_DETAIL_MAKE_PIX_TYPE(PL, SIGNED, X32_Y32_Z32_W32)

/** One channel of 32-bit IEEE 754 floating-point value. */
#define VPI_PIXEL_TYPE_F32  VPI_DETAIL_MAKE_PIX_TYPE(PL, FLOAT, X32)
/** Two interleaved channels of 32-bit IEEE 754 floating-point values. */
#define VPI_PIXEL_TYPE_2F32 VPI_DETAIL_MAKE_PIX_TYPE(PL, FLOAT, X32_Y32)
/** Three interleaved channels of 32-bit IEEE 754 floating-point values. */
#define VPI_PIXEL_TYPE_3F32 VPI_DETAIL_MAKE_PIX_TYPE(PL, FLOAT, X32_Y32_Z32)
/** Four interleaved channels of 32-bit IEEE 754 floating-point values. */
#define VPI_PIXEL_TYPE_4F32 VPI_DETAIL_MAKE_PIX_TYPE(PL, FLOAT, X32_Y32_Z32_W32)

/** One channel of unsigned 64-bit value. */
#define VPI_PIXEL_TYPE_U64  VPI_DETAIL_MAKE_PIX_TYPE(PL, UNSIGNED, X64)
/** Two interleaved channels of unsigned 64-bit values. */
#define VPI_PIXEL_TYPE_2U64 VPI_DETAIL_MAKE_PIX_TYPE(PL, UNSIGNED, X64_Y64)
/** Three interleaved channels of unsigned 64-bit values. */
#define VPI_PIXEL_TYPE_3U64 VPI_DETAIL_MAKE_PIX_TYPE(PL, UNSIGNED, X64_Y64_Z64)
/** Four interleaved channels of unsigned 64-bit values. */
#define VPI_PIXEL_TYPE_4U64 VPI_DETAIL_MAKE_PIX_TYPE(PL, UNSIGNED, X64_Y64_Z64_W64)

/** One channel of signed 64-bit value. */
#define VPI_PIXEL_TYPE_S64  VPI_DETAIL_MAKE_PIX_TYPE(PL, SIGNED, X64)
/** Two interleaved channels of signed 64-bit values. */
#define VPI_PIXEL_TYPE_2S64 VPI_DETAIL_MAKE_PIX_TYPE(PL, SIGNED, X64_Y64)
/** Three interleaved channels of signed 64-bit values. */
#define VPI_PIXEL_TYPE_3S64 VPI_DETAIL_MAKE_PIX_TYPE(PL, SIGNED, X64_Y64_Z64)
/** Four interleaved channels of signed 64-bit values. */
#define VPI_PIXEL_TYPE_4S64 VPI_DETAIL_MAKE_PIX_TYPE(PL, SIGNED, X64_Y64_Z64_W64)

/** One channel of 64-bit IEEE 754 floating-point value. */
#define VPI_PIXEL_TYPE_F64  VPI_DETAIL_MAKE_PIX_TYPE(PL, FLOAT, X64)
/** Two interleaved channels of 64-bit IEEE 754 floating-point values. */
#define VPI_PIXEL_TYPE_2F64 VPI_DETAIL_MAKE_PIX_TYPE(PL, FLOAT, X64_Y64)
/** Three interleaved channels of 64-bit IEEE 754 floating-point values. */
#define VPI_PIXEL_TYPE_3F64 VPI_DETAIL_MAKE_PIX_TYPE(PL, FLOAT, X64_Y64_Z64)
/** Four interleaved channels of 64-bit IEEE 754 floating-point values. */
#define VPI_PIXEL_TYPE_4F64 VPI_DETAIL_MAKE_PIX_TYPE(PL, FLOAT, X64_Y64_Z64_W64)

/* clang-format on */

/** Creates a user-defined pixel type constant using abbreviated parameters.
 *
 * This macro allows passing abbreviated format parameters (without the parameter type prefix).
 * Example to create a block-linear format two interleaved 32-bit floating point channels:
 * \code{.c}
 *     VPIPixelType type = VPI_MAKE_PIXEL_TYPE_ABBREV(BL, FLOAT, X32_Y32);
 * \endcode
 *
 * @param[in] memLayout \ref VPIMemLayout to be used, without the VPI_MEM_LAYOUT_ prefix.
 * @param[in] dataType  \ref VPIDataType to be used, without the VPI_DATA_TYPE_ prefix.
 * @param[in] packing   Format packing used, which also defines the number of channels, without the VPI_PACKING_ prefix.
 *
 * @returns The user-defined pixel type.
 */
#ifdef VPI_DOXYGEN
#    define VPI_MAKE_PIXEL_TYPE_ABBREV(memLayout, dataType, packing)
#else
#    define VPI_MAKE_PIXEL_TYPE_ABBREV (VPIPixelType) VPI_DETAIL_MAKE_PIX_TYPE
#endif

/** Creates a user-defined pixel type constant.
 *
 * Example to create a block-linear format two interleaved 32-bit floating point channels:
 * \code{.c}
 *     VPIPixelType type = VPI_MAKE_PIXEL_TYPE(VPI_MEM_LAYOUT_BL, VPI_DATA_TYPE_FLOAT, VPI_PACKING_X32_Y32);
 * \endcode
 *
 * @param[in] memLayout \ref VPIMemLayout to be used.
 * @param[in] dataType  \ref VPIDataType to be used.
 * @param[in] packing   Format packing used, which also defines the number of channels.
 *
 * @returns The user-defined pixel type.
 */
#ifdef VPI_DOXYGEN
#    define VPI_MAKE_PIXEL_TYPE(memLayout, dataType, packing)
#else
#    define VPI_MAKE_PIXEL_TYPE (VPIPixelType) VPI_DETAIL_MAKE_PIXEL_TYPE
#endif

/** Creates a user-defined pixel type.
 * When the pre-defined pixel types aren't enough, user-defined formats can be created.
 *
 * @param[in] memLayout \ref VPIMemLayout to be used.
 *                     + Mandatory, must not be \ref VPI_MEM_LAYOUT_INVALID.
 * 
 * @param[in] dataType \ref VPIDataType to be used.
 *                     + Mandatory, must not be \ref VPI_DATA_TYPE_INVALID.
 * 
 * @param[in] packing Format packing used, which also defines the number of channels.
 *                     + Mandatory, must not be \ref VPI_PACKING_INVALID.
 *
 * @returns The user-defined pixel type.
 *
 * @retval #VPI_PIXEL_TYPE_INVALID Invalid \p memLayout.
 * @retval #VPI_PIXEL_TYPE_INVALID Invalid \p dataType.
 * @retval #VPI_PIXEL_TYPE_INVALID Invalid \p packing.
 */
VPI_PUBLIC VPIPixelType vpiMakePixelType(VPIMemLayout memLayout, VPIDataType dataType, VPIPacking packing);

/** Get the packing of a pixel type.
 *
 * @param[in] type Pixel type to be queried.
 *                 + Mandatory, must not be \ref VPI_PIXEL_TYPE_INVALID.
 *
 * @returns The format's packing.
 *
 * @retval #VPI_PACKING_0 \p type is invalid.
 */
VPI_PUBLIC VPIPacking vpiPixelTypeGetPacking(VPIPixelType type);

/** Get the number of bits per pixel of a pixel type.
 *
 * @param[in] type Pixel type to be queried.
 *                 + Mandatory, must not be \ref VPI_PIXEL_TYPE_INVALID.
 *
 * @returns The number of bits per pixel.
 *
 * @retval 0 \p type is invalid.
 */
VPI_PUBLIC int vpiPixelTypeGetBitsPerPixel(VPIPixelType type);

/** Get the number of bits per channel of a pixel type.
 *
 * @param[in] type Pixel type to be queried.
 *                 + Mandatory, must not be \ref VPI_PIXEL_TYPE_INVALID.
 *
 * @param[out] bits Pointer to an int32_t array with 4 elements where output will be stored.
 *                  If it is NULL, the function is a no-op, i.e., buffer pointed by \p bits is unchanged.
 */
VPI_PUBLIC void vpiPixelTypeGetBitsPerChannel(VPIPixelType type, int32_t *bits);

/** Get the data type of a pixel type.
 *
 * @param[in] type Pixel type to be queried.
 *                 + Mandatory, must not be \ref VPI_PIXEL_TYPE_INVALID.
 *
 * @returns The data type of the pixel type.
 *
 * @retval #VPI_DATA_TYPE_INVALID \p type is invalid.
 */
VPI_PUBLIC VPIDataType vpiPixelTypeGetDataType(VPIPixelType type);

/** Get the memory layout of a pixel type.
 *
 * @param[in] type Pixel type to be queried.
 *                 + Mandatory, must not be \ref VPI_PIXEL_TYPE_INVALID.
 *
 * @returns The memory layout of the pixel type.
 *
 * @retval #VPI_MEM_LAYOUT_INVALID \p type is invalid.
 */
VPI_PUBLIC VPIMemLayout vpiPixelTypeGetMemLayout(VPIPixelType type);

/** Set the memory layout of a pixel type.
 *
 * @param[in] type Pixel type to have its memory layout set.
 *                 + Mandatory, must not be \ref VPI_PIXEL_TYPE_INVALID.
 *
 * @param[in] layout The memory layout to be set.
 *                 + Mandatory, must not be \ref VPI_MEM_LAYOUT_INVALID.
 *
 * @returns The pixel type with the requested memory layout.
 *
 * @retval #VPI_PIXEL_TYPE_INVALID \p type or layout are invalid.
 */
VPI_PUBLIC VPIPixelType vpiPixelTypeSetMemLayout(VPIPixelType type, VPIMemLayout layout);

/** Get the number of channels of a pixel type.
 *
 * @param[in] type Pixel type to be queried.
 *                 + Mandatory, must not be \ref VPI_PIXEL_TYPE_INVALID.
 *
 * @returns The number of channels of the pixel type.
 *
 * @retval 0 \p type is invalid.
 */
VPI_PUBLIC int vpiPixelTypeGetChannelCount(VPIPixelType type);

/** Returns a string representation of the pixel type.
 *
 * @param[in] type Pixel type to be returned.
 *
 * @returns The string representation of the pixel type.
 *          Returned string is valid until next call of this function from the same calling thread.
 *          Returned pointer must not be freed.
 */
VPI_PUBLIC const char *vpiPixelTypeGetName(VPIPixelType type);

/** Get the pixel type for a given channel index.
 *
 * It returns a single-channel pixel type that corresponds to the given channel
 * of the input pixel type.
 *
 * For instance: The channel #2 of \ref VPI_PIXEL_TYPE_3U8 is \ref VPI_PIXEL_TYPE_U8.
 *
 * + The requested channel must have a type whose packing is one of the following:
 *   - \ref VPI_PACKING_X1
 *   - \ref VPI_PACKING_X2
 *   - \ref VPI_PACKING_X4
 *   - \ref VPI_PACKING_X8
 *   - \ref VPI_PACKING_X16
 *   - \ref VPI_PACKING_X24
 *   - \ref VPI_PACKING_X32
 *   - \ref VPI_PACKING_X48
 *   - \ref VPI_PACKING_X64
 *   - \ref VPI_PACKING_X96
 *   - \ref VPI_PACKING_X128
 *   - \ref VPI_PACKING_X192
 *   - \ref VPI_PACKING_X256
 *
 * @param[in] type Pixel type to be queried.
 *                 + Mandatory, must not be \ref VPI_PIXEL_TYPE_INVALID.
 *
 * @param[in] channel Channel whose pixel type is to be returned.
 *                 + Must be between 0 and the maximum number of channels in \p type.
 *
 * @returns The pixel type of the given channel. The memory layout and data type are the same as \p type.
 *
 * @retval VPI_PIXEL_TYPE_INVALID \p type is invalid.
 * @retval VPI_PIXEL_TYPE_INVALID \p channel is outside valid range.
 */
VPI_PUBLIC VPIPixelType vpiPixelTypeGetChannelType(VPIPixelType type, int channel);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* NV_VPI_PIXEL_TYPE_H */
