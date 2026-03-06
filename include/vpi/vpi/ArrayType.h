/*
 * Copyright 2020-2024 NVIDIA Corporation. All rights reserved.
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
 * @file ArrayType.h
 *
 * Defines array types.
 */

#ifndef NV_VPI_ARRAY_TYPE_H
#define NV_VPI_ARRAY_TYPE_H

#include "Export.h"
#include "Version.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Array element formats.
 * @ingroup VPI_Array
 **/
typedef enum
{
    VPI_ARRAY_TYPE_INVALID = 0,              /**< Signal type conversion errors. */
    VPI_ARRAY_TYPE_S8,                       /**< Signed 8-bit. */
    VPI_ARRAY_TYPE_U8,                       /**< Unsigned 8-bit. */
    VPI_ARRAY_TYPE_S16,                      /**< Signed 16-bit. */
    VPI_ARRAY_TYPE_U16,                      /**< Unsigned 16-bit. */
    VPI_ARRAY_TYPE_U32,                      /**< Unsigned 32-bit. */
    VPI_ARRAY_TYPE_KEYPOINT_F32,             /**< \ref VPIKeypointF32 element. */
    VPI_ARRAY_TYPE_HOMOGRAPHY_TRANSFORM_2D,  /**< \ref VPIHomographyTransform2D element. */
    VPI_ARRAY_TYPE_KLT_TRACKED_BOUNDING_BOX, /**< \ref VPIKLTTrackedBoundingBox element. */
    VPI_ARRAY_TYPE_F32,                      /**< Floating point 32-bit. */
    VPI_ARRAY_TYPE_KEYPOINT_U32,             /**< \ref VPIKeypointU32 element in U32 format. */
    VPI_ARRAY_TYPE_KEYPOINT_UQ1616,          /**< \ref VPIKeypointU32 element in UQ1616 fixed-point format. */
    VPI_ARRAY_TYPE_STATISTICS,               /**< \ref VPIStats element. */
    VPI_ARRAY_TYPE_BRIEF_DESCRIPTOR,         /**< \ref VPIBriefDescriptor element. */
    VPI_ARRAY_TYPE_MATCHES,                  /**< \ref VPIMatches element. */
    VPI_ARRAY_TYPE_DCF_TRACKED_BOUNDING_BOX, /**< \ref VPIDCFTrackedBoundingBox element. */
    VPI_ARRAY_TYPE_PYRAMIDAL_KEYPOINT_F32,   /**< \ref VPIPyramidalKeypointF32 element. */
    VPI_ARRAY_TYPE_APRILTAG_DETECTION,       /**< \ref VPIAprilTagDetection element. */
    VPI_ARRAY_TYPE_POSE,                     /**< \ref VPIPose element. */
} VPIArrayType;

/** Returns a string representation of the array type.
 *
 * @param[in] type Array type whose name is to be returned.
 *
 * @returns The string representation of the image type.
 *          Returned string is valid until next call of this function from the same calling thread.
 *          Returned pointer must not be freed.
 */
VPI_PUBLIC const char *vpiArrayTypeGetName(VPIArrayType type);

/** Returns the size in bytes of each array element with the given type.
 *
 * @param[in] type The array type to be queried.
 *                 + The array type must be valid.
 *
 * @returns The size in bytes of the array type.
 * @retval 0 An invalid array \p type was given.
 */
VPI_PUBLIC int vpiArrayTypeGetSize(VPIArrayType type);

#ifdef __cplusplus
}
#endif

#endif /* NV_VPI_ARRAY_TYPE_H */
