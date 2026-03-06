/*
 * Copyright 2023 NVIDIA Corporation. All rights reserved.
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
 * @file BruteForceMatcher.h
 *
 * Declares functions that implement brute force matcher algorithms.
 */

#ifndef NV_VPI_ALGORITHMS_BRUTEFORCE_MATCHER_H
#define NV_VPI_ALGORITHMS_BRUTEFORCE_MATCHER_H

#include "../AlgoFlags.h"
#include "../Export.h"
#include "../Status.h"
#include "../Types.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup VPI_BruteForceMatcher Brute Force Matcher
 * @ingroup VPI_API_Algorithms
 * @{
 *
 * Runs a brute force matcher algorithm on descriptors.
 * 
 * Brute force matcher uses query descriptors and reference descriptors provided by the user
 * and calculates matches for every query descriptor by determining the closest reference descriptors.
 * The distance calculation metric can be chosen by the user and also the number of matches to be 
 * returned per query descriptor.  
 */

/**
 * Runs a brute force matcher algorithm on descriptors.
 *
 * @param[in] stream A stream handle where the operation will be queued into.
 *
 * @param[in] backend Backend that will execute the algorithm.
 *                    + Valid values: 
 *                      - \ref VPI_BACKEND_CPU
 *                      - \ref VPI_BACKEND_CUDA
 *
 * @param[in] queryDescriptor Query descriptor array.
 *                            + Must be of type VPI_ARRAY_TYPE_BRIEF_DESCRIPTOR
 *                            + Must not be NULL.
 * 
 * @param[in] referenceDescriptor Input Descriptor array to be used as refernce for matches.
 *                                + Must be of type VPI_ARRAY_TYPE_BRIEF_DESCRIPTOR
 *                                + Must not be NULL.
 * 
 * @param[in] normType Determines the type of norm calculation.
 *                     + Must be \ref VPI_NORM_HAMMING.
 * 
 * @param[in] maxMatchesPerQuery Maximum number of closest matches per query to be added in the output array.
 *                               + Must be >= 1 and <= \ref VPI_MAX_MATCHES_PER_DESCRIPTOR.
 *                               + If \ref VPI_ENABLE_CROSS_CHECK is passed as flags, maxMatchesPerQuery must be 1.
 *
 * @param[out] matches Output array.
 *                     + Must be of type \ref VPI_ARRAY_TYPE_MATCHES
 *                     + Must not be NULL.
 * 
 * @param[in] algoFlag Matches calculation flags.
 *                     + The accepted flags are:
 *                       - \ref VPI_ENABLE_CROSS_CHECK to enable cross check. 
 * 
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p stream is NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p queryDescriptor , \p referenceDescriptor or \p matches are NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p matches capacity >= \p queryDescriptor capacity.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p algoFlag not supported.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p backend is invalid or unknown.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p queryDescriptor and \p referenceDescriptor formats aren't supported.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p normType is not supported.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p matches formats is not supported.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p maxMatchesPerQuery outside valid range.
 * @retval #VPI_ERROR_NOT_IMPLEMENTED      Brute Force Matcher algorithm is not supported by given backend.
 * @retval #VPI_ERROR_INVALID_OPERATION    Backend hardware not available.
 * @retval #VPI_ERROR_INVALID_OPERATION    The needed backends aren't enabled in \p stream, \p queryDescriptor, \p referenceDescriptor or \p matches.
 * @retval #VPI_SUCCESS                    Operation executed successfully.
 */
VPI_PUBLIC VPIStatus vpiSubmitBruteForceMatcher(VPIStream stream, uint64_t backend, VPIArray queryDescriptor,
                                                VPIArray referenceDescriptor, VPINormType normType,
                                                int32_t maxMatchesPerQuery, VPIArray matches, uint32_t algoFlag);
/** @} */

#ifdef __cplusplus
}
#endif

#endif /* NV_VPI_ALGORITHMS_BRUTEFORCE_MATCHER_H */
