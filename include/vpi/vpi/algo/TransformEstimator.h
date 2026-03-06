/*
 * Copyright 2023-2024 NVIDIA Corporation. All rights reserved.
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
 * @file TransformEstimator.h
 *
 * Declares functions that implement the Transform Estimator algorithm.
 */

#ifndef NV_VPI_ALGORITHMS_TRANSFORM_ESTIMATOR_H
#define NV_VPI_ALGORITHMS_TRANSFORM_ESTIMATOR_H

#include "../AlgoFlags.h"
#include "../Export.h"
#include "../Status.h"
#include "../Types.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup VPI_TransformEstimator Transform Estimator
 * @ingroup VPI_API_Algorithms
 * @{
 *
 * Estimate the transform between source and target keypoints.
 * Refer to \ref algo_xform_estim for more details and usage examples.
 */

/**
 * Creates payload for \ref vpiSubmitTransformEstimator
 *
 * @param[in] backends VPI backend that will execute the algorithm.
 *                     + Valid values:
 *                       - \ref VPI_BACKEND_CPU
 *                     + Backend must be enabled in current context.
 *
 * @param[in] maxKeypoints Maximum number of keypoints processed.
 *                                    + Must be > 0.
 *
 * @param[out] payload Pointer to the payload variable that receives the created handle.
 *
 * @retval #VPI_ERROR_INVALID_ARGUMENT  \p payload is NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT  \p maxKeypints outside valid range.
 * @retval #VPI_ERROR_INVALID_ARGUMENT  \p backend refers to an invalid backend.
 * @retval #VPI_ERROR_NOT_IMPLEMENTED   Transform Estimator algorithm is not supported by given backend.
 * @retval #VPI_ERROR_INVALID_CONTEXT   Current context was destroyed.
 * @retval #VPI_ERROR_INVALID_OPERATION Backend isn't enabled in current context.
 * @retval #VPI_ERROR_OUT_OF_MEMORY     Cannot allocate required resources.
 * @retval #VPI_SUCCESS                 Operation executed successfully.
 */
VPI_PUBLIC VPIStatus vpiCreateTransformEstimator(uint64_t backends, int32_t maxKeypoints, VPIPayload *payload);

/** Holds the configuration of a constrained 2d homography transform. */
typedef struct
{
    /** Valid rotation range, in radians.
     * If min==-FLT_MAX, do not impose restriction on minimum rotation.
     * If max==FLT_MAX, do not impose restriction on maximum rotation.
     * If min==max, no rotation will be estimated, it'll be considered to be min.
     * @{ */
    float minRotation, maxRotation;
    /* @} */

    /** Valid horizontal scale range.
     * If isIsotropicScale is != 0 (true), this range is for the isotropic scale.
     * If 0, don't restrict.
     * If min==max, no horizontal scaling will be estimated, it'll be considered to be min.
     * * If max != 0, min must be <= max.
     * @{ */
    float minXScale, maxXScale;
    /** @} */

    /** Valid vertical scale range.
     * Only used if isIsotropicScale is 0 (false).
     * If 0, don't restrict.
     * If min==max, no vertical scaling will be estimated, it'll be considered to be min.
     * * If max != 0, min must be <= max.
     * @{ */
    float minYScale, maxYScale;
    /** @} */

    /** Valid horizontal translation range.
     * If min==-FLT_MAX, do not impose restriction on minimum horizontal translation.
     * If max==FLT_MAX, do not impose restriction on maximum  horizontal translation.
     * If min==max, no horizontal translation will be estimated, it'll be considered to be min.
     * * min must be <= max.
     * @{ */
    float minXTranslation, maxXTranslation;
    /** @} */

    /** Valid vertical translation range.
     * If min==-FLT_MAX, do not impose restriction on minimum vertical translation.
     * If max==FLT_MAX, do not impose restriction on maximum vertical translation.
     * If min==max, no vertical translation will be estimated, it'll be considered to be min.
     * * min must be <= max.
     * @{ */
    float minYTranslation, maxYTranslation;
    /** @} */

    /** Valid shearing range.
     * If min==-FLT_MAX, do not impose restriction on minimum shear.
     * If max==FLT_MAX, do not impose restriction on maximum shear.
     * If min==max, no shear will be estimated, it'll be considered to be min.
     * * min must be <= max.
     * @{ */
    float minShear, maxShear;
    /** @} */

    /** Whether to restrict to affine transforms.
     * If != 0, restrict transform to affine.
     * If 0, allow perspective transform. */
    int8_t isAffine;

    /** Whether scaling is uniform in all directions (isotropic) or not.
     * If != 0, assume scale is isotropic.
     * If 0, allow different horizontal and vertical scaling. */
    int8_t isIsotropicScale;
} VPIConstrainedHomography2DConfig;

/** Types of transformation supported. */
typedef enum
{
    /** Represents a constrained 2D homography transform. */
    VPI_XFORM_CONSTRAINED_HOMOGRAPHY_2D
} VPITransformType;

/** Transformation parameters. */
typedef union
{
    VPIConstrainedHomography2DConfig constrainedHomography2D;
} VPITransformConfig;

/** Transform estimation method */
typedef enum
{
    /** Use least squares minimization method. */
    VPI_XFORM_ESTIM_METHOD_REGULAR,
    /** Use robust estimation with RANSAC and least squares. */
    VPI_XFORM_ESTIM_METHOD_RANSAC,
} VPITransfomEstimationMethod;

/** Parameters used to tune Transform Estimator algorithm. */
typedef struct
{
    /** Method to use for transform estimation. */
    VPITransfomEstimationMethod method;

    /** Maximum number of iterations the non-linear solver should take. */
    int32_t solverMaxIterations;

    /** Maximum number of RANSAC iterations. */
    int32_t ransacMaxIterations;
    /** Maximum allowed reprojection error for a point to be treated as an inlier. */
    float ransacReprojErrorTolerance;
    /** Maximum allowed reprojection error for a point to be treated as an inlier. */
    float ransacConfidenceLevel;
    /** Random number seed to be used. */
    int32_t ransacSeed;
    /** Maximum number of iterations for model refinement step.
     *  Pass 0 to skip refinement. It's usually needed when transform isn't affine. */
    int32_t maxRefinementIterations;

    /** Type of transform to estimate */
    VPITransformType xftype;
    /** Transform configuration, such as trusted boundaries, etc. */
    VPITransformConfig xfcfg;
} VPITransformEstimatorParams;

/**
 * Initializes the VPITransformEstimatorParams with default values for a given transform type.
 *
 * The default values remove all parameter constraints.
 * It sets the following parameters:
 * - method: \ref VPI_XFORM_ESTIM_METHOD_RANSAC
 * - ransacMaxIterations: 2000
 * - ransacReprojErrorTolerance: 3
 * - ransacConfidenceLevel: 0.95
 * - ransacSeed: 0
 * - solverMaxIterations: 50
 * - solverReprojErrorTolerance: 1e-6
 * - solverParameterTolerance: 1e-8
 * - maxRefinementIterations: 10
 * 
 * - xftype: (type passed)
 * If xftype == VPI_XFORM_CONSTRAINED_HOMOGRAPHY_2D:
 * - xfcfg.constrainedHomography2D.minRotation: -FLT_MAX
 * - xfcfg.constrainedHomography2D.maxRotation: +FLT_MAX
 * - xfcfg.constrainedHomography2D.minXScale: 0
 * - xfcfg.constrainedHomography2D.maxXScale: 0
 * - xfcfg.constrainedHomography2D.minYScale: 0
 * - xfcfg.constrainedHomography2D.maxYScale: 0
 * - xfcfg.constrainedHomography2D.minXTranslation: -FLT_MAX
 * - xfcfg.constrainedHomography2D.maxXTranslation: FLT_MAX
 * - xfcfg.constrainedHomography2D.minYTranslation: -FLT_MAX
 * - xfcfg.constrainedHomography2D.maxYTranslation: FLT_MAX
 * - xfcfg.constrainedHomography2D.isAffine: 0
 * - xfcfg.constrainedHomography2D.isIsotropicScale: 0
 *
 * @param[in] type Type of transform to initialize the structure to.
 *                 * Accepted types:
 *                   - \ref VPI_XFORM_CONSTRAINED_HOMOGRAPHY_2D
 * @param[in,out] params Pointer to structure to be initialized.
 *                       + Must not be NULL.
 *
 * @retval #VPI_ERROR_INVALID_ARGUMENT      \p params is NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT      Transform type not accepted.
 * @retval #VPI_SUCCESS                     Operation executed successfully.
 */
VPI_PUBLIC VPIStatus vpiInitTransformEstimatorParams(VPITransformType type, VPITransformEstimatorParams *params);

/**
 * Submits a \ref algo_xform_estim "Transform Estimator" operation to the stream.
 *
 * @param[in] stream The stream where the operation will be queued in.
 *                   + Must not be NULL.
 *                   + Stream must have enabled the backends that will execute the algorithm.
 *
 * @param[in] backend Backend that will execute the algorithm.
 *                    + Must be the backend specified during payload creation or 0 as a shorthand to use this backend.
 *
 * @param[in] payload Payload created with \ref vpiCreateTransformEstimator.
 *                    + Must not be NULL.
 *
 * @param[in] srcKeypoints Source keypoints.
 *                         It defines a set of keypoints to which the estimated
 *                         transform is to be applied to.
 *                         * Must not be NULL.
 *                         * Valid array types:
 *                           - \ref VPI_ARRAY_TYPE_KEYPOINT_F32
 *                           - \ref VPI_ARRAY_TYPE_PYRAMIDAL_KEYPOINT_F32
 *                         * If \p matches is not NULL, \p srcKeypoints and \p matches must have the same size.
 *
 * @param[in] tgtKeypoints Target keypoints.
 *                         It defines a set of keypoints that are to be the
 *                         result of applying the transform to the source
 *                         keypoints.
 *                         * Must not be NULL.
 *                         * Valid array types:
 *                           - \ref VPI_ARRAY_TYPE_KEYPOINT_F32
 *                           - \ref VPI_ARRAY_TYPE_PYRAMIDAL_KEYPOINT_F32
 *                         * If \p matches is NULL, \p srcKeypoints and \p tgtKeypoints must have the same size.
 *                         * If \p matches is not NULL, it must be guaranteed that the first element of
 *                           \ref VPIMatches::refIndex exists in \p tgtKeypoints.
 *
 * @param[in] matches Correspondence between source and target keypoints.
 *                    If not NULL, for each i-th element in matches, the correspondence is given by:
 *                    - src = srcKeypoints[i]
 *                    - tgt = tgtKeypoints[matches[i].refIndex[0]]
 *                    If it's NULL, for each i-th element in srcKeypoints, the correspondence is given by:
 *                    - src = srcKeypoints[i]
 *                    - tgt = tgtKeypoints[i]
 *
 *                    * If not NULL, it must have type \ref VPI_ARRAY_TYPE_MATCHES.
 *
 * @param[out] outTransform Where the estimated transformation will be written to.
 *                          Array's type specify what transform is to be estimated.
 *                          * Must not be NULL.
 *                          * Capacity must be >= 1.
 *                          * Array type must be \ref VPI_ARRAY_TYPE_HOMOGRAPHY_TRANSFORM_2D.
 *
 * @param[out] outInliers Indices of the inlier matches.
 *                        Optional, if not needed pass NULL.
 *                        * Capacity must be >= number of keypoints in srcKeypoints.
 *                        * The array type must be \ref VPI_ARRAY_TYPE_U32
 *
 * @param[in] params Parameters specifying the parameters of the transform to be estimated.
 *                   If NULL, will perform an unconstrained estimation for the transform given by the
 *                   output transform array type.
 *                   If not NULL, the following restrictions apply:
 *                   * Transform type must be consistent with output transform array type:
 *                     | Transform Type                           | Output transform array type                 |
 *                     |------------------------------------------|---------------------------------------------|
 *                     | \ref VPI_XFORM_CONSTRAINED_HOMOGRAPHY_2D | \ref VPI_ARRAY_TYPE_HOMOGRAPHY_TRANSFORM_2D |
 *
 * @retval #VPI_ERROR_INVALID_ARGUMENT      \p stream is NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT      \p payload is NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT      \p srcKeypoints or \p tgtKeypoints are NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT      \p outTransform is NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT      \p backend is invalid or unknown.
 * @retval #VPI_ERROR_INVALID_ARGUMENT      \p srcKeypoints, \p tgtKeypoints aren't consistent.
 * @retval #VPI_ERROR_INVALID_ARGUMENT      Transform type not consistent with output transform array type.
 * @retval #VPI_ERROR_INVALID_ARRAY_TYPE    Array type of \p srcKeypoints, \p tgtKeypoints,  \p matches or \p outInliers is not correct.
 * @retval #VPI_ERROR_NOT_IMPLEMENTED       Transform Estimator algorithm is not supported by given backend.
 * @retval #VPI_ERROR_INVALID_OPERATION     Backend hardware not available.
 * @retval #VPI_ERROR_INVALID_OPERATION     The needed backends aren't enabled in \p stream, \p srcKeypoints, \p tgtKeypoints or \p matches.
 * @retval #VPI_SUCCESS                     Operation executed successfully.
 */
VPI_PUBLIC VPIStatus vpiSubmitTransformEstimator(VPIStream stream, uint64_t backend, VPIPayload payload,
                                                 VPIArray srcKeypoints, VPIArray tgtKeypoints, VPIArray matches,
                                                 VPIArray outTransform, VPIArray outInliers,
                                                 const VPITransformEstimatorParams *params);

/** @} end of VPI_TransformEstimator */

#ifdef __cplusplus
}
#endif

#endif /* NV_VPI_ALGORITHMS_TRANSFORM_ESTIMATOR_H */
