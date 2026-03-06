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
 * @file Interpolation.h
 *
 * Defines all interpolation types used in VPI.
 */

#ifndef NV_VPI_INTERPOLATION_H
#define NV_VPI_INTERPOLATION_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Interpolation types supported by several algorithms
 * @ingroup VPI_Types
 */
typedef enum
{
    VPI_INTERP_NULL = 0,

    /** Nearest neighbor interpolation.
       \f[
          P(x,y) = \mathit{in}[\lfloor x+0.5 \rfloor, \lfloor y+0.5 \rfloor]
       \f]
     */
    VPI_INTERP_NEAREST = 1,

    /** Linear interpolation.
        Interpolation weights are defined as:
        \f{align*}{
            w_0(t)& \triangleq t-\lfloor t \rfloor \\
            w_1(t)& \triangleq 1 - w_0(t) \\
        \f}

        Bilinear-interpolated value is given by the formula below:

        \f[
            P(x,y) = \sum_{p=0}^1 \sum_{q=0}^1 \mathit{in}[\lfloor x \rfloor+p, \lfloor y \rfloor+q]w_p(x)w_q(y)
        \f]
    */
    VPI_INTERP_LINEAR = 2,

    /** Catmull-Rom cubic interpolation.
       Catmull-Rom interpolation weights with \f$A=-0.5\f$ are defined as follows:
      \f{eqnarray*}{
          w_0(t) &\triangleq& A(t+1)^3 &-& 5A(t+1)^2 &+& 8A(t+1) &-& 4A \\
          w_1(t) &\triangleq& (A+2)t^3 &-& (A+3)t^2 &\nonumber& &+& 1 \\
          w_2(t) &\triangleq& (A+2)(1-t)^3 &-& (A+3)(1-t)^2 &\nonumber& &+& 1 \\
          w_3(t) &\triangleq& \rlap{1 - w_0(t) - w_1(t) - w_2(t) }
      \f}

      Bicubic-interpolated value is given by the formula below:
      \f[
          P(x,y) = \sum_{p=-1}^2 \sum_{q=-1}^2 \mathit{in}[\lfloor x \rfloor+p, \lfloor y \rfloor+q]w_p(x)w_q(y)
      \f]
    */
    VPI_INTERP_CATMULL_ROM = 3,
} VPIInterpolationType;

#ifdef __cplusplus
}
#endif

#endif /* NV_VPI_INTERPOLATION_H */
