/*
 * SPDX-FileCopyrightText: Copyright (c) 2025-2026 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 * SPDX-License-Identifier: LicenseRef-NvidiaProprietary
 *
 * NVIDIA CORPORATION, its affiliates and licensors retain all intellectual
 * property and proprietary rights in and to this material, related
 * documentation and any modifications thereto. Any use, reproduction,
 * disclosure or distribution of this material and related documentation
 * without an express license agreement from NVIDIA CORPORATION or
 * its affiliates is strictly prohibited.
 */

#ifndef NVSIPLCAMERATYPES_HPP
#define NVSIPLCAMERATYPES_HPP

#include <array>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <limits>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#include "NvSIPLCapStructs.h"

namespace nvsipl::sensorconfig {

using MacAddress = std::array<uint8_t, 6>;

/**
 * @brief Describes an Interrupt GPIO configuration.
 */
struct InterruptGpioInfo
{
    /**
     * @brief The interrupt GPIO index.
     *
     * GPIO indices are defined in the platform configuration.
     */
    uint32_t idx {UINT32_MAX};

    /**
     * @brief Whether to enable driver error localization upon interrupt.
     */
    bool enableGetStatus {false};

    /**
     * @brief Error localization timeout duration.
     *
     * Set to zero to disable timeout.
     */
    std::chrono::milliseconds timeout {0};
};

/**
 * @brief Defines GPIO mapping from the serializer to the deserializer.
 */
struct SerdesGPIOPinMap
{
    /**
     * @brief The source multifunction pin (MFP) number on the serializer.
     */
    uint8_t sourceGpio {UINT8_MAX};

    /**
     * @brief The destination multifunction pin (MFP) number on the deserializer.
     */
    uint8_t destGpio {UINT8_MAX};
};

/**
 * @brief Information about the I2C address of a device.
 */
struct I2CAddressInfo
{
    using Address = uint16_t;
    static constexpr Address INVALID {std::numeric_limits<Address>::max()};

    /**
     * @brief Native I2C address of the device.
     *
     * Valid value: [0, 0x7F] for 7-bit addressing, [0, 0x3FF] for 10-bit addressing
     */
    Address i2cAddress {INVALID};

    /**
     * @brief Requested virtual I2C address of the device.
     *
     * Valid value: [0, 0x7F] for 7-bit addressing. If specified, must be different
     * than the @ref i2cAddress value.
     */
    std::optional<Address> virtualI2CAddress;
};

/**
 * @brief Common information for all sensors.
 */
struct CommonSensorConfig
{
    /** @brief Defines the image resolution. */
    struct Resolution
    {
        /**
         * @brief Holds the width in pixels.
         *
         * Actual values will vary based on sensor HW and sensor driver support.
         *
         * Valid value: [@ref NVSIPL_CAP_MIN_IMAGE_WIDTH, @ref NVSIPL_CAP_MAX_IMAGE_WIDTH]
         */
        uint32_t width {0U};

        /**
         * @brief Holds the height in pixels.
         *
         * Actual values will vary based on sensor HW and sensor driver support.
         *
         * Valid value: [@ref NVSIPL_CAP_MIN_IMAGE_HEIGHT, @ref NVSIPL_CAP_MAX_IMAGE_HEIGHT]
         */
        uint32_t height {0U};
    };

    /** @brief Defines the user defined datatype capture configuration. */
    struct UserDataTypeCaptureConfig
    {
        /**
         * @brief Holds the number of lines of the user defined datatype.
         *
         * Actual values will vary based on sensor HW and sensor driver support.
         *
         * Valid value: [0, UINT32_MAX]
         */
        uint32_t lines {0U};

        /**
         * @brief Holds the input format.
         *
         * Actual values will vary based on sensor HW and sensor driver support.
         *
         * Valid value: [@ref NvSiplCapInputFormatType]
         */
        NvSiplCapInputFormatType inputFormat {NVSIPL_CAP_INPUT_FORMAT_TYPE_USER_DEFINED_8};

        /**
         * @brief Holds the bits per pixel.
         *
         * Valid value: [@ref NvSiplBitsPerPixel]
         */
        NvSiplBitsPerPixel bitsPerPixel {NVSIPL_BITS_PER_PIXEL_8};
    };

    /**
     * @brief Defines the information of a virtual channel/single exposure.
     */
    struct VirtualChannelInfo
    {
        /**
         * @brief Holds the source virtual channel id.
         *
         * Actual values will vary based on sensor HW and sensor driver support.
         *
         * Valid value: [0, 15U]
         */
        uint32_t vcIdSrc {UINT32_MAX};

        /**
         * @brief Holds the destination virtual channel id.
         *
         * Actual values will vary based on sensor HW and sensor driver support.
         *
         * Valid value: [0, 15U]
         */
        uint32_t vcIdDst {UINT32_MAX};

        /**
         * @brief Holds the Bayer color filter array order of the sensor.
         *
         * NVIDIA recommends setting this element with the NVSIPL_PIXEL_ORDER_* family of macros,
         * e.g. @ref NVSIPL_PIXEL_ORDER_ALPHA.
         *
         * Actual values will vary based on sensor HW and sensor driver support.
         *
         * Valid value: [@ref NVSIPL_PIXEL_ORDER_*]
         */
        uint32_t cfa {0U};

        /**
         * @brief Holds the number of top embedded lines.
         *
         * Actual values will vary based on sensor HW and sensor driver support.
         *
         * Valid value: [0, UINT32_MAX]
         */
        uint32_t embeddedTopLines {UINT32_MAX};

        /**
         * @brief Holds the number of bottom embedded lines.
         *
         * Actual values will vary based on sensor HW and sensor driver support.
         *
         * Valid value: [0, UINT32_MAX]
         */
        uint32_t embeddedBottomLines {UINT32_MAX};

        /**
         * @brief Holds the input format.
         *
         * Actual values will vary based on sensor HW and sensor driver support.
         *
         * Valid value: [@ref NvSiplCapInputFormatType]
         */
        NvSiplCapInputFormatType inputFormat {NVSIPL_CAP_INPUT_FORMAT_TYPE_RAW12};

        /**
         * @brief Holds the @ref Resolution of the captured frame.
         */
        Resolution resolution;

        /**
         * @brief Holds the average number of frames per second.
         *
         * Actual values will vary based on sensor HW and sensor driver support.
         *
         * Valid value: [@ref NVSIPL_CAP_MIN_FRAME_RATE, @ref NVSIPL_CAP_MAX_FRAME_RATE].
         */
        float_t fps {0.0F};

        /**
         * @brief Indicates whether the embedded data is coming in CSI packet with different data.
         *
         * Embedded data enablement will vary based on the sensor HW and sensor driver support.
         *
         * The default value is false.
         */
        bool isEmbeddedDataTypeEnabled {false};

        /**
         * @brief Optional user defined datatype capture configuration.
         *
         * When set, enables capture of user defined datatype for this virtual channel.
         * When nullopt, user defined datatype capture is disabled.
         */
        std::optional<UserDataTypeCaptureConfig> userDataTypeCapture;
    };

    /**
     * @brief Holds the identification of the pipeline index in the platform configuration.
     *
     * Valid value: [0, @ref MAX_CAMERAMODULES_PER_PLATFORM - 1]
     */
    uint32_t id {UINT32_MAX};

    /** @brief Holds the name of the sensor, for example, "AR0231". */
    std::string name;

    /** @brief Holds the description of the sensor. */
    std::string description;

    /**
     * @brief Holds I2C address information for the sensor.
     */
    I2CAddressInfo address;

    /** @brief Holds information on all virtual channels per sensor. */
    std::vector<VirtualChannelInfo> vcInfoList;

    /**
     * @brief Holds a flag which indicates whether trigger mode is enabled.
     */
    bool isTriggerModeEnabled {false};

    /** @brief Holds Interrupt GPIO configurations for the sensor. */
    std::vector<InterruptGpioInfo> errGpios;

    /**
     * @brief Holds the number of capture contexts a sensor supports.
     *
     * Valid value: [1U, 4U]
     */
    uint16_t numContext {1U};

    /**
     * @brief Sensors with the same sensorGroup are grouped into one logical camera.
     *
     * Valid value: [0, UINT32_MAX]
     */
    uint32_t sensorGroup {0};

    /**
     * @brief Device index of this sensor.
     *
     * If there are multiple sensors of the same type in a single module,
     * this index will be used to identify the sensor. This index must be
     * consistent with the driver's understanding of device index.
     */
    uint8_t deviceIndex {0};

    CommonSensorConfig& operator=(CommonSensorConfig const&) & = default;
};

/** @brief Defines the EEPROM information. */
struct EEPROMConfig
{
    /** @brief Holds the name of the EEPROM, for example, "N24C64". */
    std::string name;

#if !NV_IS_SAFETY
    /** @brief Holds the description of the EEPROM. */
    std::string description;
#endif // !NV_IS_SAFETY

    /**
     * @brief Holds I2C address information for the EEPROM.
     */
    I2CAddressInfo address;

    EEPROMConfig& operator=(EEPROMConfig const&) & = default;
};

/** @brief Defines the serializer information. */
struct SerializerConfig
{
    /** @brief Holds the name of the serializer, for example, "MAX96705". */
    std::string name;

#if !NV_IS_SAFETY
    /** @brief Holds the description of the serializer. */
    std::string description;
#endif // !NV_IS_SAFETY

    /**
     * @brief Holds I2C address information for the serializer.
     */
    I2CAddressInfo address;

    /** @brief Holds Interrupt GPIO configurations for the serializer. */
    std::vector<InterruptGpioInfo> errGpios;

    /** @brief Holds the information about GPIO mapping from the serializer to the deserializer. */
    std::vector<SerdesGPIOPinMap> serdesGPIOPinMappings;

    SerializerConfig& operator=(SerializerConfig const&) & = default;
};

/**
 * Settings for an individual sensor within a camera module.
 */
struct CoECameraSensorConfig: public CommonSensorConfig
{
    /**
     * @brief MAC address of the sensor.
     *
     * Used for identifying the sensor on the Ethernet network.
     */
    MacAddress macAddress;

    /**
     * @brief Optional IP address for this sensor's data path.
     *
     * Only required for non-1722 datapath where HSB uses different IP addresses
     * to communicate with different sensors.
     * Set to 0 if using 1722 datapath or if sensor uses HSB's IP.
     */
    uint32_t ipAddress {0};

    CoECameraSensorConfig& operator=(CoECameraSensorConfig const&) & = default;
};

/**
 * @brief Variant type for all CoE sensor types.
 *
 * Currently only CoECameraSensorConfig is supported.
 * Additional sensor types may be added in future releases.
 */
using CoESensorConfig = std::variant<CoECameraSensorConfig>;

/**
 * @brief Settings for a camera module connected to an HSB.
 */
struct CoEModule
{
    /**
     * @brief Configuration for each sensor in this camera module.
     */
    std::vector<CoESensorConfig> sensorConfigs;

    /**
     * @brief Whether simulator mode is enabled for this module.
     *
     * When true, no hardware access is performed — used for
     * reprocessing raw frames from file without an actual HSB or
     * Ethernet sensor connected.
     */
    bool isSimulatorModeEnabled {false};

    CoEModule& operator=(CoEModule const&) & = default;
};

/**
 * @brief Settings for the HSB (Holoscan Sensor Bridge).
 */
struct CoETransportConfig
{
    /** @brief Ethernet interface name on the host. */
    std::string interfaceName;

    /**
     * @brief IP address of the HSB.
     */
    uint32_t ipAddress {0};

    /**
     * @brief Flag indicating if VLAN tagging is enabled.
     *
     * Set to true if the network requires VLAN tagging.
     */
    bool vlanEnable {false};

    /**
     * @brief Whether multiple sensors on this HSB should be synchronized.
     */
    bool syncSensors {true};

    CoETransportConfig& operator=(CoETransportConfig const&) & = default;
};

/**
 * @brief Defines GMSL power control information for both module and deserializer power management.
 */
struct GmslPowerControlConfig
{
    /**
     * @brief Module power control configuration.
     */
    struct ModuleInfo
    {
        /** @brief Name of the module power control device. */
        std::string name;

        /** @brief I2C address information for the module power control device. */
        I2CAddressInfo address;

        ModuleInfo& operator=(ModuleInfo const&) & = default;
    } moduleInfo;

    /**
     * @brief Deserializer power control configuration.
     */
    struct DeserializerInfo
    {
        /** @brief Name of the deserializer power control device. */
        std::string name;

        /**
         * @brief GPIO pin index for direct GPIO power control (optional).
         *
         * Only used when deserializer power control device uses GPIO backend.
         */
        std::optional<uint32_t> gpioPinIndex;

        /** @brief I2C address information for the deserializer power control device. */
        I2CAddressInfo address;

        DeserializerInfo& operator=(DeserializerInfo const&) & = default;
    } deserializerInfo;

    GmslPowerControlConfig& operator=(GmslPowerControlConfig const&) & = default;
};

/**
 * @brief Defines GMSL link mode options for deserializer-serializer connections.
 */
enum class GmslLinkMode : uint8_t {
    LINK_MODE_NONE = 0,       ///< No link mode (e.g., TPG mode)
    LINK_MODE_GMSL1,          ///< GMSL version 1
    LINK_MODE_GMSL2_6GBPS,    ///< GMSL version 2, 6GBPS mode
    LINK_MODE_GMSL2_3GBPS,    ///< GMSL version 2, 3GBPS mode
    LINK_MODE_GMSL3,          ///< GMSL version 3
    LINK_MODE_GMSL3_GMSL2,    ///< GMSL3 in deserializer, GMSL2 in serializer
};

/**
 * @brief Defines GMSL frame sync mode options for deserializer.
 */
enum class GmslFsyncMode : uint8_t {
    FSYNC_INVALID = 0,    ///< Invalid FSYNC mode
    FSYNC_OSC_MANUAL,     ///< Oscillator Manual (internal)
    FSYNC_EXTERNAL,       ///< External FSYNC from Tegra
};

struct GmslCameraSensorConfig: public CommonSensorConfig
{
    GmslCameraSensorConfig& operator=(GmslCameraSensorConfig const&) & = default;
};

/**
 * @brief Variant type for all GMSL sensor types.
 *
 * Currently only GmslCameraSensorConfig is supported.
 * Additional sensor types may be added in future releases.
 */
using GmslSensorConfig = std::variant<GmslCameraSensorConfig>;

struct GmslModule
{
    /** @brief Deserializer link index for this module. */
    uint32_t linkIndex {UINT32_MAX};

    /** @brief Whether simulator mode is enabled for this module. */
    bool isSimulatorModeEnabled {false};

    /** @brief GMSL link mode for serializer communication. Defaults to GMSL2 6GBPS. */
    GmslLinkMode linkMode {GmslLinkMode::LINK_MODE_GMSL2_6GBPS};

    /** @brief Frame sync mode for this module. Defaults to external. */
    GmslFsyncMode fsyncMode {GmslFsyncMode::FSYNC_EXTERNAL};

    /** @brief Serializer configuration for this module. */
    SerializerConfig serializerConfig;

    /** @brief EEPROM configurations for this module. */
    std::vector<EEPROMConfig> eepromConfigs;

    /** @brief Sensor configurations for this module. */
    std::vector<GmslSensorConfig> sensorConfigs;

    GmslModule& operator=(GmslModule const&) & = default;
};

struct GmslDeserializerConfig
{
    /** @brief Name of the deserializer driver (e.g., "Max96724GmslDeserializer"). */
    std::string name;

    /** @brief I2C address information for the deserializer. */
    I2CAddressInfo address;

    /** @brief Interrupt GPIO configurations for the deserializer. */
    std::vector<InterruptGpioInfo> errGpios;

    /** @brief Default constructor */
    GmslDeserializerConfig() = default;

    /** @brief Copy constructor */
    GmslDeserializerConfig(GmslDeserializerConfig const&) = default;

    /** @brief Move constructor */
    GmslDeserializerConfig(GmslDeserializerConfig&&) = default;

    /** @brief Copy assignment operator */
    GmslDeserializerConfig& operator=(GmslDeserializerConfig const&) & = default;

    /** @brief Move assignment operator */
    GmslDeserializerConfig& operator=(GmslDeserializerConfig&&) & = default;

    /** @brief Destructor */
    ~GmslDeserializerConfig() = default;
};

struct GmslTransportConfig
{
    /** @brief CSI port connected to this deserializer. */
    NvSiplCapInterfaceType csiPort {NVSIPL_CAP_CSI_INTERFACE_TYPE_CSI_A};

    /** @brief Deserializer configuration. */
    GmslDeserializerConfig deserializerConfig;

    /** @brief Power control configuration. */
    GmslPowerControlConfig powerControlConfig;

    /** @brief I2C device bus number. */
    uint32_t i2cDevice {UINT32_MAX};

    /** @brief Deserializer I2C port number. */
    uint32_t deserializerI2CPort {UINT32_MAX};

    /** @brief Deserializer Tx port number. */
    uint32_t deserializerTxPort {UINT32_MAX};

    /** @brief Power port number. */
    uint32_t powerPort {UINT32_MAX};

    /** @brief Whether passive mode is enabled. */
    bool isPassiveModeEnabled {false};

    /**
     * @brief PHY replication entry for CSI data cloning between TX ports.
     *
     * Each entry maps a source PHY to a destination PHY. The deserializer
     * hardware supports up to two replication channels.
     */
    struct ReplicationEntry {
        uint32_t srcPhy {UINT32_MAX};  ///< Source PHY index
        uint32_t dstPhy {UINT32_MAX};  ///< Destination PHY index
    };
    std::vector<ReplicationEntry> replicationInfo {};  ///< Optional PHY replication pairs (max 2)

    /** @brief Boot accelerator tag. */
    std::string bootAcceleratorTag;

    /** @brief Whether to use staggered start with boot accelerator. */
    bool bootAcceleratorUseStaggeredStart {false};

    GmslTransportConfig& operator=(GmslTransportConfig const&) & = default;
};

/**
 * @brief Enum for MIPI lane polarity options.
 */
enum class NvSiplMipiPolarity : uint8_t
{
    POLARITY_ABC = 0,  ///< Default polarity configuration (A-B-C for CPHY, no swap for DPHY)
    POLARITY_ACB = 1,  ///< A-C-B polarity configuration for CPHY, swap polarity for DPHY
    POLARITY_BCA = 2,  ///< B-C-A polarity configuration for CPHY
    POLARITY_BAC = 3,  ///< B-A-C polarity configuration for CPHY
    POLARITY_CAB = 4,  ///< C-A-B polarity configuration for CPHY
    POLARITY_CBA = 5,  ///< C-B-A polarity configuration for CPHY
    POLARITY_MAX       ///< Max enum value (used for bounds checking)
};

/**
 * @brief Enum for MIPI lane swizzle patterns.
 */
enum class NvSiplMipiSwizzle : uint8_t
{
    LANE_SWIZZLE_A0A1B0B1 = 0,  ///< Lane swizzle pattern A0A1B0B1 (default)
    LANE_SWIZZLE_A0A1B1B0 = 1,  ///< Lane swizzle pattern A0A1B1B0
    LANE_SWIZZLE_A0B0B1A1 = 2,  ///< Lane swizzle pattern A0B0B1A1
    LANE_SWIZZLE_A0B0A1B1 = 3,  ///< Lane swizzle pattern A0B0A1B1
    LANE_SWIZZLE_A0B1A1B0 = 4,  ///< Lane swizzle pattern A0B1A1B0
    LANE_SWIZZLE_A0B1B0A1 = 5,  ///< Lane swizzle pattern A0B1B0A1
    LANE_SWIZZLE_MAX            ///< Max enum value (used for bounds checking)
};

/**
 * @brief MIPI configuration parameters for camera connections.
 */
struct MipiSettings
{
    /**
     * @brief Number of data lanes used.
     *
     * Valid range: [1, 4]
     */
    uint8_t lanes {4};

    /**
     * @brief LP bypass mode.
     */
    bool lpBypassMode {false};

    /**
     * @brief MIPI TCLK-SETTLE timing.
     *
     * Valid range: [0, UINT8_MAX]
     */
    uint8_t tClkSettle {0};

    /**
     * @brief MIPI clock rate for D-Phy. Symbol rate for C-Phy.
     *
     * Valid range for DPHY in kbps: [40000, 1250000]
     * Valid range for CPHY in ksps: [80000, 3000000]
     */
    uint32_t mipiClockRate {0};

    /**
     * @brief THS settle time of the MIPI lane, in nanoseconds.
     *
     * If set to 0, the driver attempts to auto-calibrate according to the mclk_multiplier parameter.
     * Calculate the range with:
     * 85 ns + 6×ui < (tHsSettle+6) × (lp_clock_period) < 145 ns + 10×ui
     * Where:
     * - lp_clock_period is 1/(204 MHz)
     * - ui is the unit interval (duration of HS state on clock lane)
     */
    uint8_t tHsSettle {0};

    /**
     * @brief Lane polarity configuration for each lane/trio.
     *
     * Index corresponds to lane/trio number (0-3):
     * - lanePolarity[0]: Lane/Trio 0 polarity
     * - lanePolarity[1]: Lane/Trio 1 polarity
     * - lanePolarity[2]: Lane/Trio 2 polarity
     * - lanePolarity[3]: Lane/Trio 3 polarity
     *
     * For DPHY: Values are 0 (no swap) or 1 (swap)
     * For CPHY: Each trio can have a polarity configuration 0-5:
     *   - 0: A-B-C (default configuration)
     *   - 1: A-C-B
     *   - 2: B-C-A
     *   - 3: B-A-C
     *   - 4: C-A-B
     *   - 5: C-B-A
     *
     * See NvSiplMipiPolarity enum for reference values.
     */
    std::array<NvSiplMipiPolarity, 4> lanePolarity {{
        NvSiplMipiPolarity::POLARITY_ABC,
        NvSiplMipiPolarity::POLARITY_ABC,
        NvSiplMipiPolarity::POLARITY_ABC,
        NvSiplMipiPolarity::POLARITY_ABC
    }};

    /**
     * @brief Lane swizzle pattern.
     *
     * Controls the ordering/remapping of physical MIPI lanes.
     * Used to accommodate different PCB routing constraints and hardware designs.
     *
     * The pattern defines how logical lanes (A0,A1,B0,B1) are mapped to
     * physical lanes. For example:
     * - LANE_SWIZZLE_A0A1B0B1 (0): Standard ordering (A0→A0, A1→A1, B0→B0, B1→B1)
     * - LANE_SWIZZLE_A0B0A1B1 (3): Alternative (A0→A0, A1→B0, B0→A1, B1→B1)
     *
     * The swizzle pattern applies to all lanes as a group and depends on the number
     * of active lanes specified in the 'lanes' field.
     *
     * @note For 2-lane configurations, only the first two positions in the swizzle
     * pattern are used. For 4-lane configurations, all positions are relevant.
     */
    NvSiplMipiSwizzle laneSwizzle {NvSiplMipiSwizzle::LANE_SWIZZLE_A0A1B0B1};

    /**
     * @brief D-PHY clock rate in kbps.
     *
     * Valid range for DPHY in kbps: [40000, 1250000]
     */
    uint32_t dphyRate {0};

    /**
     * @brief C-PHY symbol rate in ksps.
     *
     * Valid range for CPHY in ksps: [80000, 3000000]
     */
    uint32_t cphyRate {0};

    /**
     * @brief PHY mode (D-PHY or C-PHY).
     */
    NvSiplCapCsiPhyMode phyMode {NVSIPL_CAP_CSI_DPHY_MODE};
};

using ModuleType    = std::variant<CoEModule, GmslModule>;
using TransportType = std::variant<CoETransportConfig, GmslTransportConfig>;

struct ModuleConfig
{
    /** @brief Name of the module. */
    std::string name;

    /** @brief Description of the module. */
    std::string description;

    /** @brief ID of the transport that this module is connected to. */
    uint32_t transportId {UINT32_MAX};

    /** @brief Module type configuration. */
    ModuleType moduleType;

    /** @brief MIPI settings for this module. */
    MipiSettings mipiSettings;

    /** @brief Platform name this module is associated with (e.g., "ferrix", "kenari"). */
    std::string platform;

    /** @brief Platform configuration identifier. */
    std::string platformConfig;

    ModuleConfig& operator=(ModuleConfig const&) & = default;
};

struct TransportConfig
{
    /** @brief Name of the transport. */
    std::string name;

    /** @brief Description of the transport. */
    std::string description;

    /** @brief Unique ID for this transport. */
    uint32_t transportId {UINT32_MAX};

    /** @brief Transport type configuration. */
    TransportType transportType;

    TransportConfig& operator=(TransportConfig const&) & = default;
};

/**
 * @brief Combined structure describing module configurations and transport configurations.
 */
struct SensorSystemConfig
{
    /** @brief Module configurations. */
    std::vector<ModuleConfig>    modules;

    /** @brief Transport configurations. */
    std::vector<TransportConfig> transports;

    SensorSystemConfig& operator=(SensorSystemConfig const&) & = default;
};

} // namespace nvsipl::sensorconfig

#endif // NVSIPLCAMERATYPES_HPP
