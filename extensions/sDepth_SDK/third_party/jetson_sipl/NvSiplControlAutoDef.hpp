/*
 * SPDX-FileCopyrightText: Copyright (c) 2020-2026 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 * SPDX-License-Identifier: LicenseRef-NvidiaProprietary
 *
 * NVIDIA CORPORATION, its affiliates and licensors retain all intellectual
 * property and proprietary rights in and to this material, related
 * documentation and any modifications thereto. Any use, reproduction,
 * disclosure or distribution of this material and related documentation
 * without an express license agreement from NVIDIA CORPORATION or
 * its affiliates is strictly prohibited.
 */

/*  NVIDIA SIPL Control Auto Definitions */

#ifndef NVSIPLCONTROLAUTODEF_HPP
#define NVSIPLCONTROLAUTODEF_HPP

#include "NvSIPLISPStat.hpp"
#include "NvSIPLCDICommon.h"

#include <cstdint>
#include <array>

/**
 * @file
 *
 * @brief <b> NVIDIA SIPL: Auto Control Settings - @ref NvSIPLAutoControl </b>
 *
 */

namespace nvsipl{

/** @addtogroup NvSIPLAutoControl
 * @{
 */

/**
  *  Defines types of SIPL Control Auto plug-ins.
  */
enum PluginType : std::uint8_t {
   NV_PLUGIN = 0,  /** NVIDIA plug-in */
   CUSTOM_PLUGIN0, /** Custom plug-in 0 */
   MAX_NUM_PLUGINS /** Maximum number of plug-ins supported. */
};

/**
 * @brief Defines the minimum number of control points for
 * Global Tone Map(GTM) plugin override settings.
 */
constexpr uint32_t ISP_GTM_PLUGIN_OVERRIDE_MIN_CTRLPOINTS = 2U;

/**
 * @brief Defines the maximum number of control points for
 * Global Tone Map(GTM) plugin override settings.
 */
constexpr uint32_t ISP_GTM_PLUGIN_OVERRIDE_MAX_CTRLPOINTS = 200U;

/**
 * @brief Sensor settings
 */
struct SiplControlAutoSensorSetting {
    /**
     * Holds the number of sensor contexts to activate. Multiple sensor contexts mode is
     * supported by some sensors, in which multiple set of settings(contexts) are programmed
     * and the sensor toggles between them at runtime. For sensors not supporting this mode
     * of operation, it shall be set to 1.
     * Valid Range: [1, @ref DEVBLK_CDI_MAX_SENSOR_CONTEXTS]
     */
    uint8_t                           numSensorContexts;
    /**
     * Holds the sensor exposure settings to set for each context, supports up to @ref DEVBLK_CDI_MAX_SENSOR_CONTEXTS settings.
     */
    std::array<DevBlkCDIExposure, DEVBLK_CDI_MAX_SENSOR_CONTEXTS> exposureControl;
    /**
     * Holds the sensor white balance settings to set for each context, supports up to @ref DEVBLK_CDI_MAX_SENSOR_CONTEXTS settings.
     */
    std::array<DevBlkCDIWhiteBalance, DEVBLK_CDI_MAX_SENSOR_CONTEXTS> wbControl;
    /**
     * Holds the sensor IR emitter settings to set for each context, supports up to @ref DEVBLK_CDI_MAX_SENSOR_CONTEXTS settings.
     * This control is applicable to RGB-IR sensors.
     */
    std::array<DevBlkCDIIllumination, DEVBLK_CDI_MAX_SENSOR_CONTEXTS> illuminationControl;
    /**
     * Holds the sensor frame context id settings to set for each context, supports up to @ref DEVBLK_CDI_MAX_SENSOR_CONTEXTS settings.
     * This control is applicable to multi-context sensors (numSensorContexts > 1)
     */
    std::array<DevBlkCDIFrameCtxID, DEVBLK_CDI_MAX_SENSOR_CONTEXTS> multiCtxIDControl;
};

/**
 * @brief Parsed frame embedded information.
 */
struct SiplControlEmbedInfo {
    /**
     * Holds the parsed embedded data frame number of exposures info for the captured frame.
     * Valid Range: [1, @ref DEVBLK_CDI_MAX_EXPOSURES]
     */
    uint32_t                          numExposures;
    /**
     * Holds the parsed embedded data sensor exposure info for the captured frame.
     */
    DevBlkCDIExposure                sensorExpInfo;
    /**
     * Holds the parsed embedded data sensor white balance info for the captured frame.
     */
    DevBlkCDIWhiteBalance            sensorWBInfo;
    /**
     * Holds the parsed embedded data sensor temperature info for the captured frame, this variable
     * is not supported in SIPL Control Auto.
     */
    DevBlkCDITemperature             sensorTempInfo;
    /**
     * Holds the parsed embedded data for IR emitter status (ON or OFF) for RGB-IR sensors.
     */
    DevBlkCDIIllumination   illuminationInfo;
    /**
     * Holds the parsed embedded data sensor frame context id info for the captured frame.
     */
    DevBlkCDIFrameCtxID      frameCtxIDInfo;
};

/**
 * @brief Embedded data and parsed information.
 */
struct SiplControlEmbedData {
    /**
     * Holds the parsed embedded info for the captured frame.
     */
    SiplControlEmbedInfo              embedInfo;
    /**
     * Holds frame sequence number for the captured frame, this variable is not supported in SIPL Control Auto.
     */
    DevBlkCDIFrameSeqNum             frameSeqNum;
    /**
     * Holds information of the embedded data buffer attached to the beginning of the frame, this variable is
     * not supported in SIPL Control Auto.
     */
    DevBlkCDIEmbeddedDataChunk       topEmbeddedData;
    /**
     * Holds information of the embedded data buffer attached to the end of the frame, this variable is not
     * supported in SIPL Control Auto.
     */
    DevBlkCDIEmbeddedDataChunk       bottomEmbeddedData;
};

/**
 * @brief Color Gains assuming order RGGB, RCCB, RCCC.
 */
struct SiplControlAutoAwbGain {
    /**
     * A Boolean flag to control whether white balance gains are valid or not.
     */
    bool                              valid;
    /**
     * Gains that applies to individual color channels
     * Valid Range: [0, 8.0]
     */
    std::array<float_t, NVSIPL_ISP_MAX_COLOR_COMPONENT> gain;
};

/**
 * @brief Automatic white balance settings.
 */
struct SiplControlAutoAwbSetting {
    /**
     * Total white balance gains, including both sensor channel gains and ISP gains
     * Valid Range: [0, 8.0]
     */
    std::array<SiplControlAutoAwbGain, NVSIPL_ISP_MAX_INPUT_PLANES> wbGainTotal;
    /**
     * Correlated color temperature.
     * Valid Range: [2000, 20000]
     */
    float_t                 cct;
    /**
     * Color correction matrix(CCM)
     * In order to apply CCM, the image needs to be first converted from YUV to RGB.
     * So final matrix programmed for Color Correction is the product of
     * (YUV to RGB Color Conversion matrix) and (CCM matrix)
     *
     * Final matrix = Color Conversion matrix * CCM matrix
     *
     * We use Rec709 YUV to RGB Color Conversion matrix for YUV to RGB conversion:
     *   | 1.0            1.0          1.0 |
     *   | 0.0           -0.1873    1.8556 |
     *   | 1.5748        -0.4681       0.0 |
     *
     * The SW will take CCM matrix values such that the Final matrix values are within [-8, 8].
     *
     * Valid Range for CCM matrix such that Final matrix values: [-8.0, 8.0]
     */
    std::array<std::array<float_t, NVSIPL_ISP_MAX_COLORMATRIX_DIM>, NVSIPL_ISP_MAX_COLORMATRIX_DIM> ccmMatrix;
};

/** @brief Structure containing ISP Stats information.
  */
struct SiplControlIspStatsInfo {
    /**
     * Holds pointers to 2 LAC stats data.
     */
    std::array<NvSiplISPLocalAvgClipStatsData const *, 2> lacData;
    /**
     * Holds pointers to 2 LAC stats settings.
     */
    std::array<NvSiplISPLocalAvgClipStats const *, 2> lacSettings;
    /**
     * Holds pointers to 3 Histogram stats data.
     */
    std::array<NvSiplISPHistogramStatsData const *, 3> histData;
    /**
     * Holds pointers to 3 Histogram stats settings.
     */
    std::array<NvSiplISPHistogramStats const *, 3> histSettings;
};

/**
 * @brief Structure containing metadata info for
 * processing AE/AWB algorithm.
 */
struct SiplControlAutoMetadata {
    /**
     * @brief power factor for statistics compression
     * Valid Range: [0.5, 1.0]
     */
    float_t    alpha;

    /**
     * @brief A Boolean flag for notifying if it is first frame
     * for processing AE/AWB algorithm without statistics.
     * Valid Range: [true, false]
     */
    bool    isFirstFrame;

    /**
     * @brief Flag to skip AE processing for this frame.
     * When set to true, AE processing will be skipped to improve processing speed.
     * Valid Range: [true, false]
     */
    bool skipAEProcessing;

    /**
     * @brief Flag to skip AWB processing for this frame.
     * When set to true, AWB processing will be skipped to improve processing speed.
     * Valid Range: [true, false]
     */
    bool skipAWBProcessing;
};

/**
 * @brief Structure containing override control settings for ISP black level correction block
 */
struct IspBlcOverrideSettings {
    /**
     * Holds a Boolean to enable ISP black level correction block processing.
     */
    bool    enable;

    /**
     * @brief Holds black level correction values in RGGB/RCCB/RCCC component order
     * for each color component.
     * @li Supported values: [0.0, 1.0]
     */
    std::array<double_t, NVSIPL_ISP_MAX_COLOR_COMPONENT> pedestal;
};

/**
 * @brief Structure containing override control settings for ISP Histogram statistics block
 */
struct IspHistogramOverrideSettings {
    /**
     * Holds a Boolean to enable ISP histogram statistics block processing.
     */
    bool enable;

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
    bool ellipticalMaskEnable;

    /**
     * Holds an elliptical mask for excluding pixels outside a specified area.
     *
     * Coordinates of the image top left and bottom right points are (0, 0) and
     * (width, height), respectively.
     */
    NvSiplISPEllipse ellipticalMask;

    /**
     * Holds a Boolean to enable override of histogram knee and range values.
     */
    bool kneeRangesOverrideEnable;

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
};

using IspLacOverrideSettings = nvsipl::NvSiplISPLocalAvgClipStats;


/**
 * @brief Structure containing override control settings for ISP GTM block
 */
struct IspGtmOverrideSettings  {
    /**
     * Holds a Boolean to enable ISP GTM block processing.
     */
    bool enable;

    /**
     * Holds Number of GTM override control points.
     * Valid Range: [@ref ISP_GTM_PLUGIN_OVERRIDE_MIN_CTRLPOINTS, @ref ISP_GTM_PLUGIN_OVERRIDE_MAX_CTRLPOINTS]
     */
    uint32_t    numGtmCtrlPoints;

    /**
     * Holds ISP GTM block override control points.
     * Valid Range:
     * @li X coordinate of control point:  [0.0, 1.0]
     * @li Y coordinate of control point: [0.0, 1.0]
     */
    std::array<NvSiplPointDouble, ISP_GTM_PLUGIN_OVERRIDE_MAX_CTRLPOINTS> ctrlPoints;
};

/**
 * @brief Structure containing override control settings for
 * ISP Lens Shading Correction(LSC) block.
 */
struct IspLscOverrideSettings {
    /**
     * @brief Holds sensor crop ROI start position horizontal offset for LSC block.
     * This optical center horizontal offset will be added to the ISP input crop offset settings.
     * Valid Range: [0U, image width]
     */
    uint32_t sensorCropRoiStartX;

    /**
     * @brief Holds sensor crop ROI start position vertical offset for LSC block.
     * This optical center vertical offset will be added to the ISP input crop offset settings.
     * Valid Range: [0U, image height]
     */
    uint32_t sensorCropRoiStartY;
};

/**
 * @brief Structure to hold ISP noise reduction override settings.
 */
struct IspNrOverrideSettings {
    /**
     * Holds a Boolean to enable ISP noise reduction block processing.
     */
    bool enable;

    /**
     * Holds the noise reduction strength override value.
     * Valid Range: [0, 1]
     */
    float_t nrStrength;

};

/**
 * @brief Structure containing dynamic override ISP block settings to be applied during runtime.
 */
struct SiplControlIspOverrideSetting {
    /**
     * Holds a Boolean to enable/disable override of ISP BLC block settings.
     */
    bool overrideBlcSettings;

    /**
     * Holds override settings for ISP BLC block.
     * This field is valid only if overrideBlcSettings is true.
     */
    IspBlcOverrideSettings blcOverrideSettings;

    /**
     * Holds a Boolean to enable/disable override of ISP Histogram-0 block settings.
     * Valid Range: [true, false]
     */
    bool overrideHist0Settings;

    /**
     * Holds override settings for ISP Histogram-0 block.
     * This field is valid only if overrideHist0Settings is true.
     * Valid Range: See @ref IspHistogramOverrideSettings
     */
    IspHistogramOverrideSettings hist0OverrideSettings;

    /**
     * Holds a Boolean to enable/disable override of ISP Histogram-1 block settings.
     */
    bool overrideHist1Settings;

    /**
     * Holds override settings for ISP Histogram-1 block.
     * This field is valid only if overrideHist1Settings is true.
     * Valid Range: See @ref IspHistogramOverrideSettings
     */
    IspHistogramOverrideSettings hist1OverrideSettings;

    /**
     * Holds a Boolean to enable/disable override of ISP LAC-0 block settings.
     * Valid Range: [true, false]
     */
    bool overrideLac0Settings;

    /**
     * Holds override settings for ISP LAC-0 block.
     * This field is valid only if overrideLac0Settings is true.
     * Valid Range: See @ref IspLacOverrideSettings
     */
    IspLacOverrideSettings lac0OverrideSettings;

    /**
     * Holds a Boolean to enable/disable override of ISP LAC-1 block settings.
     * Valid Range: [true, false]
     */
    bool overrideLac1Settings;

    /**
     * Holds override settings for ISP LAC-1 block.
     * This field is valid only if overrideLac1Settings is true.
     * Valid Range: See @ref IspLacOverrideSettings
     */
    IspLacOverrideSettings lac1OverrideSettings;

    /**
     * Holds a Boolean to enable/disable override of AWB settings.
     * Valid Range: [true, false]
     */
    bool overrideAwbSettings;

    /**
     * Holds total Auto White Balance(AWB) settings
     * This field is valid only if overrideAwbSettings is true.
     * Valid Range: See @ref SiplControlAutoAwbSetting
     */
    SiplControlAutoAwbSetting       awbOverrideSettings;
    /**
     * Holds a Boolean to enable/disable override of ISP Global Tone Map(GTM) Settings.
     * Valid Range: [true, false]
     */
    bool overrideGtmSettings;

    /**
     * Holds override settings for ISP GTM block.
     * This field is valid only if overrideGtmSettings is true.
     * Valid Range: See @ref IspGtmOverrideSettings
     */
    IspGtmOverrideSettings gtmOverrideSettings;

    /**
     * Holds a Boolean to enable/disable override of ISP Lens Shading Correction(LSC) block settings.
     * Valid Range: [true, false]
     */
    bool overrideLscSettings;

    /**
     * Holds override settings for ISP LSC block.
     * This field is valid only if overrideLscSettings is true.
     * Valid Range: See @ref IspLscOverrideSettings
     */
    IspLscOverrideSettings lscOverrideSettings;

    /**
     * Holds a Boolean to enable/disable override of ISP Noise Reduction block settings.
     * Valid Range: [true, false]
     */
    bool overrideNrSettings;

    /**
     * Holds override settings for ISP Noise Reduction block.
     * This field is valid only if overrideNrSettings is true.
     * Valid Range: See @ref IspNrOverrideSettings
     */
    IspNrOverrideSettings nrOverrideSettings;

    /**
     * Holds a Boolean to enable/disable override of ISP Digital gain settings.
     * Valid Range: [true, false]
     */
    bool overrideIspDigitalGain;

    /**
     * Holds digital gain to be applied in ISP
     * This field is valid only if overrideIspDigitalGain is true.
     * Valid Range: [0.0, 8.0]
     */
    float_t      ispDigitalGainOverride;

    /**
     * Holds a Boolean to enable/disable override of scene brightness key value.
     * Valid Range: [true, false]
     */
    bool overrideSceneBrightnessKey;

    /**
     * Holds scene brightness key value to be used for computing
     * ISP scene adaptive block settings
     * This field is valid only if overrideSceneBrightnessKey is true.
     * Valid Range: [0.0, 1.0]
     */
    float_t      sceneBrightnessKeyOverrideValue;
};

/**
 * @brief Input parameters for processing AE/AWB
 */
struct SiplControlAutoInputParam {
    /**
     * Embedded settings
     */
    SiplControlEmbedData        embedData;
    /**
     * Sensor attributes
     */
    DevBlkCDISensorAttributes  sensorAttr;
    /**
     * Stats buffers and settings
     */
    SiplControlIspStatsInfo     statsInfo;
    /**
     * Metadata info for algorithm
     */
    SiplControlAutoMetadata   autoMetadata;
};

/**
 * @brief AE/AWB Output parameters
 */
struct SiplControlAutoOutputParam {
    /**
     * Sensor exposure and gain settings
     */
    SiplControlAutoSensorSetting    sensorSetting;
    /**
     * AWB settings
     */
    SiplControlAutoAwbSetting       awbSetting;
    /**
     * Digital gain to be applied in ISP
     * Valid Range: [0.0, 8.0]
     */
    float_t                         ispDigitalGain;
};


/** @} */

}  // namespace nvsipl

#endif /* NVSIPLCONTROLAUTODEF_HPP */
