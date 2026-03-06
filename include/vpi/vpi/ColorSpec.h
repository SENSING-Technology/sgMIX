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
 * @file ColorSpec.h
 *
 * Defines types and functions to handle color specs.
 */

#ifndef NV_VPI_COLORSPEC_H
#define NV_VPI_COLORSPEC_H

#include "Export.h"
#include "detail/FormatUtils.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
* @defgroup VPI_ColorSpec Color Spaces
* @ingroup VPI_API_Misc
* @{
*/

/** Defines color models.
 * A color model gives meaning to each channel of an image format. They are specified
 * in a canonical XYZW ordering that can then be swizzled to the desired ordering. 
 */
typedef enum
{
    VPI_COLOR_MODEL_UNDEFINED = 0,     /**< Color model is undefined. */
    VPI_COLOR_MODEL_YCbCr     = 1,     /**< Luma + chroma (blue-luma, red-luma). */
    VPI_COLOR_MODEL_RGB       = 2,     /**< red, green, blue components. */
    VPI_COLOR_MODEL_RAW       = 2 + 7, /**< RAW color model, used for Bayer image formats. */
    VPI_COLOR_MODEL_XYZ,               /**< CIE XYZ tristimulus color spec. */
} VPIColorModel;

/** Defines the color primaries and the white point of a \ref VPIColorSpec. */
typedef enum
{
    VPI_COLOR_SPACE_SENSOR, /**< Color space from the sensor used to capture the image. */
    VPI_COLOR_SPACE_BT601,  /**< Color primaries from ITU-R BT.601/625 lines standard, also known as EBU 3213-E. */
    VPI_COLOR_SPACE_BT709,  /**< Color primaries from ITU-R BT.709 standard, D65 white point. */
    VPI_COLOR_SPACE_BT2020, /**< Color primaries from ITU-R BT.2020 standard, D65 white point. */
    VPI_COLOR_SPACE_DCIP3,  /**< Color primaries from DCI-P3 standard, D65 white point. */

    VPI_COLOR_SPACE_UNDEFINED = INT32_MAX, /**< Color space not defined. */
} VPIColorSpace;

/** Defines the white point associated with a \ref VPIColorSpace. */
typedef enum
{
    VPI_WHITE_POINT_D65, /**< D65 white point, K = 6504. */

    VPI_WHITE_POINT_UNDEFINED = INT32_MAX /**< White point not defined. */
} VPIWhitePoint;

/** Defines the YCbCr encoding used in a particular \ref VPIColorSpec. */
typedef enum
{
    VPI_YCbCr_ENC_UNDEFINED = 0, /**< Encoding not defined. Usually used by non-YCbCr color specs. */
    VPI_YCbCr_ENC_BT601,         /**< Encoding specified by ITU-R BT.601 standard. */
    VPI_YCbCr_ENC_BT709,         /**< Encoding specified by ITU-R BT.709 standard. */
    VPI_YCbCr_ENC_BT2020,        /**< Encoding specified by ITU-R BT.2020 standard. */
    VPI_YCbCr_ENC_BT2020c,       /**< Encoding specified by ITU-R BT.2020 with constant luminance. */
    VPI_YCbCr_ENC_SMPTE240M,     /**< Encoding specified by SMPTE 240M standard. */
} VPIYCbCrEncoding;

/** Defines the color transfer function in a particular \ref VPIColorSpec. */
typedef enum
{
    VPI_COLOR_XFER_LINEAR,    /**< Linear color transfer function. */
    VPI_COLOR_XFER_sRGB,      /**< Color transfer function specified by sRGB standard. */
    VPI_COLOR_XFER_sYCC,      /**< Color transfer function specified by sYCC standard. */
    VPI_COLOR_XFER_PQ,        /**< Perceptual quantizer color transfer function. */
    VPI_COLOR_XFER_BT709,     /**< Color transfer function specified by ITU-R BT.709 standard. */
    VPI_COLOR_XFER_BT2020,    /**< Color transfer function specified by ITU-R BT.2020 standard. */
    VPI_COLOR_XFER_SMPTE240M, /**< Color transfer function specified by SMPTE 240M standard. */
} VPIColorTransferFunction;

/** Defines the color range of a particular \ref VPIColorSpec. */
typedef enum
{
    VPI_COLOR_RANGE_FULL,   /**< Values cover the full underlying type range. */
    VPI_COLOR_RANGE_LIMITED /**< Values cover a limited range of the underlying type. */
} VPIColorRange;

/** Chroma sampling location. */
typedef enum
{
    VPI_CHROMA_LOC_EVEN   = 0, /**< Sample the chroma with even coordinate. */
    VPI_CHROMA_LOC_CENTER = 1, /**< Sample the chroma exactly between the even and odd coordinate. */
    VPI_CHROMA_LOC_ODD    = 2, /**< Sample the chroma with odd coordinate. */
    VPI_CHROMA_LOC_BOTH   = 3, /**< Sample chroma from even and odd coordinates.
                                    This is used when no sub-sampling is taking place. */
} VPIChromaLocation;

/** Color spec definitions.
 * These color specs define how color information is to be interpreted.
 * It is defined by several parameters:
 * - \ref VPIColorModel
 * - \ref VPIColorSpace
 * - \ref VPIWhitePoint
 * - \ref VPIYCbCrEncoding
 * - \ref VPIColorTransferFunction
 * - \ref VPIColorRange
 * - \ref VPIChromaLocation
 *
 * These parameters together defines how the color representation maps to its
 * corresponding absolute color in a chromacity diagram.
 */

/* clang-format off */
typedef enum
{
    /** Invalid color spec. This is to be used when no color spec is selected. */
    VPI_COLOR_SPEC_INVALID = INT32_MAX,

    /** Default color spec. Informs that the color spec is to be inferred. */
    VPI_COLOR_SPEC_DEFAULT          = VPI_DETAIL_MAKE_CSPC(SPACE_UNDEFINED, ENC_UNDEFINED, XFER_LINEAR, RANGE_FULL,    LOC_BOTH,   LOC_BOTH),

    /** No color spec defined. Used when color spec isn't relevant or is not defined.
     *  The color spec may be inferred from the context. If this isn't possible, the values for each
     *  color spec component defined below will be used. */
    VPI_COLOR_SPEC_UNDEFINED        = VPI_DETAIL_MAKE_CSPC(SPACE_BT709,  ENC_UNDEFINED, XFER_LINEAR,    RANGE_FULL,    LOC_BOTH,   LOC_BOTH),

    /** Color spec defining ITU-R BT.601 standard, limited range, with BT.709 chrominancies and transfer function. */
    VPI_COLOR_SPEC_BT601            = VPI_DETAIL_MAKE_CSPC(SPACE_BT709,  ENC_BT601,     XFER_BT709,     RANGE_LIMITED, LOC_EVEN,   LOC_EVEN),

    /** Color spec defining ITU-R BT.601 standard, full range, with BT.709 chrominancies and transfer function. */
    VPI_COLOR_SPEC_BT601_ER         = VPI_DETAIL_MAKE_CSPC(SPACE_BT709,  ENC_BT601,     XFER_BT709,     RANGE_FULL,    LOC_EVEN,   LOC_EVEN),

    /** Color spec defining ITU-R BT.709 standard, limited range. */
    VPI_COLOR_SPEC_BT709            = VPI_DETAIL_MAKE_CSPC(SPACE_BT709,  ENC_BT709,     XFER_BT709,     RANGE_LIMITED, LOC_EVEN,   LOC_EVEN),

    /** Color spec defining ITU-R BT.709 standard, full range. */
    VPI_COLOR_SPEC_BT709_ER         = VPI_DETAIL_MAKE_CSPC(SPACE_BT709,  ENC_BT709,     XFER_BT709,     RANGE_FULL,    LOC_EVEN,   LOC_EVEN),

    /** Color spec defining ITU-R BT.709 standard, limited range and linear transfer function. */
    VPI_COLOR_SPEC_BT709_LINEAR     = VPI_DETAIL_MAKE_CSPC(SPACE_BT709,  ENC_BT709,     XFER_LINEAR,    RANGE_LIMITED, LOC_EVEN,   LOC_EVEN),

    /** Color spec defining ITU-R BT.2020 standard, limited range. */
    VPI_COLOR_SPEC_BT2020           = VPI_DETAIL_MAKE_CSPC(SPACE_BT2020, ENC_BT2020,    XFER_BT2020,    RANGE_LIMITED, LOC_EVEN,   LOC_EVEN),

    /** Color spec defining ITU-R BT.2020 standard, full range. */
    VPI_COLOR_SPEC_BT2020_ER        = VPI_DETAIL_MAKE_CSPC(SPACE_BT2020, ENC_BT2020,    XFER_BT2020,    RANGE_FULL,    LOC_EVEN,   LOC_EVEN),

    /** Color spec defining ITU-R BT.2020 standard, limited range and linear transfer function. */
    VPI_COLOR_SPEC_BT2020_LINEAR    = VPI_DETAIL_MAKE_CSPC(SPACE_BT2020, ENC_BT2020,    XFER_LINEAR,    RANGE_LIMITED, LOC_EVEN,   LOC_EVEN),

    /** Color spec defining ITU-R BT.2020 standard, limited range and perceptual quantizer transfer function. */
    VPI_COLOR_SPEC_BT2020_PQ        = VPI_DETAIL_MAKE_CSPC(SPACE_BT2020, ENC_BT2020,    XFER_PQ,        RANGE_LIMITED, LOC_EVEN,   LOC_EVEN),

    /** Color spec defining ITU-R BT.2020 standard, full range and perceptual quantizer transfer function. */
    VPI_COLOR_SPEC_BT2020_PQ_ER     = VPI_DETAIL_MAKE_CSPC(SPACE_BT2020, ENC_BT2020,    XFER_PQ,        RANGE_FULL,    LOC_EVEN,   LOC_EVEN),

    /** Color spec defining ITU-R BT.2020 standard for constant luminance, limited range. */
    VPI_COLOR_SPEC_BT2020c          = VPI_DETAIL_MAKE_CSPC(SPACE_BT2020, ENC_BT2020c,   XFER_BT2020,    RANGE_LIMITED, LOC_EVEN,   LOC_EVEN),

    /** Color spec defining ITU-R BT.2020 standard for constant luminance, full range. */
    VPI_COLOR_SPEC_BT2020c_ER       = VPI_DETAIL_MAKE_CSPC(SPACE_BT2020, ENC_BT2020c,   XFER_BT2020,    RANGE_FULL,    LOC_EVEN,   LOC_EVEN),

    /** Color spec defining MPEG2 standard using ITU-R BT.601 encoding. */
    VPI_COLOR_SPEC_MPEG2_BT601      = VPI_DETAIL_MAKE_CSPC(SPACE_BT709,  ENC_BT601,     XFER_BT709,     RANGE_FULL,    LOC_EVEN,   LOC_CENTER),

    /** Color spec defining MPEG2 standard using ITU-R BT.709 encoding. */
    VPI_COLOR_SPEC_MPEG2_BT709      = VPI_DETAIL_MAKE_CSPC(SPACE_BT709,  ENC_BT709,     XFER_BT709,     RANGE_FULL,    LOC_EVEN,   LOC_CENTER),

    /** Color spec defining MPEG2 standard using SMPTE 240M encoding. */
    VPI_COLOR_SPEC_MPEG2_SMPTE240M  = VPI_DETAIL_MAKE_CSPC(SPACE_BT709,  ENC_SMPTE240M, XFER_SMPTE240M, RANGE_FULL,    LOC_EVEN,   LOC_CENTER),

    /** Color spec defining sRGB standard. */
    VPI_COLOR_SPEC_sRGB             = VPI_DETAIL_MAKE_CSPC(SPACE_BT709,  ENC_UNDEFINED, XFER_sRGB,      RANGE_FULL,    LOC_BOTH,   LOC_BOTH),

    /** Color spec defining sYCC standard. */
    VPI_COLOR_SPEC_sYCC             = VPI_DETAIL_MAKE_CSPC(SPACE_BT709,  ENC_BT601,     XFER_sYCC,      RANGE_FULL,    LOC_CENTER, LOC_CENTER),

    /** Color spec defining SMPTE 240M standard, limited range. */
    VPI_COLOR_SPEC_SMPTE240M        = VPI_DETAIL_MAKE_CSPC(SPACE_BT709,  ENC_SMPTE240M, XFER_SMPTE240M, RANGE_LIMITED, LOC_EVEN,   LOC_EVEN),

    /** Color spec defining Display P3 standard, with sRGB color transfer function. */
    VPI_COLOR_SPEC_DISPLAYP3        = VPI_DETAIL_MAKE_CSPC(SPACE_DCIP3,  ENC_UNDEFINED, XFER_sRGB,      RANGE_FULL,    LOC_BOTH,   LOC_BOTH),

    /** Color spec defining Display P3 standard, with linear color transfer function. */
    VPI_COLOR_SPEC_DISPLAYP3_LINEAR = VPI_DETAIL_MAKE_CSPC(SPACE_DCIP3,  ENC_UNDEFINED, XFER_LINEAR,    RANGE_FULL,    LOC_BOTH,   LOC_BOTH),

    /** Color spec used for images coming from an image sensor, right after demosaicing. */
    VPI_COLOR_SPEC_SENSOR           = VPI_DETAIL_MAKE_CSPC(SPACE_SENSOR, ENC_UNDEFINED, XFER_LINEAR,    RANGE_FULL,    LOC_BOTH,   LOC_BOTH),
} VPIColorSpec;
/* clang-format on */

/** Creates a user-defined color spec constant using abbreviated parameters.
 *
 * Example:
 * \code{.c}
 *   VPIColorSpec cspec = VPI_MAKE_COLOR_SPEC_ABBREV(SPACE_BT709, ENC_sRGB, XFER_sYCC, RANGE_FULL, LOC_ODD, LOC_EVEN);
 * \endcode
 *
 * @param[in] cspace   Color Space.
 * @param[in] encoding R'G'B' <-> Y'CbCr encoding.
 * @param[in] xferFunc Color transfer function.
 * @param[in] range    Color quantization range.
 * @param[in] locHoriz Horizontal chroma location.
 * @param[in] locVert  Vertical chroma location.
 *
 * @returns The user-defined \ref VPIColorSpec constant.
 */
#ifdef VPI_DOXYGEN
#    define VPI_MAKE_COLOR_SPEC_ABBREV(cspace, encoding, xferFunc, range, locHoriz, locVert)
#else
#    define VPI_MAKE_COLOR_SPEC_ABBREV (VPIColorSpec) VPI_DETAIL_MAKE_CSPC
#endif

/** Creates a user-defined color spec constant.
 *
 * Example:
 * \code{.c}
 *   VPIColorSpec cspec = VPI_MAKE_COLOR_SPEC_ABBREV(VPI_COLOR_SPACE_BT709, VPI_YCbCr_ENC_sRGB, VPI_COLOR_XFER_sYCC,
 *                                                      VPI_COLOR_RANGE_FULL, VPI_CHROMA_LOC_ODD, VPI_CHROMA_LOC_EVEN);
 * \endcode
 *
 * @param[in] cspace   Color Space.
 * @param[in] encoding R'G'B' <-> Y'CbCr encoding.
 * @param[in] xferFunc Color transfer function.
 * @param[in] range    Color quantization range.
 * @param[in] locHoriz Horizontal chroma location.
 * @param[in] locVert  Vertical chroma location.
 *
 * @returns The user-defined \ref VPIColorSpec constant.
 */
#ifdef VPI_DOXYGEN
#    define VPI_MAKE_COLOR_SPEC(cspace, encoding, xferFunc, range, locHoriz, locVert)
#else
#    define VPI_MAKE_COLOR_SPEC (VPIColorSpec) VPI_DETAIL_MAKE_COLOR_SPEC
#endif

/** Creates a user-defined \ref VPIColorSpec.
 *
 * @param[in] cspace   Color space.
 * @param[in] encoding R'G'B' <-> Y'CbCr encoding.
 * @param[in] xferFunc Color transfer function.
 * @param[in] range    Color quantization range.
 * @param[in] locHoriz Horizontal chroma location.
 * @param[in] locVert  Vertical chroma location.
 *
 * @returns The user-defined \ref VPIColorSpec.
 */
VPI_PUBLIC VPIColorSpec vpiMakeColorSpec(VPIColorSpace cspace, VPIYCbCrEncoding encoding,
                                         VPIColorTransferFunction xferFunc, VPIColorRange range,
                                         VPIChromaLocation locHoriz, VPIChromaLocation locVert);

/** Defines Bayer patterns used by RAW color model.
 * R,G,B represent the color primaries red, green, blue.
 * C represent a clear channel, it lets all light pass. */
typedef enum
{
    VPI_RAW_INVALID, /**< Invalid raw pattern. */

    /** Bayer format with X channel mapped to samples as follows:
     * - span 1: R G R G R G R G
     * - span 2: G B G B G B G B
     * (Y,Z,W are discarded) */
    VPI_RAW_BAYER_RGGB,
    /** Bayer format with X channel mapped to samples as follows:
     * - span 1: B G B G B G B G
     * - span 2: G R G R G R G R
     * \n(Y,Z,W are discarded)
     */
    VPI_RAW_BAYER_BGGR,
    /** Bayer format with X channel mapped to samples as follows:
     * - span 1: G R G R G R G R
     * - span 2: B G B G B G B G
     * \n(Y,Z,W are discarded)
     */
    VPI_RAW_BAYER_GRBG,
    /** Bayer format with X channel mapped to samples as follows:
     * - span 1: G B G B G B G B
     * - span 2: R G R G R G R G
     * \n(Y,Z,W are discarded)
     */
    VPI_RAW_BAYER_GBRG,

    /** Bayer format with X channel mapped to samples as follows:
     * - span 1: R C R C R C R C
     * - span 2: C B C B C B C B
     * \n(Y,Z,W are discarded)
     */
    VPI_RAW_BAYER_RCCB,
    /** Bayer format with X channel mapped to samples as follows:
     * - span 1: B C B C B C B C
     * - span 2: C R C R C R C R
     * \n(Y,Z,W are discarded)
     */
    VPI_RAW_BAYER_BCCR,
    /** Bayer format with X channel mapped to samples as follows:
     * - span 1: C R C R C R C R
     * - span 2: B C B C B C B C
     * \n(Y,Z,W are discarded)
     */
    VPI_RAW_BAYER_CRBC,
    /** Bayer format with X channel mapped to samples as follows:
     * - span 1: C B C B C B C B
     * - span 2: R C R C R C R C
     * \n(Y,Z,W are discarded)
     */
    VPI_RAW_BAYER_CBRC,

    /** Bayer format with X channel mapped to samples as follows:
     * - span 1: R C R C R C R C
     * - span 2: C C C C C C C C
     * \n(Y,Z,W are discarded)
     */
    VPI_RAW_BAYER_RCCC,
    /** Bayer format with X channel mapped to samples as follows:
     * - span 1: C R C R C R C R
     * - span 2: C C C C C C C C
     * \n(Y,Z,W are discarded)
     */
    VPI_RAW_BAYER_CRCC,
    /** Bayer format with X channel mapped to samples as follows:
     * - span 1: C C C C C C C C
     * - span 2: R C R C R C R C
     * \n(Y,Z,W are discarded)
     */
    VPI_RAW_BAYER_CCRC,
    /** Bayer format with X channel mapped to samples as follows:
     * - span 1: C C C C C C C C
     * - span 2: C R C R C R C R
     * \n(Y,Z,W are discarded)
     */
    VPI_RAW_BAYER_CCCR,

    /** Bayer format with X channel mapped to samples as follows:
     * - span 1: C C C C C C C C
     * - span 2: C C C C C C C C
     * \n(Y,Z,W are discarded)
     */
    VPI_RAW_BAYER_CCCC,

    /** \cond Do not use. */
    VPI_RAW_FORCE8 = UINT8_MAX
    /* \endcond */
} VPIRawPattern;

/** Defines how chroma-subsampling is done.
 * This is only applicable to image formats whose color model is YUV.
 * Other image formats must use \ref VPI_CSS_NONE.
 * Chroma subsampling is defined by 2 parameters:
 * - Horizontal resolution relative to luma resolution.
 * - Vertical resolution relative to luma resolution.
 */
typedef enum
{
    VPI_CSS_INVALID = -1, /**< Invalid chroma subsampling. */

    /** Used when no chroma subsampling takes place, specially for color specs without chroma components. */
    VPI_CSS_NONE = 0,

    /** 4:4:4 sub-sampling. Chroma has full horizontal and vertical resolution, meaning no chroma subsampling. */
    VPI_CSS_444 = VPI_CSS_NONE,

    /** 4:2:2 BT.601 sub-sampling. Chroma has half horizontal and full vertical resolutions.*/
    VPI_CSS_422,

    /** 4:2:2R BT.601 sub-sampling. Chroma has full horizontal and half vertical resolutions.*/
    VPI_CSS_422R,

    /** 4:1:1 sub-sampling. Chroma has 1/4 horizontal and full vertical resolutions.*/
    VPI_CSS_411,

    /** 4:1:1 sub-sampling. Chroma has full horizontal and 1/4 vertical resolutions.*/
    VPI_CSS_411R,

    /** 4:2:0 sub-sampling. Chroma has half horizontal and vertical resolutions.*/
    VPI_CSS_420,
} VPIChromaSubsampling;

/** Creates a \ref VPIChromaSubsampling given the horizontal and vertical sampling.
 *
 * @param[in] samplesHoriz Number of horizontal samples, 1, 2 or 4.
 * 
 * @param[in] samplesVert Number of vertical samples, 1, 2 or 4.
 *
 * @returns The chroma subsampling enumeration.
 * @retval #VPI_CSS_NONE Chroma subsampling isn't defined.
 */
VPI_PUBLIC VPIChromaSubsampling vpiMakeChromaSubsampling(int samplesHoriz, int samplesVert);

/** Get the number of chroma samples for each group of 4 horizontal luma samples.
 *
 * @param[in] css Chroma subsampling to be queried.
 *                + \p css must be valid.
 * 
 *
 * @returns The number of chroma samples for each group of 4 horizontal luma samples.
 * @retval 0 \p css is invalid.
 */
VPI_PUBLIC int vpiChromaSubsamplingGetSamplesHoriz(VPIChromaSubsampling css);

/** Get the number of chroma samples for each group of 4 vertical luma samples.
 *
 * @param[in] css Chroma subsampling to be queried.
 *                + \p css must be valid.
 *
 * @returns The number of chroma samples for each group of 4 vertical luma samples.
 * @retval 0 \p css is invalid.
 */
VPI_PUBLIC int vpiChromaSubsamplingGetSamplesVert(VPIChromaSubsampling css);

/** Get the chroma horizontal sampling location of a given color spec.
 *
 * @param[in] cspec Color spec to be queried.
 *                  + Color spec must be valid.
 *
 * @returns Chroma sample location with respect to luma horizontal coordinate.
 * @retval #VPI_CHROMA_LOC_BOTH \p cspec is invalid.
 */
VPI_PUBLIC VPIChromaLocation vpiColorSpecGetChromaLocHoriz(VPIColorSpec cspec);

/** Get the chroma vertical sample location of a given color spec
 *
 * @param[in] cspec Color spec to be queried.
 *                  + Color spec must be valid.
 *
 * @returns Chroma sample location with respect to luma vertical coordinate.
 * @retval #VPI_CHROMA_LOC_BOTH \p cspec is invalid.
 */
VPI_PUBLIC VPIChromaLocation vpiColorSpecGetChromaLocVert(VPIColorSpec cspec);

/** Set the chroma sample location of a given color spec
 *
 * @param[in] cspec The color spec to be modified.
 *                  + \p cspec must be valid.
 * 
 * @param[in] locHoriz Horizontal chroma sampling location with respect to luma coordinate.
 * 
 * @param[in] locVert Vertical chroma sampling location with respect to luma coordinate.
 *
 * @returns The new color spec with the updated chroma location.
 * @retval #VPI_COLOR_SPEC_INVALID \p cspec is invalid.
 */
VPI_PUBLIC VPIColorSpec vpiColorSpecSetChromaLoc(VPIColorSpec cspec, VPIChromaLocation locHoriz,
                                                 VPIChromaLocation locVert);

/** Get the color_space of a given color spec.
 *
 * @param[in] cspec Color spec to be queried.
 *                  + \p cspec must be valid.
 *
 * @returns The color_space associated with the color spec.
 * @retval #VPI_COLOR_SPACE_UNDEFINED \p cspec is invalid.
 */
VPI_PUBLIC VPIColorSpace vpiColorSpecGetSpace(VPIColorSpec cspec);

/** Set the color_space of a given color spec.
 *
 * @param[in] cspec Color spec to be updated.
 *                  + \p cspec must be valid.
 * 
 * @param[in] cspace The new color_space.
 *
 * @returns The new color spec with the updated color_space.
 * @retval #VPI_COLOR_SPEC_INVALID \p cspec is invalid.
 * @retval #VPI_COLOR_SPEC_INVALID \p cspace is invalid.
 */
VPI_PUBLIC VPIColorSpec vpiColorSpecSetSpace(VPIColorSpec cspec, VPIColorSpace cspace);

/** Get the R'G'B' <-> Y'CbCr encoding scheme of a given color spec.
 *
 * @param[in] cspec Color spec to be queried.
 *                  + \p cspec must be valid.
 *
 * @returns The Y'CbCr encoding scheme associated with the color spec.
 * @retval #VPI_YCbCr_ENC_UNDEFINED \p cspec is invalid.
 */
VPI_PUBLIC VPIYCbCrEncoding vpiColorSpecGetYCbCrEncoding(VPIColorSpec cspec);

/** Set the R'G'B' <-> Y'CbCr encoding scheme of a given color spec.
 *
 * @param[in] cspec Color spec to be updated.
 *                  + \p cspec must be valid.
 * 
 * @param[in] encoding The new Y'CbCr encoding scheme.
 *
 * @returns The new color spec with the updated Y'CbCr encoding scheme.
 * @retval #VPI_COLOR_SPEC_INVALID \p cspec is invalid.
 */
VPI_PUBLIC VPIColorSpec vpiColorSpecSetYCbCrEncoding(VPIColorSpec cspec, VPIYCbCrEncoding encoding);

/** Get the color transfer function of a given color spec.
 *
 * @param[in] cspec Color spec to be queried.
 *                  + \p cspec must be valid.
 *
 * @returns The color transfer function of a given color spec.
 * @retval #VPI_COLOR_XFER_LINEAR if \p cspec is invalid.
 */
VPI_PUBLIC VPIColorTransferFunction vpiColorSpecGetTransferFunction(VPIColorSpec cspec);

/** Set the color transfer function of a given color spec.
 *
 * @param[in] cspec Color spec to be updated.
 *                  + \p cspec must be valid.
 * 
 * @param[in] xferFunc The new color transfer function.
 *
 * @returns The new color spec with the updated color transfer function.
 * @retval #VPI_COLOR_SPEC_INVALID \p cspec is invalid.
 */
VPI_PUBLIC VPIColorSpec vpiColorSpecSetTransferFunction(VPIColorSpec cspec, VPIColorTransferFunction xferFunc);

/** Get the color quantization range of a given color spec.
 *
 * @param[in] cspec Color spec to be queried.
 *                  + \p cspec must be valid.
 *
 * @returns The color quantization range of a given color spec.
 * @retval #VPI_COLOR_RANGE_FULL \p cspec is invalid.
 */
VPI_PUBLIC VPIColorRange vpiColorSpecGetRange(VPIColorSpec cspec);

/** Set the color quantization range of a given color spec.
 *
 * @param[in] cspec Color spec to be updated.
 *                  + \p cspec must be valid.
 * 
 * @param[in] range The new color quantization range. 
 *
 * @returns The new color spec with the updated color quantization range.
 * @retval #VPI_COLOR_SPEC_INVALID \p cspec is invalid.
 */
VPI_PUBLIC VPIColorSpec vpiColorSpecSetRange(VPIColorSpec cspec, VPIColorRange range);

/** Returns a string representation of the color spec.
 *
 * @param[in] cspec Color spec whose name is to be returned.
 *
 * @returns The string representation of the color spec.
 *          Returned string is valid until next call of this function from the same calling thread.
 *          Returned pointer must not be freed.
 */
VPI_PUBLIC const char *vpiColorSpecGetName(VPIColorSpec cspec);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* NV_VPI_COLORSPEC_H */
