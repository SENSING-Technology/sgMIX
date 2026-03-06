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
 * @file WarpMap.h
 *
 * Declares functions that implement the WarpMap structure and related functions.
 */

#ifndef NV_VPI_WARPMAP_H
#define NV_VPI_WARPMAP_H

#include "Export.h"
#include "Status.h"
#include "Types.h"
#include "WarpGrid.h"

#include <stdint.h>

/**
 * Defines the \ref VPIWarpMap object and related functions.
 *
 * Warp map holds the mapping from output to input images, used in \ref
 * algo_remap algorithm. It allows both dense and sparse mapping.
 *
 * @defgroup VPI_WarpMap WarpMap
 * @ingroup VPI_API_Core
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/** Defines the mapping between input and output images' pixels.
 *
 * This structure is used as input to \ref algo_remap. It defines the
 * control point positions in the input image. The corresponding positions in
 * the output image is implicitly defined by the warp grid definition.
 */
typedef struct
{
    /** Warp grid control point structure definition.
     *  It implicitly defines the control point positions in the output image. */
    VPIWarpGrid grid;

    /** Number of points horizontally.
     *  + Must match the number of points defined by the grid. This is calculated by \ref vpiWarpMapAllocData. */
    int16_t numHorizPoints;

    /** Number of points vertically. 
     *  + Must match the number of points defined by the grid. This is calculated by \ref vpiWarpMapAllocData. */
    int16_t numVertPoints;

    /** Number of bytes between one control point and the one immediately below.
     *  + Must be at least `sizeof(VPIKeypointF32)*numHorizPoints`. */
    int32_t pitchBytes;

    /** Pointer to an array with control point positions in the input image
     * corresponding to those in the output image.
     * Coordinates are absolute, (0,0) is the top/left corner of output image.
     */
    VPIKeypointF32 *keypoints;
} VPIWarpMap;

/** Allocates the warp map's control point array for a given warp grid.
 *
 * This function will read the warp grid structure and allocated an appropriately sized control point array,
 * filling the numHorizPoints, numVertPoints, strideBytes and keypoints attributes of \ref VPIWarpMap.
 * The warp map must be deallocated by \ref vpiWarpMapFreeData when no longer needed.
 *
 * @param[in,out] warpMap The warp map whose keypoint array will be allocated. 
 *                        + The warp grid attribute must be already filled out with the grid structure.
 *                        + Keypoints must be NULL. 
 *                        + All attributes other then grid and keypoints will be overwriten.
 * 
 * @retval #VPI_ERROR_INVALID_ARGUMENT \p warpMap is NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT Parameters to generate \p warpMap are invalid.
 * @retval #VPI_SUCCESS                Operation executed successfully.
 */
VPI_PUBLIC VPIStatus vpiWarpMapAllocData(VPIWarpMap *warpMap);

/** Deallocates the warp map control points allocated by \ref vpiWarpMapAllocData.
 * This function does nothing if control points array is NULL. It sets `numHorizPoints`,
 * `numVertPoints` and `strideBytes` to zero, and `keypoints` to NULL.
 *
 * @param[in,out] warpMap Warp map whose control points array needs to be deallocated.
 * 
 * @retval #VPI_ERROR_INVALID_ARGUMENT \p warpMap is NULL.
 */
VPI_PUBLIC void vpiWarpMapFreeData(VPIWarpMap *warpMap);

/** Fills the given warp map with an identity mapping.
 * This function is useful if the user wants to specify their own mapping. It
 * sets the control points coordinates to the destination coordinates as
 * defined implicitly by the warp grid. The user then can iterate through these
 * points and apply a custom mapping function to each one.
 *
 * @param[in,out] warpMap Warp map with allocated control point array to be filled with identity mapping.
 *                        + WarpMap must be allocated before generating identity.
 * 
 * @retval #VPI_ERROR_INVALID_ARGUMENT \p warpMap is NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT \p warpMap is not allocated.
 * @retval #VPI_SUCCESS                Operation executed successfully.
 */
VPI_PUBLIC VPIStatus vpiWarpMapGenerateIdentity(VPIWarpMap *warpMap);

#ifdef __cplusplus
}
#endif

/** @} */

#endif /* NV_VPI_WARPMAP_H */
