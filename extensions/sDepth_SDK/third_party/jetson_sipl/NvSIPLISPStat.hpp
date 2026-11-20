/*
 * SPDX-FileCopyrightText: Copyright (c) 2021-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
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
 * @file NvSIPLISPStat.hpp
 * @brief <b> NvSipl ISP statistics data structures for Tegra </b>
 */

#ifndef NVSIPL_ISP_STAT_H
#define NVSIPL_ISP_STAT_H

#include <cstdint>
#include <array>
#include "NvSIPLCommon.hpp"

/**
 * @brief Number of histogram bins.
 */
#define NVSIPL_ISP_HIST_BINS                   (256U)
/**
 * @brief Maximum number of color components.
 */
#define NVSIPL_ISP_MAX_COLOR_COMPONENT         (4U)

/**
 * @brief Number of histogram knee points.
 */
#define NVSIPL_ISP_HIST_KNEE_POINTS            (8U)

/**
 * @brief Number of radial transfer function control points.
 */
#define NVSIPL_ISP_RADTF_POINTS                (6U)

/**
 * @brief Maximum number of local average and clip statistic block
 * regions of interest.
 */
#define NVSIPL_ISP_MAX_LAC_ROI                 (4U)

/**
 * @brief Maximum number of input planes.
 */
#define NVSIPL_ISP_MAX_INPUT_PLANES            (3U)

/**
 * @brief Maximum matrix dimension.
 */
#define NVSIPL_ISP_MAX_COLORMATRIX_DIM         (3U)

/**
 * @brief Maximum number of windows for local average and clip in a region of
 * interest.
 * - value is (32U * 32U)
 */
#define NVSIPL_ISP_MAX_LAC_ROI_WINDOWS         (1024U)

namespace nvsipl
{

/**
 * @defgroup NvSIPLISPStats NvSIPL ISP Stats
 *
 * @brief NvSipl ISP Defines for ISP Stat structures.
 *
 * @ingroup NvSIPLCamera_API
 */
/** @addtogroup NvSIPLISPStats
 * @{
 */
/**
 * ISP decoded stats including the stats header info
 */
struct NvIspStatsHeaderInfo
{
    /**
     * Stream ID
     */
    uint32_t StreamID;
    /**
     * Frame ID
     */
    uint32_t FrameID;
    /**
     * Program ID, only available in ISP7, 0 for ISP5/6
     */
    uint32_t ProgramID;
};

/**
 * @brief Holds bad pixel statistics (BP Stats).
 */
struct NvSiplISPBadPixelStatsData {
    /**
     * Holds bad pixel count for pixels corrected upward within the window.
     * Valid Range: [0, image_width x image_height]
     */
    uint32_t highInWin;
    /**
     * Holds bad pixel count for pixels corrected downward within the window.
     * Valid Range: [0, image_width x image_height]
     */
    uint32_t lowInWin;
    /**
     * Holds accumulated pixel adjustment for pixels corrected upward within the
     * window.
     * Valid Range: [0, UINT32_MAX]
     */
    uint32_t highMagInWin;
    /**
     * Holds accumulated pixel adjustment for pixels corrected downward within
     * the window.
     * Valid Range: [0, UINT32_MAX]
     */
    uint32_t lowMagInWin;
    /**
     * Holds bad pixel count for pixels corrected upward outside the window.
     * Valid Range: [0, image_width x image_height]
     */
    uint32_t highOutWin;
    /**
     * Holds bad pixel count for pixels corrected downward outside the window.
     * Valid Range: [0, image_width x image_height]
     */
    uint32_t lowOutWin;
    /**
     * Holds accumulated pixel adjustment for pixels corrected upward outside
     * the window.
     * Valid Range: [0, UINT32_MAX]
     */
    uint32_t highMagOutWin;
    /**
     * Holds accumulated pixel adjustment for pixels corrected downward outside
     * the window.
     * Valid Range: [0, UINT32_MAX]
     */
    uint32_t lowMagOutWin;
};

/**
 * @brief Holds dead pixel correction statistics (DPC Stats) metrics.
 * \note These are provided in NvSiplISPDeadPixelCorrectionStatsData for both inside and outside the
 * programmable ROI.
 * Supported in ISP7
 */
struct NvSiplISPDeadPixelCorrectionStatsROIData
{
    /**
     * Number of pixels flagged as outlier outside of the ROI
     * Valid Range: [0, UINT32_MAX]
     */
    uint32_t outlierOutWin;
    /**
     * Number of pixels flagged as inlier under Condition 1 outside of the ROI
     * Valid Range: [0, UINT32_MAX]
     */
    uint32_t inlierCond1OutWin;
    /**
     * Number of pixels flagged as inlier under Condition 2 outside of the ROI
     * Valid Range: [0, UINT32_MAX]
     */
    uint32_t inlierCond2OutWin;
    /**
     * Number of pixels flagged as inlier under Condition 3 outside of the ROI
     * Valid Range: [0, UINT32_MAX]
     */
    uint32_t inlierCond3OutWin;
    /**
     * Number of pixels flagged as inlier under Condition 4 outside of the ROI
     * Valid Range: [0, UINT32_MAX]
     */
    uint32_t inlierCond4OutWin;
    /**
     * Number of pixels flagged as inlier under Condition 5 outside of the ROI
     * Valid Range: [0, UINT32_MAX]
     */
    uint32_t inlierCond5OutWin;
    /**
     * Number of pixels flagged as inlier under Condition 6 outside of the ROI
     * Valid Range: [0, UINT32_MAX]
     */
    uint32_t inlierCond6OutWin;
    /**
     * Number of pixels for which a local gradient is detected outside of the ROI
     * Valid Range: [0, UINT32_MAX]
     */
    uint32_t detectedOutWin;
    /**
     * Number of pixels detected as bottom outlier if extremum thresholding is enabled
     * outside of the ROI
     * Valid Range: [0, UINT32_MAX]
     */
    uint32_t bottomOutlierOutWin;
    /**
     * Number of pixels detected as top outlier if extremum thresholding is enabled
     * outside of the ROI
     * Valid Range: [0, UINT32_MAX]
     */
    uint32_t topOutlierOutWin;
};
/**
 * @brief Holds dead pixel correction statistics (DPC Stats).
 * \note DPC stats are generated identically for both within the programmable ROI and outside the
 * ROI.
 * Supported in ISP7
 */
struct NvSiplISPDeadPixelCorrectionStatsData
{
    /**
     * Holds the statistics for dead pixels inside the ROI specified
     */
    NvSiplISPDeadPixelCorrectionStatsROIData insideRoi;
    /**
     * Holds the statistics for dead pixels outside the ROI specified
     */
    NvSiplISPDeadPixelCorrectionStatsROIData outsideRoi;
    /**
     * @brief stats header info
     */
    NvIspStatsHeaderInfo statsInfo;
};

/**
 * @brief Holds histogram statistics (HIST Stats).
 */
struct NvSiplISPHistogramStatsData {
    /**
     * Holds histogram data for each color component in RGGB/RCCB/RCCC order.
     * Valid Ranges: [0, (W x H)/4].
     * W is the input width
     * H is the input height
     */
    std::array<std::array<uint32_t, NVSIPL_ISP_MAX_COLOR_COMPONENT>, NVSIPL_ISP_HIST_BINS> data;
    /**
     * Holds the number of pixels excluded by the elliptical mask for each
     * color component.
     * Valid Ranges: [0, (W x H)/4].
     * W is the input width
     * H is the input height
     */
    std::array<uint32_t, NVSIPL_ISP_MAX_COLOR_COMPONENT> excludedCount;
    /**
     * @brief stats header info
     */
    NvIspStatsHeaderInfo statsInfo;
};

/**
 * @brief Holds local average and clip statistics data for a region of interest.
 */
struct NvSiplISPLocalAvgClipStatsROIData{
    /**
     * Holds number of windows horizontally in one region of interest.
     * Valid Range: [1, 32]
     */
    uint32_t numWindowsH;
    /**
     * Holds number of windows vertically in one region of interest.
     * Valid Range: [1, 32]
     */
    uint32_t numWindowsV;
    /**
     * Holds average pixel value for each color component in each window in
     * RGGB/RCCB/RCCC order.
     * Valid Range: [0.0, 1.0]
     */
    std::array<std::array<float_t, NVSIPL_ISP_MAX_COLOR_COMPONENT>, NVSIPL_ISP_MAX_LAC_ROI_WINDOWS> average;
    /**
     * Holds the number of pixels excluded by the elliptical mask for each
     * color component in each window
     * in RGGB/RCCB/RCCC order.
     * Valid Range: [0, M/4]
     * M is the number of pixels per color component in the window.
     */
    std::array<std::array<uint32_t, NVSIPL_ISP_MAX_COLOR_COMPONENT>, NVSIPL_ISP_MAX_LAC_ROI_WINDOWS> maskedOffCount;
    /**
     * Holds number of clipped pixels for each color component in each window in
     * RGGB/RCCB/RCCC order.
     * Valid Range: [0, M/4]
     * M is the number of pixels per color component in the window.
     */
    std::array<std::array<uint32_t, NVSIPL_ISP_MAX_COLOR_COMPONENT>, NVSIPL_ISP_MAX_LAC_ROI_WINDOWS> clippedCount;
    /**
     * @brief stats header info
     */
    NvIspStatsHeaderInfo statsInfo;
};

/**
 * @brief Defines an ellipse.
 */
struct NvSiplISPEllipse {
    /**
     * Holds center of the ellipse.
     * Valid Range:
     * @li X coordinate of the center: [0, input width - 1]
     * @li Y coordinate of the center: [0, input height - 1]
     */
    NvSiplPointFloat center;
    /**
     * Holds horizontal axis of the ellipse.
     * Valid Range: [17, 2 x input width]
     */
    uint32_t horizontalAxis;
    /**
     * Holds vertical axis of the ellipse.
     * Valid Range: [17, 2 x input height]
     */
    uint32_t verticalAxis;
    /**
     * Holds angle of the ellipse horizontal axis from X axis in degrees in
     * clockwise direction.
     * Valid Range: [0.0, 360.0]
     */
    float_t angle;
};

/**
 * @brief Defines the windows used in ISP LAC stats calculations.
 *
 * @code
 * ------------------------------------------------------------------------------
 * |         startOffset    horizontalInterval                                  |
 * |                    \  |--------------|                                     |
 * |                     - *******        *******        *******                |
 * |                     | *     *        *     *        *     *                |
 * |                     | *     *        *     *        *     *                |
 * |                     | *     *        *     *        *     *                |
 * |                     | *******        *******        *******                |
 * |  verticalInterval-->|                                        \             |
 * |                     |                                          numWindowsV |
 * |                     |                                        /             |
 * |                     - *******        *******        *******                |
 * |                     | *     *        *     *        *     *                |
 * |            height-->| *     *        *     *        *     *                |
 * |                     | *     *        *     *        *     *                |
 * |                     - *******        *******        *******                |
 * |                       |-----|                                              |
 * |                        width     \      |     /                            |
 * |                                    numWindowsH                             |
 * ------------------------------------------------------------------------------
 * @endcode
 */
struct NvSiplISPStatisticsWindows{
    /**
     * Holds width of the window in pixels.
     * Valid Range: [2, 256] and must be an even number
     */
    uint32_t width;
    /**
     * Holds height of the window in pixels.
     * Valid Range: [2, 256]
     */
    uint32_t height;
    /**
     * Holds number of windows horizontally.
     * Valid Range: [1, 32]
     */
    uint32_t numWindowsH;
    /**
     * Holds number of windows vertically.
     * Valid Range: [1, 32]
     */
    uint32_t numWindowsV;
    /**
     * Holds the distance between the left edge of one window and a horizontally
     * adjacent window.
     * Valid Range: [max(4, LAC window width), LAC ROI width] and must be an even number
     */
    uint32_t horizontalInterval;
    /**
     * Holds the distance between the top edge of one window and a vertically
     * adjacent window.
     * Valid Range: [max(2, LAC window height), LAC ROI height]
     */
    uint32_t verticalInterval;
    /**
     * Holds the position of the top left pixel in the top left window.
     * Valid Range:
     * @li X coordinate of start offset: [0, LAC ROI width-3] and must be an even number
     * @li Y coordinate of start offset: [0, LAC ROI height-3]
     * @li startOffset.x + horizontalInterval * (numWindowH - 1) + winWidth <= LAC ROI width
     * @li startOffset.y + veritcallInterval * (numWindowV - 1) + winHeight <= LAC ROI height
     */
    NvSiplPoint startOffset;
};


/**
 * @brief Defines a spline control point.
 */
struct NvSiplISPSplineControlPoint {
    /**
     * Holds X coordinate of the control point.
     * Valid Range: [0.0, 2.0]
     */
    float_t x;
    /**
     * Holds Y coordinate of the control point.
     * Valid Range: [0.0, 2.0]
     */
    float_t y;
    /**
     * Holds slope of the spline curve at the control point.
     * Valid Range: \f$[-2^{16}, 2^{16}]\f$
     */
    double_t slope;
};

/**
 * @brief Defines a radial transform.
 */
struct NvSiplISPRadialTF {
    /**
     * Holds ellipse for radial transform.
     *
     * Coordinates of the image's top left and bottom right points are (0, 0)
     * and (width, height) respectively.
     */
    NvSiplISPEllipse radialTransform;
    /**
     * Defines spline control point for radial transfer function.
     */
    std::array<NvSiplISPSplineControlPoint, NVSIPL_ISP_RADTF_POINTS> controlPoints;
};

/**
 * @brief Holds controls for histogram statistics (HIST Stats).
 */
struct NvSiplISPHistogramStats {
    /**
     * Holds a Boolean to enable histogram statistics block.
     */
    NvSiplBool enable;
    /**
     * Holds offset to be applied to input data prior to bin mapping.
     * Valid Range: [-2.0, 2.0]
     */
    float_t offset;
    /**
     * Holds bin index specifying different zones in the histogram. Each zone
     * can have a different number of bins.
     * Valid Range: [1, 255]
     */
    std::array<uint8_t, NVSIPL_ISP_HIST_KNEE_POINTS> knees;
    /**
     * Holds \f$log_2\f$ range of the pixel values to be considered for each
     * zone. The whole pixel range is divided into NVSIPL_ISP_HIST_KNEE_POINTS
     * zones.
     * Valid Range: [0, 21]
     */
    std::array<uint8_t, NVSIPL_ISP_HIST_KNEE_POINTS> ranges;
    /**
     * Holds a rectangular mask for excluding pixels outside a specified area.
     *
     * The coordinates of image top left and bottom right points are (0, 0) and
     * (width, height), respectively. Set the rectangle mask to include the
     * full image (or cropped image for the case input cropping is enabled)
     * if no pixels need to be excluded.
     *
     * The rectangle settings(x0, y0, x1, y1) must follow the constraints listed below:
     * - (x0 >= 0) and (y0 >= 0)
     * - x0 and x1 should be even
     * - (x1 <= image width) and (y1 <= image height)
     * - rectangle width(x1 - x0) >= 2 and height(y1 - y0) >= 2
     */
    NvSiplRect rectangularMask;
    /**
     * Holds a Boolean to enable an elliptical mask for excluding pixels
     * outside a specified area.
     */
    NvSiplBool ellipticalMaskEnable;
    /**
     * Holds an elliptical mask for excluding pixels outside a specified area.
     *
     * Coordinates of the image top left and bottom right points are (0, 0) and
     * (width, height), respectively.
     */
    NvSiplISPEllipse ellipticalMask;
    /**
     * Holds a Boolean to enable elliptical weighting of pixels based on spatial
     * location. This can be used to compensate for lens shading when the
     * histogram is measured before lens shading correction.
     */
    NvSiplBool ellipticalWeightEnable;
    /**
     * Holds a radial transfer function for elliptical weight.
     * Valid Range: Check the declaration of @ref NvSiplISPRadialTF.
     */
    NvSiplISPRadialTF radialTF;
};

/**
 * @brief Holds local average and clip statistics block (LAC Stats).
 */
struct NvSiplISPLocalAvgClipStatsData{
    /**
     * Holds statistics data for each region of interest.
     */
    std::array<NvSiplISPLocalAvgClipStatsROIData, NVSIPL_ISP_MAX_LAC_ROI> data;
};

/**
 * @brief Holds controls for local average and clip statistics (LAC Stats).
 */
struct NvSiplISPLocalAvgClipStats{
    /**
     * Holds a Boolean to enable the local average and clip statistics block.
     */
    NvSiplBool enable;
    /**
     * Holds minimum value of pixels in RGGB/RCCB/RCCC order.
     * Valid Range: [0.0, 1.0]
     */
    std::array<float_t, NVSIPL_ISP_MAX_COLOR_COMPONENT> min;
    /**
     * Holds maximum value of pixels in RGGB/RCCB/RCCC order.
     * Valid Range: [0.0, 1.0], max >= min
     */
    std::array<float_t, NVSIPL_ISP_MAX_COLOR_COMPONENT> max;
    /**
     * Holds a Boolean to enable an individual region of interest.
     */
    std::array<NvSiplBool, NVSIPL_ISP_MAX_LAC_ROI> roiEnable;
    /**
     * Holds local average and clip windows for each region of interest.
     */
    std::array<NvSiplISPStatisticsWindows, NVSIPL_ISP_MAX_LAC_ROI> windows;
    /**
     * Holds a Boolean to enable an elliptical mask for excluding pixels
     * outside a specified area for each region of interest.
     */
    std::array<NvSiplBool, NVSIPL_ISP_MAX_LAC_ROI> ellipticalMaskEnable;
    /**
     * Holds an elliptical mask for excluding pixels outside specified area.
     *
     * Coordinates of the image top left and bottom right points are (0, 0) and
     * (width, height), respectively.
     */
    NvSiplISPEllipse ellipticalMask;
};


/**
 * @brief Holds controls for bad pixel statistics (BP Stats).
 */
struct NvSiplISPBadPixelStats{
    /**
     * Holds a Boolean to enable the bad pixel statistics block.
     * @note Bad Pixel Correction must also be enabled to get bad pixel
     *  statistics.
     */
    NvSiplBool enable;
    /**
     * Holds rectangular mask for excluding pixel outside a specified area.
     *
     * Coordinates of the image's top left and bottom right points are (0, 0)
     * and (width, height), respectively. Set the rectangle to include the
     * full image (or cropped image for the case input cropping is enabled)
     * if no pixels need to be excluded.
     *
     * Valid Range: Rectangle must be within the input image and must
     *  be a valid rectangle ((right > left) && (bottom > top)). The minimum
     *  supported rectangular mask size is 4x4.
     * Constraints: All left, top, bottom, and right coordinates must be even.
     */
    NvSiplRect rectangularMask;
};


/**
 * @brief SIPL ISP Histogram Statistics Override Params
 */
struct NvSiplISPHistogramStatsOverride{
    /**
     * Holds a Boolean to enable histogram statistics Control block.
     */
    NvSiplBool enable;
    /**
     * Holds offset to be applied to input data prior to bin mapping.
     * Valid Range: [-2.0, 2.0]
     */
    float_t offset;
    /**
     * Holds bin index specifying different zones in the histogram. Each zone
     * can have a different number of bins.
     * Valid Range: [1, 255]
     */
    std::array<uint8_t, NVSIPL_ISP_HIST_KNEE_POINTS> knees;
    /**
     * Holds \f$log_2\f$ range of the pixel values to be considered for each
     * zone. The whole pixel range is divided into NVSIPL_ISP_HIST_KNEE_POINTS
     * zones.
     * Valid Range: [0, 21]
     */
    std::array<uint8_t, NVSIPL_ISP_HIST_KNEE_POINTS> ranges;
    /**
     * Holds a rectangular mask for excluding pixels outside a specified area.
     *
     * The coordinates of image top left and bottom right points are (0, 0) and
     * (width, height), respectively. Set the rectangle mask to include the
     * full image (or cropped image for the case input cropping is enabled)
     * if no pixels need to be excluded.
     *
     * The rectangle settings(x0, y0, x1, y1) must follow the constraints listed below:
     * - (x0 >= 0) and (y0 >= 0)
     * - x0 and x1 should be even
     * - (x1 <= image width) and (y1 <= image height)
     * - rectangle width(x1 - x0) >= 2 and height(y1 - y0) >= 2
     */
    NvSiplRect rectangularMask;
    /**
     * Holds a Boolean to enable an elliptical mask for excluding pixels
     * outside a specified area.
     */
    NvSiplBool ellipticalMaskEnable;
    /**
     * Holds an elliptical mask for excluding pixels outside a specified area.
     *
     * Coordinates of the image top left and bottom right points are (0, 0) and
     * (width, height), respectively.
     */
    NvSiplISPEllipse ellipticalMask;
    /**
     * @brief boolean flag to disable lens shading compensation for histogram statistics block
     */
    NvSiplBool disableLensShadingCorrection;
};

/**
 * @brief SIPL ISP Statistics Override Parameters.
 *
 * ISP Statistics settings enabled in this struct will override
 * the corresponding statistics settings provided in NITO.
 *
 * @note ISP histStats[0] and lacStats[0] statistics are consumed by internal
 * algorithms to generate new sensor and ISP settings. Incorrect usage or
 * disabling these statistics blocks would result in failure or
 * image quality degradation. Please refer to the safety manual
 * for guidance on overriding histStats[0] and lacStats[0] statistics settings.
 */
struct NvSIPLIspStatsOverrideSetting {

    /**
     * @brief boolean flag to enable histogram statistics settings override
     */
    std::array<NvSiplBool, 2> enableHistStatsOverride;
    /**
     * @brief Structure containing override settings for histogram statistics block
     */
    std::array<NvSiplISPHistogramStatsOverride, 2> histStats;
    /**
     * @brief boolean flag to enable local average clip statistics settings override
     */
    std::array<NvSiplBool, 2> enableLacStatsOverride;
    /**
     * @brief Structure containing override settings for local average clip statistics block
     */
    std::array<NvSiplISPLocalAvgClipStats, 2> lacStats;
    /**
     * @brief boolean flag to enable bad pixel statistics settings override
     */
    std::array<NvSiplBool, 1> enableBpStatsOverride;
    /**
     * @brief Structure containing override settings for bad pixel statistics block
     */
    std::array<NvSiplISPBadPixelStats, 1> bpStats;
};

/** @brief ISP statistics container for settings attributes */
struct IspStatsSettings {
    /**
     * @brief Structure containing settings for histogram statistics block
     */
    std::array<NvSiplISPHistogramStats, 3> histStatsSettings;
    /**
     * @brief Structure containing settings for local average clip statistics block
     */
    std::array<NvSiplISPLocalAvgClipStats, 2> lacStatsSettings;
    /**
     * @brief Structure containing settings for bad pixel statistics block
     */
    NvSiplISPBadPixelStats bpStatsSettings;
};

/** @brief ISP statistics container for data attributes */
struct IspStatsData {
    /**
     * @brief Structure containing histogram statistics block data
     */
    std::array<NvSiplISPHistogramStatsData, 3> histStatsData;
    /**
     * @brief Structure containing local average clip statistics block data
     */
    std::array<NvSiplISPLocalAvgClipStatsData, 2> lacStatsData;
    /**
     * @brief Structure containing bad pixel statistics block data (ISP6)
     */
    NvSiplISPBadPixelStatsData bpStatsData;
    /**
     * @brief Structure containing dead pixel statistics block data (ISP7)
     */
    NvSiplISPDeadPixelCorrectionStatsData dpStatsData;
};

/** @brief ISP statistics container for settings and data */
struct IspStatsInfo
{
    /** Holds the ISP statistics settings for the previous ISP output frame */
    IspStatsSettings ispStatsSettings;
    /** Holds the ISP statistics data for the previous ISP output frame */
    IspStatsData ispStatsData;
};

/** @} */

} // namespace nvsipl

#endif /* NVSIPL_ISP_STAT_H */
