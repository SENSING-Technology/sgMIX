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
 * @file AlgoFlags.h
 *
 * Functions and structures for dealing with VPI images.
 */

#ifndef NV_VPI_ALGOFLAGS_H
#define NV_VPI_ALGOFLAGS_H

#include "Export.h"
#include "ImageFormat.h"
#include "PixelType.h"
#include "Status.h"
#include "Types.h"

#include <stdint.h>

/**
 * @ingroup VPI_Algorithms
 * @{
 */

/** 
 * Informs that the matrix used in warp/map operations is already inverted.
 *
 * If user has the transform matrix already inverted, setting this flag allows
 * the warp operation to use it directly instead of inverting it itself prior
 * processing. This is particularly useful if matrix inversion would result
 * in large numerical errors, such as when its determinant is close to zero.
 */
#define VPI_WARP_INVERSE (1u << 0)

/** Informs that the algorithm will choose a precise implementation.
 * By default algorithms will choose an implementation that might trade
 * speed over precision.
 */
#define VPI_PRECISE (1u << 1)

/** Leave output denormalized.
 *
 * Flag currently used in \ref algo_ifft, where it signals that
 * output is to be left denormalized. This leads to faster execution as
 * normalization isn't usually needed.  Absense of this flag will scale the
 * output, dividing it by the total pixel count. This makes the output the
 * exact inverse of direct Fast Fourier Transform.
 */
#define VPI_DENORMALIZED_OUTPUT (1u << 2)

/** Defines whether cross check is enabled
 * 
 * When cross check is enabled, only those pairs of query and reference descriptors
 * are selected which are closest to each other both ways. Meaning if A's(query descriptor)
 * closest reference descriptor is B and if B's closest query descriptor is A only then the pair is selected.
 */
#define VPI_ENABLE_CROSS_CHECK (1U << 0)
/**@}*/

/** Accumulate the counting in the output. 
 *
 * Once it is set, the output will not be cleared at the beginning.
 */
#define VPI_ACCUMULATE_OUTPUT (1u << 3)

/** Informs the algorithm to sample pixels aligned to corner, i.e. top-left corner, instead of the pixel center.
 */
#define VPI_SAMPLING_ALIGNED_TO_CORNER (1u << 5)

/** Informs the algorithm to consider relative coordinates.
 *
 * For instance in algo_dynamic_remap, relative mapping means that map coordinates are offsets relative to the
 * output coordinate grid.  When this flag is not set, it means absolute mapping, i.e. offsets relative to the
 * output top-left corner.  Please refer to the algorithm for further details.
 */
#define VPI_RELATIVE_MAP (1u << 6)

/** Informs the algorithm to consider normalized coordinates.
 *
 * For instance in algo_dynamic_remap, normalized mapping means that the coordinates are not dependent on the
 * output dimensions.  When this flag is not set, it means denormalized mapping, i.e. coordinates represent
 * actual pixels in the output domain.  Please refer to the algorithm for further details.
 */
#define VPI_NORMALIZED_MAP (1u << 7)

#endif /* NV_VPI_ALGOFLAGS_H */
