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

#ifndef NVSIPLDEVICEBLOCKINFO_HPP
#define NVSIPLDEVICEBLOCKINFO_HPP

#include "NvSIPLCapStructs.h"

#include <cstdint>
#include <string>
#include <memory>
#include <vector>
#include <cmath>

/**
 * @file
 *
 * @brief <b> NVIDIA SIPL: DeviceBlock Information - @ref NvSIPLDevBlkInfo </b>
 *
 */

namespace nvsipl
{
/** @defgroup NvSIPLDevBlkInfo NvSIPL DeviceBlock Information
 *
 *  @brief Describes information about devices supported by SIPL Device Block.
 *
 *  @ingroup NvSIPLCamera_API
 */

 /** @addtogroup NvSIPLDevBlkInfo
 * @{
 */

/** @brief Indicates the maximum number of device blocks per platform. */
#if !NV_IS_SAFETY
static constexpr uint32_t MAX_DEVICEBLOCKS_PER_PLATFORM {6U};
#else
static constexpr uint32_t MAX_DEVICEBLOCKS_PER_PLATFORM {5U};
#endif

/** @brief Indicates the maximum number of camera modules per device block. */
static constexpr uint32_t MAX_CAMERAMODULES_PER_BLOCK {8U};

/**
 * @brief Indicates the maximum number of virtual channels per sensor.
 *
 * @note Considering that a single CSI port can handle up to 16 virtual channels, and
 *       a Deserializer may connect to 4 camera modules, there is a software-imposed
 *       restriction of 4 virtual channels per sensor to ensure the limit of
 *       16 virtual channels per CSI port is not surpassed.
 */
static constexpr uint32_t MAX_VIRTUAL_CHANNELS_PER_SENSOR {4U};

/** @brief Indicates the maximum number of camera modules per platform. */
static constexpr uint32_t MAX_CAMERAMODULES_PER_PLATFORM {MAX_DEVICEBLOCKS_PER_PLATFORM * MAX_CAMERAMODULES_PER_BLOCK};

/** @brief Indicates the maximum number of sensors per platform. */
static constexpr uint32_t MAX_SENSORS_PER_PLATFORM {MAX_CAMERAMODULES_PER_PLATFORM};

/** @brief Indicates the maximum number of CSI lane configurations */
static constexpr std::uint32_t MAX_CSI_LANE_CONFIGURATION {2U};

/** @brief Indicates the index for CSI 2 lanes */
static constexpr std::uint32_t X2_CSI_LANE_CONFIGURATION {0U};

/** @brief Indicates the index for CSI 4 lanes */
static constexpr std::uint32_t X4_CSI_LANE_CONFIGURATION {1U};

/** @brief Describes an Interrupt GPIO configuration */
struct IntrGpioInfo
{
    /**
     * @brief The interrupt GPIO index.
     *
     * GPIO indicies are defined in the platform's Device Block Device Tree configuration under the
     * "tegra/gpios" subnode. Device Block base nodes are identified by a compatible equal to
     * "nvidia,cdi-mgr".
     *
     * Valid value: [GPIO indice from the platform's Device Block Device Tree configuration]
     */
    uint32_t idx;
    /**
     * @brief Whether to enable driver error localization upon interrupt.
     */
    bool enableGetStatus;
    /**
     * @brief Error localization timeout duration [ms], 0 to disable.
     *
     * Valid value: [0, UINT32_MAX]
     */
    uint32_t timeout_ms;
};

/** @brief Defines the image sensor information. */
struct SensorInfo
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

    /** @brief Defines the user defined datatype capture settings. */
    struct UserDefDTCaptureSetting
    {
        /**
         * @brief Indicates whether the user defined datatype is enabled.
         *
         * The default value is false.
         */
        bool isDataTypeEnabled {false};
        /**
         * @brief Holds the height of the user defined datatype.
         *
         * Actual values will vary based on sensor HW and sensor driver support.
         *
         * Valid value: [0, UINT32_MAX]
         */
        uint32_t height {0U};
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
         * @brief Holds the virtual channel id.
         *
         * Actual values will vary based on sensor HW and sensor driver support.
         *
         * Valid value: [0, 15U]
         */
        uint32_t vcId {UINT32_MAX};
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
         * @brief Holds the @ref UserDefDTCaptureSetting for the virtual channel.
         */
        UserDefDTCaptureSetting userDefDTCapSetting;
    };

    /**
     * @brief Holds the identification of the pipeline index in the platform configuration.
     *
     * Valid value: [0, @ref MAX_CAMERAMODULES_PER_PLATFORM - 1]
     */
    uint32_t id = UINT32_MAX;
    /** @brief Holds the name of the image sensor, for example, "AR0231". */
    std::string name = "";
#if !NV_IS_SAFETY
    /** Holds the description of the image sensor. */
    std::string description = "";
#endif // !NV_IS_SAFETY
    /**
     * @brief Holds the native I2C address of the image sensor.
     *
     * Valid value: [0, UINT8_MAX]
     */
    uint8_t i2cAddress {static_cast<uint8_t>UINT8_MAX};
    /**
     * @brief Holds virtual channel information.
     *
     * @note This will be deprecated. Use @ref vcInfoList instead.
     */
    VirtualChannelInfo vcInfo;
    /**
     * @brief Holds a flag which indicates whether to use @ref vcInfoList instead of @ref vcInfo.
     *
     * @note Default value is false, this flag will be deprecated with @ref vcInfo.
     */
    bool useVcInfoList {false};
    /**
     * @brief Holds the number of virtual channels enabled for a sensor.
     *
     * Valid value: [1, @ref MAX_VIRTUAL_CHANNELS_PER_SENSOR]
     */
    uint32_t numVCs {1U};
    /** @brief Holds an array of virtual channel information per sensor. */
    VirtualChannelInfo vcInfoList[MAX_VIRTUAL_CHANNELS_PER_SENSOR];
    /**
     * @brief Holds a flag which indicates whether trigger mode is enabled.
     *
     * The default value is false.
     */
    bool isTriggerModeEnabled {false};
    /** @brief Holds Interrupt GPIO configurations for the sensor. */
    std::vector<IntrGpioInfo> errGpios {};
    /**
     * @brief Holds a flag which indicates whether CDI (Camera Device Interface) is using the new
     * version 2 API for device.
     *
     * The default value is false for version 1 API.
     *
     * @note The flag is only used for non-Safety build. For Safety build, CDI is always using
     * version 2 API.
     */
    bool useCDIv2API {false};
#if defined(NVMEDIA_QNX) || defined(NVMEDIA_LINUX)
    /**
     * @brief Holds a flag which indicates whether sensor itself and all data exchange with sensor
     * (image data and i2c communication) must be cryptographically authenticated.
     *
     * @note If sensor does not support authentication then no authentication will be performed
     * regardless of this flag's value.
     */
    bool isAuthEnabled {true};
#else
    /**
     * @brief Holds a flag which indicates whether sensor itself and all data exchange with sensor
     * (image data and i2c communication) must be cryptographically authenticated.
     *
     * @note This flag has no effect; authentication is not supported on Non-Embedded platforms.
     */
    bool isAuthEnabled {false};
#endif // NVMEDIA_QNX
#if !NV_IS_SAFETY
    /**
     * @brief Holds a flag which indicates whether the sensor requires configuration in Test Pattern
     * Generator (TPG) mode.
     *
     * The default value is false.
     */
    bool isTPGEnabled {false};
    /** @brief Holds the Test Pattern Generator (TPG) pattern mode. */
    uint32_t patternMode {0U};
#endif // !NV_IS_SAFETY
    /**
     * @brief Holds the requested virtual I2C address of the image sensor.
     *
     * @note: By default (e.g. left unset), SIPL will choose the virtual address.
     *
     * @note: On QNX, this address must be from the virtual address pool.
     *        Consult the Device Block Device Tree configuration SDK section for address ranges.
     *
     * Valid value: [0, UINT8_MAX]. Must be different from the @ref SensorInfo::i2cAddress value.
     */
    uint8_t virtualI2CAddress {static_cast<uint8_t>UINT8_MAX};

    /**
     * @brief Holds the number of capture contexts a sensor supports.
     *
     * Valid value: [1U, 4U]
     */
    uint16_t numContext {1U};
};

/** @brief Defines the EEPROM information. */
struct EEPROMInfo
{
    /** @brief Holds the name of the EEPROM, for example, "N24C64". */
    std::string name = "";
#if !NV_IS_SAFETY
    /** @brief Holds the description of the EEPROM. */
    std::string description = "";
#endif //!NV_IS_SAFETY
    /**
     * @brief Holds the native I2C address of the EEPROM.
     *
     * Valid value: [0, UINT8_MAX]
     */
    uint8_t i2cAddress {static_cast<uint8_t>UINT8_MAX};
    /**
     * @brief Holds a flag which indicates whether CDI (Camera Device Interface) is using the new
     * version 2 API for device.
     *
     * The default value is false for version 1 API.
     *
     * @note The flag is only used for non-Safety build. For Safety build, CDI is always using
     * version 2 API.
     */
    bool useCDIv2API {false};
    /**
     * @brief Holds the requested virtual I2C address of the EEPROM.
     *
     * @note: By default (e.g. left unset), SIPL will choose the virtual address.
     *
     * @note: On QNX, this address must be from the virtual address pool.
     *        Consult the Device Block Device Tree configuration SDK section for address ranges.
     *
     * Valid value: [0, UINT8_MAX]. Must be different from the @ref EEPROMInfo::i2cAddress value.
     */
    uint8_t virtualI2CAddress {static_cast<uint8_t>UINT8_MAX};
};

/** @brief Defines GPIO mapping from the serializer to the deserializer */
struct SerdesGPIOPinMap {
    /**
     * @brief The source multifunction pin (MFP) number on the serializer.
     *
     * MFP numbers are implementation defined based on the Serdes hardware topology.
     */
    uint8_t sourceGpio;
    /**
     * @brief The destination multifunction pin (MFP) number on the deserializer.
     *
     * MFP numbers are implementation defined based on the Serdes hardware topology.
     */
    uint8_t destGpio;
};

/** @brief Defines the serializer information. */
struct SerInfo
{
    /** @brief Holds the name of the serializer, for example, "MAX96705". */
    std::string name = "";
#if !NV_IS_SAFETY
    /** Holds the description of the serializer. */
    std::string description = "";
#endif // !NV_IS_SAFETY
    /**
     * @brief Holds the native I2C address.
     *
     * Valid value: [0, UINT8_MAX].
     */
    uint8_t i2cAddress {static_cast<uint8_t>UINT8_MAX};
#if !NV_IS_SAFETY
    /** Holds long cable support */
    bool longCable {false};
#endif // !NV_IS_SAFETY
    /** @brief Holds Interrupt GPIO configurations for the serializer. */
    std::vector<IntrGpioInfo> errGpios {};
    /**
     * @brief Holds a flag which indicates whether CDI (Camera Device Interface) is using the new
     * version 2 API for device.
     *
     * The default value is false for version 1 API.
     *
     * @note The flag is only used for non-Safety build. For Safety build, CDI is always using
     * version 2 API.
     */
    bool useCDIv2API {false};
    /** @brief Holds the information about GPIO mapping from the serializer to the deserializer. */
    std::vector<SerdesGPIOPinMap> serdesGPIOPinMappings {};
    /**
     * @brief Holds the requested virtual I2C address of the serializer.
     *
     * @note: By default (e.g. left unset), SIPL will choose the virtual address.
     *
     * @note: On QNX, this address must be from the virtual address pool.
     *        Consult the Device Block Device Tree configuration SDK section for address ranges.
     *
     * Valid value: [0, UINT8_MAX]. Must be different from the @ref SerInfo::i2cAddress value.
     */
    uint8_t virtualI2CAddress {static_cast<uint8_t>UINT8_MAX};
};

/** @brief Describes a single globally-visible crypto key object. */
class CryptoKeyInfo {
    /**
     * @brief Holds a purpose of the key. Supported purposes are specific to sensor model.
     *
     * The purpose is communicated to sensor driver which must know how to create and use a key of
     * the specified "purpose".
     */
    std::string keyPurpose;
    /**
     * @brief Holds the name of a key.
     *
     * The name is unique and global within a process. If the same name is specified for multiple
     * modules in the same config - the same key object will be shared.
     */
    std::string keyName;
    /**
     * @brief Numeric ID of a crypto channel which must be used for all operations using the key.
     *
     * List of supported crypto channel IDs are available in platform device tree, under compatible
     * "nvvseivccfg,channel-db". Each channel has an associated HW crypto resources allocated to it,
     * defined by platform configuration.
     *
     * Valid value: [Channel ID as defined in the platform device tree]
     */
    uint32_t cryptoChanID;
public:
    /**
     * @brief Create key info entry object with a specified purpose, name and channel ID.
     *
     * @param[in]   purpose Purpose of the key. See @ref CryptoKeyInfo::keyPurpose for details.
     * @param[in]   name    Name of the key. See @ref CryptoKeyInfo::keyName for details.
     * @param[in]   chanID  Cyrpto channel ID. See @ref CryptoKeyInfo::cryptoChanID for details.
     */
    CryptoKeyInfo(std::string const &purpose,
                  std::string const &name,
                  uint32_t const chanID):
                      keyPurpose(purpose), keyName(name), cryptoChanID(chanID) { };

    /**
     * @brief Returns a name of the key.
     *
     * @retval  name    The name of the key.
     */
    std::string const &name() const noexcept { return keyName; };
    /**
     * @brief Returns purpose of the key.
     *
     * @retval purpose  The purpose of the key.
     */
    std::string const &purpose() const noexcept { return keyPurpose; };
    /**
     * @brief Returns a crypto channel ID associated with the key.
     *
     * @retval channelID    The crypto channel ID of this key.
     */
    uint32_t chanID() const noexcept { return cryptoChanID; };
};

/**
 * @brief Defines the data transmission mode for serializer-deserializer link
 *
 * @note This enum is used to specify the data transmission mode for serializer-deserializer link.
 * @note The default value is PIXEL_MODE.
 * @note The PIXEL_MODE is used to process the CSI-2 packets and forward the video data to the deserializer.
 * @note The TUNNELING_MODE is used to bypass the video pipeline and forward the raw data directly to the deserializer.
 */
enum class SerdesDataTransmissionMode : uint8_t
{
    /** @brief Pixel mode - Serializer processes CSI-2 packets (default) */
    PIXEL_MODE = 0U,

    /** @brief Tunneling mode - Serializer bypasses video pipeline, forwards raw data */
    TUNNELING_MODE = 1U
};

/**
 * @brief Defines information for the camera module.
 *
 * A camera module is a physical grouping of a serializer, image sensor(s), and associated
 * EEPROM(s).
 */
struct CameraModuleInfo
{
    /** @brief Holds the name of the camera module, for example, "SF3324". */
    std::string name = "";
#if !NV_IS_SAFETY
    /** Holds the description of the camera module. */
    std::string description = "";
#endif // !NV_IS_SAFETY
    /**
     * @brief Holds the index of the deserializer link to which this module is connected.
     *
     * Valid value: [0, @ref MAX_CAMERAMODULES_PER_PLATFORM - 1]
     */
    uint32_t linkIndex {UINT32_MAX};
    /**
     *
     * @brief Holds a per-link flag which indicates whether simulator mode has been enabled for a
     * camera module.
     *
     * This is used for the ISP reprocessing use case to simulate the presence of a device block.
     *
     * @note The flag should be false for Safety build. For non-Safety build, true or false is allowed.
     */
    bool isSimulatorModeEnabled {false};
    /** @brief Holds the @ref SerInfo of the serializer. */
    SerInfo serInfo;
    /**
     * @brief Holds EEPROM support.
     *
     * EEPROM support varies based on sensor HW and sensor driver support.
     */
    bool isEEPROMSupported {false};
    /** @brief Holds the information about EEPROM device in a camera module. */
    EEPROMInfo eepromInfo;
    /** @brief Holds the information about the sensor in a camera module. */
    SensorInfo sensorInfo;
    /**
     * @brief Allows to specify crypto keys IDs used by the camera module driver and to associate
     * each key with specific crypto accelerator.
     *
     * Platform crypto accelerator resources (bandwidth, keyslots, max latency) are limited and this
     * configuration allows to distribute limited crypto resources among enabled cameras to satisfy
     * performance requirements for a specific usecase.
     *
     * Available HW resources are described in a platform Device Tree as a list of crypto
     * "channels". Channels can be distributed as needed between cameras allowing to share specific
     * HW engine resources or give resources exclusively to specific cameras.

     * Crypto hardware is usually used by camera drivers to accelerate cryptographic authentication
     * of cameras, camera images and control signals. Therefore keyList configuration is only
     * meaningful for camera drivers which support and implement authentication feature.
     *
     * If keyList configuration is not provided in this config, but authentication is enabled on a
     * camera - camera stack will use some default crypto settings which should enable
     * functionality, but may not give expected level of performance or latency. Key object sharing
     * between cameras will not be done if not explicitly requested in this keysList configuration.
     */
    std::vector<CryptoKeyInfo> cryptoKeysList {};

    /**
     * @brief Data transmission mode for the serializer-deserializer link.
     *
     * Specifies whether the link operates in pixel mode (default) or tunneling mode.
     *
     * Default: SerdesDataTransmissionMode::PIXEL_MODE
     */
    SerdesDataTransmissionMode dataTransmissionMode {SerdesDataTransmissionMode::PIXEL_MODE};
};

/** @brief Defines the deserializer information. */
struct DeserInfo
{
    /** @brief Holds the name of the deserializer, for example, "MAX96712". */
    std::string name = "";
#if !NV_IS_SAFETY
    /** Holds the description of the deserializer. */
    std::string description = "";
#endif // !NV_IS_SAFETY
    /**
     * @brief Holds the native I2C address of the deserializer.
     *
     * Valid value: [0, UINT8_MAX].
     */
    uint8_t i2cAddress {static_cast<uint8_t>UINT8_MAX};
    /** @brief Holds Interrupt GPIO configurations for the deserializer. */
    std::vector<IntrGpioInfo> errGpios {};
    /**
     * @brief Holds a flag which indicates whether CDI (Camera Device Interface) is using the new
     * version 2 API for device.
     *
     * The default value is false for version 1 API.
     *
     * @note The flag is only used for non-Safety build. For Safety build, CDI is always using
     * version 2 API.
     */
    bool useCDIv2API {false};
    /**
     * @brief Holds flag to indicate that the deserializer needs to run a reset all sequence at
     * startup.
     */
    bool resetAll {false};
};

/**
 * @brief Defines the DeviceBlock information.
 *
 * A DeviceBlock represents a grouping of a deserializer (which is connected to the SoC's CSI
 * interface) and the camera modules connected to the links of the deserializer.
 */
struct DeviceBlockInfo
{
    /**
     * @brief Holds the @ref NvSiplCapInterfaceType that specifies the CSI port of the SoC to which
     * the  deserializer is connected.
     */
    NvSiplCapInterfaceType csiPort {NVSIPL_CAP_CSI_INTERFACE_TYPE_CSI_A};
    /** @brief Holds the @ref NvSiplCapCsiPhyMode Phy mode. */
    NvSiplCapCsiPhyMode phyMode = NVSIPL_CAP_CSI_DPHY_MODE;
    /**
     * @brief Holds the I2C device bus number used to connect the deserializer with the SoC.
     *
     * Valid I2C device bus numbers are defined in the platform's Device Block Device Tree
     * configuration under the "tegra/i2c-bus" property. Device Block base nodes are identified by a
     * compatible equal to "nvidia,cdi-mgr".
     *
     * Valid value: [I2C device bus number from the platform's Device Block Device Tree configuration]
     */
    uint32_t i2cDevice {UINT32_MAX};
    /** @brief Holds the @ref DeserInfo deserializer information. */
    DeserInfo deserInfo;
    /**
     * @brief Holds the number of camera modules connected to the deserializer.
     *
     * Valid value: [1, @ref MAX_CAMERAMODULES_PER_BLOCK]
     */
    uint32_t numCameraModules {0U};
    /** @brief Holds an array of information about each camera module in the device block. */
    CameraModuleInfo cameraModuleInfoList[MAX_CAMERAMODULES_PER_BLOCK] = {};
    /**
     * @brief Holds the deserializer I2C port number connected with the SoC.
     *
     * The I2C port number is implementation defined based on what the deserializer supports.
     */
    std::uint32_t desI2CPort {UINT32_MAX};
    /**
     * @brief Holds the deserializer Tx port number connected with the SoC.
     *
     * The Tx port number is implementation defined based on the hardware topology.
     */
    std::uint32_t desTxPort {UINT32_MAX};
    /**
     * @brief Holds the power port number.
     *
     * The power port number is implementation defined based on the hardware topology.
     */
    std::uint32_t pwrPort = UINT32_MAX;
    /**
     * @brief Holds the deserializer's data rate in DPHY mode(kHz)
     *
     * Valid value: [Array of size @ref MAX_CSI_LANE_CONFIGURATION with individual DPHY data rates
     *               for each lane configuration ranging from 0 to UINT32_MAX kHz]
     */
    std::uint32_t dphyRate[MAX_CSI_LANE_CONFIGURATION] = {0U};
    /**
     * @brief Holds the deserializer's data rate in CPHY mode(ksps)
     *
     * Valid value: [Array of size @ref MAX_CSI_LANE_CONFIGURATION with individual CPHY data rates
     *               for each lane configuration ranging from 0 to UINT32_MAX kHz]
     */
    std::uint32_t cphyRate[MAX_CSI_LANE_CONFIGURATION] = {0U};
    /**
     * @brief Holds a flag which indicates whether passive mode must be enabled.
     *
     * Used when an NVIDIA DRIVE&trade; AGX SoC connected to the deserializer does not have an I2C
     * connection to control it.
     */
    bool isPassiveModeEnabled {false};
    /** @brief Holds a flag which indicates whether group initialization is enabled. */
    bool isGroupInitProg {false};
    /**
     * @brief Holds non-error monitoring GPIO indices for the Device Block.
     *
     * GPIO indicies are defined in the platform's Device Block Device Tree configuration under the
     * "tegra/gpios" subnode. Device Block base nodes are identified by a compatible equal to
     * "nvidia,cdi-mgr".
     *
     * Valid value: [GPIO indices from the platform's Device Block Device Tree configuration]
     */
    std::vector<uint32_t> gpios {};
#if !NV_IS_SAFETY
    /** @brief Holds a flag which indicates whether power control is disabled on the platform. */
    bool isPwrCtrlDisabled {false};
    /** @brief Holds long cable support */
    bool longCables[MAX_CAMERAMODULES_PER_BLOCK] = {false};
#endif // !NV_IS_SAFETY
    /** @brief Reset all sequence is needed when starting deserializer. */
    bool resetAll;
    /**
     * @brief Tag for the boot initialization sequence expected to have been executed by the camera
     * boot accelerator.
     *
     * By default the boot accelerator tag is an empty string. This string must remain empty unless
     * the camera boot accelerator has been configured for the desired use case and Device Block.
     *
     * This property is only valid for QNX.
     */
    std::string bootAcceleratorTag = "";
    /**
     * @brief When the camera boot accelerator has been configured, begin streaming on camera
     * modules one-by-one in the order they appear in the platform configuration.
     *
     * By default this behavior is disabled - all camera modules will begin streaming at the same
     * time.
     *
     * This flag will not affect the behavior of the @ref isGroupInitProg flag when initialization
     * is being performed.
     *
     * This flag has no effect if the @ref bootAcceleratorTag is not set.
     */
    bool bootAcceleratorUseStaggeredStart = {false};
};

/**
 * @brief Init Error Codes
 */
enum class InitErrorCode : uint32_t {
    /**
     * Default Init Error Code None
     */
    DEFAULT_INIT_ERROR_CODE_NONE = 0U,
    /**
     * Pipeline recovered from init error on retry
     */
    PIPELINE_RECOVERED_FROM_INIT_ERROR_ON_RETRY = 1U,

    // CBA operation failures
    /**
     * CBA Failed
     */
    CBA_FAILURE = 10U,

    // DDI API based failures
    /**
     * Deserializer Power Failure causing Initialization issue in the camera pipeline
     */
    DESER_POWER_FAILURE = 100U,
    /**
     * Deserializer Init Failure causing failure in the camera pipeline
     */
    DESER_INIT_FAILURE = 101U,
    /**
     * Deserializer Link Enable Failure causing failure in the camera pipeline
     */
    DESER_EN_LINK_FAILURE = 102U,
    /**
     * Deserializer Start Failure causing failure in the camera pipeline
     */
    DESER_START_FAILURE = 103U,
    /**
     * Deserializer Check Link Lock Failure causing failure in the camera pipeline
     */
    DESER_CHECK_LINK_LOCK_FAILURE = 104U,
    /**
     * Camera module Power Failure causing Initialization issue in the camera pipeline
     */
    CAMERA_POWER_FAILURE = 200U,
    /**
     * Camera module Init Failure causing failure in the camera pipeline
     */
    CAMERA_INIT_FAILURE = 201U,
    /**
     * Camera module Link Detect Failure causing failure in the camera pipeline
     */
    CAMERA_EN_LINK_DETECT_FAILURE = 202U,
    /**
     * Camera module Authentication Failure causing failure in the camera pipeline
     */
    CAMERA_AUTH_FAILURE = 203U,
    /**
     * Camera module Post Init Failure causing failure in the camera pipeline
     */
    CAMERA_POST_INIT_FAILURE = 204U,
    /**
     * Camera module Start Failure causing failure in the camera pipeline
     */
    CAMERA_START_FAILURE = 205U,
    /**
     * Camera module Post Start Failure causing failure in the camera pipeline
     */
    CAMERA_POST_START_FAILURE = 206U,

    // Power based failures
    /**
     * Camera Over Voltage (Short to Battery)
     */
    CAMERA_OVER_VOLTAGE = 300U,
    /**
     * Camera Under Voltage
     */
    CAMERA_UNDER_VOLTAGE = 301U,
    /**
     * Line to Line Fault
     */
    CAMERA_LINE_TO_LINE_FAULT = 302U,
    /**
     * Camera Over Current (Short to Ground)
     */
    CAMERA_OVER_CURRENT = 303U,
    /**
     * Camera Thermal Shutdown
     */
    CAMERA_THERMAL_SHUTDOWN = 304U,
    /**
     * Circuit Open/Disconnection
     */
    CAMERA_CIRCUIT_OPEN_DISCONNECTION = 305U,
    /**
     * Incorrect Impedance
     */
    CAMERA_INCORRECT_IMPEDANCE = 306U,
};

/** @} */

} // namespace nvsipl

#endif //NVSIPLDEVICEBLOCKINFO_HPP
