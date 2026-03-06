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
 * @file LensDistortionModels.h
 *
 * Declares functions to generate warp maps based on common lens distortion models.
 */

#ifndef NV_VPI_LENSDISTORTIONMODELS_H
#define NV_VPI_LENSDISTORTIONMODELS_H

#include "Export.h"
#include "Status.h"
#include "WarpMap.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup VPI_LDC Lens Distortion Correction
 * @ingroup VPI_API_Algorithms
 * @{
 */

/** Supported fisheye lens mapping types */
typedef enum
{
    /** Specifies the equidistant fisheye mapping.
     * Mapping is defined by:
     * \f[r = f\theta\f]
     * where:
     * - \f$\theta\f$ is the angle from the optical axis.
     * - \f$f\f$ is the focal length.
     * - \f$r\f$ is the distance of a pixel from the image center.
     */
    VPI_FISHEYE_EQUIDISTANT,
    /** Specifies the equisolid fisheye mapping.
     * Mapping is defined by:
     * \f[r = 2f\sin\left(\frac{\theta}{2}\right)\f]
     * where:
     * - \f$\theta\f$ is the angle from the optical axis.
     * - \f$f\f$ is the focal length.
     * - \f$r\f$ is the distance of a pixel from the image center.
     */
    VPI_FISHEYE_EQUISOLID,
    /** Specifies the orthographic fisheye mapping.
     * Mapping is defined by:
     * \f[r = f\sin(\theta)\f]
     * where:
     * - \f$\theta\f$ is the angle from the optical axis.
     * - \f$f\f$ is the focal length.
     * - \f$r\f$ is the distance of a pixel from the image center.
     */
    VPI_FISHEYE_ORTHOGRAPHIC,
    /** Specifies the stereographic fisheye mapping.
     * Mapping is defined by:
     * \f[r = 2f\tan\left(\frac{\theta}{2}\right)\f]
     * where:
     * - \f$\theta\f$ is the angle from the optical axis.
     * - \f$f\f$ is the focal length.
     * - \f$r\f$ is the distance of a pixel from the image center.
     */
    VPI_FISHEYE_STEREOGRAPHIC
} VPIFisheyeMapping;

/**
 * Holds coefficients for fisheye lens distortion model.
 *
 * The fisheye lens distortion model is defined by:
 *
 * \f{align*}{
 * r &= \sqrt{u^2 + v^2} \\
 * \theta &= \arctan(r) \\
 * r_d &= D(\theta + k_1 * \theta^3 + k_2 * \theta^5 + k_3 * \theta^7 + k_4 * \theta^9) \\
 * c_d &= r_d/r \\
 * u_d &= u*c_d \\
 * v_d &= v*c_d
 * \f}
 *
 * where:
 * - \f$k_1,k_2,k_3,k_4\f$ are the distortion coefficients.
 * - \f$(u_d,v_d)\f$ is the coordinate of a point in the distorted image.
 * - \f$(u,v)\f$ is the coordinate of a point in the corrected image.
 * - \f$D(\theta)\f$ defines the mapping from point angle and pixel
 *   distance to image center. See \ref VPIFisheyeMapping for details.
 */
typedef struct
{
    /** Mapping between pixel angle and pixel distance to image center.
     *    + Valid mapping values: 
     *      - \ref VPI_FISHEYE_EQUIDISTANT
     *      - \ref VPI_FISHEYE_EQUISOLID
     *      - \ref VPI_FISHEYE_ORTHOGRAPHIC
     *      - \ref VPI_FISHEYE_STEREOGRAPHIC
     */
    VPIFisheyeMapping mapping;

    /**@{ Fisheye distortion coefficients.
     */
    float k1, k2, k3, k4;
    /**@} */
} VPIFisheyeLensDistortionModel;

/** Generates a mapping that corrects image distortions caused by fisheye lenses.
 * Given camera intrinsic and extrinsic parameters, and fisheye lens distortion
 * model, generates a \ref VPIWarpMap suitable to be used by \ref
 * algo_remap to undistort images captured by cameras equipped with
 * fisheye lens.
 *
 * The mapping is defined by the following matrix equation:
 *
 * \f[
 * y = (K_{out} L X K_{in}) x
 * \f]
 *
 * where:
 * - \f$x\f$ is the input homogeneous coordinate
 * - \f$y\f$ is the corresponding homogeneous coordinate on the distorted image.
 * - \f$L\f$ is the fisheye lens distortion model (not a matrix), see \ref VPIFisheyeLensDistortionModel.
 * - remaining variables are the input parameters to this function.
 *
 * @param[in] Kin Camera intrinsic parameters.
 * 
 * @param[in] X Camera extrinsic parameters that defines the camera center position and its heading in world coordinates.
 * 
 * @param[in] Kout New camera intrinsic parameters applied to the undistorted image. 
 *                 For monocular cameras, Kout is usually equal to Kin.
 * 
 * @param[in] distModel Fisheye lens distortion model.
 * 
 * @param[out] warpMap Resulting warp map that serves as input to \ref algo_remap algorithm.
 * 
 * @retval #VPI_ERROR_INVALID_ARGUMENT \p warpMap is NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT \p Kin is NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT \p X is NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT Output \p Kout is NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT \p distModel is NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT Fisheye model in \p distModel is invalid.
 * @retval #VPI_SUCCESS                Operation executed successfully.
 */
VPI_PUBLIC VPIStatus vpiWarpMapGenerateFromFisheyeLensDistortionModel(const VPICameraIntrinsic Kin,
                                                                      const VPICameraExtrinsic X,
                                                                      const VPICameraIntrinsic Kout,
                                                                      const VPIFisheyeLensDistortionModel *distModel,
                                                                      VPIWarpMap *warpMap);

/**
 * Holds coefficients for polynomial lens distortion model.
 *
 * The polynomial lens distortion model is defined by:
 *
 * \f{align*}{
 * r &= \sqrt{u^2 + v^2} \\
 * k_r &= \frac{1 + k_1r^2 + k_2 r^4 + k_3 r^6}{1 + k_4 r^2 + k_5 r^4 + k_6 r^6} \\
 * u_d &= k_r u + p_1(2uv) + p_2 * (r^2 + 2u^2) \\
 * v_d &= k_r v + p_1(r^2 + 2v^2) + p_2(2uv)
 * \f}
 *
 * where:
 * - \f$k_1,k_2,k_3,k_4,k_5,k_6\f$ are the radial distortion coefficients.
 * - \f$p_1,p_2\f$ are the tangential distortion coefficients.
 * - \f$(u_d,v_d)\f$ is the coordinate of a point in the distorted image.
 * - \f$(u,v)\f$ is the coordinate of a point in the corrected image.
 */
typedef struct
{
    /**@{ Radial distortion coefficients. */
    float k1;
    float k2;
    float k3;
    float k4;
    float k5;
    float k6;
    /**@}*/

    /**@{ Tangential distortion coefficients. */
    float p1;
    float p2;
    /**@}*/
} VPIPolynomialLensDistortionModel;

/** Generates a mapping that corrects image using polynomial lens distortion model.
 * Given camera intrinsic and extrinsic parameters, and the polynomial lens
 * distortion model, generates a \ref VPIWarpMap suitable to be used by \ref
 * algo_remap to undistort images.
 *
 * The mapping is defined by the following matrix equation:
 *
 * \f[
 * y = (K_{out} L X K_{in}) x
 * \f]
 *
 * where:
 * - \f$x\f$ is the input homogeneous coordinate
 * - \f$y\f$ is the corresponding homogeneous coordinate on the distorted image.
 * - \f$L\f$ is the polynomial lens distortion model (not a matrix), see \ref VPIPolynomialLensDistortionModel
 * - remaining variables are the input parameters to this function.
 *
 * @param[in] Kin Camera intrinsic parameters.
 * 
 * @param[in] X Camera extrinsic parameters that defines the camera center position and its heading in world coordinates.
 * 
 * @param[in] Kout New camera intrinsic parameters applied to the undistorted image.
 *                 For monocular cameras, Kout is usually equal to Kin.
 * 
 * @param[in] distModel Polynomial lens distortion model.
 * 
 * @param[out] warpMap Resulting warp map that serves as input to \ref algo_remap algorithm.
 * 
 * @retval #VPI_ERROR_INVALID_ARGUMENT \p warpMap is NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT \p Kin is NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT \p X is NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT Output \p Kout is NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT \p distModel is NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT Polynomial model in \p distModel is invalid.
 * @retval #VPI_SUCCESS                Operation executed successfully.
 */
VPI_PUBLIC VPIStatus vpiWarpMapGenerateFromPolynomialLensDistortionModel(
    const VPICameraIntrinsic Kin, const VPICameraExtrinsic X, const VPICameraIntrinsic Kout,
    const VPIPolynomialLensDistortionModel *distModel, VPIWarpMap *warpMap);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* NV_VPI_LENSMODELS_H */
