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

#ifndef NVSIPL_CAMERA_QUERY_HPP
#define NVSIPL_CAMERA_QUERY_HPP

#include "NvSIPLCommon.hpp"
#include "NvSIPLCameraTypes.hpp"
#include <memory>
#include <vector>
#include <string>

namespace nvsipl {

/**
 * @brief Defines a list of all external image devices supported by SIPL
 *
 * This structure provides comprehensive device information including
 * complete module/transport configurations for both CoE and GMSL camera systems.
 */
struct CameraDeviceInfoList {
    /** Holds a vector of @ref sensorconfig::ModuleConfig objects for Camera-over-Ethernet
     * (CoE) modules. */
    std::vector<sensorconfig::ModuleConfig> coeModuleList;
    /** Holds a vector of @ref sensorconfig::ModuleConfig objects for GMSL modules. */
    std::vector<sensorconfig::ModuleConfig> gmslModuleList;
    /** Holds a vector of @ref sensorconfig::TransportConfig objects for
     * Camera-over-Ethernet transport configurations. */
    std::vector<sensorconfig::TransportConfig> coeTransportList;
    /** Holds a vector of @ref sensorconfig::TransportConfig objects for GMSL transport
     * configurations. */
    std::vector<sensorconfig::TransportConfig> gmslTransportList;
    /** Holds a @ref sensorconfig::SensorSystemConfig object. */
    sensorconfig::SensorSystemConfig sensorSystemConfig;
};

/**
 * @class INvSIPLCameraQuery NvSIPLCameraQuery.hpp
 *
 * @brief Defines the public data structures and describes the interfaces
 * for NvSIPLCameraQuery.
 */
class INvSIPLCameraQuery {
public:

    static constexpr uint32_t MAJOR_VER = 1u;
    static constexpr uint32_t MINOR_VER = 1u;
    static constexpr uint32_t PATCH_VER = 0u;

    /** @brief Holds version information for NvSIPLCameraQuery API. */
    struct Version {
        /** Holds the major version number. */
        uint32_t uMajor = MAJOR_VER;
        /** Holds the minor version number. */
        uint32_t uMinor = MINOR_VER;
        /** Holds the patch version number. */
        uint32_t uPatch = PATCH_VER;
    };

    /** @brief Returns the library version.
     *
     * @param[out] version A reference to the object containing the version
     * information.
     *
     * @pre None.
     *
     * @usage
     * - Allowed context for the API call
     *   - Interrupt handler: No
     *   - Signal handler: No
     *   - Thread-safe: No
     *   - Re-entrant: No
     *   - Async/Sync: Sync
     * - Required privileges: No
     * - Allowed execution state
     *   - Initialization: Yes
     *   - Operational: No
     *   - Reinit/Deinit Preparation: No
     * \if (NV_ASIL)
     *   - Implementation ASIL: N/A
     * \endif
     *   - Maturity: GA
     */
    static void GetVersion(Version& version);

    /** @brief Gets a handle to an instance of INvSIPLCameraQuery.
     *
     * This static function creates an instance of INvSIPLCameraQuery and returns
     * a handle to the object.
     * The object is automatically destroyed when the variable holding the
     * return value goes out of scope.
     *
     * @pre None.
     *
     * @returns unique_ptr A pointer to an instance of INvSIPLCameraQuery.
     *
     * @usage
     * - Allowed context for the API call
     *   - Interrupt handler: No
     *   - Signal handler: No
     *   - Thread-safe: No
     *   - Re-entrant: No
     *   - Async/Sync: Sync
     * - Required privileges: No
     * - Allowed execution state
     *   - Initialization: Yes
     *   - Operational: No
     *   - Reinit/Deinit Preparation: No
     * \if (NV_ASIL)
     *   - Implementation ASIL: N/A
     * \endif
     *   - Maturity: GA
     */
    static std::unique_ptr<INvSIPLCameraQuery> GetInstance(void);

    /** @brief Parses the built-in JSON database and updates the internal state of
     * the implementation class. This function must be called before any
     * other non-static functions.
     *
     * @pre None.
     *
     * @returns::SIPLStatus The status of the operation.
     *
     * @usage
     * - Allowed context for the API call
     *   - Interrupt handler: No
     *   - Signal handler: No
     *   - Thread-safe: No
     *   - Re-entrant: No
     *   - Async/Sync: Sync
     * - Required privileges: No
     * - Allowed execution state
     *   - Initialization: Yes
     *   - Operational: No
     *   - Reinit/Deinit Preparation: No
     * \if (NV_ASIL)
     *   - Implementation ASIL: N/A
     * \endif
     *   - Maturity: GA
     */
    virtual SIPLStatus ParseDatabase(void) = 0;

    /** @brief Parses the input JSON file containing a list of
     * user-specified camera configurations.
     *
     * @pre This function must be called only after @ref ParseDatabase().
     *
     * @param[in] fileName Full name, including the path, of the
     * JSON file containing the camera configuration. The external
     * devices referenced in the configuration must be supported
     * by the library.
     *
     * @returns::SIPLStatus The status of the operation.
     *
     * @usage
     * - Allowed context for the API call
     *   - Interrupt handler: No
     *   - Signal handler: No
     *   - Thread-safe: No
     *   - Re-entrant: No
     *   - Async/Sync: Sync
     * - Required privileges: No
     * - Allowed execution state
     *   - Initialization: Yes
     *   - Operational: No
     *   - Reinit/Deinit Preparation: No
     * \if (NV_ASIL)
     *   - Implementation ASIL: N/A
     * \endif
     *   - Maturity: GA
     */
    virtual SIPLStatus ParseJsonFile(std::string fileName) = 0;

    /** @brief Returns a pointer to the list of all external image devices
     * supported by NvSIPLCameraQuery API and SIPL Device Block drivers.
     *
     * @pre This function must be called only after @ref ParseDatabase().
     *
     * @returns A @c const pointer to CameraDeviceInfoList.
     *
     * @usage
     * - Allowed context for the API call
     *   - Interrupt handler: No
     *   - Signal handler: No
     *   - Thread-safe: No
     *   - Re-entrant: No
     *   - Async/Sync: Sync
     * - Required privileges: No
     * - Allowed execution state
     *   - Initialization: Yes
     *   - Operational: No
     *   - Reinit/Deinit Preparation: No
     * \if (NV_ASIL)
     *   - Implementation ASIL: N/A
     * \endif
     *   - Maturity: GA
     */
    virtual const CameraDeviceInfoList* GetDeviceInfoList() const = 0;

    /** @brief Returns a ModuleConfig object by name.
     *
     * @pre This function must be called only after @ref ParseDatabase().
     *
     * @param[in]  name     Name of the module configuration to retrieve.
     * @param[out] oConfig  A reference to the module configuration object.
     *
     * @returns::SIPLStatus The status of the operation.
     *
     * @usage
     * - Allowed context for the API call
     *   - Interrupt handler: No
     *   - Signal handler: No
     *   - Thread-safe: No
     *   - Re-entrant: No
     *   - Async/Sync: Sync
     * - Required privileges: No
     * - Allowed execution state
     *   - Initialization: Yes
     *   - Operational: No
     *   - Reinit/Deinit Preparation: No
     * \if (NV_ASIL)
     *   - Implementation ASIL: N/A
     * \endif
     *   - Maturity: GA
     */
    virtual SIPLStatus GetModuleConfig(const std::string& name,
                                       sensorconfig::ModuleConfig& oConfig) const = 0;

    /** @brief Returns transport settings by name.
     *
     * @pre This function must be called only after @ref ParseDatabase().
     *
     * @param[in]  name       Name of the transport settings to retrieve.
     * @param[out] oSettings  A reference to the transport configuration object.
     *
     * @returns::SIPLStatus The status of the operation.
     *
     * @usage
     * - Allowed context for the API call
     *   - Interrupt handler: No
     *   - Signal handler: No
     *   - Thread-safe: No
     *   - Re-entrant: No
     *   - Async/Sync: Sync
     * - Required privileges: No
     * - Allowed execution state
     *   - Initialization: Yes
     *   - Operational: No
     *   - Reinit/Deinit Preparation: No
     * \if (NV_ASIL)
     *   - Implementation ASIL: N/A
     * \endif
     *   - Maturity: GA
     */
    virtual SIPLStatus GetTransportSettings(const std::string& name,
                                           sensorconfig::TransportConfig& oSettings) const = 0;

    /** @brief Builds a complete sensor system configuration from module template.
     *
     * Creates a sensor system configuration by instantiating modules for each
     * transport link and populating MIPI settings.
     *
     * @pre This function must be called only after @ref ParseDatabase().
     *
     * @param[in]  name     Name of the module template to use.
     * @param[out] oConfig  A reference to the sensor system configuration object.
     *
     * @returns::SIPLStatus The status of the operation.
     *
     * @usage
     * - Allowed context for the API call
     *   - Interrupt handler: No
     *   - Signal handler: No
     *   - Thread-safe: No
     *   - Re-entrant: No
     *   - Async/Sync: Sync
     * - Required privileges: No
     * - Allowed execution state
     *   - Initialization: Yes
     *   - Operational: No
     *   - Reinit/Deinit Preparation: No
     * \if (NV_ASIL)
     *   - Implementation ASIL: N/A
     * \endif
     *   - Maturity: GA
     */
    virtual SIPLStatus GetSensorSystemConfig(const std::string& name,
                                            sensorconfig::SensorSystemConfig& oConfig) const = 0;

    /** @brief Defines link enable masks for deserializers/transports.
     *
     * To construct a complete mask for a deserializer, perform an <em>OR</em>
     * operation between the link mask values.
     * For example, to enable link 0 and 3 of a deserializer, set the mask to
     * @c LINK_0 | @c LINK_3. */
    enum EnableMask
    {
        LINK_0 = 0x0001, /**< 1st Link */
        LINK_1 = 0x0010, /**< 2nd Link */
        LINK_2 = 0x0100, /**< 3rd Link */
        LINK_3 = 0x1000  /**< 4th Link */
    };

    /** @brief Applies masks to the input sensor system configuration.
     *
     * Creates a custom configuration by modifying the input configuration
     * object to enable only specific links of the transport using the input masks.
     *
     * @pre This function must be called only after @ref ParseDatabase().
     *
     * @param[in,out] sysConfig  A reference to the sensor system configuration
     *                           that is modified.
     * @param[in]     vMasks     A vector of integers describing the mask value for
     *                           each transport in the sensor system configuration.
     *                           The number of masks in the vector must be same as
     *                           the number of transports in the configuration.
     *
     * @returns::SIPLStatus The status of the operation.
     *
     * @usage
     * - Allowed context for the API call
     *   - Interrupt handler: No
     *   - Signal handler: No
     *   - Thread-safe: No
     *   - Re-entrant: No
     *   - Async/Sync: Sync
     * - Required privileges: No
     * - Allowed execution state
     *   - Initialization: Yes
     *   - Operational: No
     *   - Reinit/Deinit Preparation: No
     * \if (NV_ASIL)
     *   - Implementation ASIL: N/A
     * \endif
     *   - Maturity: GA
     */
    virtual SIPLStatus ApplyMask(sensorconfig::SensorSystemConfig& sysConfig,
                                const std::vector<uint32_t>& vMasks) const = 0;

    /** @brief Returns a list of camera configuration names.
     *
     * @returns A vector of strings containing the names of all camera configurations.
     */
    virtual const std::vector<std::string>& GetCameraConfigNames() const = 0;

    virtual ~INvSIPLCameraQuery() = default;
};

} // namespace nvsipl
#endif // NVSIPL_CAMERA_QUERY_HPP
