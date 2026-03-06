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
 * @file ConvertImageFormat.h
 *
 * Declares functions that handle image format conversion.
 */

#ifndef NV_VPI_ALGORITHMS_CONVERT_IMAGE_FORMAT_H
#define NV_VPI_ALGORITHMS_CONVERT_IMAGE_FORMAT_H

#include "../AlgoFlags.h"
#include "../Export.h"
#include "../Status.h"
#include "../Types.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup VPI_ConvertImageFormat Convert Image Format
 * @ingroup VPI_API_Algorithms
 * @{
 *
 * Converts the image contents to the desired format, with optional pixel value scaling and offset.
 * Refer to \ref algo_imageconv for more details and usage examples.
 */

/** Parameters for customizing image format conversion.
 * These parameters are used to customize how the conversion will be made.
 * Make sure to call \ref vpiInitConvertImageFormatParams to initialize this
 * structure before updating its attributes. This guarantees that new attributes
 * added in future versions will have a suitable default value assigned.
 */
typedef struct
{
    /** Conversion policy to be used.
     *  + VIC backend only supports \ref VPI_CONVERSION_CLAMP. */
    VPIConversionPolicy policy;

    /** Scaling factor.
     *  Pass 1 for no scaling. 
     *  + VIC backend doesn't support scaling, it must be 1.*/
    float scale;

    /** Offset factor. 
     *  Pass 0 for no offset.
     *  + VIC backend doesn't support offset, it must be 0. */
    float offset;

    /** Control flags.
     *  + Valid values:
     *    - 0: default, negation of all other flags.
     *    - \ref VPI_PRECISE : precise, potentially slower implementation.
     */
    uint64_t flags;

    /** Interpolation to use for chroma upsampling.
     * + Valid values:
     *   | Interpolation type          | CPU | CUDA | VIC |
     *   |-----------------------------|:---:|:----:|:---:|
     *   | \ref VPI_INTERP_NEAREST     |  *  |   *  |  1  |
     *   | \ref VPI_INTERP_LINEAR      |     |      |  *  |
     *   | \ref VPI_INTERP_CATMULL_ROM |     |      |  *  |
     *   (1) Nearest-neighbor interpolation is currently accepted,
     *   but linear is performed instead.
     */
    VPIInterpolationType chromaUpFilter;

    /** Interpolation to use for chroma downsampling.
     * + Valid values:
     *   | Interpolation type          | CPU | CUDA | VIC |
     *   |-----------------------------|:---:|:----:|:---:|
     *   | \ref VPI_INTERP_NEAREST     |  *  |   *  |  *  |
     *   | \ref VPI_INTERP_LINEAR      |     |      |     |
     *   | \ref VPI_INTERP_CATMULL_ROM |     |      |     |
     */
    VPIInterpolationType chromaDownFilter;
} VPIConvertImageFormatParams;

/** Initialize \ref VPIConvertImageFormatParams with default values.
 *
 *  The following parameters are set:
 * - policy: \ref VPI_CONVERSION_CLAMP
 * - scale:  1
 * - offset: 0
 * - flags:  0
 * - chromaUpFilter:   \ref VPI_INTERP_NEAREST
 * - chromaDownFilter: \ref VPI_INTERP_NEAREST
 *
 * @param[out] params Pointer to structure to be filled. Must not be NULL.
 * 
 * @retval #VPI_ERROR_INVALID_ARGUMENT \p params is NULL.
 * @retval #VPI_SUCCESS Operation executed successfully.
 */
VPI_PUBLIC VPIStatus vpiInitConvertImageFormatParams(VPIConvertImageFormatParams *params);

/**
 * Converts the image contents to the desired format, with optional scaling and offset.
 * The input and output formats are inferred from the corresponding images passed as parameters.
 * When scaling and offset are fractional, input is converted to 32-bit floating point prior conversion takes place.
 * The formula that relates input and output pixels is:
 *
 * \f[
 *    out(x,y) = clamp_{[\mathsf{min_{out}},\mathsf{max_{out}}]}(in(x,y)*\alpha + \beta)
 * \f]
 *
 * where:
 * - \f$\mathsf{min_{out}}\f$ and \f$\mathsf{max_{out}}\f$ are the minimum and maximum
 *   representable value by output image format. Exception is for floating point types,
 *   where limits are \f$[-\infty,\infty]\f$, i.e., no clamping is done.
 * - \f$\alpha\f$ is the scaling.
 * - \f$\beta\f$ is the offset.
 *
 * Floating point to integer conversion does returns the nearest integer number, rounding halfway cases away from zero.
 *
 * Supported format conversions.
 *  + Conversion between image format's marked with an '*' aren't implemented.
 *  + On CPU and CUDA, any pitch-linear format can be converted to itself,
 *    provided that scale is 1 and offset is 0.
 *  + Formats within each table below can be converted between themselves.
 *
 *   |          CPU and CUDA             |                VIC                 |
 *   |-----------------------------------|------------------------------------|
 *   | \ref VPI_IMAGE_FORMAT_U8          | \ref VPI_IMAGE_FORMAT_Y8           |
 *   | \ref VPI_IMAGE_FORMAT_U16         | \ref VPI_IMAGE_FORMAT_Y8_BL        |
 *   | \ref VPI_IMAGE_FORMAT_S8          | \ref VPI_IMAGE_FORMAT_Y8_BL16      |
 *   | \ref VPI_IMAGE_FORMAT_S16         | \ref VPI_IMAGE_FORMAT_Y8_ER        |
 *   | \ref VPI_IMAGE_FORMAT_Y8_ER       | \ref VPI_IMAGE_FORMAT_Y8_ER_BL     |    
 *   | \ref VPI_IMAGE_FORMAT_Y16_ER      | \ref VPI_IMAGE_FORMAT_Y8_ER_BL16   |
 *   | \ref VPI_IMAGE_FORMAT_NV12_ER*    | \ref VPI_IMAGE_FORMAT_NV12         |
 *   | \ref VPI_IMAGE_FORMAT_NV24_ER*    | \ref VPI_IMAGE_FORMAT_NV12_BL      |
 *   | \ref VPI_IMAGE_FORMAT_RGB8        | \ref VPI_IMAGE_FORMAT_NV12_BL16    |
 *   | \ref VPI_IMAGE_FORMAT_BGR8        | \ref VPI_IMAGE_FORMAT_NV12_ER      |
 *   | \ref VPI_IMAGE_FORMAT_RGBA8       | \ref VPI_IMAGE_FORMAT_NV12_ER_BL   |
 *   | \ref VPI_IMAGE_FORMAT_BGRA8       | \ref VPI_IMAGE_FORMAT_NV12_ER_BL16 |
 *   | \ref VPI_IMAGE_FORMAT_F32         | \ref VPI_IMAGE_FORMAT_NV24         |
 *   |                                   | \ref VPI_IMAGE_FORMAT_NV24_BL      |
 *   |                                   | \ref VPI_IMAGE_FORMAT_NV24_BL16    |
 *   |                                   | \ref VPI_IMAGE_FORMAT_NV24_ER      |
 *   |                                   | \ref VPI_IMAGE_FORMAT_NV24_ER_BL   |
 *   |                                   | \ref VPI_IMAGE_FORMAT_NV24_ER_BL16 | 
 *   |                                   | \ref VPI_IMAGE_FORMAT_YUYV         |
 *   |                                   | \ref VPI_IMAGE_FORMAT_YUYV_BL      |
 *   |                                   | \ref VPI_IMAGE_FORMAT_YUYV_BL16    |
 *   |                                   | \ref VPI_IMAGE_FORMAT_YUYV_ER      |
 *   |                                   | \ref VPI_IMAGE_FORMAT_YUYV_ER_BL   |
 *   |                                   | \ref VPI_IMAGE_FORMAT_YUYV_ER_BL16 |
 *   |                                   | \ref VPI_IMAGE_FORMAT_UYVY         |
 *   |                                   | \ref VPI_IMAGE_FORMAT_UYVY_BL      |
 *   |                                   | \ref VPI_IMAGE_FORMAT_UYVY_BL16    |
 *   |                                   | \ref VPI_IMAGE_FORMAT_UYVY_ER      |
 *   |                                   | \ref VPI_IMAGE_FORMAT_UYVY_ER_BL   |
 *   |                                   | \ref VPI_IMAGE_FORMAT_UYVY_ER_BL16 |
 *   |                                   | \ref VPI_IMAGE_FORMAT_RGBA8        |  
 *   |                                   | \ref VPI_IMAGE_FORMAT_BGRA8        | 
 *
 *
 *   |          CPU and CUDA             |                VIC                 |
 *   |-----------------------------------|------------------------------------|
 *   | \ref VPI_IMAGE_FORMAT_2F32        | \ref VPI_IMAGE_FORMAT_Y16          |
 *   |                                   | \ref VPI_IMAGE_FORMAT_Y16_BL       |
 *   |                                   | \ref VPI_IMAGE_FORMAT_Y16_BL16     |
 *
 *   |                VIC                |
 *   |-----------------------------------|
 *   | \ref VPI_IMAGE_FORMAT_Y16_ER      |
 *   | \ref VPI_IMAGE_FORMAT_Y16_ER_BL   |
 *   | \ref VPI_IMAGE_FORMAT_Y16_ER_BL16 |
 *
 *   |                VIC                |
 *   |-----------------------------------|
 *   | \ref VPI_IMAGE_FORMAT_2S16        |
 *   | \ref VPI_IMAGE_FORMAT_2S16_BL     |
 *   | \ref VPI_IMAGE_FORMAT_2S16_BL16   |
 *
 *  + Conversions from left format to right and vice-versa are supported.
 *   |                           CPU and CUDA                                 |
 *   |-----------------------------------|------------------------------------|
 *   | \ref VPI_IMAGE_FORMAT_Y8          | \ref VPI_IMAGE_FORMAT_S8           |
 *   | \ref VPI_IMAGE_FORMAT_Y16         | \ref VPI_IMAGE_FORMAT_S16          |
 *   |                                   | \ref VPI_IMAGE_FORMAT_U8           |
 *   |                                   | \ref VPI_IMAGE_FORMAT_U16          |
 *   |                                   | \ref VPI_IMAGE_FORMAT_F32          |
 *
 *  + Conversions from left format to right are supported.
 *   |                                 CUDA                                   |
 *   |-----------------------------------|------------------------------------|
 *   | \ref VPI_IMAGE_FORMAT_RGB8        | \ref VPI_IMAGE_FORMAT_Y8_ER_BL     |
 *   | \ref VPI_IMAGE_FORMAT_BGR8        | \ref VPI_IMAGE_FORMAT_Y8_ER_BL16   |
 *   | \ref VPI_IMAGE_FORMAT_RGBA8       | \ref VPI_IMAGE_FORMAT_Y16_ER_BL    |
 *   | \ref VPI_IMAGE_FORMAT_BGRA8       | \ref VPI_IMAGE_FORMAT_Y16_ER_BL16  |
 *
 * Conversion among format with extended or studio ranges and non-color formats
 * (i.e. U8, F32, S16, ...) will keep range unchanged, except for clamping or
 * casting specified in \ref VPIConvertImageFormatParams::policy.
 *
 * @param[in] stream A stream handle where the operation will be queued into.
 *                   + Must not be NULL.
 *                   + Stream must have enabled the backends that will execute the algorithm.
 *
 * @param[in] backend Backend that will execute the algorithm.
 *                    + Valid values:
 *                      - \ref VPI_BACKEND_CPU
 *                      - \ref VPI_BACKEND_CUDA
 *                      - \ref VPI_BACKEND_VIC
 *
 * @param[in] input Input image to be converted.
 *                   + Must not be NULL.
 *                   + Image must have enabled the backends that will execute the algorithm.
 *                   + On VIC, when converting from format with 4:2:2(R) chroma subsampling to
 *                     a format isn't YUV or chroma-subsampling is not 4:4:4, the image width
 *                     must be even (no restriction on height).
 *
 * @param[out] output Output image where result will be written to, with the desired format.
 *                    + Must not be NULL.
 *                    + Must have same dimensions as input image.
 *                    + Image must have enabled the backends that will execute the algorithm.
 *                    + On VIC, When converting to format with 4:2:0 or 4:2:2(R) chroma subsampling
 *                      (i.e. YUYV or NV12), the image width and height must be even.
 *
 * @param[in] params Parameters to fine tune the conversion.
 *                   Pass NULL to use default values given by \ref vpiInitConvertImageFormatParams. 
 *
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p stream is NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p input or \p output are NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     Invalid flags in \p params.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     Invalid conversion type in \p params.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p input and \p output image dimensions do not match.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p output format can't be used with given image dimensions.
 * @retval #VPI_ERROR_INVALID_IMAGE_FORMAT Format conversion not supported.
 * @retval #VPI_ERROR_NOT_IMPLEMENTED      Convert Image Format algorithm is not supported by given backend.
 * @retval #VPI_ERROR_INVALID_OPERATION    Hardware backend not available.
 * @retval #VPI_ERROR_INVALID_OPERATION    The needed backends aren't enabled in \p stream, \p input or \p output.
 * @retval #VPI_SUCCESS                    Operation executed successfully.
 */
VPI_PUBLIC VPIStatus vpiSubmitConvertImageFormat(VPIStream stream, uint64_t backend, VPIImage input, VPIImage output,
                                                 const VPIConvertImageFormatParams *params);

/**
 * Converts the pyramid contents to the desired format, with optional scaling and offset.
 * The input and output formats are inferred from the corresponding pyramids passed as parameters.
 * When scaling and offset are fractional, input is converted to 32-bit floating point prior conversion takes place.
 * The formula that relates input and output pixels is:
 *
 * \f[
 *    out(x,y) = clamp_{[\mathsf{min_{out}},\mathsf{max_{out}}]}(in(x,y)*\alpha + \beta)
 * \f]
 *
 * where:
 * - \f$\mathsf{min_{out}}\f$ and \f$\mathsf{max_{out}}\f$ are the minimum and maximum
 *   representable value by output image format. Exception is for floating point types,
 *   where limits are \f$[-\infty,\infty]\f$, i.e., no clamping is done.
 * - \f$\alpha\f$ is the scaling.
 * - \f$\beta\f$ is the offset.
 *
 * Floating point to integer conversion does returns the nearest integer number, rounding halfway cases away from zero.
 *
 * Supported format conversions.
 *  + Conversion between image format's marked with an '*' aren't implemented.
 *  + On CPU and CUDA, any pitch-linear format can be converted to itself,
 *    provided that scale is 1 and offset is 0.
 *  + Formats within each table below can be converted between themselves.
 *
 *   |          CPU and CUDA             |                  VIC               |
 *   |-----------------------------------|------------------------------------|
 *   | \ref VPI_IMAGE_FORMAT_U8          | \ref VPI_IMAGE_FORMAT_Y8           |
 *   | \ref VPI_IMAGE_FORMAT_U16         | \ref VPI_IMAGE_FORMAT_Y8_BL        |
 *   | \ref VPI_IMAGE_FORMAT_S8          | \ref VPI_IMAGE_FORMAT_Y8_BL16      |
 *   | \ref VPI_IMAGE_FORMAT_S16         | \ref VPI_IMAGE_FORMAT_Y8_ER        |
 *   | \ref VPI_IMAGE_FORMAT_Y8_ER       | \ref VPI_IMAGE_FORMAT_Y8_ER_BL     |   
 *   | \ref VPI_IMAGE_FORMAT_Y16_ER      | \ref VPI_IMAGE_FORMAT_Y8_ER_BL16   |
 *   | \ref VPI_IMAGE_FORMAT_NV12_ER*    | \ref VPI_IMAGE_FORMAT_NV12         |
 *   | \ref VPI_IMAGE_FORMAT_NV24_ER*    | \ref VPI_IMAGE_FORMAT_NV12_BL      |
 *   | \ref VPI_IMAGE_FORMAT_RGB8        | \ref VPI_IMAGE_FORMAT_NV12_BL16    |
 *   | \ref VPI_IMAGE_FORMAT_BGR8        | \ref VPI_IMAGE_FORMAT_NV12_ER      |
 *   | \ref VPI_IMAGE_FORMAT_RGBA8       | \ref VPI_IMAGE_FORMAT_NV12_ER_BL   |
 *   | \ref VPI_IMAGE_FORMAT_BGRA8       | \ref VPI_IMAGE_FORMAT_NV12_ER_BL16 |
 *   | \ref VPI_IMAGE_FORMAT_F32         | \ref VPI_IMAGE_FORMAT_NV24         |
 *   |                                   | \ref VPI_IMAGE_FORMAT_NV24_BL      |
 *   |                                   | \ref VPI_IMAGE_FORMAT_NV24_BL16    |
 *   |                                   | \ref VPI_IMAGE_FORMAT_NV24_ER      |
 *   |                                   | \ref VPI_IMAGE_FORMAT_NV24_ER_BL   |
 *   |                                   | \ref VPI_IMAGE_FORMAT_NV24_ER_BL16 | 
 *   |                                   | \ref VPI_IMAGE_FORMAT_YUYV         |
 *   |                                   | \ref VPI_IMAGE_FORMAT_YUYV_BL      |
 *   |                                   | \ref VPI_IMAGE_FORMAT_YUYV_BL16    |
 *   |                                   | \ref VPI_IMAGE_FORMAT_YUYV_ER      |
 *   |                                   | \ref VPI_IMAGE_FORMAT_YUYV_ER_BL   |
 *   |                                   | \ref VPI_IMAGE_FORMAT_YUYV_ER_BL16 |
 *   |                                   | \ref VPI_IMAGE_FORMAT_UYVY         |
 *   |                                   | \ref VPI_IMAGE_FORMAT_UYVY_BL      |
 *   |                                   | \ref VPI_IMAGE_FORMAT_UYVY_BL16    |
 *   |                                   | \ref VPI_IMAGE_FORMAT_UYVY_ER      |
 *   |                                   | \ref VPI_IMAGE_FORMAT_UYVY_ER_BL   |
 *   |                                   | \ref VPI_IMAGE_FORMAT_UYVY_ER_BL16 |
 *   |                                   | \ref VPI_IMAGE_FORMAT_RGBA8        |  
 *   |                                   | \ref VPI_IMAGE_FORMAT_BGRA8        | 
 *
 *   |          CPU and CUDA             |                VIC                 |
 *   |-----------------------------------|------------------------------------|
 *   | \ref VPI_IMAGE_FORMAT_2F32        | \ref VPI_IMAGE_FORMAT_Y16          |
 *   |                                   | \ref VPI_IMAGE_FORMAT_Y16_BL       |
 *   |                                   | \ref VPI_IMAGE_FORMAT_Y16_BL16     |
 *
 *   |                VIC                |
 *   |-----------------------------------|
 *   | \ref VPI_IMAGE_FORMAT_Y16_ER      |
 *   | \ref VPI_IMAGE_FORMAT_Y16_ER_BL   |
 *   | \ref VPI_IMAGE_FORMAT_Y16_ER_BL16 |
 *
 *   |                VIC                |
 *   |-----------------------------------|
 *   | \ref VPI_IMAGE_FORMAT_S16         |
 *   | \ref VPI_IMAGE_FORMAT_S16_BL      |
 *   | \ref VPI_IMAGE_FORMAT_S16_BL16    |
 *
 *  + Conversions from left format to right and vice-versa are supported.
 *   |                           CPU and CUDA                                 |
 *   |-----------------------------------|------------------------------------|
 *   | \ref VPI_IMAGE_FORMAT_Y8          | \ref VPI_IMAGE_FORMAT_S8           |
 *   | \ref VPI_IMAGE_FORMAT_Y16         | \ref VPI_IMAGE_FORMAT_S16          |
 *   |                                   | \ref VPI_IMAGE_FORMAT_U8           |
 *   |                                   | \ref VPI_IMAGE_FORMAT_U16          |
 *   |                                   | \ref VPI_IMAGE_FORMAT_F32          |
 *
 * Conversion among format with extended or studio ranges and non-color formats
 * (i.e. U8, F32, S16, ...) will keep range unchanged, except for clamping or
 * casting specified in \ref VPIConvertImageFormatParams::policy.
 *
 * @param[in] stream A stream handle where the operation will be queued into.
 *                   + Must not be NULL.
 *                   + Stream must have enabled the backends that will execute the algorithm.
 *
 * @param[in] backend Backend that will execute the algorithm.
 *                    + Valid values:
 *                      - \ref VPI_BACKEND_CPU
 *                      - \ref VPI_BACKEND_CUDA
 *                      - \ref VPI_BACKEND_VIC
 *
 * @param[in] input Input pyramid to be converted.
 *                   + Must not be NULL.
 *                   + Image must have enabled the backends that will execute the algorithm.
 *                   + On VIC, when converting from format with 4:2:2(R) chroma subsampling to
 *                     a format isn't YUV or chroma-subsampling is not 4:4:4, the pyramid width in all levels
 *                     must be even (no restriction on height).
 *
 * @param[out] output Output pyramid where result will be written to, with the desired format.
 *                    + Must not be NULL.
 *                    + Must have same dimensions as input pyramid.
 *                    + Image must have enabled the backends that will execute the algorithm.
 *                    + On VIC, When converting to format with 4:2:0 or 4:2:2(R) chroma subsampling
 *                      (i.e. YUYV or NV12), the pyramid width and height in all levels must be even.
 *
 * @param[in] params Parameters to fine tune the conversion.
 *                   Pass NULL to use default values given by \ref vpiInitConvertImageFormatParams. 
 *
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p stream is NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p input or \p output are NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     Invalid flags in \p params.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     Invalid conversion type in \p params.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p input and \p output pyramid dimensions or number of levels do not match.
 * @retval #VPI_ERROR_INVALID_ARGUMENT     \p output format can't be used with given pyramid dimensions.
 * @retval #VPI_ERROR_INVALID_IMAGE_FORMAT Format conversion not supported.
 * @retval #VPI_ERROR_NOT_IMPLEMENTED      Convert Image Format algorithm is not supported by given backend.
 * @retval #VPI_ERROR_INVALID_OPERATION    Hardware backend not available.
 * @retval #VPI_ERROR_INVALID_OPERATION    The needed backends aren't enabled in \p stream, \p input or \p output.
 * @retval #VPI_SUCCESS                    Operation executed successfully.
 */
VPI_PUBLIC VPIStatus vpiSubmitConvertImageFormatPyramid(VPIStream stream, uint64_t backend, VPIPyramid input,
                                                        VPIPyramid output, const VPIConvertImageFormatParams *params);

/** @} end of VPI_ConvertImageFormat */

#ifdef __cplusplus
}
#endif

#endif /* NV_VPI_ALGORITHMS_CONVERT_IMAGE_FORMAT_H */
