/*
 * SPDX-FileCopyrightText: Copyright (c) 2025-2026 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <iostream>
#include <cstring>

#include "NvNvSciBufHelper.h"
#include "nvbufsurface_nvscibuf.h"
#include "nvscibuf.h"
#include "nvbufsurface.h"

/* Currently only 709_ER Color Space is supported */

struct NvBufNvSciColorCtx {
    /* NvBufSurface color format from nvbufsurface.h */
    NvBufSurfaceColorFormat pixfmt;
    /* Surface count */
    uint32_t surfaceCount;
    /* NvSci Color standard from nvscibuf.h */
    NvSciBufAttrValColorStd sciColorStd;
    /* NvSci Sample type from nvscibuf.h */
    NvSciBufSurfSampleType sciSampleType;
    NvSciBufSurfBPC sciSurfBPC;
};

static NvBufNvSciColorCtx nvmmColorFormats[] = {

    {NVBUF_COLOR_FORMAT_YUV420_709_ER, 3, NvSciColorStd_REC709_ER, NvSciSurfSampleType_420, NvSciSurfBPC_8},
    {NVBUF_COLOR_FORMAT_NV12_709_ER, 2, NvSciColorStd_REC709_ER, NvSciSurfSampleType_420, NvSciSurfBPC_8},
};

typedef struct
{
    NvSciBufType bufType;
    uint64_t size;
    uint32_t planeCount;
    NvSciBufAttrValImageLayoutType layout;
    uint32_t widthBase;
    uint32_t heightBase;
    uint32_t planeWidths[3];
    uint32_t planeHeights[3];
    uint32_t planePitches[3];
    uint32_t planeBitsPerPixels[3];
    uint32_t planeAlignedHeights[3];
    uint64_t planeAlignedSizes[3];
    uint8_t planeChannelCounts[3];
    uint64_t planeOffsets[3];
    uint32_t topPadding[3];
    uint32_t bottomPadding[3];
    bool needSwCacheCoherency;
    NvSciBufAttrValColorFmt planeColorFormats[3];
    NvSciBufAttrValColorStd sciColorStd;
    NvSciBufSurfSampleType sciSampleType;
    NvSciBufSurfBPC sciSurfBPC;
} BufferAttrs;

/**
 * Maps NvSci color information to corresponding NvBufSurfaceColorFormat
 * @param coe_bufAttrs Buffer attributes containing NvSci color information
 * @param colorFormat Output parameter for the mapped NvBufSurfaceColorFormat
 * @return NvSciError_Success on success, NvSciError_BadParameter if no match found
 */
static NvSciError MapNvSciToNvBufColorFormat(const BufferAttrs &coe_bufAttrs, NvBufSurfaceColorFormat &colorFormat)
{
    const uint32_t numFormats = sizeof(nvmmColorFormats) / sizeof(nvmmColorFormats[0]);

    for (uint32_t i = 0; i < numFormats; i++)
    {
        const NvBufNvSciColorCtx &format = nvmmColorFormats[i];

        // Check if surface count matches
        if (format.surfaceCount != coe_bufAttrs.planeCount)
        {
            continue;
        }

        // Check if color standard, sample type, and BPC match
        if (format.sciColorStd != coe_bufAttrs.sciColorStd ||
            format.sciSampleType != coe_bufAttrs.sciSampleType ||
            format.sciSurfBPC != coe_bufAttrs.sciSurfBPC)
        {
            continue;
        }

        // Found a match based on surface count, color standard, sample type, and BPC
        colorFormat = format.pixfmt;
        return NvSciError_Success;
    }

    printf("%s: No matching color format found\n", __func__);
    printf("%s: Available formats in lookup table:\n", __func__);
    for (uint32_t i = 0; i < numFormats; i++)
    {
        printf("%s:   [%u] pixfmt: %d, surfaces: %u, colorStd: %d, sampleType: %d, BPC: %d\n",
               __func__, i, nvmmColorFormats[i].pixfmt, nvmmColorFormats[i].surfaceCount,
               nvmmColorFormats[i].sciColorStd, nvmmColorFormats[i].sciSampleType,
               nvmmColorFormats[i].sciSurfBPC);
    }
    return NvSciError_BadParameter;
}

NvSciError SetNvBufNvSciBufAttributes(NvSciBufAttrList *attrList, NvSciBufAttrList coeAttrList)
{
    NvSciError sciErr = NvSciError_Success;
    BufferAttrs coe_bufAttrs;
    memset(&coe_bufAttrs, 0, sizeof(coe_bufAttrs));

    NvSciBufAttrKeyValuePair imgAttrs[] = {
        {NvSciBufImageAttrKey_SurfWidthBase, NULL, 0}, //0
        {NvSciBufImageAttrKey_SurfHeightBase, NULL, 0}, //1
        {NvSciBufImageAttrKey_SurfSampleType, NULL, 0}, //2
        {NvSciBufImageAttrKey_SurfColorStd, NULL, 0}, //3
        {NvSciBufImageAttrKey_SurfMemLayout, NULL, 0}, //4
        {NvSciBufImageAttrKey_SurfBPC, NULL, 0}, //5
        {NvSciBufImageAttrKey_Layout, NULL, 0}, //6
    };

    sciErr = NvSciBufAttrListGetAttrs(coeAttrList, imgAttrs, sizeof(imgAttrs) / sizeof(imgAttrs[0]));
    if (sciErr != NvSciError_Success)
    {
        printf("%s: NvSciBufAttrListGetAttrs failed\n", __func__);
        return sciErr;
    }

    coe_bufAttrs.widthBase = *(static_cast<const uint32_t *>(imgAttrs[0].value));
    coe_bufAttrs.heightBase = *(static_cast<const uint32_t *>(imgAttrs[1].value));
    coe_bufAttrs.sciSampleType = *(NvSciBufSurfSampleType *)imgAttrs[2].value;
    coe_bufAttrs.sciColorStd = *(NvSciBufAttrValColorStd *)imgAttrs[3].value;
    NvSciBufSurfMemLayout memLayout = *(NvSciBufSurfMemLayout*)imgAttrs[4].value;
    if (memLayout == NvSciSurfMemLayout_SemiPlanar)
        coe_bufAttrs.planeCount = 2;
    else if (memLayout == NvSciSurfMemLayout_Planar)
        coe_bufAttrs.planeCount = 3;
    else
        coe_bufAttrs.planeCount = 0;

    coe_bufAttrs.sciSurfBPC = *(NvSciBufSurfBPC*)imgAttrs[5].value;
    coe_bufAttrs.layout = *(NvSciBufAttrValImageLayoutType*)imgAttrs[6].value;

    NvmmImageParams nvm_img_params = {0};
    nvm_img_params.gpuId = 0;
    nvm_img_params.width = coe_bufAttrs.widthBase;
    nvm_img_params.height = coe_bufAttrs.heightBase;
    // Map NvSci color information to NvBuf color format
    NvBufSurfaceColorFormat mappedColorFormat;
    sciErr = MapNvSciToNvBufColorFormat(coe_bufAttrs, mappedColorFormat);
    if (sciErr != NvSciError_Success)
    {
        printf("%s: MapNvSciToNvBufColorFormat failed Error: %d\n", __func__, sciErr);
        return sciErr;
    }
    nvm_img_params.colorFormat = mappedColorFormat;
    if (coe_bufAttrs.layout == NvSciBufImage_PitchLinearType)
    {
        nvm_img_params.layout = NVBUF_LAYOUT_PITCH;
    }
    else
    {
        nvm_img_params.layout =  NVBUF_LAYOUT_BLOCK_LINEAR;
    }
    nvm_img_params.displayscanformat = NVBUF_DISPLAYSCANFORMAT_PROGRESSIVE;
    nvm_img_params.isProtected = false;

    int ret = NvmmImageFillNvSciBufAttrList(&nvm_img_params, *attrList);
    if (ret != 0)
    {
        printf("%s: NvmmImageFillNvSciBufAttrList failed Error: %d\n", __func__, ret);
        return NvSciError_BadParameter;
    }
    return sciErr;
}

NvSciError SetNvSciBufAttributes(NvSciBufAttrList *attrList, bool bl_output)
{
    NvSciError sciErr = NvSciError_Success;
    NvSciBufType bufType = NvSciBufType_Image;
    NvSciBufAttrValAccessPerm accessPerm = NvSciBufAccessPerm_ReadWrite;
    bool isCpuAcccessReq = true;
    bool isCpuCacheEnabled = true;
    uint32_t count = 0U;

    NvSciBufSurfMemLayout memLayout = NvSciSurfMemLayout_SemiPlanar;
    NvSciBufAttrValImageLayoutType layoutType = NvSciBufImage_BlockLinearType;
    if (bl_output == false)
    {
        layoutType = NvSciBufImage_PitchLinearType;
    }

    NvSciBufSurfType surfType = NvSciSurfType_YUV;
    NvSciBufSurfSampleType sampleType = NvSciSurfSampleType_420;
    NvSciBufSurfBPC surfBPC = NvSciSurfBPC_8;
    NvSciBufSurfComponentOrder compOrder = NvSciSurfComponentOrder_YUV;
    NvSciBufAttrValColorStd surfColorStd[] = {NvSciColorStd_REC709_ER};

    NvSciBufAttrKeyValuePair attrKvp[] = {
        {NvSciBufGeneralAttrKey_Types, &bufType, sizeof(bufType)},
        {NvSciBufGeneralAttrKey_RequiredPerm, &accessPerm, sizeof(accessPerm)},
        {NvSciBufGeneralAttrKey_NeedCpuAccess, &isCpuAcccessReq, sizeof(isCpuAcccessReq)},
        {NvSciBufGeneralAttrKey_EnableCpuCache, &isCpuCacheEnabled, sizeof(isCpuCacheEnabled)},
        {NvSciBufImageAttrKey_Layout, &layoutType, sizeof(layoutType)},
        {NvSciBufImageAttrKey_SurfType, &surfType, sizeof(surfType)},
        {NvSciBufImageAttrKey_SurfMemLayout, &memLayout, sizeof(memLayout)},
        {NvSciBufImageAttrKey_SurfSampleType, &sampleType, sizeof(sampleType)},
        {NvSciBufImageAttrKey_SurfBPC, &surfBPC, sizeof(surfBPC)},
        {NvSciBufImageAttrKey_SurfComponentOrder, &compOrder, sizeof(compOrder)},
        {NvSciBufImageAttrKey_SurfColorStd, &surfColorStd, sizeof(surfColorStd)},
    };

    count = sizeof(attrKvp) / sizeof(attrKvp[0]);
    sciErr = NvSciBufAttrListSetAttrs(*attrList, attrKvp, count);
    if (sciErr != NvSciError_Success)
    {
        printf("%s: NvSciBufAttrListSetAttrs failed Error: %d\n", __func__, sciErr);
        return sciErr;
    }
    return sciErr;
}
