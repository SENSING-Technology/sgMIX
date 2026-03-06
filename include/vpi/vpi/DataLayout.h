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
 * @file DataLayout.h
 *
 * Defines types and functions to handle data layouts.
 */

#ifndef NV_VPI_DATALAYOUT_H
#define NV_VPI_DATALAYOUT_H

#include "Export.h"
#include "detail/FormatUtils.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
* @defgroup VPI_DataLayout Data layout utilities 
* @ingroup VPI_API_Misc
* @{
*
*/

/** 
 * Maximum VPI channel count
 */
#define VPI_MAX_CHANNEL_COUNT (4)

/** Defines how channels are packed into an image plane element.
 *
 * Packing encodes how many channels the plane element has, and how they
 * are arranged in memory.
 * 
 * Up to 4 channels (denoted by X, Y, Z, W) can be packed into an image
 * plane element, each one occupying a specified number of bits.
 *
 * When two channels are specified one right after the other, they are
 * ordered from most-significant bit to least-significant bit. Words are
 * separated by underscores. For example:
 *
 * X8Y8Z8W8 = a single 32-bit word containing 4 channels, 8 bits each.
 *
 * In little-endian architectures:
 * <pre>
 *      Address  0   ||  1   ||  2   ||  3
 *            WWWWWWWWZZZZZZZZYYYYYYYYXXXXXXXX
 * </pre>
 *
 * In big-endian architectures:
 * <pre>
 *      Address  0   ||  1   ||  2   ||  3
 *            XXXXXXXXYYYYYYYYZZZZZZZZWWWWWWWW
 * </pre>
 *
 * X8_Y8_Z8_W8 = four consecutive 8-bit words, corresponding to 4 channels, 8 bits each.
 *
 * In little-endian architectures:
 * <pre>
 *      Address  0   ||  1   ||  2   ||  3
 *            XXXXXXXXYYYYYYYYZZZZZZZZWWWWWWWW
 * </pre>
 *
 * In big-endian architectures:
 * <pre>
 *      Address  0   ||  1   ||  2   ||  3
 *            XXXXXXXXYYYYYYYYZZZZZZZZWWWWWWWW
 * </pre>
 *
 * In cases where a word is less than 8 bits (e.g., X1 1-bit channel), channels
 * are ordered from LSB to MSB within a word.
 *
 * @note Also note equivalences such as the following:
 * @note In little-endian: X8_Y8_Z8_W8 = W8Z8Y8X8.
 * @note In big-endian: X8_Y8_Z8_W8 = X8Y8Z8W8.
 *
 * Some formats allow different packings when pixels' horizontal coordinate is
 * even or odd. For instance, every pixel of YUV422 packed format contains an Y
 * channel, while only even pixels contain the U channel, and odd pixels contain
 * V channel. Such formats use a double-underscore to separate the even pixels from the odd
 * pixels. The packing just described might be referred to X8_Y8__X8_Z8, where X = luma, 
 * Y = U chroma, Z = V chroma.
 */
typedef enum
{
    /** No channels. */
    VPI_PACKING_0 = 0,

    /** One 1-bit channel. */
    VPI_PACKING_X1 = VPI_DETAIL_BPP_NCH(1, 1),
    /** One 2-bit channel. */
    VPI_PACKING_X2 = VPI_DETAIL_BPP_NCH(2, 1),
    /** One 4-bit channel. */
    VPI_PACKING_X4 = VPI_DETAIL_BPP_NCH(4, 1),
    /** One 8-bit channel. */
    VPI_PACKING_X8 = VPI_DETAIL_BPP_NCH(8, 1),
    /** Two 4-bit channels in one word. */
    VPI_PACKING_X4Y4 = VPI_DETAIL_BPP_NCH(8, 2),
    /** Three 3-, 3- and 2-bit channels in one 8-bit word. */
    VPI_PACKING_X3Y3Z2 = VPI_DETAIL_BPP_NCH(8, 3),

    /** One 16-bit channel. */
    VPI_PACKING_X16 = VPI_DETAIL_BPP_NCH(16, 1),
    /** One LSB 10-bit channel in one 16-bit word. */
    VPI_PACKING_b6X10,
    /** One MSB 10-bit channel in one 16-bit word. */
    VPI_PACKING_X10b6,
    /** One LSB 12-bit channel in one 16-bit word. */
    VPI_PACKING_b4X12,
    /** One MSB 12-bit channel in one 16-bit word. */
    VPI_PACKING_X12b4,
    /** One LSB 14-bit channel in one 16-bit word. */
    VPI_PACKING_b2X14,

    /** Two 8-bit channels in two 8-bit words. */
    VPI_PACKING_X8_Y8 = VPI_DETAIL_BPP_NCH(16, 2),

    /** Three 5-, 5- and 6-bit channels in one 16-bit word. */
    VPI_PACKING_X5Y5Z6 = VPI_DETAIL_BPP_NCH(16, 3),
    /** Three 5-, 6- and 5-bit channels in one 16-bit word. */
    VPI_PACKING_X5Y6Z5,
    /** Three 6-, 5- and 5-bit channels in one 16-bit word. */
    VPI_PACKING_X6Y5Z5,
    /** Three 4-bit channels in one 16-bit word. */
    VPI_PACKING_b4X4Y4Z4,
    /** Three 5-bit channels in one 16-bit word. */
    VPI_PACKING_b1X5Y5Z5,
    /** Three 5-bit channels in one 16-bit word. */
    VPI_PACKING_X5Y5b1Z5,

    /** Four 1-, 5-, 5- and 5-bit channels in one 16-bit word. */
    VPI_PACKING_X1Y5Z5W5 = VPI_DETAIL_BPP_NCH(16, 4),
    /** Four 4-bit channels in one 16-bit word. */
    VPI_PACKING_X4Y4Z4W4,
    /** Four 5-, 1-, 5- and 5-bit channels in one 16-bit word. */
    VPI_PACKING_X5Y1Z5W5,
    /** Four 5-, 5-, 1- and 5-bit channels in one 16-bit word. */
    VPI_PACKING_X5Y5Z1W5,
    /** Four 5-, 5-, 5- and 1-bit channels in one 16-bit word. */
    VPI_PACKING_X5Y5Z5W1,

    /** 2 pixels of 2 8-bit channels each, totalling 4 8-bit words. */
    VPI_PACKING_X8_Y8__X8_Z8,
    /** 2 pixels of 2 swapped 8-bit channels each, totalling 4 8-bit words. */
    VPI_PACKING_Y8_X8__Z8_X8,

    /** One 24-bit channel. */
    VPI_PACKING_X24 = VPI_DETAIL_BPP_NCH(24, 1),

    /** Three 8-bit channels in three 8-bit words. */
    VPI_PACKING_X8_Y8_Z8 = VPI_DETAIL_BPP_NCH(24, 3),

    /** One 32-bit channel. */
    VPI_PACKING_X32 = VPI_DETAIL_BPP_NCH(32, 1),
    /** One LSB 20-bit channel in one 32-bit word. */
    VPI_PACKING_b12X20,

    /** Two 16-bit channels in two 16-bit words. */
    VPI_PACKING_X16_Y16 = VPI_DETAIL_BPP_NCH(32, 2),
    /** Two MSB 10-bit channels in two 16-bit words. */
    VPI_PACKING_X10b6_Y10b6,
    /** Two MSB 12-bit channels in two 16-bit words. */
    VPI_PACKING_X12b4_Y12b4,

    /** Three 10-, 11- and 11-bit channels in one 32-bit word. */
    VPI_PACKING_X10Y11Z11 = VPI_DETAIL_BPP_NCH(32, 3),
    /** Three 11-, 11- and 10-bit channels in one 32-bit word. */
    VPI_PACKING_X11Y11Z10,

    /** Four 8-bit channels in one 32-bit word. */
    VPI_PACKING_X8_Y8_Z8_W8 = VPI_DETAIL_BPP_NCH(32, 4),
    /** Four 2-, 10-, 10- and 10-bit channels in one 32-bit word. */
    VPI_PACKING_X2Y10Z10W10,
    /** Four 10-, 10-, 10- and 2-bit channels in one 32-bit word. */
    VPI_PACKING_X10Y10Z10W2,

    /** One 48-bit channel. */
    VPI_PACKING_X48 = VPI_DETAIL_BPP_NCH(48, 1),
    /** Three 16-bit channels in three 16-bit words. */
    VPI_PACKING_X16_Y16_Z16 = VPI_DETAIL_BPP_NCH(48, 3),

    /** One 64-bit channel. */
    VPI_PACKING_X64 = VPI_DETAIL_BPP_NCH(64, 1),
    /** Two 32-bit channels in two 32-bit words. */
    VPI_PACKING_X32_Y32 = VPI_DETAIL_BPP_NCH(64, 2),
    /** Four 16-bit channels in one 64-bit word. */
    VPI_PACKING_X16_Y16_Z16_W16 = VPI_DETAIL_BPP_NCH(64, 4),

    /** One 96-bit channel. */
    VPI_PACKING_X96 = VPI_DETAIL_BPP_NCH(96, 1),
    /** Three 32-bit channels in three 32-bit words. */
    VPI_PACKING_X32_Y32_Z32 = VPI_DETAIL_BPP_NCH(96, 3),

    /** One 128-bit channel. */
    VPI_PACKING_X128 = VPI_DETAIL_BPP_NCH(128, 1),
    /** Two 64-bit channels in two 64-bit words. */
    VPI_PACKING_X64_Y64 = VPI_DETAIL_BPP_NCH(128, 2),
    /** Four 32-bit channels in three 32-bit words. */
    VPI_PACKING_X32_Y32_Z32_W32 = VPI_DETAIL_BPP_NCH(128, 4),

    /** One 192-bit channel. */
    VPI_PACKING_X192 = VPI_DETAIL_BPP_NCH(192, 1),
    /** Three 64-bit channels in three 64-bit words. */
    VPI_PACKING_X64_Y64_Z64 = VPI_DETAIL_BPP_NCH(192, 3),

    /** One 128-bit channel. */
    VPI_PACKING_X256 = VPI_DETAIL_BPP_NCH(256, 1),
    /** Four 64-bit channels in four 64-bit words. */
    VPI_PACKING_X64_Y64_Z64_W64 = VPI_DETAIL_BPP_NCH(256, 4),

    /** Denotes an invalid packing. */
    VPI_PACKING_INVALID = INT32_MAX
} VPIPacking;

/** Defines the channel data type. */
typedef enum
{
    /** Represents an invalid data type.
     *  Used in some function to represent error return values. */
    VPI_DATA_TYPE_INVALID = 0,

    VPI_DATA_TYPE_UNSIGNED, /**< Channels are unsigned integer values. */
    VPI_DATA_TYPE_SIGNED,   /**< Channels are signed integer values. */
    VPI_DATA_TYPE_FLOAT,    /**< Channel are floating point values. */
    VPI_DATA_TYPE_FLOATISP  /**< Proprietary floating point format from NVIDIA. */
} VPIDataType;

/** Defines how the 2D plane pixels are laid out in memory.
 * This defines how a pixel are addressed, i.e., given its \f$(x,y)\f$ coordinate,
 * what's its memory address.
 * Block-linear formats have a proprietary memory representation and aren't supposed to
 * be addressed by the user directly.
 */
typedef enum
{
    /** Represents an invalid memory layout. */
    VPI_MEM_LAYOUT_INVALID = 0,

    /** Pixels are laid out in row-major order.
     * \f$(x,y) = y \times \mathit{pitch} + x \times \mathit{pixel stride}\f$. */
    VPI_MEM_LAYOUT_PITCH_LINEAR,

    /** Pixels are laid out in block-linear format with height = 1. */
    VPI_MEM_LAYOUT_BLOCK1_LINEAR,

    /** Pixels are laid out in block-linear format with height = 2. */
    VPI_MEM_LAYOUT_BLOCK2_LINEAR,

    /** Pixels are laid out in block-linear format with height = 4. */
    VPI_MEM_LAYOUT_BLOCK4_LINEAR,

    /** Pixels are laid out in block-linear format with height = 8. */
    VPI_MEM_LAYOUT_BLOCK8_LINEAR,

    /** Pixels are laid out in block-linear format with height = 16. */
    VPI_MEM_LAYOUT_BLOCK16_LINEAR,

    /** Pixels are laid out in block-linear format with height = 32. */
    VPI_MEM_LAYOUT_BLOCK32_LINEAR,

    /** Default block-linear format.
     * It's guaranteed to be valid in all algorithms that support block-linear format. */
    VPI_MEM_LAYOUT_BLOCK_LINEAR = VPI_MEM_LAYOUT_BLOCK2_LINEAR,

    /** @{ Useful aliases. */
    VPI_MEM_LAYOUT_PL = VPI_MEM_LAYOUT_PITCH_LINEAR,
    VPI_MEM_LAYOUT_BL = VPI_MEM_LAYOUT_BLOCK_LINEAR
    /** @} */
} VPIMemLayout;

/** Defines the format channel names.
 * The channels are color model-agnostic. */
typedef enum
{
    VPI_CHANNEL_INVALID = 7, /**< Represents an invalid swizzle channel. */

    VPI_CHANNEL_0 = 0, /**< Don't select a channel. */
    VPI_CHANNEL_X,     /**< Selects the first channel of the color model. */
    VPI_CHANNEL_Y,     /**< Selects the second channel of the color model. */
    VPI_CHANNEL_Z,     /**< Selects the third channel of the color model. */
    VPI_CHANNEL_W,     /**< Selects the fourth channel of the color model. */
    VPI_CHANNEL_1      /**< Sets the corresponding channel to have its maximum value. */
} VPIChannel;

/** Defines the supported channel swizzle operations.
 *
 * The operations map an input vector \f$(x,y,z,w)\f$ into an output vector
 * \f$(x',y',z',w')\f$. Any output channel can select any of the input
 * channels, or the constants zero or one. For example, the swizzle "X000"
 * selects the first channel, whereas swizzle "ZYXW" swaps the X and Z
 * channels, needed for conversion between RGBA and BGRA image formats.
 */
typedef enum
{
    /** Represents an invalid swizzle. */
    VPI_SWIZZLE_INVALID = VPI_DETAIL_MAKE_SWZL(INVALID, INVALID, INVALID, INVALID),

    /** @{ Swizzle operation. */
    VPI_DETAIL_DEF_SWIZZLE_ENUM(0, 0, 0, 0),
    VPI_DETAIL_DEF_SWIZZLE_ENUM(1, 0, 0, 0),
    VPI_DETAIL_DEF_SWIZZLE_ENUM(0, 0, 0, 1),
    VPI_DETAIL_DEF_SWIZZLE_ENUM(X, Y, Z, W),
    VPI_DETAIL_DEF_SWIZZLE_ENUM(Z, Y, X, W),
    VPI_DETAIL_DEF_SWIZZLE_ENUM(W, X, Y, Z),
    VPI_DETAIL_DEF_SWIZZLE_ENUM(W, Z, Y, X),
    VPI_DETAIL_DEF_SWIZZLE_ENUM(Y, Z, W, X),
    VPI_DETAIL_DEF_SWIZZLE_ENUM(X, Y, Z, 1),
    VPI_DETAIL_DEF_SWIZZLE_ENUM(X, Y, Z, 0),
    VPI_DETAIL_DEF_SWIZZLE_ENUM(Y, Z, W, 1),
    VPI_DETAIL_DEF_SWIZZLE_ENUM(X, X, X, 1),
    VPI_DETAIL_DEF_SWIZZLE_ENUM(X, Z, Y, 1),
    VPI_DETAIL_DEF_SWIZZLE_ENUM(Z, Y, X, 1),
    VPI_DETAIL_DEF_SWIZZLE_ENUM(Z, Y, X, 0),
    VPI_DETAIL_DEF_SWIZZLE_ENUM(W, Z, Y, 1),
    VPI_DETAIL_DEF_SWIZZLE_ENUM(X, 0, 0, 0),
    VPI_DETAIL_DEF_SWIZZLE_ENUM(0, X, 0, 0),
    VPI_DETAIL_DEF_SWIZZLE_ENUM(0, 0, X, 0),
    VPI_DETAIL_DEF_SWIZZLE_ENUM(0, 0, 0, X),
    VPI_DETAIL_DEF_SWIZZLE_ENUM(Y, 0, 0, 0),
    VPI_DETAIL_DEF_SWIZZLE_ENUM(0, Y, 0, 0),
    VPI_DETAIL_DEF_SWIZZLE_ENUM(0, 0, Y, 0),
    VPI_DETAIL_DEF_SWIZZLE_ENUM(0, 0, 0, Y),
    VPI_DETAIL_DEF_SWIZZLE_ENUM(0, X, Y, 0),
    VPI_DETAIL_DEF_SWIZZLE_ENUM(X, X, X, Y),
    VPI_DETAIL_DEF_SWIZZLE_ENUM(Y, Y, Y, X),
    VPI_DETAIL_DEF_SWIZZLE_ENUM(0, Y, X, 0),
    VPI_DETAIL_DEF_SWIZZLE_ENUM(X, 0, 0, Y),
    VPI_DETAIL_DEF_SWIZZLE_ENUM(Y, 0, 0, X),
    VPI_DETAIL_DEF_SWIZZLE_ENUM(X, 0, 0, 1),
    VPI_DETAIL_DEF_SWIZZLE_ENUM(X, Y, 0, 1),
    VPI_DETAIL_DEF_SWIZZLE_ENUM(X, Y, 0, 0),
    VPI_DETAIL_DEF_SWIZZLE_ENUM(0, X, Z, 0),
    VPI_DETAIL_DEF_SWIZZLE_ENUM(0, Z, X, 0),
    VPI_DETAIL_DEF_SWIZZLE_ENUM(X, Z, Y, 0),
    VPI_DETAIL_DEF_SWIZZLE_ENUM(Y, Z, X, 1),
    VPI_DETAIL_DEF_SWIZZLE_ENUM(Z, Y, W, 1),
    VPI_DETAIL_DEF_SWIZZLE_ENUM(0, Y, X, 1),
    VPI_DETAIL_DEF_SWIZZLE_ENUM(X, Y, X, Z),
    VPI_DETAIL_DEF_SWIZZLE_ENUM(Y, X, Z, X),
    VPI_DETAIL_DEF_SWIZZLE_ENUM(X, Z, 0, 0),
    VPI_DETAIL_DEF_SWIZZLE_ENUM(W, Y, X, Z),
    VPI_DETAIL_DEF_SWIZZLE_ENUM(Y, X, 0, 0),
    VPI_DETAIL_DEF_SWIZZLE_ENUM(Y, X, 0, 1),
    VPI_DETAIL_DEF_SWIZZLE_ENUM(0, 0, Y, X),
    VPI_DETAIL_DEF_SWIZZLE_ENUM(0, 0, X, Y),
    VPI_DETAIL_DEF_SWIZZLE_ENUM(0, X, Y, 1),
    VPI_DETAIL_DEF_SWIZZLE_ENUM(0, X, 0, 1),
    VPI_DETAIL_DEF_SWIZZLE_ENUM(Y, Z, X, W),
    VPI_DETAIL_DEF_SWIZZLE_ENUM(Y, W, 0, 0),
    VPI_DETAIL_DEF_SWIZZLE_ENUM(X, Y, W, 0),
    VPI_DETAIL_DEF_SWIZZLE_ENUM(Y, Z, W, 0),
    /** @} */
} VPISwizzle;

/** Creates a user-defined swizzle operation, using abbreviated parameters.
 * This macro is used to create a user-defined swizzle operation.
 * Parameters are specified literally by X, Y, Z, 0, 1. They correpond to \ref VPIChannel values.
 * Example:
 * \code{.c}
 *   VPISwizzle sw = VPI_MAKE_SWIZZLE_ABBREV(0,Y,Z,W);
 * \endcode
 *
 * @param[in] x Channel that will correspond to the first component, without the VPI_CHANNEL prefix.
 * @param[in] y Channel that will correspond to the second component, without the VPI_CHANNEL prefix.
 * @param[in] z Channel that will correspond to the third component, without the VPI_CHANNEL prefix.
 * @param[in] w Channel that will correspond to the fourth component, without the VPI_CHANNEL prefix.
 *
 * @returns the user-defined \ref VPISwizzle operation.
 */
#ifdef VPI_DOXYGEN
#    define VPI_MAKE_SWIZZLE_ABBREV(x, y, z, w)
#else
#    define VPI_MAKE_SWIZZLE_ABBREV (VPISwizzle) VPI_DETAIL_MAKE_SWZL
#endif

/** Creates a user-defined swizzle operation.
 * This macro is used to create a user-defined swizzle operation if it's not predefined.
 * Example:
 * \code{.c}
 *   VPISwizzle sw = VPI_MAKE_SWIZZLE(VPI_CHANNEL_0,VPI_CHANNEL_Y,VPI_CHANNEL_Z,VPI_CHANNEL_W);
 * \endcode
 *
 * @param[in] x Channel that will correspond to the first component.
 * @param[in] y Channel that will correspond to the second component.
 * @param[in] z Channel that will correspond to the third component.
 * @param[in] w Channel that will correspond to the fourth component.
 *
 * @returns the user-defined \ref VPISwizzle operation.
 */
#ifdef VPI_DOXYGEN
#    define VPI_MAKE_SWIZZLE(x, y, z, w)
#else
#    define VPI_MAKE_SWIZZLE (VPISwizzle) VPI_DETAIL_MAKE_SWIZZLE
#endif

/** Creates a user-defined \ref VPISwizzle operation.
 * This is similar to \ref VPI_MAKE_SWIZZLE, but accepts the swizzle channels as runtime variables.
 * 
 * @param[in] x Channel that will correspond to the first component.
 *
 * @param[in] y Channel that will correspond to the second component.
 *
 * @param[in] z Channel that will correspond to the third component.
 *
 * @param[in] w Channel that will correspond to the fourth component.
 *
 * @returns Swizzle operation as defined by the given channel order.
 * @retval #VPI_SWIZZLE_INVALID Input channel is invalid.
 */
VPI_PUBLIC VPISwizzle vpiMakeSwizzle(VPIChannel x, VPIChannel y, VPIChannel z, VPIChannel w);

/** Get the swizzle channels.
 *
 * For example, given swizzle \ref VPI_SWIZZLE_YZWX, it returns
 * \ref VPI_CHANNEL_Y, \ref VPI_CHANNEL_Z, \ref VPI_CHANNEL_W and
 * \ref VPI_CHANNEL_X.
 *
 * @param[in] swizzle Swizzle to be queried.
 *
 * @param[out] channels Output channel array with 4 elements.
 */
VPI_PUBLIC void vpiSwizzleGetChannels(VPISwizzle swizzle, VPIChannel *channels);

/** Get the number of channels specified by the given swizzle.
 *
 * Only the following count as channels:
 * - \ref VPI_CHANNEL_X
 * - \ref VPI_CHANNEL_Y
 * - \ref VPI_CHANNEL_Z
 * - \ref VPI_CHANNEL_W
 *
 * @param[in] swizzle Swizzle to be queried.
 *
 * @returns The channel count specified by swizzle.
 */
VPI_PUBLIC int vpiSwizzleGetChannelCount(VPISwizzle swizzle);

/** Endianness of a \ref VPIPacking value. */
typedef enum
{
    VPI_INVALID_ENDIAN, /**< Invalid endianness value. */
    VPI_HOST_ENDIAN,    /**< Endianness of the host machine. */
    VPI_BIG_ENDIAN      /**< Big endian, where most significant byte has lower memory address. */
} VPIEndianness;

/** Defines the parameters encoded in a \ref VPIPacking. */
typedef struct
{
    /** Component ordering in a word. */
    VPIEndianness endianness;

    /** Channel ordering.  */
    VPISwizzle swizzle;

    /** Number of bits in each channel.
     *  If channel doesn't exist, corresponding bits==0. */
    int bits[VPI_MAX_CHANNEL_COUNT];

} VPIPackingParams;

/** Returns a pre-defined \ref VPIPacking given its params.
 *
 * This function calculates the \ref VPIPacking based on the channel characteristics at run time.
 *
 *
 * @param[in] params Packing parameters.
 *                   If \ref VPIPackingParams::swizzle is set to \ref VPI_SWIZZLE_0000 or \ref VPI_SWIZZLE_INVALID, 
 *                   the swizzle will be inferred from \ref VPIPackingParams::bits
 *
 * @returns The packing enum corresponding to \p params.
 *
 * @retval #VPI_PACKING_INVALID Requested packing isn't defined or is malformed.
 * @retval #VPI_PACKING_INVALID \p params is NULL.
 */
VPI_PUBLIC VPIPacking vpiMakePacking(const VPIPackingParams *params);

/** Returns channels' information from a format packing.
 *
 * @param[in] packing The format packing to be queried.
 * 
 * @param[out] params The packing parameters.
 *                    Passing NULL is allowed, to which the function simply does nothing.
 */
VPI_PUBLIC void vpiPackingGetParams(VPIPacking packing, VPIPackingParams *params);

/** Returns the number of components defined by the given packing.
 *
 * @param[in] packing The format packing to be queried.
 *
 * @returns Number of components from the given format packing. It's value between 0 and 4.
 */
VPI_PUBLIC int vpiPackingGetComponentCount(VPIPacking packing);

/** Returns the number of bits per packing component.
 *
 * @param[in] packing The format packing to be queried.
 * 
 * @param[out] bits Pointer to an int32_t array with 4 elements where output will be stored. 
 *                  Passing NULL is allowed, to which the function simply does nothing.
 *
 */
VPI_PUBLIC void vpiPackingGetBitsPerComponent(VPIPacking packing, int32_t *bits);

/** Returns the number of bits per pixel of the given packing.
 *
 * @param[in] packing The format packing to be queried.
 *
 * @returns Total number of bits per pixel of the given packing. 
 *          It's the sum of number of bits occupied by all packing channels.
 */
VPI_PUBLIC int vpiPackingGetBitsPerPixel(VPIPacking packing);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* NV_VPI_DATALAYOUT_H */
