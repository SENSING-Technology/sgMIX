/*
 * Copyright 2019-2023 NVIDIA Corporation. All rights reserved.
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
 * @file VersionDef.h
 *
 * Macros for handling NV_VPI library version.
 */

#ifndef NV_VPI_VERSIONDEF_H
#define NV_VPI_VERSIONDEF_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Declarations of entities to handle NV_VPI versioning.
 *
 * These utilities allow querying the NV_VPI header and library versions and
 * properly handle NV_VPI forward- or backward-compatibility .
 *
 * @defgroup NV_VPI_Version Versioning
 * @ingroup NV_VPI_API_Misc
 * @{
 */

/** Make a NV_VPI version identifier with four components.
 * @param[in] major,minor,patch,tweak Version components to be converted to a number.
 * @returns The numeric version representation.
 */
#define NV_VPI_MAKE_VERSION4(major, minor, patch, tweak) \
    ((major)*1000000U + (minor)*10000U + (patch)*100U + (tweak))

/** Make a NV_VPI version identifier with three components.
 *
 * The tweak version component is considered to be 0.
 *
 * @param[in] major,minor,patch Version components to be converted to a number.
 * @returns The numeric version representation.
 */
#define NV_VPI_MAKE_VERSION3(major, minor, patch) \
    NV_VPI_MAKE_VERSION4(major, minor, patch, 0)

/** Make a NV_VPI version identifier with two components.
 *
 * The patch and tweak version components are considered to be 0.
 *
 * @param[in] major,minor Version components to be converted to a number.
 * @returns The numeric version representation.
 */
#define NV_VPI_MAKE_VERSION2(major, minor) \
    NV_VPI_MAKE_VERSION4(major, minor, 0, 0)

/** Make a NV_VPI version identifier with one component.
 *
 * The minor, patch and tweak version components are considered to be 0.
 *
 * @param[in] major Major version component to be converted to a number.
 * @returns The numeric version representation.
 */
#define NV_VPI_MAKE_VERSION1(major) \
    NV_VPI_MAKE_VERSION4(major, 0, 0, 0)

/** Assemble an integer version from its components.
 * This makes it easy to conditionally compile code for different NV_VPI versions, e.g:
 * \code
 * #if NV_VPI_VERSION < NV_VPI_MAKE_VERSION(1,0,0)
 *    // code that runs on versions prior 1.0.0
 * #else
 *    // code that runs on versions after that, including 1.0.0
 * #endif
 * \endcode
 *
 * @param[in] major Major version component, mandatory.
 * @param[in] minor Minor version component. If ommitted, it's considered to be 0.
 * @param[in] patch Patch version component. If ommitted, it's considered to be 0.
 * @param[in] tweak Tweak version component. If ommitted, it's considered to be 0.
 * @returns The numeric version representation.
 */
#if VPI_DOXYGEN
#   define NV_VPI_MAKE_VERSION(major,minor,patch,tweak)
#else
#define NV_VPI_DETAIL_GET_MACRO(_1,_2,_3,_4,NAME,...) NAME
#define NV_VPI_MAKE_VERSION(...) \
    NV_VPI_DETAIL_GET_MACRO(__VA_ARGS__, NV_VPI_MAKE_VERSION4, NV_VPI_MAKE_VERSION3, NV_VPI_MAKE_VERSION2, NV_VPI_MAKE_VERSION1)(__VA_ARGS__)
#endif

/** Major version number component.
 * This is incremented every time there's a incompatible ABI change.
 * In the special case of major version 0, compatibility between minor versions
 * is not guaranteed.
 */
#define NV_VPI_VERSION_MAJOR 3

/** Minor version number component.
 * This is incremented every time there's a new feature added to NV_VPI that
 * doesn't break backward compatibility. This number is reset to zero when
 * major version changes.
 */
#define NV_VPI_VERSION_MINOR 2

/** Patch version number component.
 * This is incremented every time a bug is fixed, but no new functionality is added
 * to the library. This number is reset to zero when minor version changes.
 */
#define NV_VPI_VERSION_PATCH 4

/** Tweak version number component.
 * Incremented for packaging or documentation updates, etc. The library itself isn't updated.
 * Gets reset to zero when patch version changes.
 */
#define NV_VPI_VERSION_TWEAK 0

/** Version suffix.
 * String appended to version number to designate special builds.
 */
#define NV_VPI_VERSION_SUFFIX ""

/** NV_VPI library version.
  * It's an integer value computed from `MAJOR*1000000 + MINOR*10000 + PATCH*100 + TWEAK`.
  * Integer versions can be compared, recent versions are greater than older ones.
  */
#define NV_VPI_VERSION NV_VPI_MAKE_VERSION(NV_VPI_VERSION_MAJOR, NV_VPI_VERSION_MINOR, NV_VPI_VERSION_PATCH, NV_VPI_VERSION_TWEAK)

/** NV_VPI library version number represented as a string. */
#define NV_VPI_VERSION_STRING "3.2.4"

/** Selected API version to use.
 * This macro selects which of the supported APIs the code will use.
 *
 * By default this equals to the highest supported API, corresponding to the current major and
 * minor versions of the library.
 *
 * User can override the version by defining this macro before including NV_VPI headers.
 */
#if VPI_DOXYGEN
#   define NV_VPI_VERSION_API
#else
#ifdef NV_VPI_VERSION_API
#   if NV_VPI_VERSION_API < NV_VPI_MAKE_VERSION(NV_VPI_VERSION_MAJOR) || \
        NV_VPI_VERSION_API > NV_VPI_MAKE_VERSION(NV_VPI_VERSION_MAJOR, NV_VPI_VERSION_MINOR)
#       error Selected NV_VPI API version not supported.
#   endif
#else
#   define NV_VPI_VERSION_API NV_VPI_MAKE_VERSION(NV_VPI_VERSION_MAJOR, NV_VPI_VERSION_MINOR)
#endif
#endif

/** Conditionally enable code when selected API version is exactly given version.
 *
 * @param[in] major,minor API version that will be considered.
 */
#define NV_VPI_VERSION_API_IS(major,minor) \
    (NV_VPI_MAKE_VERSION(major,minor) == NV_VPI_VERSION_API)

/** Conditionally enable code when selected API version is at least given version.
 *
 * @param[in] major,minor Minimum API version that will be considered.
 */
#define NV_VPI_VERSION_API_AT_LEAST(major,minor) \
    (NV_VPI_MAKE_VERSION(major,minor) <= NV_VPI_VERSION_API)

/** Conditionally enable code when selected API version is at most given version.
 *
 * @param[in] major,minor Maximum API version that will be considered.
 */
#define NV_VPI_VERSION_API_AT_MOST(major,minor) \
    (NV_VPI_MAKE_VERSION(major,minor) >= NV_VPI_VERSION_API)

/** Conditionally enable code when selected API version is between two versions.
 *
 * @param[in] min_major,min_minor Minimum API version that will be considered.
 * @param[in] max_major,max_minor Maximum API version that will be considered.
 */
#define NV_VPI_VERSION_API_IN_RANGE(min_major,min_minor,max_major,max_minor) \
    (NV_VPI_VERSION_API_AT_LEAST(min_major, min_minor) && NV_VPI_VERSION_API_AT_MOST(max_major, max_minor))

/** @} */

#ifdef __cplusplus
}
#endif

#endif // NV_VPI_VERSIONDEF_H
