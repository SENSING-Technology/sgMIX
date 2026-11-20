/*
 * SPDX-FileCopyrightText: Copyright (c) 2013-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 * SPDX-License-Identifier: LicenseRef-NvidiaProprietary
 *
 * NVIDIA CORPORATION, its affiliates and licensors retain all intellectual
 * property and proprietary rights in and to this material, related
 * documentation and any modifications thereto. Any use, reproduction,
 * disclosure or distribution of this material and related documentation
 * without an express license agreement from NVIDIA CORPORATION or
 * its affiliates is strictly prohibited.
 */

/// @file
/// @brief <b> NVIDIA Media Interface: 2D Processing Control </b>
///
/// @b Description: This file contains the #image_2d_api "Image 2D
/// Processing API."
///

#ifndef NVMEDIA_2D_H
#define NVMEDIA_2D_H

#include "nvmedia_core.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/// @defgroup 6x_nvmedia_2d_api 2D Processing
///
/// The 2D Processing API encompasses all NvMedia 2D image processing
/// related functionality.
///
/// @ingroup 6x_nvmedia_top
/// @{
///

/// @brief Maximum number of horizontal LDC regions.
#define NVMEDIA_2D_MAX_LDC_REGIONS_X 4U

/// @brief Maximum number of vertical LDC regions.
#define NVMEDIA_2D_MAX_LDC_REGIONS_Y 4U

/// @brief 2D filter mode.
///
/// This enum describes the filter modes that are supported by NvMedia 2D.
///
/// If no filter mode is explicitly set then an appropriate mode will be
/// automatically selected, depending on the current scaling factor and
/// hardware restrictions.
///
/// @sa NvMedia2DSetFilterBuffer()
///
typedef enum
{
    /// @brief Filtering is disabled.
    ///
    /// This mode results in nearest-neighbor filtering being used.
    ///
    NVMEDIA_2D_FILTER_OFF = 0x1,

    /// @brief Low quality filtering.
    ///
    /// This mode results in bilinear filtering being used.
    ///
    NVMEDIA_2D_FILTER_LOW,

    /// @brief Medium quality filtering.
    ///
    /// This mode results in 5-tap filtering being used. By default, the filter
    /// used is a Lanczos filter. The coefficients can be overridden with
    /// NvMedia2DSetFilterBuffer().
    ///
    NVMEDIA_2D_FILTER_MEDIUM,

    /// @brief Highest quality filtering.
    ///
    /// This mode results in 10-tap filtering being used. By default, the filter
    /// used is a Lanczos filter. The coefficients can be overridden with
    /// NvMedia2DSetFilterBuffer().
    ///
    NVMEDIA_2D_FILTER_HIGH

} NvMedia2DFilter;

/// @brief 2D rotation/transform.
///
/// This enum describes the 2D transform to apply during a blit operation.
///
/// Transformations are used to rotate and mirror the source surface of
/// a blit operation. The destination rectangle is not affected by any
/// transformation settings.
///
/// @ref NvMediaTransform identifies the transformations that can be applied
/// as a combination of rotation and mirroring.
///
/// Specifically, given a hypothetical 2x2 image, applying these operations
/// would yield the following results:
/// @code
///
///       INPUT                      OUTPUT
///
///      +---+---+                  +---+---+
///      | 0 | 1 |     IDENTITY     | 0 | 1 |
///      +---+---+ ---------------> +---+---+
///      | 2 | 3 |                  | 2 | 3 |
///      +---+---+                  +---+---+
///
///      +---+---+                  +---+---+
///      | 0 | 1 |    ROTATE_90     | 1 | 3 |
///      +---+---+ ---------------> +---+---+
///      | 2 | 3 |                  | 0 | 2 |
///      +---+---+                  +---+---+
///
///      +---+---+                  +---+---+
///      | 0 | 1 |    ROTATE_180    | 3 | 2 |
///      +---+---+ ---------------> +---+---+
///      | 2 | 3 |                  | 1 | 0 |
///      +---+---+                  +---+---+
///
///      +---+---+                  +---+---+
///      | 0 | 1 |    ROTATE_270    | 2 | 0 |
///      +---+---+ ---------------> +---+---+
///      | 2 | 3 |                  | 3 | 1 |
///      +---+---+                  +---+---+
///
///      +---+---+                  +---+---+
///      | 0 | 1 |  FLIP_HORIZONTAL | 1 | 0 |
///      +---+---+ ---------------> +---+---+
///      | 2 | 3 |                  | 3 | 2 |
///      +---+---+                  +---+---+
///
///      +---+---+                  +---+---+
///      | 0 | 1 |  INVTRANSPOSE    | 3 | 1 |
///      +---+---+ ---------------> +---+---+
///      | 2 | 3 |                  | 2 | 0 |
///      +---+---+                  +---+---+
///
///      +---+---+                  +---+---+
///      | 0 | 1 |  FLIP_VERTICAL   | 2 | 3 |
///      +---+---+ ---------------> +---+---+
///      | 2 | 3 |                  | 0 | 1 |
///      +---+---+                  +---+---+
///
///      +---+---+                  +---+---+
///      | 0 | 1 |    TRANSPOSE     | 0 | 2 |
///      +---+---+ ---------------> +---+---+
///      | 2 | 3 |                  | 1 | 3 |
///      +---+---+                  +---+---+
///
/// @endcode
///
typedef enum
{
    /// @brief No transform. This is the default behavior.
    NVMEDIA_2D_TRANSFORM_NONE = 0x0,

    /// @brief Rotate 90 degrees anti-clockwise.
    NVMEDIA_2D_TRANSFORM_ROTATE_90,

    /// @brief Rotate 180 degrees.
    NVMEDIA_2D_TRANSFORM_ROTATE_180,

    /// @brief Rotate 270 degrees anti-clockwise.
    NVMEDIA_2D_TRANSFORM_ROTATE_270,

    /// @brief Flip horizontally (mirror along Y axis).
    NVMEDIA_2D_TRANSFORM_FLIP_HORIZONTAL,

    /// @brief Apply inverse transpose (mirror along diagonal axis from
    /// top-right to bottom-left).
    NVMEDIA_2D_TRANSFORM_INV_TRANSPOSE,

    /// @brief Flip vertically (mirror along X axis).
    NVMEDIA_2D_TRANSFORM_FLIP_VERTICAL,

    /// @brief Apply transpose (mirror along diagonal axis from top-left to
    /// bottom-right).
    NVMEDIA_2D_TRANSFORM_TRANSPOSE

} NvMedia2DTransform;

/// @brief Blending to use when compositing surfaces.
///
typedef enum
{
    /// @brief Disable blending.
    ///
    /// The color and alpha channels of the source surfaces are copied to the
    /// destination surface as is. The constant alpha factor is ignored. This is
    /// the default mode.
    ///
    NVMEDIA_2D_BLEND_MODE_DISABLED,

    /// @brief Ignore the alpha channel values.
    ///
    /// Ignore the alpha channel values in the pixel data and only use the
    /// constant alpha factor that was passed when configuring blending.
    ///
    /// The color channels are blended with:
    ///
    /// `result = src * constantAlpha + dst * (1 - constantAlpha)`
    ///
    NVMEDIA_2D_BLEND_MODE_CONSTANT_ALPHA,

    /// @brief Treat color and alpha channels independently.
    ///
    /// Use the product of the alpha channel values in the pixel data and the
    /// constant alpha factor to interpolate between color channel values of the
    /// blended surfaces.
    ///
    /// The color channels are blended with:
    ///
    /// `result = src * srcAlpha * constantAlpha + dst * (1 - srcAlpha * constantAlpha)`
    ///
    NVMEDIA_2D_BLEND_MODE_STRAIGHT_ALPHA,

    /// @brief Treat color channels as having the alpha channel premultiplied.
    ///
    /// As NVMEDIA_2D_BLEND_MODE_STRAIGHT_ALPHA, but assume color channel values
    /// have been premultiplied by the alpha channel value.
    ///
    /// The color channels are blended with:
    ///
    /// `result = src * constantAlpha + dst * (1 - srcAlpha * constantAlpha)`
    ///
    NVMEDIA_2D_BLEND_MODE_PREMULTIPLIED_ALPHA

} NvMedia2DBlendMode;

/// @brief Attributes structure for #NvMedia2DCreate().
///
/// This type holds the attributes to control the behaviour of the NvMedia2D
/// context. These attributes take effect during the call to #NvMedia2DCreate().
///
/// @sa NvMedia2DGetDefaultAttributes()
/// @sa NvMedia2DCreate()
///
typedef struct
{
    /// @brief Number of compose parameters objects to allocate.
    ///
    /// Must be in range [1, 16].
    ///
    /// @sa NvMedia2DComposeParameters
    /// @sa NvMedia2DGetComposeParameters()
    ///
    uint32_t numComposeParameters;

    /// @brief Maximum number of buffers that can be registered.
    ///
    /// Must be in range [1, 1024].
    ///
    /// @sa NvMedia2DRegisterNvSciBufObj()
    ///
    uint32_t maxRegisteredBuffers;

    /// @brief Maximum number of sync objects that can be registered.
    ///
    /// Must be in range [1, 256].
    ///
    /// @sa NvMedia2DRegisterNvSciSyncObj()
    ///
    uint32_t maxRegisteredSyncs;

    /// @brief Maximum number of filter buffers that can be created.
    ///
    /// Must be in range [0, 16].
    ///
    /// @sa NvMedia2DFilterBuffer
    /// @sa NvMedia2DCreateFilterBuffer()
    uint32_t maxFilterBuffers;

    /// @brief Maximum number of LDC warp maps that can be created.
    ///
    /// Must be in range [0, 64].
    ///
    /// @sa NvMedia2DLdcWarpMap
    /// @sa NvMedia2DCreateLdcWarpMap()
    uint32_t maxLdcWarpMaps;

    /// @brief Maximum number of LDC mask maps that can be created.
    ///
    /// Must be in range [0, 64].
    ///
    /// @sa NvMedia2DLdcMaskMap
    /// @sa NvMedia2DCreateLdcMaskMap()
    uint32_t maxLdcMaskMaps;

    /// @brief Maximum number of TNR states that can be created.
    ///
    /// Must be in range [0, 64].
    ///
    /// @sa NvMedia2DTnrState
    /// @sa NvMedia2DCreateTnrState()
    uint32_t maxTnrStates;

    /// @brief Internal use only.
    uint32_t flags;

} NvMedia2DAttributes;

/// @brief Attributes structure for #NvMedia2DCreateLdcWarpMap().
///
/// This type holds the attributes to control the fixed characteristics of an
/// LDC warp map object. These attributes take effect during the call to
/// NvMedia2DCreateLdcWarpMap() and are used to determine what resources need
/// to be allocated for the warp map.
///
/// @sa NvMedia2DGetDefaultLdcWarpMapAttributes()
/// @sa NvMedia2DCreateLdcWarpMap()
///
typedef struct
{
     /// @brief Maximum allowed number of control points horizontally
     ///        in a warp map. This controls the amount of memory allocated
     ///        for warp map.
     ///
     /// When NvMedia2DCapabilities::usesLdcRegionConfig is false, this value
     /// is used directly as the warp map width. When
     /// NvMedia2DCapabilities::usesLdcRegionConfig is true, this value only
     /// controls memory allocation and the actual warp map dimensions are
     /// determined by the region configuration parameters.
     ///
     /// Must be in range [1, 16384].
     ///
    uint32_t maxWarpMapWidth;

     /// @brief Maximum allowed number of control points vertically
     ///        in a warp map. This controls the amount of memory allocated
     ///        for warp map.
     ///
     /// When NvMedia2DCapabilities::usesLdcRegionConfig is false, this value
     /// is used directly as the warp map height. When
     /// NvMedia2DCapabilities::usesLdcRegionConfig is true, this value only
     /// controls memory allocation and the actual warp map dimensions are
     /// determined by the region configuration parameters.
     ///
     /// Must be in range [1, 16384].
     ///
    uint32_t maxWarpMapHeight;

} NvMedia2DLdcWarpMapAttributes;

/// @brief Attributes structure for #NvMedia2DCreateLdcMaskMap().
///
/// This type holds the attributes to control the fixed characteristics of an
/// LDC mask map object. These attributes take effect during the call to
/// NvMedia2DCreateLdcMaskMap() and are used to determine what resources need
/// to be allocated for the mask map.
///
/// @sa NvMedia2DGetDefaultLdcMaskMapAttributes()
/// @sa NvMedia2DCreateLdcMaskMap()
///
typedef struct
{
    /// @brief Maximum allowed destination rectangle width.
    ///        These control the amount of memory allocated for the mask map.
    ///
    /// If used, must be in range [64, 16384].
    ///
    uint32_t maxDstWidth;

    /// @brief Maximum allowed destination rectangle height.
    ///        These control the amount of memory allocated for the mask map.
    ///
    /// If used, must be in range [16, 16384].
    ///
    uint32_t maxDstHeight;

} NvMedia2DLdcMaskMapAttributes;

/// @brief Attributes structure for #NvMedia2DCreateTnrState().
///
/// This type holds the attributes to control the fixed characteristics of a
/// TNR state object. These attributes take effect during the call to
/// NvMedia2DCreateTnrState() and are used to determine what resources need
/// to be allocated for the object.
///
/// @sa NvMedia2DGetDefaultTnrStateAttributes()
/// @sa NvMedia2DCreateTnrState()
///
typedef struct
{
    /// @brief Maximum allowed destination rectangle width.
    ///        These control the amount of memory allocated for the internal
    ///        TNR memory resources.
    ///
    /// If used, must be in range [64, 16384].
    ///
    uint32_t maxDstWidth;

    /// @brief Maximum allowed destination rectangle height.
    ///        These control the amount of memory allocated for the internal
    ///        TNR memory resources.
    ///
    /// If used, must be in range [16, 16384].
    ///
    uint32_t maxDstHeight;

} NvMedia2DTnrStateAttributes;

/// @brief Capabilities of the NvMedia2D context.
///
/// This type holds the capabilities offered by the NvMedia2D context. These
/// capabilities are dependent on the current VIC chip.
///
/// @sa NvMedia2DGetCapabilities()
///
typedef struct
{
    /// @brief Maximum number of input layers supported by compose operation.
    uint32_t maxComposeLayers;

    /// @brief Minimum downscale factor supported by compose operation.
    float minScaleFactor;

    /// @brief Maximum upscale factor supported by compose operation.
    float maxScaleFactor;

    /// @brief Minimum height of surface.
    uint32_t minSurfaceHeight;

    /// @brief Maximum height of surface.
    uint32_t maxSurfaceHeight;

    /// @brief Minimum width of surface.
    uint32_t minSurfaceWidth;

    /// @brief Maximum width of surface.
    uint32_t maxSurfaceWidth;

    /// @brief Width of the tiles the HW processes the surfaces in.
    uint32_t tileWidth;

    /// @brief Height of the tiles the HW processes the surfaces in.
    uint32_t tileHeight;

    /// @brief Compare CRC operation is supported.
    bool supportsCompareCrc;

    /// @brief Read CRC operation is supported.
    bool supportsReadCrc;

    /// @brief Compose operation is supported.
    bool supportsCompose;

    /// @brief Per-layer stand-alone LDC operation is supported.
    bool supportsLdc;

    /// @brief Per-layer LDC + Compose operation is supported.
    bool supportsComposeLdc;

    /// @brief Compose + TNR output operation is supported.
    bool supportsComposeTnrOutput;

    /// @brief Compose + XSobel post-process operation is supported.
    bool supportsComposeXSobel;

    /// @brief LDC mask map is supported.
    bool supportsLdcMaskMap;

    /// @brief Processing 16-bit YUV formats natively is supported.
    bool supports16BitYUVNatively;

    /// @brief Non-aligned destination and clip rectangles with chroma
    ///        sub-sampled YUV formats are supported.
    bool supportsNonAlignedRects;

    /// @brief Filter buffers are supported.
    bool supportsFilterBuffer;

    /// @brief TNR is supported.
    bool supportsTnr;

    /// @brief Context priority is supported.
    bool supportsContextPriority;

    /// @brief LDC region configuration is used.
    bool usesLdcRegionConfig;

    /// @brief XSobel post-process operation is supported.
    bool supportsXSobel;

    /// @brief Transform matrix is supported.
    bool supportsTransformMatrix;

    /// @brief Source, destination and clip rectangles are supported.
    bool supportsRectangles;

    /// @brief Non-filled background is supported.
    bool supportsNonFilledBackground;

    /// @brief Task status is supported.
    bool supportsTaskStatus;

} NvMedia2DCapabilities;

/// @brief Stores configuration for the NvMedia2DCompose() operation.
///
/// This object stores the information needed to configure the 2D operation that
/// is executed inside the NvMedia2DCompose() function.
///
/// The underlying object cannot be instantiated directly by the client. Instead
/// use the #NvMedia2DGetComposeParameters() function to retrieve a handle to
/// an available instance.
///
/// Value 0 is never a valid handle value, and can be used to initialize an
/// NvMedia2DComposeParameters handle to a known value.
///
/// @sa NvMedia2DGetComposeParameters()
/// @sa NvMedia2DReleaseComposeParameters()
/// @sa NvMedia2DInsertPreNvSciSyncFence()
/// @sa NvMedia2DSetNvSciSyncObjforEOF()
/// @sa NvMedia2DSetSrcGeometry()
/// @sa NvMedia2DSetSrcFilter()
/// @sa NvMedia2DSetSrcLdcTransformMatrix()
/// @sa NvMedia2DSetSrcLdcWarpMap()
/// @sa NvMedia2DSetSrcLdcMaskMap()
/// @sa NvMedia2DSetSrcNvSciBufObj()
/// @sa NvMedia2DSetDstNvSciBufObj()
/// @sa NvMedia2DCompose()
///
typedef uint32_t NvMedia2DComposeParameters;

/// @brief Stores a filter buffer which coefficients can be configured.
///
/// The underlying object cannot be instantiated directly by the client. Instead
/// use the #NvMedia2DCreateFilterBuffer() function to create and retrieve
/// a handle to an available instance with resources allocated.
/// Use #NvMedia2DDestroyFilterBuffer() function to deallocate resources
/// allocated by #NvMedia2DCreateFilterBuffer() and release an instance.
///
/// Value 0 is never a valid handle value, and can be used to initialize an
/// NvMedia2DFilterBuffer handle to a known value.
///
/// @sa NvMedia2DCreateFilterBuffer()
/// @sa NvMedia2DDestroyFilterBuffer()
///
typedef uint32_t NvMedia2DFilterBuffer;

/// @brief Stores resources for an LDC warp map.
///
/// The underlying object cannot be instantiated directly by the client. Instead
/// use the #NvMedia2DCreateLdcWarpMap() function to create and retrieve
/// a handle to an available instance with resources allocated.
/// Use #NvMedia2DDestroyLdcWarpMap() function to deallocate resources
/// allocated by #NvMedia2DCreateLdcWarpMap() and release an instance.
///
/// Value 0 is never a valid handle value, and can be used to initialize an
/// NvMedia2DLdcWarpMap handle to a known value.
///
/// @sa NvMedia2DCreateLdcWarpMap()
/// @sa NvMedia2DDestroyLdcWarpMap()
///
typedef uint32_t NvMedia2DLdcWarpMap;

/// @brief Stores resources for an LDC mask map.
///
/// The underlying object cannot be instantiated directly by the client. Instead
/// use the #NvMedia2DCreateLdcMaskMap() function to create and retrieve
/// a handle to an available instance with resources allocated.
/// Use #NvMedia2DDestroyLdcMaskMap() function to deallocate resources
/// allocated by #NvMedia2DCreateLdcMaskMap() and release an instance.
///
/// Value 0 is never a valid handle value, and can be used to initialize an
/// NvMedia2DLdcMaskMap handle to a known value.
///
/// @sa NvMedia2DCreateLdcMaskMap()
/// @sa NvMedia2DDestroyLdcMaskMap()
///
typedef uint32_t NvMedia2DLdcMaskMap;

/// @brief Stores state and resources for TNR operation.
///
/// The underlying object cannot be instantiated directly by the client. Instead
/// use the #NvMedia2DCreateTnrState() function to create and retrieve
/// a handle to an available instance with resources allocated.
/// Use #NvMedia2DDestroyTnrState() function to deallocate resources
/// allocated by #NvMedia2DCreateTnrState() and release an instance.
///
/// Value 0 is never a valid handle value, and can be used to initialize an
/// NvMedia2DTnrState handle to a known value.
///
/// @sa NvMedia2DCreateTnrState()
/// @sa NvMedia2DDestroyTnrState()
///
typedef uint32_t NvMedia2DTnrState;

/// @brief Stores information returned from NvMedia2DCompose().
///
/// This type holds the information about an operation that has been submitted
/// to NvMedia2DCompose() for execution.
///
/// This struct itself can be considered a POD type, so it does not have any
/// functions to create/destroy it.
///
/// @sa NvMedia2DCompose()
///
typedef struct
{
    /// @brief ID number for operation that was submitted to NvMedia2DCompose().
    ///
    /// The number will wrap once the uint64_t range has been exceeded. A value
    /// of 0 indicates that no operation was submitted.
    ///
    uint64_t operationId;

} NvMedia2DComposeResult;

/// @brief Coefficients values structure for 5-tap custom filter.
///
/// This type holds the list of coefficients values for a 1D polyphasic
/// filter with 5 taps and 32 phases.
///
/// This struct itself can be considered a POD type, so it does not have any
/// functions to create/destroy it.
///
/// @sa NvMedia2DComputeFilterCoefficients5Tap()
///
typedef struct
{
    /// @brief Array of coefficients values, ordered by phase, then by tap.
    ///
    /// The coefficient values are interpreted as 10-bit signed binary fixed
    /// point numbers in two's complement format, with (from least to most
    /// significant bits):
    /// - 8 fraction bits
    /// - 1 integer bit
    /// - 1 sign bit
    ///
    /// Other bits of the values are discarded.
    ///
    int16_t coeffs[32][5];

} NvMedia2DFilterCoefficients5Tap;

/// @brief Coefficients values structure for 10-tap custom filter.
///
/// This type holds the list of coefficients values for a 1D polyphasic
/// filter with 10 taps and 32 phases.
///
/// This struct itself can be considered a POD type, so it does not have any
/// functions to create/destroy it.
///
/// @sa NvMedia2DComputeFilterCoefficients10Tap()
///
typedef struct
{
    /// @brief Array of coefficients values, ordered by phase, then by tap.
    ///
    /// The coefficient values are interpreted as 10-bit signed binary fixed
    /// point numbers in two's complement format, with (from least to most
    /// significant bits):
    /// - 8 fraction bits
    /// - 1 integer bit
    /// - 1 sign bit
    ///
    /// Other bits of the values are discarded.
    ///
    int16_t coeffs[32][10];

} NvMedia2DFilterCoefficients10Tap;

/// @brief Holds the NvMedia LDC region configuration.
///
/// This structure defines the layout of the control points
/// in the destination image.
///
/// The control points are used as the basis for geometric transformation from
/// source image to destination image. The remaining points are transformed
/// based on the interpolation. Thus the density of the control points controls
/// the quality of the geometric transformation.
///
/// This is an example of defining regions in the image:
/// @code
///
///           (dstRect->x1 - dstRect->x0)
///        /                              \'
///       /                                \'
///      /                                  \'
///
///  regionWidth[0]             regionWidth[numRegionsX -1]
///      /     \                     /      \'
///     |-------|                   |--------|
///     --------------------------------------                                      \'
///     |******** ******** ...      ******** |--                                     \'
///     |* +  + * *      * ...      *      * |   \                                    \'
///     |* +  + * *      *          *      * | regionHeight[0]                         \'
///     |* +  + * *      *          *      * |   /                                      \'
///     |******** ********          ******** |--                                         \'
///     |..                         ..       |                                            \'
///     |..                         ..       |
///     |..                         ..       |                           (dstRect->y1 - dstRect->y0)
///     |                                    |
///     |                                    |                                            /
///     |******** ********...       ******** |--                                         /
///     |*      * *      *...       *      * |  \                                       /
///     |*      * *      *          *      * | regionHeight[numRegionsY -1]            /
///     |*      * *      *          *      * |  /                                     /
///     |******** ********          ******** |--                                     /
///     --------------------------------------
///
/// @endcode
///
/// This is an example of defining control points in one region:
/// @code
///     *********
///     *  +  + *-- \'
///     *       *     (1 << controlPointYSpacingLog2)
///     *  +  + *-- /
///     *       *
///     *********
///        |--|
///      (1 << controlPointXSpacingLog2)
///
/// @endcode
/// See #NvMedia2DLdcWarpMapParameters for additional details of how
/// the control points are organized.
///
/// Restrictions
///
/// * numRegionsX cannot exceed #NVMEDIA_2D_MAX_LDC_REGIONS_X.
/// * numRegionsY cannot exceed #NVMEDIA_2D_MAX_LDC_REGIONS_Y.
/// * Alignment restrictions:
///   - regionWidth[N] must be equal or greater than and a multiple of
///     NvMedia2DCapabilities::tileWidth for each N, except N = numRegionsX - 1.
///   - regionHeight[N] must be equal or greater than and a multiple of
///     NvMedia2DCapabilities::tileHeight for each N, except N = numRegionsY -
///     1.
/// * Region width/height restrictions:
///  - regionWidth[N] must be less than or equal to 16384 for each N.
///  - regionHeight[N] must be less than or equal to 16384 for each N.
///  - The sum of regionWidth[] must be equal to the width of the dstRect
///    argument passed to #NvMedia2DSetSrcGeometry(), and less than or equal to
///    16384.
///  - The sum of regionHeight[] must be equal to the height of dstRect
///    argument passed to #NvMedia2DSetSrcGeometry(), and less than or equal to
///    16384.
/// * Control point interval restrictions:
///  - controlPointXSpacingLog2[N] must be in range [0, 7] for each N.
///  - controlPointYSpacingLog2[N] must be in range [0, 7] for each N.
///
/// @sa NvMedia2DCapabilities
/// @sa NvMedia2DLdcTransformMatrix
/// @sa NvMedia2DLdcWarpMapParameters
/// @sa NvMedia2DFillLdcWarpMap()
/// @sa NvMedia2DSetSrcLdcTransformMatrix()
///
typedef struct
{
    /// @brief Holds the number of horizontal regions.
    ///        Allowed values are [1, 4], inclusive.
    uint32_t numRegionsX;

    /// @brief Holds the number of vertical regions.
    ///        Allowed values are [1, 4], inclusive.
    uint32_t numRegionsY;

    /// @brief Holds the width of regions.
    uint32_t regionWidth[NVMEDIA_2D_MAX_LDC_REGIONS_X];

    /// @brief Holds the height of regions.
    uint32_t regionHeight[NVMEDIA_2D_MAX_LDC_REGIONS_Y];

    /// @brief Holds the horizontal interval between the control points
    ///        in each region in log2 space.
    uint32_t controlPointXSpacingLog2[NVMEDIA_2D_MAX_LDC_REGIONS_X];

    /// @brief Holds the vertical interval between the control points
    ///        in each region in log2 space.
    uint32_t controlPointYSpacingLog2[NVMEDIA_2D_MAX_LDC_REGIONS_Y];

} NvMedia2DLdcRegionParameters;

/// @brief Holds configutation for LDC transform matrix.
///
/// @sa NvMedia2DSetSrcLdcTransformMatrix()
///
typedef struct
{
    /// @brief Specifies the regions parameters.
    NvMedia2DLdcRegionParameters regionParams;

    /// @brief Holds the inverse perspective matrix,
    ///        defined in the following way:
    ///            |p[0][0]  p[0][1]  p[0][2]|
    ///            |p[1][0]  p[1][1]  p[1][2]|
    ///            |p[2][0]  p[2][1]  p[2][2]|
    float matrixCoeffs[3][3];

} NvMedia2DLdcTransformMatrix;

/// @brief Represents a LDC warp map control point.
///
/// @sa NvMedia2DLdcWarpMapParameters
///
typedef struct
{
    /// @brief Horizontal coordinate.
    ///
    /// Clamped to range [-32768, 32767.96875]. NaN is mapped to -32768.
    ///
    float x;

    /// @brief Vertical coordinate.
    ///
    /// Clamped to range [-32768, 32767.96875]. NaN is mapped to -32768.
    ///
    float y;

} NvMedia2DLdcControlPoint;

/// @brief Holds the NvMedia definition of an LDC Warp Map.
///
/// Warp Map stores the mappings of each control point from destination
/// image to input image.
///
/// **When NvMedia2DCapabilities::usesLdcRegionConfig is true:**
/// The coordinates of control points in a destination image are defined by regionParams.
/// If warp map region N has controlPointXSpacingLog2[N] = 0 it is considered to
/// have "full columns", and with controlPointYSpacingLog2[N] = 0 "full rows".
/// Otherwise, the region has "sparse columns/rows".
///
/// The warp map processing always operates on "tiles" of size
/// NvMedia2DCapabilities::tileWidth x NvMedia2DCapabilities::tileHeight pixels.
/// When determining the number of control points in warp map region N,
/// regionWidth[N] is rounded up to the next multiple of tile width and
/// regionHeight[N] to the next multiple of tile height.
///
/// In a warp map region with full columns/rows, the number of columns/rows of
/// control points equals the tile size rounded up width/height of the region.
///
/// In a warp map region with sparse columns/rows,
/// - First control point column/row is at position 0 within the region.
/// - There's always an odd number of columns/rows of control points.
/// - The position of the last control point column/row is always outside the
///   tile size rounded up width/height of the region.
///
/// For example, if region width is 192 and control point interval is 64, it
/// will result in 5 columns of control points with X-coordinates 0, 64, 128,
/// 192 and 256.
///
/// The code below shows how to calculate the exact number of control points
/// for a single region:
///
/// @code
///     NvMedia2DCapabilities caps;
///     NvMedia2DLdcRegionParameters regionInfo;
///     ...
///     w = RoundUp(regionInfo.regionWidth[i], caps.tileWidth);
///     dx = 1 << regionInfo.controlPointXSpacingLog2[i];
///     numColumns = RoundUp(w, 2 * dx) / dx + (dx == 1 ? 0 : 1);
///
///     h = RoundUp(regionInfo.regionHeight[i], caps.tileHeight);
///     dy = 1 << regionInfo.controlPointYSpacingLog2[i];
///     numRows = RoundUp(h, 2 * dy) / dy + (dy == 1 ? 0 : 1);
/// @endcode
///
/// Where `RoundUp()` rounds the first argument up to the nearest multiple of
/// the second argument.
///
/// **When NvMedia2DCapabilities::usesLdcRegionConfig is false:**
/// The warp map operates as a simple rectangular grid where regionParams is ignored.
/// The control points are evenly distributed across the output surface
/// dimensions with the grid size determined by maxWarpMapWidth × maxWarpMapHeight
/// from NvMedia2DLdcWarpMapAttributes.
///
/// @sa NvMedia2DCapabilities
/// @sa NvMedia2DLdcRegionParameters
/// @sa NvMedia2DFillLdcWarpMap()
/// @sa NvMedia2DSetSrcLdcTransformMatrix()
///
typedef struct
{
    /// @brief Specifies the regions paramters.
    ///
    /// Ignored when NvMedia2DCapabilities::usesLdcRegionConfig is false.
    ///
    NvMedia2DLdcRegionParameters regionParams;

    /// @brief Number of control points in the controlPoints array.
    ///
    /// When NvMedia2DCapabilities::usesLdcRegionConfig is true, must equal
    /// the number of control points specified by regionParams.
    /// When NvMedia2DCapabilities::usesLdcRegionConfig is false, must equal
    /// maxWarpMapWidth * maxWarpMapHeight from the warp map attributes.
    ///
    uint32_t numControlPoints;

    /// @brief Array of control points across all the regions
    ///        stored in a row-major order.
    NvMedia2DLdcControlPoint const *controlPoints;

} NvMedia2DLdcWarpMapParameters;

/// @brief Holds the LDC Mask Map information.
///
/// With the destination rectangle, this mask map surface defines the region of
/// interest in the destination image.
/// The dstRect argument passed to #NvMedia2DSetSrcGeometry() defines the
/// destination rectangle.
///
/// @sa NvMedia2DFillLdcMaskMap()
///
typedef struct
{
    /// @brief Holds the width in pixels of the mask map surface,
    ///        which must be equal to the width of the destination rectangle.
    uint32_t width;

    /// @brief Holds the height in pixels of the mask map surface,
    ///        which must be equal to the height of the destination rectangle.
    uint32_t height;

    /// @brief Indicates whether to fill the masked pixel with
    ///        the mask color.
    bool useMaskColor;

    /// @brief Holds the Y channel value of the mask color.
    ///
    /// If used, must be in range [0, 1].
    ///
    float maskColorY;

    /// @brief Holds the U channel value of the mask color.
    ///
    /// If used, must be in range [0, 1].
    ///
    float maskColorU;

    /// @brief Holds the V channel value of the mask color.
    ///
    /// If used, must be in range [0, 1].
    ///
    float maskColorV;

    /// @brief Holds the value for the Mask Map surface.
    ///
    /// The Mask Map surface is stored row by row. Each stored bool value is
    /// used to indicate whether this pixel has been masked or not.
    /// A true value means that the pixel is not to be masked.
    /// The buffer needs to hold width * height bool objects.
    bool const *pixelMasks;

} NvMedia2DLdcMaskMapParameters;

/// @brief Holds the TNR3 initialization parameters.
///
/// @sa NvMedia2DSetTnrParameters()
///
typedef struct
{
    /// @brief Holds the sigma of the luma for spatial filter.
    ///
    /// Must be in range [1, 46340].
    ///
    uint32_t spatialSigmaLuma;

    /// @brief Holds the sigma of the chroma for spatial filter.
    ///
    /// Must be in range [1, 46340].
    ///
    uint32_t spatialSigmaChroma;

    /// @brief Holds the sigma of the luma for range filter.
    ///
    /// Must be in range [1, 46340].
    ///
    uint32_t rangeSigmaLuma;

    /// @brief Holds the sigma of the chroma for range filter.
    ///
    /// Must be in range [1, 46340].
    ///
    uint32_t rangeSigmaChroma;

    /// @brief Holds the SAD multiplier parameter.
    ///
    /// Must be in range [0, 1.125].
    ///
    float sadMultiplier;

    /// @brief Holds the weight of luma when calculating SAD.
    ///
    /// Must be in range [0, 1.96875].
    ///
    float sadWeightLuma;

    /// @brief Holds a flag which enables or disables the spatial alpha smooth.
    bool alphaSmoothEnable;

    /// @brief Holds the temporal alpha restrict increase capablility.
    ///
    /// Must be in range [0, 1.9990234375].
    ///
    float alphaIncreaseCap;

    /// @brief Holds the alpha scale IIR for strength.
    ///
    /// Must be in range [0, 1.9990234375].
    ///
    float alphaScaleIIR;

    /// @brief Holds the max luma value in Alpha Clip Calculation.
    ///
    /// Must be in range [0, 1.9990234375].
    ///
    float alphaMaxLuma;

    /// @brief Holds the min luma value in Alpha Clip Calculation.
    ///
    /// Must be in range [0, 1.9990234375].
    ///
    float alphaMinLuma;

    /// @brief Holds the max chroma value in Alpha Clip Calculation.
    ///
    /// Must be in range [0, 1.9990234375].
    ///
    float alphaMaxChroma;

    /// @brief Holds the min chroma value in Alpha Clip Calculation.
    ///
    /// Must be in range [0, 1.9990234375].
    ///
    float alphaMinChroma;

    /// @brief Holds parameter BetaX1 in Beta Calculation.
    ///
    /// Must be in range [0, 1.9990234375].
    ///
    float betaX1;

    /// @brief Holds parameter BetaX2 in Beta Calculation.
    ///
    /// Must be in range [0, 1.9990234375].
    ///
    float betaX2;

    /// @brief Holds parameter MaxBeta threshold in Beta Calculation.
    ///
    /// Must be in range [0, 1.9990234375].
    ///
    float maxBeta;

    /// @brief Holds parameter BetaX2 in Beta Calculation.
    ///
    /// Must be in range [0, 1.9990234375].
    ///
    float minBeta;
} NvMedia2DTnrParameters;

/// @brief Number of bytes in NvMedia 2D checksums.
#define NVMEDIA_2D_CHECKSUM_NUM_BYTES 24u

/// @brief Represents a checksum.
typedef struct
{
    /// @brief The checksum data.
    uint8_t data[NVMEDIA_2D_CHECKSUM_NUM_BYTES];
} NvMedia2DChecksum;

/// @brief NvMedia 2D checksum mode.
///
/// This enum describes the different checksum calculation modes that are
/// supported by NvMedia 2D.
///
typedef enum
{
    /// @brief No checksum calculated. This is the default behavior.
    NVMEDIA_2D_CHECKSUM_MODE_DISABLED,

    /// @brief Checksum calculated from source surface pixel data.
    NVMEDIA_2D_CHECKSUM_MODE_SRC_SURFACE,

    /// @brief Checksum calculated from destination surface pixel data.
    NVMEDIA_2D_CHECKSUM_MODE_DST_SURFACE,
} NvMedia2DChecksumMode;

/// @brief Context priority levels for NvMedia2D.
///
/// Priority values are sorted in descending order. A comparison operation on the enum values
/// can be used to determine the priority order.
///
/// @note Certain priority levels may require special permissions on some platforms.
///
typedef enum
{
    /// @brief The highest priority level. Useful for safety critical applications.
    ///
    /// This level is typically only available to safety critical contexts.
    ///
    NVMEDIA_2D_CONTEXT_PRIORITY_HIGHEST,

    /// @brief High priority level.
    ///
    /// This level is typically only available to safety critical contexts.
    ///
    NVMEDIA_2D_CONTEXT_PRIORITY_HIGH,

    /// @brief Above medium priority level.
    ///
    NVMEDIA_2D_CONTEXT_PRIORITY_ABOVE_MEDIUM,

    /// @brief Medium priority level.
    ///
    /// This is the default priority level for safety critical contexts.
    ///
    NVMEDIA_2D_CONTEXT_PRIORITY_MEDIUM,

    /// @brief Below medium priority level.
    ///
    NVMEDIA_2D_CONTEXT_PRIORITY_BELOW_MEDIUM,

    /// @brief Low priority level.
    ///
    /// This is the default priority level for non-safety critical contexts.
    ///
    NVMEDIA_2D_CONTEXT_PRIORITY_LOW,

    /// @brief Below low priority level.
    ///
    NVMEDIA_2D_CONTEXT_PRIORITY_BELOW_LOW,

    /// @brief The lowest priority level. Useful for non-interactive tasks.
    ///
    NVMEDIA_2D_CONTEXT_PRIORITY_LOWEST
} NvMedia2DContextPriority;

/// @brief NvMedia2D Context.
///
/// This type represents a context for the NvMedia2D library.
/// This context is an opaque data type that encapsulates the state needed to
/// service the NvMedia2D API calls.
///
/// @sa NvMedia2DCreate()
///
typedef struct NvMedia2D NvMedia2D;

/// @brief Return the default values for NvMedia2DAttributes.
///
/// This function returns a NvMedia2DAttributes structure with
/// default values.
///
/// Default attributes:
/// - numComposeParameters: 1
/// - maxRegisteredBuffers: 64
/// - maxRegisteredSyncs: 16
/// - maxFilterBuffers: 0
/// - maxLdcWarpMaps: 16
/// - maxLdcMaskMaps: 16
/// - maxTnrStates: 16
/// - flags: undefined
///
/// @return A NvMedia2DAttributes structure with default values.
///
/// @pre None.
///
/// @usage
/// - Allowed context for the API call
///   - Interrupt handler: No
///   - Signal handler: No
///   - Thread-safe: Yes
///   - Re-entrant: No
///   - Async/Sync: Sync
/// - Required privileges: None
/// - API group
///   - Init: Yes
///   - Runtime: No
///   - De-Init: No
/// \if (NV_ASIL)
/// - Implementation ASIL:
///   - ASIL-B
/// \endif
/// - Maturity: GA
///
/// @sa NvMedia2DAttributes
///
NvMedia2DAttributes
NvMedia2DGetDefaultAttributes(void);

/// @brief Return the default values for NvMedia2DLdcWarpMapAttributes.
///
/// This function returns a NvMedia2DLdcWarpMapAttributes structure with
/// default values.
///
/// Default attributes:
/// - maxWarpMapWidth: 0
/// - maxWarpMapHeight: 0
///
/// @return A NvMedia2DLdcWarpMapAttributes structure with default values.
///
/// @pre None.
///
/// @usage
/// - Allowed context for the API call
///   - Interrupt handler: No
///   - Signal handler: No
///   - Thread-safe: Yes
///   - Re-entrant: No
///   - Async/Sync: Sync
/// - Required privileges: None
/// - API group
///   - Init: Yes
///   - Runtime: No
///   - De-Init: No
/// \if (NV_ASIL)
/// - Implementation ASIL:
///   - ASIL-B
/// \endif
/// - Maturity: GA
///
/// @sa NvMedia2DLdcWarpMapAttributes
///
NvMedia2DLdcWarpMapAttributes
NvMedia2DGetDefaultLdcWarpMapAttributes(void);

/// @brief Return the default values for NvMedia2DLdcMaskMapAttributes.
///
/// This function returns a NvMedia2DLdcMaskMapAttributes structure with
/// default values.
///
/// Default attributes:
/// - maxDstWidth: 0
/// - maxDstHeight: 0
///
/// @return A NvMedia2DLdcMaskMapAttributes structure with default values.
///
/// @pre None.
///
/// @usage
/// - Allowed context for the API call
///   - Interrupt handler: No
///   - Signal handler: No
///   - Thread-safe: Yes
///   - Re-entrant: No
///   - Async/Sync: Sync
/// - Required privileges: None
/// - API group
///   - Init: Yes
///   - Runtime: No
///   - De-Init: No
/// \if (NV_ASIL)
/// - Implementation ASIL:
///   - ASIL-B
/// \endif
/// - Maturity: GA
///
/// @sa NvMedia2DLdcMaskMapAttributes
///
NvMedia2DLdcMaskMapAttributes
NvMedia2DGetDefaultLdcMaskMapAttributes(void);

/// @brief Return the default values for NvMedia2DTnrStateAttributes.
///
/// This function returns a NvMedia2DTnrStateAttributes structure with
/// default values.
///
/// Default attributes:
/// - maxDstWidth: 0
/// - maxDstHeight: 0
///
/// @return A NvMedia2DTnrStateAttributes structure with default values.
///
/// @pre None.
///
/// @usage
/// - Allowed context for the API call
///   - Interrupt handler: No
///   - Signal handler: No
///   - Thread-safe: Yes
///   - Re-entrant: No
///   - Async/Sync: Sync
/// - Required privileges: None
/// - API group
///   - Init: Yes
///   - Runtime: No
///   - De-Init: No
/// \if (NV_ASIL)
/// - Implementation ASIL:
///   - ASIL-B
/// \endif
/// - Maturity: GA
///
/// @sa NvMedia2DTnrStateAttributes
///
NvMedia2DTnrStateAttributes
NvMedia2DGetDefaultTnrStateAttributes(void);

/// @brief Creates a new NvMedia2D context.
///
/// This function creates a new instance of an NvMedia2D context, and returns a
/// pointer to that context. Ownership of this context is passed to the caller.
/// When no longer in use, the caller must destroy the context using the
/// #NvMedia2DDestroy() function.
///
/// @param[out] handle  Pointer to receive the handle to the new NvMedia2D
///                     context.
/// @param[in]  attr    Pointer to NvMedia2DAttributes struct, or NULL for
///                     default attributes.
///
/// @return An #NvMediaStatus return code.
/// @retval NVMEDIA_STATUS_OK             Context created successfully.
/// @retval NVMEDIA_STATUS_BAD_PARAMETER  @a handle is NULL, or @a attr has bad
///                                       attribute values.
/// @retval NVMEDIA_STATUS_INVALID_STATE  The function was called in incorrect
///                                       system state.
/// @retval NVMEDIA_STATUS_NOT_SUPPORTED  NvMedia 2D is not supported on this
///                                       hardware platform.
/// @retval NVMEDIA_STATUS_OUT_OF_MEMORY  Memory allocation failed for internal
///                                       data structures or device memory
///                                       buffers.
/// @retval NVMEDIA_STATUS_ERROR          An internal failure occurred when
///                                       trying to create the context.
///
/// @pre
/// - Any NvMedia2DAttributes struct passed in @a attr must have been
///   initialized with NvMedia2DGetDefaultAttributes().
///
/// @usage
/// - Allowed context for the API call
///   - Interrupt handler: No
///   - Signal handler: No
///   - Thread-safe: Yes, with the following conditions:
///     - Each thread uses different NvMedia2D handle
///   - Re-entrant: No
///   - Async/Sync: Sync
/// - Required privileges: None
/// - API group
///   - Init: Yes
///   - Runtime: No
///   - De-Init: No
/// \if (NV_ASIL)
/// - Implementation ASIL:
///   - ASIL-B
/// \endif
/// - Maturity: GA
///
/// @sa NvMedia2DAttributes
/// @sa NvMedia2D
/// @sa NvMedia2DGetDefaultAttributes()
/// @sa NvMedia2DDestroy()
///
NvMediaStatus
NvMedia2DCreate(NvMedia2D ** const handle,
                NvMedia2DAttributes const * const attr);

/// @brief Destroys the NvMedia2D context.
///
/// This function destroys the specified NvMedia2D context.
///
/// Before calling this function, the caller must ensure:
/// - There are no NvSciSync or NvSyncBuf objects still registered against the
///   NvMedia2D context.
/// - There are no NvMedia2DFilterBuffer objects still alive.
/// - All previous 2D operations submitted using NvMedia2DCompose() have
//    completed.
///
/// @param[in] handle  Pointer to the NvMedia2D context.
///
/// @return An #NvMediaStatus return code.
/// @retval NVMEDIA_STATUS_OK             Context destroyed successfully.
/// @retval NVMEDIA_STATUS_BAD_PARAMETER  @a handle is NULL.
/// @retval NVMEDIA_STATUS_INVALID_STATE  The function was called in incorrect
///                                       system state.
/// @retval NVMEDIA_STATUS_PENDING        There are still some NvSciSync or
///                                       NvSciBuf objects registered against
///                                       the NvMedia2D context, or there are
///                                       some NvMedia2DFilterBuffer objects
///                                       that have not been destroyed.
/// @retval NVMEDIA_STATUS_ERROR          An internal failure occurred when
///                                       trying to destroy the context. The
///                                       context is in state where the only
///                                       valid operation is to attempt to
///                                       destroy it again.
///
/// @pre
/// - @a handle must be valid NvMedia2D handle created with NvMedia2DCreate().
///
/// @usage
/// - Allowed context for the API call
///   - Interrupt handler: No
///   - Signal handler: No
///   - Thread-safe: Yes, with the following conditions:
///     - Each thread uses different NvMedia2D handle
///   - Re-entrant: No
///   - Async/Sync: Sync
/// - Required privileges: None
/// - API group
///   - Init: Yes
///   - Runtime: No
///   - De-Init: Yes
/// \if (NV_ASIL)
/// - Implementation ASIL:
///   - ASIL-B
/// \endif
/// - Maturity: GA
///
/// @sa NvMedia2D
/// @sa NvMedia2DCreate()
///
NvMediaStatus
NvMedia2DDestroy(NvMedia2D const * const handle);

/// @brief Set global priority of this context.
///
/// This function sets a global system-wide priority to the NvMedia2D context. The implementation
/// prioritizes hardware resource allocation for higher-priority contexts, helping them maintain
/// stable latency and throughput even under heavy system load from lower-priority contexts.
///
/// This function can be called at any time. All future tasks submitted to hardware after a call
/// to this function will be processed based on the priority level.
///
/// For contexts with the same priority, the implementation may decide which context is
/// processed first.
///
/// @note Access to certain high priority levels may be restricted on some platforms and
///       may require specific permissions.
///
/// @param[in] handle    The NvMedia2D context handle.
/// @param[in] priority  The #NvMedia2DContextPriority to set.
///
/// @return An #NvMediaStatus status code.
/// @retval NVMEDIA_STATUS_OK               Priority was set successfully.
/// @retval NVMEDIA_STATUS_BAD_PARAMETER    @a handle is NULL or @a priority is
///                                         invalid.
/// @retval NVMEDIA_STATUS_NOT_SUPPORTED    Requested operation is not supported
///                                         by current platform.
/// @retval NVMEDIA_STATUS_ERROR            An internal error occurred when
///                                         trying to set the priority.
///
/// @pre
/// - @a handle must be valid NvMedia2D handle created with NvMedia2DCreate().
///
/// @usage
/// - Allowed context for the API call
///   - Interrupt handler: No
///   - Signal handler: No
///   - Thread-safe: Yes, with the following conditions:
///     - Each thread uses different NvMedia2D handle
///   - Re-entrant: No
///   - Async/Sync: Sync
/// - Required privileges: None
/// - API group
///   - Init: Yes
///   - Runtime: Yes
///   - De-Init: Yes
/// \if (NV_ASIL)
/// - Implementation ASIL:
///   - ASIL-B
/// \endif
/// - Maturity: Experimental
///
/// @sa NvMedia2DContextPriority
///
NvMediaStatus
NvMedia2DSetContextPriority(NvMedia2D const * const handle,
                            NvMedia2DContextPriority const priority);

/// @brief Returns the NvMedia2D context capabilities.
///
/// This function returns a pointer to a read-only #NvMedia2DCapabilities struct,
/// populated with the capabilities of the NvMedia2D context.
///
/// @param[in] handle  The NvMedia2D context handle.
///
/// @return Pointer to a read-only #NvMedia2DCapabilities struct. Ownership of this
///         pointer is retained by the NvMedia2D context. The return value will be
///         a null pointer if the specified context is invalid.
///
/// @pre
/// - @a handle must be valid NvMedia2D handle created with NvMedia2DCreate().
///
/// @usage
/// - Allowed context for the API call
///   - Interrupt handler: No
///   - Signal handler: No
///   - Thread-safe: Yes, with the following conditions:
///     - Each thread uses different NvMedia2D handle
///   - Re-entrant: No
///   - Async/Sync: Sync
/// - Required privileges: None
/// - API group
///   - Init: Yes
///   - Runtime: Yes
///   - De-Init: Yes
/// \if (NV_ASIL)
/// - Implementation ASIL:
///   - ASIL-B
/// \endif
/// - Maturity: GA
///
/// @sa NvMedia2DCapabilities
///
NvMedia2DCapabilities const *
NvMedia2DGetCapabilities(NvMedia2D const * const handle);

/// @brief Returns an #NvMedia2DComposeParameters instance.
///
/// This functions returns a handle to an NvMedia2DComposeParameters object.
/// The object will be initialized and ready to use. The caller takes ownership
/// of this handle. Ownership will be passed back to the NvMedia2D context when
/// it is subsequently used in the #NvMedia2DCompose() operation, or explicitly
/// released with #NvMedia2DReleaseComposeParameters().
///
/// The handle returned in @a params is tied to the specific NvMedia2D context
/// instance passed in @a handle and cannot be used with other context
/// instances.
///
/// @param[in]  handle  Pointer to the NvMedia2D context.
/// @param[out] params  Pointer to an #NvMedia2DComposeParameters, which will
///                     be populated with the handle.
///
/// @return An #NvMediaStatus return code.
/// @retval NVMEDIA_STATUS_OK                      Parameters instance is
///                                                initialized successfully.
/// @retval NVMEDIA_STATUS_BAD_PARAMETER           One of the parameters has an
///                                                invalid value, either:
///                                                - @a handle is NULL
///                                                - @a params is NULL.
/// @retval NVMEDIA_STATUS_INSUFFICIENT_BUFFERING  There is no free instance
///                                                available.
/// @retval NVMEDIA_STATUS_ERROR                   An internal failure occurred
///                                                when trying to retrieve the
///                                                parameters object.
///
/// @pre
/// - @a handle must be valid NvMedia2D handle created with NvMedia2DCreate().
///
/// @usage
/// - Allowed context for the API call
///   - Interrupt handler: No
///   - Signal handler: No
///   - Thread-safe: Yes, with the following conditions:
///     - Each thread uses different NvMedia2D handle
///   - Re-entrant: No
///   - Async/Sync: Sync
/// - Required privileges: None
/// - API group
///   - Init: No
///   - Runtime: Yes
///   - De-Init: No
/// \if (NV_ASIL)
/// - Implementation ASIL:
///   - ASIL-B
/// \endif
/// - Maturity: GA
///
/// @sa NvMedia2DComposeParameters
/// @sa NvMedia2DReleaseComposeParameters()
/// @sa NvMedia2DCompose()
///
NvMediaStatus
NvMedia2DGetComposeParameters(NvMedia2D const * const handle,
                              NvMedia2DComposeParameters * const params);

/// @brief Releases an #NvMedia2DComposeParameters instance.
///
/// This function releases an #NvMedia2DComposeParameters instance acquired with
/// #NvMedia2DGetComposeParameters() without executing the compose operation
/// described by the parameters. This can be useful in error handling scenarios.
///
/// If the operation is submitted with #NvMedia2DCompose(), there is no need to
/// call this function.
///
/// @param[in] handle  Pointer to the #NvMedia2D context.
/// @param[in] params  The #NvMedia2DComposeParameters instance to release.
///
/// @return An #NvMediaStatus return code.
/// @retval NVMEDIA_STATUS_OK             Parameters instance released
///                                       successfully.
/// @retval NVMEDIA_STATUS_BAD_PARAMETER  One of the parameters has an invalid
///                                       value, either:
///                                       - @a handle is NULL
///                                       - @a params is invalid.
///
/// @pre
/// - @a handle must be valid NvMedia2D handle created with NvMedia2DCreate().
/// - @a params must be valid NvMedia2DComposeParameters handle created with
///   NvMedia2DGetComposeParameters().
///
/// @usage
/// - Allowed context for the API call
///   - Interrupt handler: No
///   - Signal handler: No
///   - Thread-safe: Yes, with the following conditions:
///     - Each thread uses different NvMedia2D handle
///   - Re-entrant: No
///   - Async/Sync: Sync
/// - Required privileges: None
/// - API group
///   - Init: No
///   - Runtime: Yes
///   - De-Init: No
/// \if (NV_ASIL)
/// - Implementation ASIL:
///   - ASIL-B
/// \endif
/// - Maturity: GA
///
/// @sa NvMedia2DComposeParameters
/// @sa NvMedia2DGetComposeParameters()
///
NvMediaStatus
NvMedia2DReleaseComposeParameters(NvMedia2D const *const handle,
                                  NvMedia2DComposeParameters const params);

/// @brief Performs a 2D compose operation.
///
/// A compose operation transfers pixels from a set of source surfaces to a
/// destination surface, applying a variety of transformations to the pixel
/// values on the way. The surfaces can have different pixel formats. NvMedia 2D
/// does the necessary conversions between the formats.
///
/// If the dimensions of the source rectangle do not match the dimensions of the
/// destination rectangle, the operation scales the pixels to fit the
/// destination rectangle. If the source or the destination rectangle is not
/// explicitly set, it will default to match the dimensions of the source or the
/// destination surface, respectively.
///
/// If the clip rectangle is not explicitly set, it will default to match the
/// dimensions of the destination rectangle if single source layer is set, or
/// the dimensions of the destination surface if multiple source layers are set.
///
/// If no filter mode is explicitly set, then an appropriate mode will be
/// automatically selected, depending on the current scaling factor and hardware
/// restrictions.
///
/// This example performs a straight pixel copy between surfaces of the same
/// dimensions (but not necessarily the same bit depth or color format):
///
/// @code
///     NvMedia2DComposeParameters params;
///     NvMedia2DGetComposeParameters(handle, &params);
///     NvMedia2DSetSrcNvSciBufObj(handle, params, 0, srcBuf);
///     NvMedia2DSetDstNvSciBufObj(handle, params, dstBuf);
///     NvMedia2DCompose(handle, params, NULL);
/// @endcode
///
/// @anchor NvMedia2DComposeParameterRestrictions
///
/// Restrictions on dimensions for input and output surfaces:
/// - Width must be within the range: [16, 16384]
/// - Height must be within the range: [16, 16384]
///
/// Additional surface dimension restrictions for chroma sub-sampled YUV
/// formats:
/// - 444:
///   - Width: no restriction
///   - Height: no restriction
/// - 422:
///   - Width: must be a multiple of 2
///   - Height: no restriction
/// - 422R:
///   - Width: no restriction
///   - Height: must be a multiple of 2
/// - 420:
///   - Width: must be a multiple of 2
///   - Height: must be a multiple of 2
///
/// Restrictions on the source rectangle:
/// - Must be within the bounds of the source surface.
/// - Width and height must be greater than zero.
///
/// Restrictions on the destination and clip rectangle:
/// - Must be within the bounds of the destination surface.
/// - Width and height must be greater than zero.
///
/// Restrictions on the source layers:
/// - The number of source layers configured must be at most
///   NvMedia2DCapabilities::maxComposeLayers.
///
/// If NvMedia2DCapabilities::supports16BitYUVNatively is false, following
/// additional restrictions apply for 16-bit YUV/YUVX/Y formats:
/// - Only scaling and cropping are supported. Pixel format conversion,
///   transformations or multi-surface composition is not supported.
/// - Surface width must be within the range: [16, 8192]
/// - Surface height must be within the range: [16, 8192]
/// - With 422 chroma sub-sampling, surface width must be at least 32
/// - With 422R chroma sub-sampling, surface height must be at least 32
/// - With 420 chroma sub-sampling, both surface width and surface height must
///   be at least 32
/// - Destination rectangle top-left corner must be zero.
///
/// If NvMedia2DCapabilities::supportsNonAlignedRects is false, following
/// additional restrictions apply for chroma sub-sampled YUV formats:
/// - All destination and clip rectangle corners must be aligned to a a multiple
///   of 2.
///
/// @anchor NvMedia2DComposeConcurrencyRestrictions
///
/// Restrictions on concurrency:
/// - There can be a maximum of 16 operations submitted through the same
///   NvMedia2D handle pending simultaneously.
///
/// If any of the restrictions are violated, this function will fail with an
/// error code.
///
/// Performance considerations:
/// - Using block linear memory layout for surfaces generally provides better
///   performance than pitch linear layout.
/// - Using (semi-)planar YUV formats generally provides better performance than
///   interleaved (Y8U8Y8V8 etc.) formats.
/// - The hardware accelerator utilized by NvMedia 2D accesses the destination
///   surface in "tiles" of size NvMedia2DCapabilities::tileWidth x
///   NvMedia2DCapabilities::tileHeight pixels. Using destination surface size
///   and destination rectangle dimensions that are exact multiples of the tile
///   size most optimally aligns with this hardware access pattern.
/// - If NvMedia2DCapabilities::supports16BitYUVNatively is false, operations
///   with 16-bit (semi-)planar YUV format surfaces will be processed with
///   multiple harware accelerator passes.
///
/// The result info returned in @a result is tied to the specific NvMedia2D
/// context instance passed in @a handle and cannot be used with other context
/// instances.
///
/// @param[in]  handle  Pointer to the NvMedia2D context.
/// @param[in]  params  An NvMedia2DComposeParameters handle.
/// @param[out] result  Pointer to NvMedia2DComposeResult struct that will be
///                     populated with result info. May be NULL.
///
/// @return An #NvMediaStatus return code.
/// @retval NVMEDIA_STATUS_OK             Operation submitted successfully.
/// @retval NVMEDIA_STATUS_BAD_PARAMETER  One of the parameters has an invalid
///                                       value. This could be:
///                                       - @a handle is NULL
///                                       - @a params is invalid
///                                       - some of the compose parameters
///                                         configured through @a params have
///                                         invalid values (see @ref
///                                         NvMedia2DComposeParameterRestrictions
///                                         "restrictions on parameters").
/// @retval NVMEDIA_STATUS_NOT_SUPPORTED  Requested operation is not supported
///                                       by current platform (see @ref
///                                       NvMedia2DComposeParameterRestrictions
///                                       "restrictions on parameters").
/// @retval NVMEDIA_STATUS_TIMED_OUT      No space available in the command
///                                       buffer for this operation, because
///                                       previous operations are still
///                                       pending (see @ref
///                                       NvMedia2DComposeConcurrencyRestrictions
///                                       "restrictions on concurrency"). The
///                                       caller should wait for the least
///                                       recently submitted operation to
///                                       complete and then try again.
/// @retval NVMEDIA_STATUS_ERROR          An internal failure occurred when
///                                       trying to perform the compose
///                                       operation. This error indicates the
///                                       system is potentially in an
///                                       unrecoverable state.
///
/// @pre
/// - @a handle must be valid NvMedia2D handle created with NvMedia2DCreate().
/// - @a params must be valid NvMedia2DComposeParameters handle created with
///   NvMedia2DGetComposeParameters().
///
/// @usage
/// - Allowed context for the API call
///   - Interrupt handler: No
///   - Signal handler: No
///   - Thread-safe: Yes, with the following conditions:
///     - Each thread uses different NvMedia2D handle
///   - Re-entrant: No
///   - Async/Sync: Async
/// - Required privileges: None
/// - API group
///   - Init: No
///   - Runtime: Yes
///   - De-Init: No
/// \if (NV_ASIL)
/// - Implementation ASIL:
///   - ASIL-B
/// \endif
/// - Maturity: GA
///
/// @sa NvMedia2DCapabilities
/// @sa NvMedia2DComposeParameters
/// @sa NvMedia2DGetComposeParameters()
/// @sa NvMedia2DInsertPreNvSciSyncFence()
/// @sa NvMedia2DSetNvSciSyncObjforEOF()
/// @sa NvMedia2DSetSrcGeometry()
/// @sa NvMedia2DSetSrcFilter()
/// @sa NvMedia2DSetSrcBlendMode()
/// @sa NvMedia2DSetSrcLdcTransformMatrix()
/// @sa NvMedia2DSetSrcLdcWarpMap()
/// @sa NvMedia2DSetSrcLdcMaskMap()
/// @sa NvMedia2DSetSrcNvSciBufObj()
/// @sa NvMedia2DSetDstNvSciBufObj()
/// @sa NvMedia2DGetEOFNvSciSyncFence()
///
NvMediaStatus
NvMedia2DCompose(NvMedia2D const * const handle,
                 NvMedia2DComposeParameters const params,
                 NvMedia2DComposeResult * const result);

/// @brief Sets the geometry for a source layer.
///
/// @param[in] handle     Pointer to the NvMedia2D context.
/// @param[in] params     An NvMedia2DComposeParameters handle.
/// @param[in] index      Index of source layer to configure. Must be in range
///                       [0, 15].
/// @param[in] dstRect    Pointer to an NvMediaRect that contains the
///                       destination rectangle, or NULL for default rectangle.
/// @param[in] srcRect    Pointer to an NvMediaRect that contains the source
///                       rectangle, or NULL for default rectangle.
/// @param[in] transform  An #NvMedia2DTransform to apply the content region.
///
/// @return An #NvMediaStatus return code.
/// @retval NVMEDIA_STATUS_OK               Parameters set successfully.
/// @retval NVMEDIA_STATUS_BAD_PARAMETER    One of the parameters has an invalid
///                                         value. This could be:
///                                         - @a handle is NULL
///                                         - @a params is invalid
///                                         - @a index is out of range
///                                         - @a srcRect is invalid
///                                         - @a dstRect is invalid
///                                         - @a transform is invalid.
/// @retval NVMEDIA_STATUS_NOT_SUPPORTED    Requested operation is not supported
///                                         by current platform.
///
/// @pre
/// - @a handle must be valid NvMedia2D handle created with NvMedia2DCreate().
/// - @a params must be valid NvMedia2DComposeParameters handle created with
///   NvMedia2DGetComposeParameters().
///
/// @usage
/// - Allowed context for the API call
///   - Interrupt handler: No
///   - Signal handler: No
///   - Thread-safe: Yes, with the following conditions:
///     - Each thread uses different NvMedia2D handle
///   - Re-entrant: No
///   - Async/Sync: Sync
/// - Required privileges: None
/// - API group
///   - Init: No
///   - Runtime: Yes
///   - De-Init: No
/// \if (NV_ASIL)
/// - Implementation ASIL:
///   - ASIL-B
/// \endif
/// - Maturity: GA
///
/// @sa NvMedia2DComposeParameters
/// @sa NvMedia2DCompose()
///
NvMediaStatus
NvMedia2DSetSrcGeometry(NvMedia2D const * const handle,
                        NvMedia2DComposeParameters const params,
                        uint32_t const index,
                        NvMediaRect const * const srcRect,
                        NvMediaRect const * const dstRect,
                        NvMedia2DTransform const transform);

/// @brief Sets the filter mode for a source layer.
///
/// @param[in] handle  Pointer to the NvMedia2D context.
/// @param[in] params  An NvMedia2DComposeParameters handle.
/// @param[in] index   Index of source layer to configure. Must be in range
///                    [0, 15].
/// @param[in] filter  An #NvMedia2DFilter to use when reading the the layer's
///                    source surface.
///
/// @return An #NvMediaStatus return code.
/// @retval NVMEDIA_STATUS_OK               Parameters set successfully.
/// @retval NVMEDIA_STATUS_BAD_PARAMETER    One of the parameters has an invalid
///                                         value. This could be:
///                                         - @a handle is NULL
///                                         - @a params is invalid
///                                         - @a index is out of range
///                                         - @a filter is invalid.
///
/// @pre
/// - @a handle must be valid NvMedia2D handle created with NvMedia2DCreate().
/// - @a params must be valid NvMedia2DComposeParameters handle created with
///   NvMedia2DGetComposeParameters().
///
/// @usage
/// - Allowed context for the API call
///   - Interrupt handler: No
///   - Signal handler: No
///   - Thread-safe: Yes, with the following conditions:
///     - Each thread uses different NvMedia2D handle
///   - Re-entrant: No
///   - Async/Sync: Sync
/// - Required privileges: None
/// - API group
///   - Init: No
///   - Runtime: Yes
///   - De-Init: No
/// \if (NV_ASIL)
/// - Implementation ASIL:
///   - ASIL-B
/// \endif
/// - Maturity: GA
///
/// @sa NvMedia2DComposeParameters
/// @sa NvMedia2DCompose()
///
NvMediaStatus
NvMedia2DSetSrcFilter(NvMedia2D const * const handle,
                      NvMedia2DComposeParameters const params,
                      uint32_t const index,
                      NvMedia2DFilter const filter);

/// @brief Sets the blend mode for a source layer.
///
/// @param[in] handle         Pointer to the NvMedia2D context.
/// @param[in] params         An NvMedia2DComposeParameters handle.
/// @param[in] index          Index of source layer to configure. Must be in
///                           range [0, 15].
/// @param[in] blendMode      Blend mode to set.
/// @param[in] constantAlpha  Constant alpha factor to use in blending.
///                           Must be in range [0, 1].
///
/// @return An #NvMediaStatus return code.
/// @retval NVMEDIA_STATUS_OK               Parameters set successfully.
/// @retval NVMEDIA_STATUS_BAD_PARAMETER    One of the parameters has an invalid
///                                         value. This could be:
///                                         - @a handle is NULL
///                                         - @a params is invalid
///                                         - @a index is out of range
///                                         - @a blendMode is invalid
///                                         - @a constantAlpha is out of range
///
/// @pre
/// - @a handle must be valid NvMedia2D handle created with NvMedia2DCreate().
/// - @a params must be valid NvMedia2DComposeParameters handle created with
///   NvMedia2DGetComposeParameters().
///
/// @usage
/// - Allowed context for the API call
///   - Interrupt handler: No
///   - Signal handler: No
///   - Thread-safe: Yes, with the following conditions:
///     - Each thread uses different NvMedia2D handle
///   - Re-entrant: No
///   - Async/Sync: Sync
/// - Required privileges: None
/// - API group
///   - Init: No
///   - Runtime: Yes
///   - De-Init: No
/// \if (NV_ASIL)
/// - Implementation ASIL:
///   - ASIL-B
/// \endif
/// - Maturity: GA
///
/// @sa NvMedia2DComposeParameters
/// @sa NvMedia2DBlendMode
/// @sa NvMedia2DCompose()
///
NvMediaStatus
NvMedia2DSetSrcBlendMode(NvMedia2D const * const handle,
                         NvMedia2DComposeParameters const params,
                         uint32_t const index,
                         NvMedia2DBlendMode const blendMode,
                         float const constantAlpha);

/// @brief Sets the LDC transform matrix for a source layer.
///
/// Only one of LDC transform matrix or LDC warp map can be set for a given
/// source layer of a compose operation.
///
/// @param[in] handle        Pointer to the NvMedia2D context.
/// @param[in] params        An NvMedia2DComposeParameters handle.
/// @param[in] index         Index of source layer to configure. Must be in
///                          range [0, 15].
/// @param[in] ldcIptParams  Pointer to an #NvMedia2DLdcTransformMatrix struct.
///
/// @return An #NvMediaStatus return code.
/// @retval NVMEDIA_STATUS_OK               Parameters set successfully.
/// @retval NVMEDIA_STATUS_BAD_PARAMETER    One of the parameters has an invalid
///                                         value. This could be:
///                                         - @a handle is NULL
///                                         - @a params is invalid
///                                         - @a index is out of range
///                                         - @a ldcIptParams is NULL.
///
/// @pre
/// - @a handle must be valid NvMedia2D handle created with NvMedia2DCreate().
/// - @a params must be valid NvMedia2DComposeParameters handle created with
///   NvMedia2DGetComposeParameters().
///
/// @usage
/// - Allowed context for the API call
///   - Interrupt handler: No
///   - Signal handler: No
///   - Thread-safe: Yes, with the following conditions:
///     - Each thread uses different NvMedia2D handle
///   - Re-entrant: No
///   - Async/Sync: Sync
/// - Required privileges: None
/// - API group
///   - Init: No
///   - Runtime: Yes
///   - De-Init: No
/// \if (NV_ASIL)
/// - Implementation ASIL:
///   - ASIL-B
/// \endif
/// - Maturity: GA
///
/// @sa NvMedia2DComposeParameters
/// @sa NvMedia2DLdcWarpMap
/// @sa NvMedia2DCompose()
///
NvMediaStatus
NvMedia2DSetSrcLdcTransformMatrix(NvMedia2D const * const handle,
                                  NvMedia2DComposeParameters const params,
                                  uint32_t const index,
                                  NvMedia2DLdcTransformMatrix const * const ldcIptParams);

/// @brief Sets the LDC warp map for a source layer.
///
/// After a successful call to this function, the compose parameters object
/// referred to by @a params will refer to the LDC warp map object passed in
/// @a ldcWarpMap. Any updates to the LDC warp map object will be reflected in
/// the compose operation described by @a params. The caller is responsible for
/// ensuring the lifetime of the LDC warp map object extends beyond the duration
/// of the compose operation.
///
/// Only one of LDC transform matrix or LDC warp map can be set for a given
/// source layer of a compose operation.
///
/// @param[in] handle      Pointer to the NvMedia2D context.
/// @param[in] params      An NvMedia2DComposeParameters handle.
/// @param[in] index       Index of source layer to configure. Must be in range
///                        [0, 15].
/// @param[in] ldcWarpMap  An #NvMedia2DLdcWarpMap handle.
///
/// @return An #NvMediaStatus return code.
/// @retval NVMEDIA_STATUS_OK               Parameters set successfully.
/// @retval NVMEDIA_STATUS_BAD_PARAMETER    One of the parameters has an invalid
///                                         value. This could be:
///                                         - @a handle is NULL
///                                         - @a params is invalid
///                                         - @a index is out of range
///                                         - @a ldcWarpMap is invalid.
///
/// @pre
/// - @a handle must be valid NvMedia2D handle created with NvMedia2DCreate().
/// - @a params must be valid NvMedia2DComposeParameters handle created with
///   NvMedia2DGetComposeParameters().
/// - @a ldcWarpMap must be valid NvMedia2DLdcWarpMap handle created with
///   NvMedia2DCreateLdcWarpMap().
///
/// @usage
/// - Allowed context for the API call
///   - Interrupt handler: No
///   - Signal handler: No
///   - Thread-safe: Yes, with the following conditions:
///     - Each thread uses different NvMedia2D handle
///   - Re-entrant: No
///   - Async/Sync: Sync
/// - Required privileges: None
/// - API group
///   - Init: No
///   - Runtime: Yes
///   - De-Init: No
/// \if (NV_ASIL)
/// - Implementation ASIL:
///   - ASIL-B
/// \endif
/// - Maturity: GA
///
/// @sa NvMedia2DComposeParameters
/// @sa NvMedia2DLdcWarpMap
/// @sa NvMedia2DCompose()
///
NvMediaStatus
NvMedia2DSetSrcLdcWarpMap(NvMedia2D const * const handle,
                          NvMedia2DComposeParameters const params,
                          uint32_t const index,
                          NvMedia2DLdcWarpMap const ldcWarpMap);

/// @brief Sets the LDC mask map for a source layer.
///
/// After a successful call to this function, the compose parameters object
/// referred to by @a params will refer to the LDC mask map object passed in
/// @a ldcMaskMap. Any updates to the LDC mask map object will be reflected in
/// the compose operation described by @a params. The caller is responsible for
/// ensuring the lifetime of the LDC mask map object extends beyond the duration
/// of the compose operation.
///
/// @param[in] handle      Pointer to the NvMedia2D context.
/// @param[in] params      An NvMedia2DComposeParameters handle.
/// @param[in] index       Index of source layer to configure. Must be in range
///                        [0, 15].
/// @param[in] ldcMaskMap  An #NvMedia2DLdcMaskMap handle.
///
/// @return An #NvMediaStatus return code.
/// @retval NVMEDIA_STATUS_OK               Parameters set successfully.
/// @retval NVMEDIA_STATUS_BAD_PARAMETER    One of the parameters has an invalid
///                                         value. This could be:
///                                         - @a handle is NULL
///                                         - @a params is invalid
///                                         - @a index is out of range
///                                         - @a ldcMaskMap is invalid.
///
/// @pre
/// - @a handle must be valid NvMedia2D handle created with NvMedia2DCreate().
/// - @a params must be valid NvMedia2DComposeParameters handle created with
///   NvMedia2DGetComposeParameters().
/// - @a ldcMaskMap must be valid NvMedia2DLdcMaskMap handle created with
///   NvMedia2DCreateLdcMaskMap().
///
/// @usage
/// - Allowed context for the API call
///   - Interrupt handler: No
///   - Signal handler: No
///   - Thread-safe: Yes, with the following conditions:
///     - Each thread uses different NvMedia2D handle
///   - Re-entrant: No
///   - Async/Sync: Sync
/// - Required privileges: None
/// - API group
///   - Init: No
///   - Runtime: Yes
///   - De-Init: No
/// \if (NV_ASIL)
/// - Implementation ASIL:
///   - ASIL-B
/// \endif
/// - Maturity: GA
///
/// @sa NvMedia2DComposeParameters
/// @sa NvMedia2DLdcMaskMap
/// @sa NvMedia2DCompose()
///
NvMediaStatus
NvMedia2DSetSrcLdcMaskMap(NvMedia2D const * const handle,
                          NvMedia2DComposeParameters const params,
                          uint32_t const index,
                          NvMedia2DLdcMaskMap const ldcMaskMap);

/// @brief Sets the clip rectangle for the compose operation.
///
/// Write to the destination surface is restricted to the region specified by
/// the clip rectangle.
///
/// @param[in] handle    Pointer to the NvMedia2D context.
/// @param[in] params    An NvMedia2DComposeParameters handle.
/// @param[in] clipRect  Pointer to an NvMediaRect that contains the clip
///                      rectangle, or NULL for default rectangle.
///
/// @return An #NvMediaStatus return code.
/// @retval NVMEDIA_STATUS_OK             Parameters set successfully.
/// @retval NVMEDIA_STATUS_BAD_PARAMETER  One of the parameters has an invalid
///                                       value. This could be:
///                                       - @a handle is NULL
///                                       - @a params is invalid.
/// @retval NVMEDIA_STATUS_NOT_SUPPORTED  Requested operation is not supported
///                                       on current platform.
///
/// @pre
/// - @a handle must be valid NvMedia2D handle created with NvMedia2DCreate().
/// - @a params must be valid NvMedia2DComposeParameters handle created with
///   NvMedia2DGetComposeParameters().
///
/// @usage
/// - Allowed context for the API call
///   - Interrupt handler: No
///   - Signal handler: No
///   - Thread-safe: Yes, with the following conditions:
///     - Each thread uses different NvMedia2D handle
///   - Re-entrant: No
///   - Async/Sync: Sync
/// - Required privileges: None
/// - API group
///   - Init: No
///   - Runtime: Yes
///   - De-Init: No
/// \if (NV_ASIL)
/// - Implementation ASIL:
///   - ASIL-B
/// \endif
/// - Maturity: GA
///
/// @sa NvMedia2DComposeParameters
/// @sa NvMedia2DCompose()
///
NvMediaStatus
NvMedia2DSetClipRect(NvMedia2D const * const handle,
                     NvMedia2DComposeParameters const params,
                     NvMediaRect const * const clipRect);

/// @brief Sets the TNR state to use for the compose operation.
///
/// After a successful call to this function, the compose parameters object
/// referred to by @a params will refer to the TNR state object passed in
/// @a tnrState. Any updates to the TNR state object will be reflected in the
/// compose operation described by @a params. The caller is responsible for
/// ensuring the lifetime of the TNR state object extends beyond the duration
/// of the compose operation.
///
/// @param[in] handle    Pointer to the NvMedia2D context.
/// @param[in] params    An NvMedia2DComposeParameters handle.
/// @param[in] tnrState  An #NvMedia2DTnrState handle.
///
/// @return An #NvMediaStatus return code.
/// @retval NVMEDIA_STATUS_OK               Parameters set successfully.
/// @retval NVMEDIA_STATUS_BAD_PARAMETER    One of the parameters has an invalid
///                                         value. This could be:
///                                         - @a handle is NULL
///                                         - @a params is invalid
///                                         - @a tnrState is invalid.
///
/// @pre
/// - @a handle must be valid NvMedia2D handle created with NvMedia2DCreate().
/// - @a params must be valid NvMedia2DComposeParameters handle created with
///   NvMedia2DGetComposeParameters().
/// - @a tnrState must be valid NvMedia2DTnrState handle created with
///   NvMedia2DCreateTnrState().
///
/// @usage
/// - Allowed context for the API call
///   - Interrupt handler: No
///   - Signal handler: No
///   - Thread-safe: Yes, with the following conditions:
///     - Each thread uses different NvMedia2D handle
///   - Re-entrant: No
///   - Async/Sync: Sync
/// - Required privileges: None
/// - API group
///   - Init: No
///   - Runtime: Yes
///   - De-Init: No
/// \if (NV_ASIL)
/// - Implementation ASIL:
///   - ASIL-B
/// \endif
/// - Maturity: GA
///
/// @sa NvMedia2DComposeParameters
/// @sa NvMedia2DTnrState
/// @sa NvMedia2DCompose()
///
NvMediaStatus
NvMedia2DSetTnrState(NvMedia2D const * const handle,
                     NvMedia2DComposeParameters const params,
                     NvMedia2DTnrState const tnrState);

/// @brief Creates and returns an #NvMedia2DFilterBuffer instance.
///
/// This functions returns a handle to an NvMedia2DFilterBuffer object. The
/// filter buffer can be used to provide custom 5-tap and 10-tap filter
/// coefficients for a compose operation.
///
/// The handle returned in @a filterBuffer is tied to the specific NvMedia2D
/// context instance passed in @a handle and cannot be used with other context
/// instances.
///
/// The buffer instance must be destroyed with #NvMedia2DDestroyFilterBuffer()
/// during the De-Init stage, before calling NvMedia2DDestroy().
///
/// @param[in]  handle        Pointer to the NvMedia2D context.
/// @param[out] filterBuffer  Pointer to an #NvMedia2DFilterBuffer, which will
///                           be populated with the handle.
///
/// @return An #NvMediaStatus return code.
/// @retval NVMEDIA_STATUS_OK                      Buffer created successfully.
/// @retval NVMEDIA_STATUS_BAD_PARAMETER           One of the parameters has an
///                                                invalid value, either:
///                                                - @a handle is NULL
///                                                - @a filterBuffer is NULL.
/// @retval NVMEDIA_STATUS_INSUFFICIENT_BUFFERING  Maximum number of buffers has
///                                                been created.
/// @retval NVMEDIA_STATUS_INVALID_STATE           The function was called in
///                                                incorrect system state.
/// @retval NVMEDIA_STATUS_OUT_OF_MEMORY           Failed to allocate memory for
///                                                the buffer.
/// @retval NVMEDIA_STATUS_ERROR                   An internal failure occurred
///                                                when trying to create the
///                                                buffer.
///
/// @pre
/// - @a handle must be valid NvMedia2D handle created with NvMedia2DCreate().
///
/// @usage
/// - Allowed context for the API call
///   - Interrupt handler: No
///   - Signal handler: No
///   - Thread-safe: Yes, with the following conditions:
///     - Each thread uses different NvMedia2D handle
///   - Re-entrant: No
///   - Async/Sync: Sync
/// - Required privileges: None
/// - API group
///   - Init: Yes
///   - Runtime: No
///   - De-Init: No
/// \if (NV_ASIL)
/// - Implementation ASIL:
///   - ASIL-B
/// \endif
/// - Maturity: GA
///
/// @sa NvMedia2DAttributes
/// @sa NvMedia2DFilterBuffer
/// @sa NvMedia2DDestroyFilterBuffer()
///
NvMediaStatus
NvMedia2DCreateFilterBuffer(NvMedia2D const * const handle,
                            NvMedia2DFilterBuffer * const filterBuffer);

/// @brief Destroys an #NvMedia2DFilterBuffer instance.
///
/// This functions destroys an NvMedia2DFilterBuffer object.
///
/// @param[in] handle        Pointer to the NvMedia2D context.
/// @param[in] filterBuffer  An #NvMedia2DFilterBuffer handle.
///
/// @return An #NvMediaStatus return code.
/// @retval NVMEDIA_STATUS_OK             Buffer are destroyed successfully.
/// @retval NVMEDIA_STATUS_BAD_PARAMETER  One of the parameters has an invalid
///                                       value, either:
///                                       - @a handle is NULL.
///                                       - @a filterBuffer is invalid.
/// @retval NVMEDIA_STATUS_INVALID_STATE  The function was called in incorrect
///                                       system state.
/// @retval NVMEDIA_STATUS_PENDING        The buffer is still being used by a
///                                       pending operation.
/// @retval NVMEDIA_STATUS_ERROR          An internal failure occurred when
///                                       trying to destroy the buffer. The
///                                       buffer is in state where the only
///                                       valid operation is to attempt to
///                                       destroy it again.
///
/// @pre
/// - @a handle must be valid NvMedia2D handle created with NvMedia2DCreate().
/// - @a filterBuffer must be valid NvMedia2DFilterBuffer handle created with
///   NvMedia2DCreateFilterBuffer().
///
/// @usage
/// - Allowed context for the API call
///   - Interrupt handler: No
///   - Signal handler: No
///   - Thread-safe: Yes, with the following conditions:
///     - Each thread uses different NvMedia2D handle
///   - Re-entrant: No
///   - Async/Sync: Sync
/// - Required privileges: None
/// - API group
///   - Init: Yes
///   - Runtime: No
///   - De-Init: Yes
/// \if (NV_ASIL)
/// - Implementation ASIL:
///   - ASIL-B
/// \endif
/// - Maturity: GA
///
/// @sa NvMedia2DFilterBuffer
/// @sa NvMedia2DCreateFilterBuffer()
///
NvMediaStatus
NvMedia2DDestroyFilterBuffer(NvMedia2D const * const handle,
                             NvMedia2DFilterBuffer const filterBuffer);

/// @brief Sets the filter buffer for an #NvMedia2DComposeParameters instance.
///
/// This function updates the NvMedia2DComposeParameters instance to indicate
/// that the specified filter buffer object shall be used to provide the custom
/// filter coefficients for the compose operation. Any updates to the filter
/// buffer object will be reflected in the compose operation described by
/// the NvMedia2DComposeParameters instance. The caller is responsible for
/// ensuring the lifetime of the filter buffer object extends beyond the
/// duration of the compose operation.
///
/// After this function returns successfully, there are a few additional
/// limitations on the compose operation:
/// - Only the first 5 source layers can be used (indexes 0 to 4).
/// - If a source layer's NvMedia2DFilter is set to NVMEDIA_2D_FILTER_MEDIUM or
///   NVMEDIA_2D_FILTER_HIGH, the custom 5-tap or 10-tap coefficients,
///   respectively, for such layer shall have been properly set in the
///   NvMedia2DFilterBuffer.
///
/// Due to the filter buffer object being read-only from the compose operation
/// perspective, there is no limitation for the same filter buffer object to
/// be set for multiple NvMedia2DComposeParameters instances.
///
/// @param[in] handle        Pointer to the NvMedia2D context.
/// @param[in] params        An #NvMedia2DComposeParameters handle.
/// @param[in] filterBuffer  An #NvMedia2DFilterBuffer handle.
///
/// @return An #NvMediaStatus return code.
/// @retval NVMEDIA_STATUS_OK             Filter buffer was set successfully.
/// @retval NVMEDIA_STATUS_BAD_PARAMETER  One of the parameters has an invalid
///                                       value, either:
///                                       - @a handle is NULL.
///                                       - @a params is invalid.
///                                       - @a filterBuffer is invalid.
///
/// @pre
/// - @a handle must be valid NvMedia2D handle created with NvMedia2DCreate().
/// - @a params must be valid NvMedia2DComposeParameters handle created with
///   NvMedia2DGetComposeParameters().
/// - @a filterBuffer must be valid NvMedia2DFilterBuffer handle created with
///   NvMedia2DCreateFilterBuffer().
///
/// @usage
/// - Allowed context for the API call
///   - Interrupt handler: No
///   - Signal handler: No
///   - Thread-safe: Yes, with the following conditions:
///     - Each thread uses different NvMedia2D handle
///   - Re-entrant: No
///   - Async/Sync: Sync
/// - Required privileges: None
/// - API group
///   - Init: No
///   - Runtime: Yes
///   - De-Init: No
/// \if (NV_ASIL)
/// - Implementation ASIL:
///   - ASIL-B
/// \endif
/// - Maturity: GA
///
/// @sa NvMedia2DFilterBuffer
/// @sa NvMedia2DCreateFilterBuffer()
/// @sa NvMedia2DFilter
/// @sa NvMedia2DSetSrcFilter()
/// @sa NvMedia2DComputeFilterCoefficients5Tap()
/// @sa NvMedia2DComputeFilterCoefficients10Tap()
///
NvMediaStatus
NvMedia2DSetFilterBuffer(NvMedia2D const * const handle,
                         NvMedia2DComposeParameters const params,
                         NvMedia2DFilterBuffer const filterBuffer);

/// @brief Computes the 5-tap filter coefficients for an #NvMedia2DFilterBuffer.
///
/// This function computes the filter coefficients values for a specific source
/// layer based on the contents of four #NvMedia2DFilterCoefficients5Tap
/// structures.
///
/// There is no restriction on multiple parameters pointing to the same
/// #NvMedia2DFilterCoefficients5Tap structure
///
/// @param[in] handle           Pointer to the NvMedia2D context.
/// @param[in] filterBuffer     An #NvMedia2DFilterBuffer handle.
/// @param[in] index            Index of source layer to configure. Must be in
///                             range [0, 4].
/// @param[in] lumaX,lumaY      Pointers to #NvMedia2DFilterCoefficients5Tap.
///                             These configure to the luma component for YUV
///                             formats, or all the components for RGB formats.
///                             There is one pointer for the horizontal
///                             direction, and one pointer for the vertical
///                             direction.
/// @param[in] chromaX,chromaY  Pointers to #NvMedia2DFilterCoefficients5Tap.
///                             These configure the chroma component for YUV
///                             formats. There is one pointer for the
///                             horizontal direction, and one pointer for the
///                             vertical direction.
///
/// @return An #NvMediaStatus return code.
/// @retval NVMEDIA_STATUS_OK             Filter buffer was updated successfully.
/// @retval NVMEDIA_STATUS_BAD_PARAMETER  One of the parameters has an invalid
///                                       value, either:
///                                       - @a handle is NULL.
///                                       - @a filterBuffer is invalid.
///                                       - @a index is out of range.
///                                       - @a lumaX is NULL.
///                                       - @a lumaY is NULL.
///                                       - @a chromaX is NULL.
///                                       - @a chromaY is NULL.
///
/// @pre
/// - @a handle must be valid NvMedia2D handle created with NvMedia2DCreate().
/// - @a filterBuffer must be valid NvMedia2DFilterBuffer handle created with
///   NvMedia2DCreateFilterBuffer().
///
/// @usage
/// - Allowed context for the API call
///   - Interrupt handler: No
///   - Signal handler: No
///   - Thread-safe: Yes, with the following conditions:
///     - Each thread uses different NvMedia2D handle
///   - Re-entrant: No
///   - Async/Sync: Sync
/// - Required privileges: None
/// - API group
///   - Init: Yes
///   - Runtime: Yes
///   - De-Init: No
/// \if (NV_ASIL)
/// - Implementation ASIL:
///   - ASIL-B
/// \endif
/// - Maturity: GA
///
/// @sa NvMedia2DFilterBuffer
/// @sa NvMedia2DFilterCoefficients5Tap
/// @sa NvMedia2DCreateFilterBuffer()
///
NvMediaStatus
NvMedia2DComputeFilterCoefficients5Tap(NvMedia2D const * const handle,
                                       NvMedia2DFilterBuffer const filterBuffer,
                                       uint32_t const index,
                                       NvMedia2DFilterCoefficients5Tap const * const lumaX,
                                       NvMedia2DFilterCoefficients5Tap const * const lumaY,
                                       NvMedia2DFilterCoefficients5Tap const * const chromaX,
                                       NvMedia2DFilterCoefficients5Tap const * const chromaY);

/// @brief Computes the 10-tap filter coefficients for an #NvMedia2DFilterBuffer.
///
/// This function computes the filter coefficients values for a specific source
/// layer based on the contents of four #NvMedia2DFilterCoefficients10Tap
/// structures.
///
/// There is no restriction on multiple parameters pointing to the same
/// #NvMedia2DFilterCoefficients10Tap structure
///
/// @param[in] handle           Pointer to the NvMedia2D context.
/// @param[in] filterBuffer     An #NvMedia2DFilterBuffer handle.
/// @param[in] index            Index of source layer to configure. Must be in
///                             range [0, 4].
/// @param[in] lumaX,lumaY      Pointers to #NvMedia2DFilterCoefficients10Tap.
///                             These configure to the luma component for YUV
///                             formats, or all the components for RGB formats.
///                             There is one pointer for the horizontal
///                             direction, and one pointer for the vertical
///                             direction.
/// @param[in] chromaX,chromaY  Pointers to #NvMedia2DFilterCoefficients10Tap.
///                             These configure the chroma component for YUV
///                             formats. There is one pointer for the
///                             horizontal direction, and one pointer for the
///                             vertical direction.
///
/// @return An #NvMediaStatus return code.
/// @retval NVMEDIA_STATUS_OK             Filter buffer was updated successfully.
/// @retval NVMEDIA_STATUS_BAD_PARAMETER  One of the parameters has an invalid
///                                       value, either:
///                                       - @a handle is NULL.
///                                       - @a filterBuffer is invalid.
///                                       - @a index is out of range.
///                                       - @a lumaX is NULL.
///                                       - @a lumaY is NULL.
///                                       - @a chromaX is NULL.
///                                       - @a chromaY is NULL.
///
/// @pre
/// - @a handle must be valid NvMedia2D handle created with NvMedia2DCreate().
/// - @a filterBuffer must be valid NvMedia2DFilterBuffer handle created with
///   NvMedia2DCreateFilterBuffer().
///
/// @usage
/// - Allowed context for the API call
///   - Interrupt handler: No
///   - Signal handler: No
///   - Thread-safe: Yes, with the following conditions:
///     - Each thread uses different NvMedia2D handle
///   - Re-entrant: No
///   - Async/Sync: Sync
/// - Required privileges: None
/// - API group
///   - Init: Yes
///   - Runtime: Yes
///   - De-Init: No
/// \if (NV_ASIL)
/// - Implementation ASIL:
///   - ASIL-B
/// \endif
/// - Maturity: GA
///
/// @sa NvMedia2DFilterBuffer
/// @sa NvMedia2DFilterCoefficients10Tap
/// @sa NvMedia2DCreateFilterBuffer()
///
NvMediaStatus
NvMedia2DComputeFilterCoefficients10Tap(NvMedia2D const * const handle,
                                        NvMedia2DFilterBuffer const filterBuffer,
                                        uint32_t const index,
                                        NvMedia2DFilterCoefficients10Tap const * const lumaX,
                                        NvMedia2DFilterCoefficients10Tap const * const lumaY,
                                        NvMedia2DFilterCoefficients10Tap const * const chromaX,
                                        NvMedia2DFilterCoefficients10Tap const * const chromaY);

/// @brief Creates and returns an #NvMedia2DLdcWarpMap instance.
///
/// This functions returns a handle to an NvMedia2DLdcWarpMap object. The
/// warp map can be used to control behaviour of an LDC operation.
///
/// The handle returned in @a ldcWarpMap is tied to the specific NvMedia2D
/// context instance passed in @a handle and cannot be used with other context
/// instances.
///
/// The warp map instance must be destroyed with #NvMedia2DDestroyLdcWarpMap()
/// during the De-Init stage.
///
/// @param[in]  handle          Pointer to the NvMedia2D context.
/// @param[in]  ldcWarpMapAttr  Pointer to NvMedia2DLdcWarpMapAttributes struct,
///                             or NULL for default attributes.
/// @param[out] ldcWarpMap      Pointer to an #NvMedia2DLdcWarpMap, which will
///                             be populated with the handle.
///
/// @return An #NvMediaStatus return code.
/// @retval NVMEDIA_STATUS_OK                      Object created successfully.
/// @retval NVMEDIA_STATUS_BAD_PARAMETER           One of the parameters has an
///                                                invalid value, either:
///                                                - @a handle is NULL
///                                                - @a ldcWarpMap is NULL.
/// @retval NVMEDIA_STATUS_NOT_SUPPORTED           LDC operations are not
///                                                supported on current platform.
/// @retval NVMEDIA_STATUS_INSUFFICIENT_BUFFERING  Maximum number of warp maps
///                                                has been created.
/// @retval NVMEDIA_STATUS_INVALID_STATE           The function was called in
///                                                incorrect system state.
/// @retval NVMEDIA_STATUS_OUT_OF_MEMORY           Failed to allocate memory for
///                                                the warp map.
/// @retval NVMEDIA_STATUS_ERROR                   An internal failure occurred
///                                                when trying to create the
///                                                warp map.
///
/// @pre
/// - @a handle must be valid NvMedia2D handle created with NvMedia2DCreate().
/// - Any NvMedia2DLdcWarpMapAttributes struct passed in @a attr must have been
///   initialized with NvMedia2DGetDefaultLdcWarpMapAttributes().
///
/// @usage
/// - Allowed context for the API call
///   - Interrupt handler: No
///   - Signal handler: No
///   - Thread-safe: Yes, with the following conditions:
///     - Each thread uses different NvMedia2D handle
///   - Re-entrant: No
///   - Async/Sync: Sync
/// - Required privileges: None
/// - API group
///   - Init: Yes
///   - Runtime: No
///   - De-Init: No
/// \if (NV_ASIL)
/// - Implementation ASIL:
///   - ASIL-B
/// \endif
/// - Maturity: GA
///
/// @sa NvMedia2DLdcWarpMapAttributes
/// @sa NvMedia2DLdcWarpMap
/// @sa NvMedia2DGetDefaultLdcWarpMapAttributes()
/// @sa NvMedia2DDestroyLdcWarpMap()
///
NvMediaStatus
NvMedia2DCreateLdcWarpMap(NvMedia2D const * const handle,
                          NvMedia2DLdcWarpMapAttributes const * const ldcWarpMapAttr,
                          NvMedia2DLdcWarpMap * const ldcWarpMap);

/// @brief Destroys an #NvMedia2DLdcWarpMap instance.
///
/// This functions destroys an NvMedia2DLdcWarpMap object.
///
/// @param[in] handle      Pointer to the NvMedia2D context.
/// @param[in] ldcWarpMap  An #NvMedia2DLdcWarpMap handle.
///
/// @return An #NvMediaStatus return code.
/// @retval NVMEDIA_STATUS_OK             Object destroyed successfully.
/// @retval NVMEDIA_STATUS_BAD_PARAMETER  One of the parameters has an invalid
///                                       value, either:
///                                       - @a handle is NULL.
///                                       - @a ldcWarpMap is invalid.
/// @retval NVMEDIA_STATUS_INVALID_STATE  The function was called in incorrect
///                                       system state.
/// @retval NVMEDIA_STATUS_PENDING        The warp map is still being used by a
///                                       pending operation.
/// @retval NVMEDIA_STATUS_ERROR          An internal failure occurred when
///                                       trying to destroy the warp map. The
///                                       warp map is in state where the only
///                                       valid operation is to attempt to
///                                       destroy it again.
///
/// @pre
/// - @a handle must be valid NvMedia2D handle created with NvMedia2DCreate().
/// - @a ldcWarpMap must be valid NvMedia2DLdcWarpMap handle created with
///   NvMedia2DCreateLdcWarpMap().
///
/// @usage
/// - Allowed context for the API call
///   - Interrupt handler: No
///   - Signal handler: No
///   - Thread-safe: Yes, with the following conditions:
///     - Each thread uses different NvMedia2D handle
///   - Re-entrant: No
///   - Async/Sync: Sync
/// - Required privileges: None
/// - API group
///   - Init: Yes
///   - Runtime: No
///   - De-Init: Yes
/// \if (NV_ASIL)
/// - Implementation ASIL:
///   - ASIL-B
/// \endif
/// - Maturity: GA
///
/// @sa NvMedia2DLdcWarpMap
/// @sa NvMedia2DCreateLdcWarpMap()
///
NvMediaStatus
NvMedia2DDestroyLdcWarpMap(NvMedia2D const * const handle,
                           NvMedia2DLdcWarpMap const ldcWarpMap);

/// @brief Creates and returns an #NvMedia2DLdcMaskMap instance.
///
/// This functions returns a handle to an NvMedia2DLdcMaskMap object. The
/// mask map can be used to control behaviour of an LDC operation.
///
/// The handle returned in @a ldcMaskMap is tied to the specific NvMedia2D
/// context instance passed in @a handle and cannot be used with other context
/// instances.
///
/// The buffer instance must be destroyed with #NvMedia2DDestroyLdcMaskMap()
/// during the De-Init stage.
///
/// @param[in]  handle          Pointer to the NvMedia2D context.
/// @param[in]  ldcMaskMapAttr  Pointer to NvMedia2DLdcMaskMapAttributes struct,
///                             or NULL for default attributes.
/// @param[out] ldcMaskMap      Pointer to an #NvMedia2DLdcMaskMap, which will
///                             be populated with the handle.
///
/// @return An #NvMediaStatus return code.
/// @retval NVMEDIA_STATUS_OK                      Object created successfully.
/// @retval NVMEDIA_STATUS_BAD_PARAMETER           One of the parameters has an
///                                                invalid value, either:
///                                                - @a handle is NULL
///                                                - @a ldcMaskMap is NULL.
/// @retval NVMEDIA_STATUS_NOT_SUPPORTED           LDC mask maps are not
///                                                supported on current platform.
/// @retval NVMEDIA_STATUS_INSUFFICIENT_BUFFERING  Maximum number of mask maps
///                                                has been created.
/// @retval NVMEDIA_STATUS_INVALID_STATE           The function was called in
///                                                incorrect system state.
/// @retval NVMEDIA_STATUS_OUT_OF_MEMORY           Failed to allocate memory for
///                                                the mask map.
/// @retval NVMEDIA_STATUS_ERROR                   An internal failure occurred
///                                                when trying to create the
///                                                buffer.
///
/// @pre
/// - @a handle must be valid NvMedia2D handle created with NvMedia2DCreate().
/// - Any NvMedia2DLdcMaskMapAttributes struct passed in @a attr must have been
///   initialized with NvMedia2DGetDefaultLdcMaskMapAttributes().
///
/// @usage
/// - Allowed context for the API call
///   - Interrupt handler: No
///   - Signal handler: No
///   - Thread-safe: Yes, with the following conditions:
///     - Each thread uses different NvMedia2D handle
///   - Re-entrant: No
///   - Async/Sync: Sync
/// - Required privileges: None
/// - API group
///   - Init: Yes
///   - Runtime: No
///   - De-Init: No
/// \if (NV_ASIL)
/// - Implementation ASIL:
///   - ASIL-B
/// \endif
/// - Maturity: GA
///
/// @sa NvMedia2DLdcMaskMapAttributes
/// @sa NvMedia2DLdcMaskMap
/// @sa NvMedia2DGetDefaultLdcMaskMapAttributes()
/// @sa NvMedia2DDestroyLdcMaskMap()
///
NvMediaStatus
NvMedia2DCreateLdcMaskMap(NvMedia2D const * const handle,
                          NvMedia2DLdcMaskMapAttributes const * const ldcMaskMapAttr,
                          NvMedia2DLdcMaskMap * const ldcMaskMap);

/// @brief Destroys an #NvMedia2DLdcMaskMap instance.
///
/// This functions destroys an NvMedia2DLdcMaskMap object.
///
/// @param[in] handle      Pointer to the NvMedia2D context.
/// @param[in] ldcMaskMap  An #NvMedia2DLdcMaskMap handle.
///
/// @return An #NvMediaStatus return code.
/// @retval NVMEDIA_STATUS_OK             Object destroyed successfully.
/// @retval NVMEDIA_STATUS_BAD_PARAMETER  One of the parameters has an invalid
///                                       value, either:
///                                       - @a handle is NULL.
///                                       - @a ldcMaskMap is invalid.
/// @retval NVMEDIA_STATUS_INVALID_STATE  The function was called in incorrect
///                                       system state.
/// @retval NVMEDIA_STATUS_PENDING        The mask map is still being used by a
///                                       pending operation.
/// @retval NVMEDIA_STATUS_ERROR          An internal failure occurred when
///                                       trying to destroy the mask map. The
///                                       mask map is in state where the only
///                                       valid operation is to attempt to
///                                       destroy it again.
///
/// @pre
/// - @a handle must be valid NvMedia2D handle created with NvMedia2DCreate().
/// - @a ldcMaskMap must be valid NvMedia2DLdcMaskMap handle created with
///   NvMedia2DCreateLdcMaskMap().
///
/// @usage
/// - Allowed context for the API call
///   - Interrupt handler: No
///   - Signal handler: No
///   - Thread-safe: Yes, with the following conditions:
///     - Each thread uses different NvMedia2D handle
///   - Re-entrant: No
///   - Async/Sync: Sync
/// - Required privileges: None
/// - API group
///   - Init: Yes
///   - Runtime: No
///   - De-Init: Yes
/// \if (NV_ASIL)
/// - Implementation ASIL:
///   - ASIL-B
/// \endif
/// - Maturity: GA
///
/// @sa NvMedia2DLdcMaskMap
/// @sa NvMedia2DCreateLdcMaskMap()
///
NvMediaStatus
NvMedia2DDestroyLdcMaskMap(NvMedia2D const * const handle,
                           NvMedia2DLdcMaskMap const ldcMaskMap);

/// @brief Populates an LDC warp map with content.
///
/// This uses the specified parameters to populate the content of the LDC warp
/// map. The map data accessed through ldcWarpMapParams is copied to the
/// NvMedia2DLdcWarpMap internal data structures.
///
/// @param[in] handle            Pointer to the NvMedia2D context.
/// @param[in] ldcWarpMap        An #NvMedia2DLdcWarpMap handle.
/// @param[in] ldcWarpMapParams  Pointer to #NvMedia2DLdcWarpMapParameters.
///
/// @return An #NvMediaStatus return code.
/// @retval NVMEDIA_STATUS_OK               Parameters set successfully.
/// @retval NVMEDIA_STATUS_BAD_PARAMETER    One of the parameters has an invalid
///                                         value. This could be:
///                                         - @a handle is NULL
///                                         - @a ldcWarpMap is invalid
///                                         - @a ldcWarpMapParams is NULL.
///
/// @pre
/// - @a handle must be valid NvMedia2D handle created with NvMedia2DCreate().
/// - @a ldcWarpMap must be valid NvMedia2DLdcWarpMap handle created with
///   NvMedia2DCreateLdcWarpMap().
///
/// @usage
/// - Allowed context for the API call
///   - Interrupt handler: No
///   - Signal handler: No
///   - Thread-safe: Yes, with the following conditions:
///     - Each thread uses different NvMedia2D handle
///   - Re-entrant: No
///   - Async/Sync: Sync
/// - Required privileges: None
/// - API group
///   - Init: Yes
///   - Runtime: Yes
///   - De-Init: No
/// \if (NV_ASIL)
/// - Implementation ASIL:
///   - ASIL-B
/// \endif
/// - Maturity: GA
///
/// @sa NvMediai2DLdcWarpMap
/// @sa NvMediai2DLdcWarpMapParameters
///
NvMediaStatus
NvMedia2DFillLdcWarpMap(NvMedia2D const * const handle,
                        NvMedia2DLdcWarpMap const ldcWarpMap,
                        NvMedia2DLdcWarpMapParameters const * const lcdWarpMapParams);

/// @brief Populates an LDC mask map with content.
///
/// This uses the specified parameters to populate the content of the LDC mask
/// map. The map data accessed through ldcMaskMapParams is copied to the
/// NvMedia2DLdcMaskMap internal data structures.
///
/// @param[in] handle            Pointer to the NvMedia2D context.
/// @param[in] ldcMaskMap        An #NvMedia2DLdcMaskMap handle.
/// @param[in] ldcMaskMapParams  Pointer to #NvMedia2DLdcMaskMapParameters.
///
/// @return An #NvMediaStatus return code.
/// @retval NVMEDIA_STATUS_OK               Parameters set successfully.
/// @retval NVMEDIA_STATUS_BAD_PARAMETER    One of the parameters has an invalid
///                                         value. This could be:
///                                         - @a handle is NULL
///                                         - @a ldcMaskMap is invalid
///                                         - @a ldcMaskMapParams is NULL.
///
/// @pre
/// - @a handle must be valid NvMedia2D handle created with NvMedia2DCreate().
/// - @a ldcMaskMap must be valid NvMedia2DLdcMaskMap handle created with
///   NvMedia2DCreateLdcMaskMap().
///
/// @usage
/// - Allowed context for the API call
///   - Interrupt handler: No
///   - Signal handler: No
///   - Thread-safe: Yes, with the following conditions:
///     - Each thread uses different NvMedia2D handle
///   - Re-entrant: No
///   - Async/Sync: Sync
/// - Required privileges: None
/// - API group
///   - Init: Yes
///   - Runtime: Yes
///   - De-Init: No
/// \if (NV_ASIL)
/// - Implementation ASIL:
///   - ASIL-B
/// \endif
/// - Maturity: GA
///
/// @sa NvMediai2DLdcMaskMap
/// @sa NvMediai2DLdcMaskMapParameters
///
NvMediaStatus
NvMedia2DFillLdcMaskMap(NvMedia2D const * const handle,
                        NvMedia2DLdcMaskMap const ldcMaskMap,
                        NvMedia2DLdcMaskMapParameters const * const ldcMaskMapParams);

/// @brief Creates and returns an #NvMedia2DTnrState instance.
///
/// This functions returns a handle to an NvMedia2DTnrState object. The
/// TNR state can be used to control behaviour of a TNR operation.
///
/// The handle returned in @a tnrState is tied to the specific NvMedia2D
/// context instance passed in @a handle and cannot be used with other context
/// instances.
///
/// The TNR state instance must be destroyed with #NvMedia2DDestroyTnrState()
/// during the De-Init stage.
///
/// @param[in]  handle        Pointer to the NvMedia2D context.
/// @param[in]  tnrStateAttr  Pointer to NvMedia2DTnrStateAttributes struct,
///                           or NULL for default attributes.
/// @param[out] tnrState      Pointer to an #NvMedia2DTnrState, which will
///                           be populated with the handle.
///
/// @return An #NvMediaStatus return code.
/// @retval NVMEDIA_STATUS_OK                      Object created successfully.
/// @retval NVMEDIA_STATUS_BAD_PARAMETER           One of the parameters has an
///                                                invalid value, either:
///                                                - @a handle is NULL
///                                                - @a tnrState is NULL.
/// @retval NVMEDIA_STATUS_INSUFFICIENT_BUFFERING  Maximum number of TNR states
///                                                has been created.
/// @retval NVMEDIA_STATUS_INVALID_STATE           The function was called in
///                                                incorrect system state.
/// @retval NVMEDIA_STATUS_OUT_OF_MEMORY           Failed to allocate memory for
///                                                the TNR state.
/// @retval NVMEDIA_STATUS_ERROR                   An internal failure occurred
///                                                when trying to create the
///                                                TNR state.
///
/// @pre
/// - @a handle must be valid NvMedia2D handle created with NvMedia2DCreate().
/// - Any NvMedia2DTnrStateAttributes struct passed in @a attr must have been
///   initialized with NvMedia2DGetDefaultTnrStateAttributes().
///
/// @usage
/// - Allowed context for the API call
///   - Interrupt handler: No
///   - Signal handler: No
///   - Thread-safe: Yes, with the following conditions:
///     - Each thread uses different NvMedia2D handle
///   - Re-entrant: No
///   - Async/Sync: Sync
/// - Required privileges: None
/// - API group
///   - Init: Yes
///   - Runtime: No
///   - De-Init: No
/// \if (NV_ASIL)
/// - Implementation ASIL:
///   - ASIL-B
/// \endif
/// - Maturity: GA
///
/// @sa NvMedia2DTnrStateAttributes
/// @sa NvMedia2DTnrState
/// @sa NvMedia2DGetDefaultTnrStateAttributes()
/// @sa NvMedia2DDestroyTnrState()
///
NvMediaStatus
NvMedia2DCreateTnrState(NvMedia2D const * const handle,
                        NvMedia2DTnrStateAttributes const * const tnrStateAttr,
                        NvMedia2DTnrState * const tnrState);

/// @brief Destroys an #NvMedia2DTnrState instance.
///
/// This functions destroys an NvMedia2DTnrState object.
///
/// @param[in] handle    Pointer to the NvMedia2D context.
/// @param[in] tnrState  An #NvMedia2DTnrState handle.
///
/// @return An #NvMediaStatus return code.
/// @retval NVMEDIA_STATUS_OK             Object destroyed successfully.
/// @retval NVMEDIA_STATUS_BAD_PARAMETER  One of the parameters has an invalid
///                                       value, either:
///                                       - @a handle is NULL.
///                                       - @a tnrState is invalid.
/// @retval NVMEDIA_STATUS_INVALID_STATE  The function was called in incorrect
///                                       system state.
/// @retval NVMEDIA_STATUS_PENDING        The TNR statemap is still being used
///                                       by a pending operation.
/// @retval NVMEDIA_STATUS_ERROR          An internal failure occurred when
///                                       trying to destroy the TNR state. The
///                                       object is in state where the only
///                                       valid operation is to attempt to
///                                       destroy it again.
///
/// @pre
/// - @a handle must be valid NvMedia2D handle created with NvMedia2DCreate().
/// - @a tnrState must be valid NvMedia2DTnrState handle created with
///   NvMedia2DCreateTnrState().
///
/// @usage
/// - Allowed context for the API call
///   - Interrupt handler: No
///   - Signal handler: No
///   - Thread-safe: Yes, with the following conditions:
///     - Each thread uses different NvMedia2D handle
///   - Re-entrant: No
///   - Async/Sync: Sync
/// - Required privileges: None
/// - API group
///   - Init: Yes
///   - Runtime: No
///   - De-Init: Yes
/// \if (NV_ASIL)
/// - Implementation ASIL:
///   - ASIL-B
/// \endif
/// - Maturity: GA
///
/// @sa NvMedia2DTnrState
/// @sa NvMedia2DCreateTnrState()
///
NvMediaStatus
NvMedia2DDestroyTnrState(NvMedia2D const * const handle,
                         NvMedia2DTnrState const tnrState);

/// @brief Sets Temporal Noise Reduction (TNR) parameters.
///
/// This updates the TNR state object to use the specified parameters for the
/// TNR operation.
///
/// @param[in] handle     Pointer to the NvMedia2D context.
/// @param[in] tnrState   An NvMedia2DTnrState handle.
/// @param[in] tnrParams  Pointer to #NvMedia2DTnrParameters.
///
/// @return An #NvMediaStatus return code.
/// @retval NVMEDIA_STATUS_OK               Parameters set successfully.
/// @retval NVMEDIA_STATUS_BAD_PARAMETER    One of the parameters has an invalid
///                                         value. This could be:
///                                         - @a handle is NULL
///                                         - @a tnrState is invalid
///                                         - @a tnrParams is NULL.
///
/// @pre
/// - @a handle must be valid NvMedia2D handle created with NvMedia2DCreate().
/// - @a tnrState must be valid NvMedia2DTnrState handle created with
///   NvMedia2DCreateTnrState().
///
/// @usage
/// - Allowed context for the API call
///   - Interrupt handler: No
///   - Signal handler: No
///   - Thread-safe: Yes, with the following conditions:
///     - Each thread uses different NvMedia2D handle
///   - Re-entrant: No
///   - Async/Sync: Sync
/// - Required privileges: None
/// - API group
///   - Init: No
///   - Runtime: Yes
///   - De-Init: No
/// \if (NV_ASIL)
/// - Implementation ASIL:
///   - ASIL-B
/// \endif
/// - Maturity: GA
///
/// @sa NvMedia2DTnrState
/// @sa NvMedia2DTnrParameters
///
NvMediaStatus
NvMedia2DSetTnrParameters(NvMedia2D const * const handle,
                          NvMedia2DTnrState const tnrState,
                          NvMedia2DTnrParameters const *const tnrParams);

/// @brief Resets TNR algorithm state.
///
/// This function resets the internal TNR algorithm state. After a successful
/// call to this function, the next frame submitted for TNR processing with
/// NvMedia2DCompose() will be treated by the algorithm as the
/// first one and no previous surface needs to be set for it with
/// NvMedia2DSetTnrPreviousNvSciBufObj().
///
/// @param[in] handle    Pointer to the NvMedia2D context.
/// @param[in] tnrState  An NvMedia2DTnrState handle.
///
/// @return An #NvMediaStatus return code.
/// @retval NVMEDIA_STATUS_OK               State reset successfully.
/// @retval NVMEDIA_STATUS_BAD_PARAMETER    One of the parameters has an invalid
///                                         value. This could be:
///                                         - @a handle is NULL
///                                         - @a tnrState is invalid.
///
/// @pre
/// - @a handle must be valid NvMedia2D handle created with NvMedia2DCreate().
/// - @a params must be valid NvMedia2DTnrState handle created with
///   NvMedia2DCreateTnrState().
///
/// @usage
/// - Allowed context for the API call
///   - Interrupt handler: No
///   - Signal handler: No
///   - Thread-safe: Yes, with the following conditions:
///     - Each thread uses different NvMedia2D handle
///   - Re-entrant: No
///   - Async/Sync: Sync
/// - Required privileges: None
/// - API group
///   - Init: No
///   - Runtime: Yes
///   - De-Init: No
/// \if (NV_ASIL)
/// - Implementation ASIL:
///   - ASIL-B
/// \endif
/// - Maturity: GA
///
/// @sa NvMedia2DTnrState
///
NvMediaStatus
NvMedia2DResetTnr(NvMedia2D const * const handle,
                  NvMedia2DTnrState const tnrState);

/// @brief Sets checksum calculation mode.
///
/// This function configures the checksums to calculate for the
/// NvMedia2DCompose() operations performed using @a params.
///
/// NvMedia 2D maintains the checksums from 16 most recent NvMedia2DCompose()
/// operations that had the checksum calculation configured using this function.
/// When more operations are performed, the oldest checksums are overwritten.
/// The client must ensure the checksums are read with NvMedia2DGetChecksum()
/// before they are overwritten by subsequent operations.
///
/// @param[in] handle        Pointer to the NvMedia2D context.
/// @param[in] params        An NvMedia2DComposeParameters handle.
/// @param[in] checksumMode  The checksum to calculate.
///
/// @return An #NvMediaStatus return code.
/// @retval NVMEDIA_STATUS_OK             Parameters set successfully.
/// @retval NVMEDIA_STATUS_BAD_PARAMETER  One of the parameters has an invalid
///                                       value. This could be:
///                                       - @a handle is NULL
///                                       - @a params is invalid
///                                       - @a checksumMode is invalid.
///
/// @pre
/// - @a handle must be valid NvMedia2D handle created with NvMedia2DCreate().
/// - @a params must be valid NvMedia2DComposeParameters handle created with
///   NvMedia2DGetComposeParameters().
///
/// @usage
/// - Allowed context for the API call
///   - Interrupt handler: No
///   - Signal handler: No
///   - Thread-safe: Yes, with the following conditions:
///     - Each thread uses different NvMedia2D handle
///   - Re-entrant: No
///   - Async/Sync: Sync
/// - Required privileges: None
/// - API group
///   - Init: No
///   - Runtime: Yes
///   - De-Init: No
/// \if (NV_ASIL)
/// - Implementation ASIL:
///   - ASIL-B
/// \endif
/// - Maturity: GA
///
/// @sa NvMedia2DChecksumMode
/// @sa NvMedia2DGetChecksum()
///
NvMediaStatus
NvMedia2DSetChecksumMode(NvMedia2D const * const handle,
                         NvMedia2DComposeParameters const params,
                         NvMedia2DChecksumMode const checksumMode);

/// @brief Gets a checksum calculated for an NvMedia2DCompose() operation.
///
/// This function gets a checksum calculated for an NvMedia2DCompose()
/// operation. The operation must have had checksum calculation configured with
/// NvMedia2DSetChecksumMode().
///
/// @param[in] handle     Pointer to the NvMedia2D context.
/// @param[in] result     Pointer to the NvMedia2DComposeResult object of the
///                       operation.
/// @param[out] checksum  Pointer to an NvMedia2DChecksum object that will be
///                       populated with the checksum.
///
/// @return An #NvMediaStatus return code.
/// @retval NVMEDIA_STATUS_OK             Checksum returned successfully in
///                                       @a checksum.
/// @retval NVMEDIA_STATUS_BAD_PARAMETER  One of the parameters has an invalid
///                                       value. This could be:
///                                       - @a handle is NULL
///                                       - @a result is NULL
///                                       - @a result indicates no operation was
///                                         submitted
///                                       - No checksum was found for operation
///                                         represented by @a result
///                                       - @a checksum is NULL.
/// @retval NVMEDIA_STATUS_NOT_SUPPORTED  Checksums are not supported on current
///                                       platform.
/// @retval NVMEDIA_STATUS_PENDING        The operation is still pending.
/// @retval NVMEDIA_STATUS_ERROR          An internal failure occurred when
///                                       trying to read the checksum.
///
/// @pre
/// - @a handle must be valid NvMedia2D handle created with NvMedia2DCreate().
/// - @a result must be valid NvMedia2DComposeResult handle returned by
///   NvMedia2DCompose().
///
/// @usage
/// - Allowed context for the API call
///   - Interrupt handler: No
///   - Signal handler: No
///   - Thread-safe: Yes, with the following conditions:
///     - Each thread uses different NvMedia2D handle
///   - Re-entrant: No
///   - Async/Sync: Sync
/// - Required privileges: None
/// - API group
///   - Init: No
///   - Runtime: Yes
///   - De-Init: No
/// \if (NV_ASIL)
/// - Implementation ASIL:
///   - ASIL-B
/// \endif
/// - Maturity: GA
///
/// @sa NvMedia2DChecksum
/// @sa NvMedia2DSetChecksumMode()
///
NvMediaStatus
NvMedia2DGetChecksum(NvMedia2D const * const handle,
                     NvMedia2DComposeResult const *const result,
                     NvMedia2DChecksum *const checksum);

/// @brief Configures the background fill for the NvMedia2DCompose() operation.
///
/// This function configures whether the NvMedia2DCompose() operation should
/// fill the area inside the clip rectangle but outside any destination
/// rectangle with a solid background color.
///
/// The default behavior is to fill the background.
///
/// The support for non-filled background can be checked with
/// NvMedia2DCapabilities::supportsNonFilledBackground.
///
/// If NvMedia2DCapabilities::supportsCompose is false, this function will fail
/// with NVMEDIA_STATUS_NOT_SUPPORTED.
///
/// @param[in] handle          Pointer to the NvMedia2D context.
/// @param[in] params          An NvMedia2DComposeParameters handle.
/// @param[in] fillBackground  Whether to fill the background.
///
/// @return An #NvMediaStatus return code.
/// @retval NVMEDIA_STATUS_OK             Parameters set successfully.
/// @retval NVMEDIA_STATUS_BAD_PARAMETER  One of the parameters has an invalid
///                                       value. This could be:
///                                       - @a handle is NULL
///                                       - @a params is invalid.
/// @retval NVMEDIA_STATUS_NOT_SUPPORTED  Requested operation is not supported
///                                       on current platform.
///
/// @pre
/// - @a handle must be valid NvMedia2D handle created with NvMedia2DCreate().
/// - @a params must be valid NvMedia2DComposeParameters handle created with
///   NvMedia2DGetComposeParameters().
///
/// @usage
/// - Allowed context for the API call
///   - Interrupt handler: No
///   - Signal handler: No
///   - Thread-safe: Yes, with the following conditions:
///     - Each thread uses different NvMedia2D handle
///   - Re-entrant: No
///   - Async/Sync: Sync
/// - Required privileges: None
/// - API group
///   - Init: No
///   - Runtime: Yes
///   - De-Init: No
/// \if (NV_ASIL)
/// - Implementation ASIL:
///   - ASIL-B
/// \endif
/// - Maturity: GA
///
/// @sa NvMedia2DCapabilities::supportsCompose
/// @sa NvMedia2DCapabilities::supportsNonFilledBackground
///
NvMediaStatus
NvMedia2DSetBackgroundFill(NvMedia2D const *const handle,
                           NvMedia2DComposeParameters const params,
                           bool const fillBackground);

///
/// @}
///

#ifdef __cplusplus
}
#endif

#endif // NVMEDIA_2D_H
