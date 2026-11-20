/*
 * SPDX-FileCopyrightText: Copyright (c) 2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 * SPDX-License-Identifier: LicenseRef-NvidiaProprietary
 *
 * NVIDIA CORPORATION, its affiliates and licensors retain all intellectual
 * property and proprietary rights in and to this material, related
 * documentation and any modifications thereto. Any use, reproduction,
 * disclosure or distribution of this material and related documentation
 * without an express license agreement from NVIDIA CORPORATION or
 * its affiliates is strictly prohibited.
 */

/**
 * @file nvbufsurface_nvscibuf.h
 * <b>NvBufSurface NvSciBuf Interop Interface </b>
 *
 * This file specifies the NvBufSurface NvSciBuf Interop API.
 *
 * The NvBufSurface NvSciBuf Interop API provides methods to convert NvSciBuf to NvBufSurface.
 */

#ifndef _NVBUFSURFACE_NVSCIBUF_H_
#define _NVBUFSURFACE_NVSCIBUF_H_

#include "nvbufsurface.h"
#include "nvscibuf.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define NVMM_STRUCTURE_PADDING 4

typedef struct NvmmImageParams {
  /** Holds the GPU ID. */
  uint32_t gpuId;
  /** Holds the width of the buffer. */
  uint32_t width;
  /** Holds the height of the buffer. */
  uint32_t height;
  /** Holds the color format of the buffer. */
  NvBufSurfaceColorFormat colorFormat;
  /** Holds the surface layout. */
  NvBufSurfaceLayout layout;
  /** Holds the display scan format. */
  NvBufSurfaceDisplayScanFormat displayscanformat;
  /** Holds the vpr flag. */
  bool isProtected;
  /** Holds the reserved space for future use. */
  void * _reserved[NVMM_STRUCTURE_PADDING];
} NvmmImageParams;

/**
  * Convert NvSciBuf to NvBufSurface.
  *
  * @param[in] bufObj : NvSciBuf object
  * @param[out] nvbuf_surf : NvBufSurface object
  *
  * @return 0 on success, -1 on failure
  */
int NvmmNvSciBufToNvBufSurface(
    NvSciBufObj bufObj,
    NvBufSurface **nvbuf_surf);

/**
  * Fill NvSciBufAttrList with the attributes of the NvSciBuf object.
  *
  * @param[in] imgParams : NvmmImageParams object
  * @param[out] attrList : NvSciBufAttrList object
  *
  * @return 0 on success, -1 on failure
  */
int NvmmImageFillNvSciBufAttrList(
    NvmmImageParams *imgParams,
    NvSciBufAttrList attrList);

#ifdef __cplusplus
}
#endif

#endif