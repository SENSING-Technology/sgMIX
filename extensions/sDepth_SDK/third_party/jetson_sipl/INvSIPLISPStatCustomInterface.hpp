/*
 * SPDX-FileCopyrightText: Copyright (c) 2024-2026 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 * SPDX-License-Identifier: LicenseRef-NvidiaProprietary
 *
 * NVIDIA CORPORATION, its affiliates and licensors retain all intellectual
 * property and proprietary rights in and to this material, related
 * documentation and any modifications thereto. Any use, reproduction,
 * disclosure or distribution of this material and related documentation
 * without an express license agreement from NVIDIA CORPORATION or
 * its affiliates is strictly prohibited.
 */

#ifndef INVSIPLSTAT_CUSTOMINTERFACE_HPP
#define INVSIPLSTAT_CUSTOMINTERFACE_HPP

#include "INvSIPLDeviceInterfaceProvider.hpp"

#include "NvSIPLClient.hpp"
#include "NvSIPLISPStat.hpp"

namespace nvsipl
{

/**
 * @brief Custom interface ID for @ref INvSIPLISPStatCustomInterface.
 */
constexpr UUID NVSIPLISPSTAT_CUSTOM_INTERFACE_ID
    { 0xB19E80D0U, 0x1333U, 0x4B4DU, 0xB5A9U,
        0x8CU, 0xCFU, 0x19U, 0x43U, 0xE2U, 0x37U };

/** @brief Custom interface to be implemented by the Tegra SIPL Pipeline HAL. */
class INvSIPLISPStatCustomInterface : public Interface
{
public:
    /** @brief Retrieve the ID of the class inheriting this interface.
     *
     * @retval Unique identifier for the interface class.
     *         Valid range: [NVSIPLISPSTAT_CUSTOM_INTERFACE_ID].
     */
    static UUID const &getClassInterfaceID() {
        return NVSIPLISPSTAT_CUSTOM_INTERFACE_ID;
    }

    /** @brief Retrieve the ID from the instance of the class inheriting this
     *         interface.
     *
     * @retval Unique identifier for the interface instance.
     *         Valid range: [NVSIPLISPSTAT_CUSTOM_INTERFACE_ID].
     */
    UUID const &getInstanceInterfaceID() const noexcept override {
        return NVSIPLISPSTAT_CUSTOM_INTERFACE_ID;
    }

    /** @brief Helper function to extract @ref IspStatsInfo (Tegra) from the
     *  @ref INvSIPLNvMBuffer object.
     *
     * @pre Should hold a reference to the buffer object on which the API is being called
     *
     * @param[in]   buffer  Pointer to the Tegra Pipeline HAL buffer object from
     *                      which to extract the Tegra ISP statistics
     *                      information. Valid values: [non-nullptr]
     *
     * @retval IspStatsInfo* if successful
     * @retval nullptr       if @a buffer does not point to a valid Tegra
     *                       Pipeline HAL buffer object
     *
     * @usage
     * - Allowed context for the API call
     *   - Interrupt handler: No
     *   - Signal handler: No
     *   - Thread-safe: Yes
     *   - Re-entrant: Yes
     *   - Async/Sync: Sync
     * - Required privileges: Yes, with the following conditions:
     *   - Grants: nonroot, allow
     *   - Abilities: public_channel
     *   - Application needs to have access to the SGIDs that SIPL depends on as mentioned in the
     *     NVIDIA DRIVE OS Safety Developer Guide
     * - Allowed execution state
     *   - Initialization: No
     *   - Operational: Yes
     *   - Reinit/Deinit Preparation: No
     * \if (NV_ASIL)
     *   - Implementation ASIL: ASIL B
     * \endif
     *   - Maturity: GA
     */
    virtual IspStatsInfo const * GetIspStatsInfo(
        INvSIPLClient::INvSIPLNvMBuffer const * const &buffer) const noexcept = 0;
};

} // namespace nvsipl

#endif // INVSIPLSTAT_CUSTOMINTERFACE_HPP
