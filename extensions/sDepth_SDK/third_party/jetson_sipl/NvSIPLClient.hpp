/*
 * SPDX-FileCopyrightText: Copyright (c) 2018-2026 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 * SPDX-License-Identifier: LicenseRef-NvidiaProprietary
 *
 * NVIDIA CORPORATION, its affiliates and licensors retain all intellectual
 * property and proprietary rights in and to this material, related
 * documentation and any modifications thereto. Any use, reproduction,
 * disclosure or distribution of this material and related documentation
 * without an express license agreement from NVIDIA CORPORATION or
 * its affiliates is strictly prohibited.
 */

#ifndef NVSIPLCLIENT_HPP
#define NVSIPLCLIENT_HPP

#include "NvSIPLCommon.hpp"
#include "NvSIPLISPStructs.hpp"

#include "NvSIPLCDICommon.h"

#include "nvscisync.h"
#include "nvscistream.h"

#include <cstdint>
#include <string>
#include <memory>

/**
 * @file
 *
 * @brief <b> NVIDIA SIPL: Client Interface - @ref NvSIPLClient_API </b>
 *
 */

/** @defgroup NvSIPLClient_API NvSIPL Client
 *
 * @brief Provides interfaces to retrieve the output of the SIPL Pipeline Manager.
 *
 * @ingroup NvSIPLCamera_API
 */

namespace nvsipl
{
/** @ingroup NvSIPLClient_API
 * @{
 */

/** @class INvSIPLClient NvSIPLClient.hpp
 *
 * @brief Defines the public data structures
 * and describes the interfaces for @ref NvSIPLClient_API.
 *
 */
class INvSIPLClient
{
public:
    /** @brief Defines the metadata associated with the image. */
    struct ImageMetaData
    {
        /** Holds the TSC timestamp of the end of frame for capture. */
        uint64_t frameCaptureTSC;
        /** Holds the TSC timestamp of the start of frame for capture. */
        uint64_t frameCaptureStartTSC;
        /** Holds the parsed embedded data frame number of exposures info for the captured frame. */
        uint32_t numExposures;
        /** Holds the parsed embedded data sensor exposure info for the captured frame. */
        DevBlkCDIExposure sensorExpInfo;
        /** Holds the parsed embedded data sensor white balance info for the captured frame. */
        DevBlkCDIWhiteBalance sensorWBInfo;
        /** Holds the parsed embedded data illumination info for the captured frame. */
        DevBlkCDIIllumination illuminationInfo;
        /** Holds the parsed embedded data sensor PWL info for the captured frame. */
        DevBlkCDIPWL sensorPWLInfo;
        /** Holds the parsed embedded data sensor crc info for the captured frame. */
        DevBlkCDICRC sensorCRCInfo;
        /** Holds the parsed embedded data frame report info for the captured frame. */
        DevBlkCDIFrameReport sensorReportInfo;
        /** Holds the parsed embedded data sensor temperature info for the captured frame. */
        DevBlkCDITemperature sensorTempInfo;
        /** Holds the parsed embedded data frame sequence number info for the captured frame. */
        DevBlkCDIFrameSeqNum frameSeqNumInfo;
        /** Holds parsed embedded data frame timestamp info for the captured frame. */
        DevBlkCDIFrameTimestamp frameTimestampInfo;
        /** Holds information on errors present in the embedded data.
         * The meaning of these values is determined by the driver. */
        int8_t errorFlag;
        /** Holds the control information. */
        NvSiplControlInfo controlInfo;

        /** Holds a flag indicating if the frame was dropped. */
        bool wasFrameDropped;

        /** Holds the parsed embedded data frame context id info for the captured frame. */
        DevBlkCDIFrameCtxID frameCtxIDInfo;
    };

     struct ImageEmbeddedData
     {
          /** Holds size of the top embedded data. */
          uint32_t embeddedBufTopSize;
          /** Holds size of the bottom embedded data. */
          uint32_t embeddedBufBottomSize;
          /** Holds pointer to the top embedded data. */
          uint8_t *embeddedBufTop;
          /** Holds pointer to the bottom embedded data. */
          uint8_t *embeddedBufBottom;
     };

    /** @class INvSIPLBuffer
     *
     * @brief Abstract interface for SIPL buffers.
     *
     */
    class INvSIPLBuffer
    {
     public:
        /** @brief Adds a reference.
         *
         * Adding a reference to the buffer ensures that this buffer is not
         * re-used by the producer of the buffer.
         *
         * @attention The user must release the references in a timely manner using @ref Release
         * to avoid starving SIPL of the buffers required for capture and/or processing
         *
         * @pre None.
         *
         * @usage
         * - Allowed context for the API call
         *   - Interrupt handler: No
         *   - Signal handler: No
         *   - Thread-safe: Yes, with the following conditions:
         *     - There is no active release operation that results in the buffer
         *       available for re-use.
         *   - Re-entrant: No
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
        virtual void AddRef() = 0;

        /** @brief Release a reference.
         *
         * Once the reference has been released as many times as the reference
         * was added, it implies that the user has finished working with the
         * buffer and the buffer is available for re-use by SIPL.
         *
         * @pre None.
         *
         * @retval NVSIPL_STATUS_OK           The releasing operation succeeds.
         * @retval NVSIPL_STATUS_BAD_ARGUMENT If the buffer is already released.
         * @retval (SIPLStatus)               Other values of SIPLStatus if there are failures due to dependencies.
         *
         * @usage
         * - Allowed context for the API call
         *   - Interrupt handler: No
         *   - Signal handler: No
         *   - Thread-safe: Yes
         *   - Re-entrant: No
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
        virtual SIPLStatus Release() = 0;

        /** @brief Add an NvSciSync prefence.
         *
         * Add an NvSciSync prefence to be used with the next ISP or ICP operation.
         * ICP operations support up to 16 unique pre-fences per pipeline
         * ISP operations support up to 16 unique pre-fences per pipeline
         *
         * @attention This function creates its own duplicate of the fence,
         * so the caller must clear their copy of the fence by calling @ref NvSciSyncFenceClear().
         *
         * @attention The caller ensure that the added prefences expire in a timely manner.
         *
         * @note Users need to ensure that the number of unique pre-fences for ISP does not exceed 16 across
         * the all output buffers if using more than one ISP outputs for the pipeline.
         *
         * @pre None.
         *
         * @param[in] prefence Prefence to be added.
         *
         * @retval NVSIPL_STATUS_OK    The function succeeds.
         * @retval NVSIPL_STATUS_ERROR If internal NvSci fence operations fail.
         * @retval (SIPLStatus)        Other values of SIPLStatus if there are failures due to dependencies.
         *
         * @usage
         * - Allowed context for the API call
         *   - Interrupt handler: No
         *   - Signal handler: No
         *   - Thread-safe: Yes
         *   - Re-entrant: No
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
        virtual SIPLStatus AddNvSciSyncPrefence(NvSciSyncFence const &prefence) = 0;

        /** @brief Retrieve the latest NvSciSync SOF fence.
         *
         * Retrieve the buffer's latest NvSciSync SOF fence associated with the engine's set
         * NvSciSync SOF object.
         *
         * @attention The caller must clear the returned fence by calling @ref NvSciSyncFenceClear().
         *
         * @attention In case of ISP buffers, a null pointer is returned.
         *
         * @pre None.
         *
         * @param[out] sofFence SOF fence being returned.
         *
         * @retval NVSIPL_STATUS_OK    The function succeeds.
         * @retval NVSIPL_STATUS_ERROR If internal NvSci fence operations fail.
         * @retval (SIPLStatus)        Other values of SIPLStatus if there are failures due to dependencies.
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
        virtual SIPLStatus GetSOFNvSciSyncFence(NvSciSyncFence *const sofFence) = 0;

        /** @brief Retrieve the latest NvSciSync EOF fence.
         *
         * Retrieve the buffer's latest NvSciSync EOF fence associated with the engine's set
         * NvSciSync EOF object.
         *
         * @attention The caller must clear the returned fence by calling @ref NvSciSyncFenceClear().
         *
         * @pre None.
         *
         * @param[out] postfence EOF fence being returned.
         *
         * @retval NVSIPL_STATUS_OK    The function succeeds.
         * @retval NVSIPL_STATUS_ERROR If internal NvSci fence operations fail.
         * @retval (SIPLStatus)        Other values of SIPLStatus if there are failures due to dependencies.
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
        virtual SIPLStatus GetEOFNvSciSyncFence(NvSciSyncFence *const postfence) = 0;

        /** @brief Default destructor. */
        virtual ~INvSIPLBuffer() = default;

        /** @brief Delete copy constructor */
        INvSIPLBuffer(INvSIPLBuffer& other) = delete;

        /** @brief Delete copy assignment operator. */
        INvSIPLBuffer& operator=(INvSIPLBuffer& other) & = delete;

        /** @brief Delete move constructor */
        INvSIPLBuffer(INvSIPLBuffer&& other) = delete;

        /** @brief Delete move assignment operator. */
        INvSIPLBuffer& operator=(INvSIPLBuffer&& other) & = delete;

    protected:
        /** @brief Default constructor declared in order to allow construction of derived objects. */
        INvSIPLBuffer() = default;

    };

    /** @class INvSIPLNvMBuffer
     *
     * @brief Describes a SIPL buffer containing an @ref NvSciBufObj.
     */
    class INvSIPLNvMBuffer : public INvSIPLBuffer
    {
     public:
        /** @brief Gets a handle to @ref NvSciBufObj.
         *
         * @pre Should be called after EOF for the capture buffer has expired.
         *
         * @returns A @ref NvSciBufObj.
         * @returns nullptr If a buffer is dropped.
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
        virtual NvSciBufObj GetNvSciBufImage() const = 0;

        /** @brief Gets an @ref nvsipl::INvSIPLClient::ImageMetaData
         * associated with @ref NvSciBufObj
         *
         * @pre Should be called after EOF for the capture buffer has expired.
         *
         * @returns A const reference to @ref nvsipl::INvSIPLClient::ImageMetaData.
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
        virtual ImageMetaData const& GetImageData() const = 0;

        /** @brief Gets an @ref nvsipl::INvSIPLClient::ImageEmbeddedData
         * this is the RAW data associated with the captured image from the sensor before
         * it is parsed to ImageMetaData structure.
         *
         * @pre Should be called after EOF for the capture buffer has expired.
         *
         * @returns A const reference to @ref nvsipl::INvSIPLClient::ImageEmbeddedData.
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
        virtual ImageEmbeddedData const& GetImageEmbeddedData() const = 0;

        /** @brief Default destructor */
        ~INvSIPLNvMBuffer() override = default;

        /** @brief Delete copy constructor */
        INvSIPLNvMBuffer(INvSIPLNvMBuffer& other) = delete;

        /** @brief Delete copy assignment operator. */
        INvSIPLNvMBuffer& operator=(INvSIPLNvMBuffer& other) & = delete;

        /** @brief Delete move constructor */
        INvSIPLNvMBuffer(INvSIPLNvMBuffer&& other) = delete;

        /** @brief Delete move assignment operator. */
        INvSIPLNvMBuffer& operator=(INvSIPLNvMBuffer&& other) & = delete;

    protected:
        /** @brief Default constructor declared in order to allow construction of derived objects. */
        INvSIPLNvMBuffer() = default;

    };

    /** @brief Describes a client of the pipeline. */
    struct ConsumerDesc
    {
        /** @brief Defines the types of the SIPL pipeline output. */
        enum class OutputType : std::uint8_t
        {
            ICP,  /** Indicates the unprocessed output of the image sensor. */
            ISP0, /** Indicates the first output of ISP. */
            ISP1, /** Indicates the second output of ISP. */
            ISP2, /** Indicates the third output of ISP. */
        };
    };

    /** @brief Default destructor. */
    virtual ~INvSIPLClient() = default;

    /** @brief Delete copy constructor. */
    INvSIPLClient(INvSIPLClient& other) = delete;

    /** @brief Delete copy assignment operation. */
    INvSIPLClient& operator=(INvSIPLClient& other) & = delete;

    /** @brief Delete move constructor. */
    INvSIPLClient(INvSIPLClient&& other) = delete;

    /** @brief Delete move assignment operation. */
    INvSIPLClient& operator=(INvSIPLClient&& other) & = delete;

protected:
    /** @brief Default constructor declared in order to allow construction of derived objects. */
    INvSIPLClient() = default;

};

/** @} */

} // namespace nvsipl

#endif // NVSIPLCLIENT_HPP
