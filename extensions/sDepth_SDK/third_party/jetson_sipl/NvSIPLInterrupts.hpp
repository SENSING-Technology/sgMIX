/*
 * SPDX-FileCopyrightText: Copyright (c) 2022-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 * SPDX-License-Identifier: LicenseRef-NvidiaProprietary
 *
 * NVIDIA CORPORATION, its affiliates and licensors retain all intellectual
 * property and proprietary rights in and to this material, related
 * documentation and any modifications thereto. Any use, reproduction,
 * disclosure or distribution of this material and related documentation
 * without an express license agreement from NVIDIA CORPORATION or
 * its affiliates is strictly prohibited.
 */

#ifndef NVSIPLINTERRUPTS_HPP
#define NVSIPLINTERRUPTS_HPP

/**
 * @file
 *
 * @brief <b> NVIDIA SIPL: Interrupts </b>
 *
 */

namespace nvsipl
{

/**
 * @brief Interrupt Status Codes
 */
enum class InterruptCode : uint32_t {
    /**
     * Device Block-level event, indicates general deserializer failure.
     */
    INTR_STATUS_DES_FAILURE = 0U,
    /**
     * Device Block-level event, indicates deserializer response timeout
     * or read failure.
     */
    INTR_STATUS_DES_TIMEOUT = 1U,
    /**
     * Device Block or Camera Link-level event, indicates deserializer
     * failure.
     */
    INTR_STATUS_DES_ERRB_ERR = 10U,
    /**
     * Camera Link-level event, indicates deserializer-serializer link
     * failure.
     */
    INTR_STATUS_DES_LOCK_ERR = 11U,

    /**
     * Device Block-level event, indicates general power load switch
     * failure.
     */
    INTR_STATUS_PWR_FAILURE = 100U,
    /**
     * Device Block-level event, indicates power load switch response
     * timeout or read failure.
     */
    INTR_STATUS_PWR_TIMEOUT = 101U,
    /**
     * Camera Link-level event, indicates power load switch overvoltage
     * failure on the corresponding output.
     */
    INTR_STATUS_PWR_OVERVOLTAGE = 102U,
    /**
     * Camera Link-level event, indicates power load switch undervoltage
     * failure on the corresponding output.
     */
    INTR_STATUS_PWR_UNDERVOLTAGE = 103U,
    /**
     * Device Block or Camera Link-level event, indicates power load switch line2line
     * fault on more than one outputs.
     */
    INTR_STATUS_PWR_LINE_TO_LINE_FAULT = 104U,
    /**
     * Camera Link-level event, indicates power load switch overcurrent
     * failure on the corresponding output.
     */
    INTR_STATUS_PWR_OVERCURRENT = 105U,
    /**
     * Camera Link-level event, indicates power load switch thermal shutdown
     * failure on the corresponding output. The corresponding output is forced
     * shutdown and remains off until the temperature drops below the thermal-shutdown
     * hysteresis temperature.
     */
    INTR_STATUS_PWR_THERMAL_SHUTDOWN = 106U,
    /**
     * Camera Link-level event, indicates power load switch circuit open
     * disconnect on the corresponding output.
     */
    INTR_STATUS_PWR_CIRCUIT_OPEN_DISCONNECT = 107U,
    /**
     * Camera Link-level event, indicates power load switch incorrect
     * impedance on the corresponding output.
     */
    INTR_STATUS_PWR_INCORRECT_IMPEDANCE = 108U,
    /**
     * Device Block level event, indicates power load switch ISET pin is open.
     * The ISET pin of powerload switch is open, could be due to a
     * short to ground or open circuit or incorrect resistance value.
     */
    INTR_STATUS_PWR_ISET_OPEN = 109U,
    /**
     * Device Block level event, indicates power load switch input voltage is below
     * the minimum operating voltage. The outputs are forced off in this condition.
     */
    INTR_STATUS_PWR_UNDERVOLTAGE_INPUT = 110U,
    /**
     * Device Block level event, indicates power load switch input voltage is above
     * the maximum operating voltage. The outputs are forced off in this condition.
     */
    INTR_STATUS_PWR_OVERVOLTAGE_INPUT = 111U,

    /**
     * Device Block level event, indicates power load switch VDD voltage is below
     * the minimum operating voltage. The outputs are forced off in this condition.
     */
    INTR_STATUS_PWR_UNDERVOLTAGE_VDD = 112U,
    /**
     * Device Block level event, indicates power load switch VDD voltage is above
     * the maximum operating voltage. The outputs are forced off in this condition.
     */
    INTR_STATUS_PWR_OVERVOLTAGE_VDD = 113U,

    /**
     * Camera Link-level event, indicates general Camera Module serializer
     * failure.
     */
    INTR_STATUS_SER_FAILURE = 1000U,
    /**
     * Camera Link-level event, indicates Camera Module serializer response
     * timeout or read failure.
     */
    INTR_STATUS_SER_TIMEOUT = 1001U,
    /**
     * Camera Link-level event, indicates serializer-deserializer link error
     * reported by Camera Module driver.
     */
    INTR_STATUS_SERDES_LINK_ERR = 1002U,

    /**
     * Camera Link-level event, indicates general Camera Module sensor
     * failure.
     */
    INTR_STATUS_SEN_FAILURE = 1100U,
    /**
     * Camera Link-level event, indicates Camera Module sensor reponse
     * timeout or read failure.
     */
    INTR_STATUS_SEN_TIMEOUT = 1101U,

    /**
     * Camera Link-level event, indicates general Camera Module PMIC
     * failure.
     */
    INTR_STATUS_PMIC_FAILURE = 1200U,
    /**
     * Camera Link-level event, indicates Camera Module PMIC response
     * timeout or read failure.
     */
    INTR_STATUS_PMIC_TIMEOUT = 1201U,

    /**
     * Camera Link-level event, indicates general Camera Module EEPROM
     * failure.
     */
    INTR_STATUS_EEPROM_FAILURE = 1300U,
    /**
     * Camera Link-level event, indicates Camera Module EEPROM response
     * timeout or read failure.
     */
    INTR_STATUS_EEPROM_TIMEOUT = 1301U,

    /**
     * Camera Link-level event, indicates general Camera Module LED
     * Driver failure.
     */
    INTR_STATUS_LED_FAILURE = 1400U,

    /**
     * Camera Link-level event, indicates Camera Module LED Driver
     * response timeout.
     */
    INTR_STATUS_LED_TIMEOUT = 1401U,

    /**
     * Camera Link-level event, indicates a Camera Module spurious
     * interrupt with no associated error cause.
     *
     * If a driver issues this notification
     * a. It precludes the SIPL error handling thread from automatically entering error state
     *    and sending INTR_STATUS_LOCALIZATION_FAILURE, if no other errors are reported.
     * b. Masks interrupts for any devices whose status cannot be queried.
     */
    INTR_STATUS_SPURIOUS_NO_ERROR = 9000U,

    /**
     * Device Block or Camera Link-level event, indicates general failure.
     */
    INTR_STATUS_FAILURE = 10000U,
    /**
     * Device Block or Camera Link-level event, indicates general response
     * timeout.
     */
    INTR_STATUS_TIMEOUT = 10001U,
    /**
     * Device Block or Camera Link-level event, indicates general localization
     * failure.
     */
    INTR_STATUS_LOCALIZATION_FAILURE = 10002U,
    /**
     * Device Block-level event, indicates recovery from error state for all links.
     *
     * This event is generated when all links that previously reported errors
     * (e.g., via GPIO interrupts) have fully recovered, indicating that the device block
     * camera subsystem is no longer in a degraded state.
     */
    INTR_STATUS_ERROR_RECOVERY = 10003U,
};

}  // namespace nvsipl


#endif // NVSIPLINTERRUPTS_HPP
