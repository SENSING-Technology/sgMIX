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
 * @file WarpGrid.h
 *
 * Declares structures and constants used for warp grid definition.
 */

#ifndef NV_VPI_WARPGRID_H
#define NV_VPI_WARPGRID_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @ingroup VPI_WarpMap
 * @{
 * */

#define VPI_WARPGRID_MAX_HORIZ_REGIONS_COUNT 4 /**< Maximum number of regions horizontally in a warp grid. */
#define VPI_WARPGRID_MAX_VERT_REGIONS_COUNT 4  /**< Maximum number of regions vertically in a warp grid. */
#define VPI_WARPGRID_MIN_REGION_WIDTH 64       /**< Minimum warp grid region width. */
#define VPI_WARPGRID_MIN_REGION_HEIGHT 16      /**< Minimum warp grid region height. */

/**
 * Holds VPI's warp grid definition.
 *
 * @ingroup VPI_WarpMap
 * This structure defines the layout of the control points in the destination
 * image of a remap operation.
 *
 * The control points are used as the basis for geometric transformation from
 * source image to destination image. The remaining points are transformed
 * based on the interpolation. Thus the density of the control points controls
 * the quality of the geometric transformation.
 *
 * This is an example of defining regions in the image:
 * \code
 *               warp grid width
 *         /                         \
 *        /                           \
 *       /                             \
 *
 *    regionWidth[0]    regionWidth[numHorizRegions-1]
 *       /    \                  /    \
 *      |------|                |------|
 *      --------------------------------                                 \
 *      |******* *******        *******|--                                \
 *      |* . . * *     *        *     *|  \                                \
 *      |* . . * *     *  ...   *     *|   regionHeight[0]                  \
 *      |* . . * *     *        *     *|  /                                  \
 *      |******* *******        *******|--                                    \
 *      |                              |                                       \
 *      |   .       .              .   |                                  
 *      |   .       .              .   |                                         warp grid height
 *      |   .       .              .   |
 *      |                              |                                       /
 *      |******* *******        *******|--                                    /
 *      |*     * *     *        *     *|  \                                  /
 *      |*     * *     *  ...   *     *|   regionHeight[numVertRegions-1]   /
 *      |*     * *     *        *     *|  /                                /
 *      |******* *******        *******|--                                /  
 *      --------------------------------                                 /
 *
 * \endcode
 *
 * This is an example of defining control points in one region:
 * \code
 *      *********
 *      *  +  + *-- \
 *      *       *    vertInterval
 *      *  +  + *-- /
 *      *       *
 *      *********
 *         |--|
 *     horizInterval
 *
 * \endcode
 *
 * Here's an example of a WxH dense grid definition:
 * \code
 * VPIWarpGrid grid;
 * grid.numHorizRegions  = 1;
 * grid.numVertRegions   = 1;
 * grid.horizInterval[0] = 1;
 * grid.vertInterval[0]  = 1;
 * grid.regionWidth[0]   = W;
 * grid.regionHeight[0]  = H;
 * \endcode
 *
 * ### Restrictions
 *
 * * numHorizRegions cannot exceed \ref VPI_WARPGRID_MAX_HORIZ_REGIONS_COUNT.
 * * numVertRegions cannot exceed \ref VPI_WARPGRID_MAX_VERT_REGIONS_COUNT.
 * * Intervals must be power-of-two.
 * * Alignment restrictions:
 *   + `regionWidth[0]` to `regionWidth[numHorizRegions-2]` must be
 *     aligned to \ref VPI_WARPGRID_MIN_REGION_WIDTH and at least \ref VPI_WARPGRID_MIN_REGION_WIDTH.
 *   + `regionWidth[numHorizRegions-1]` must be at least \ref VPI_WARPGRID_MIN_REGION_WIDTH.
 *   + `regionHeight[0]` to `regionHeight[numVertRegions-2]` must be
 *     aligned to \ref VPI_WARPGRID_MIN_REGION_HEIGHT and at least \ref VPI_WARPGRID_MIN_REGION_HEIGHT.
 *   + `regionHeight[numVertRegions-1]` must be at least \ref VPI_WARPGRID_MIN_REGION_HEIGHT.
 *
 */

typedef struct
{
    /** Number of regions horizontally. */
    int8_t numHorizRegions;

    /** Number of regions vertically. */
    int8_t numVertRegions;

    /** Width of each region. */
    int16_t regionWidth[VPI_WARPGRID_MAX_HORIZ_REGIONS_COUNT];

    /** Height of each region. */
    int16_t regionHeight[VPI_WARPGRID_MAX_VERT_REGIONS_COUNT];

    /** Horizontal spacing between control points within a given region.
     *  + Must be power-of-two.
     *  + Must be >= 1.
     *  + On PVA, maximum is 128. */
    int16_t horizInterval[VPI_WARPGRID_MAX_HORIZ_REGIONS_COUNT];

    /** Vertical spacing between control points within a given region.
     *  + Must be power-of-two.
     *  + Must be >= 1.
     *  + On PVA, maximum is 128. */
    int16_t vertInterval[VPI_WARPGRID_MAX_VERT_REGIONS_COUNT];

} VPIWarpGrid;

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* NV_VPI_WARPGRID_H */
