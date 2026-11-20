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

#ifndef NVSIPLPIPELINEMGR_HPP
#define NVSIPLPIPELINEMGR_HPP

#include "NvSIPLCommon.hpp"
#include "NvSIPLClient.hpp"
#include "NvSIPLPlatformCfg.hpp"
#include "NvSIPLInterrupts.hpp"

#include <cstdint>
#include <vector>

/**
 * @file
 *
 * @brief <b> NVIDIA SIPL: Pipeline Manager -
 *     @ref NvSIPLPipelineMgr </b>
 *
 */

namespace nvsipl
{
/** @defgroup NvSIPLPipelineMgr NvSIPL Pipeline Manager
 *
 * @brief Programs Video Input (VI) and Image Signal Processor (ISP)
 *  hardware blocks to create image processing pipelines
 *  for each sensor.
 *
 * @ingroup NvSIPLCamera_API
 * @{
 */
/** @brief The default number of capture buffers configured for a given pipeline */
static constexpr uint16_t DEFAULT_SET_CAPTURE_BUFFER_COUNT {40U};
/** @brief The default number of ISP buffers configured for a given pipeline */
static constexpr uint16_t DEFAULT_SET_ISP_BUFFER_COUNT {64U};

/**
 * @brief Defines the capture path type for the pipeline
 */
enum class CapturePathType : uint8_t {
    VI = 0,    /** VI path */
#if defined(NV_EMBEDDED_L4T)
    COE = 1    /** Camera over Ethernet path */
#endif
};

/**
 *
 * @class NvSIPLPipelineNotifier
 *
 * @brief Describes the interfaces of the SIPL pipeline notification handler.
 *
 * This class defines data structures and interfaces that must be implemented by
 * a SIPL pipeline notification handler.
 */
class NvSIPLPipelineNotifier
{
    /** Indicates the maximum number of gpio indices. */
    static constexpr uint32_t MAX_DEVICE_GPIOS {16U};

public:
    /** @brief Defines the events of the image processing pipeline and the device block. */
    enum NotificationType : std::uint16_t
    {
        /**
         * Pipeline event, indicates ICP processing is finished.
         * @note Only eNotifType, uIndex & frameCaptureTSC are valid in @ref NotificationData for this event.
         */
        NOTIF_INFO_ICP_PROCESSING_DONE = 0,

        /**
         * Pipeline event, indicates CDI processing is finished.
         * @note This event is sent only if the Auto Exposure and Auto White Balance algorithm produces
         * new sensor settings that need to be updated in the image sensor.
         * @note Only eNotifType, uIndex & frameCaptureTSC are valid in @ref NotificationData for this event.
         */
        NOTIF_INFO_CDI_PROCESSING_DONE = 3,

        /**
         * Pipeline event, indicates image authentication success.
         * @note Only eNotifType, uIndex, frameSeqNumber & frameCaptureTSC are valid in
         * @ref NotificationData for this event.
         */
        NOTIF_INFO_ICP_AUTH_SUCCESS = 4,

        /**
         * Pipeline event, indicates pipeline was forced to drop a frame due to a slow consumer or system issues.
         * @note Only eNotifType & uIndex are valid in @ref NotificationData for this event.
         */
        NOTIF_WARN_ICP_FRAME_DROP = 100,

        /**
         * Pipeline event, indicates a discontinuity was detected in parsed embedded data frame sequence number.
         * @note Only eNotifType & uIndex are valid in @ref NotificationData for this event.
         */
        NOTIF_WARN_ICP_FRAME_DISCONTINUITY = 101,

        /**
         * Pipeline event, indicates occurrence of timeout while capturing.
         * @note Only eNotifType & uIndex are valid in @ref NotificationData for this event.
         */
        NOTIF_WARN_ICP_CAPTURE_TIMEOUT = 102,

        /**
         * Pipeline event, indicates occurrence of prefence timeout while capturing,
         * increasing risk of future dropped frames or failure.
         * @note Only eNotifType & uIndex are valid in @ref NotificationData for this event.
         */
        NOTIF_WARN_ICP_APP_BACK_PRESSURE = 103,

        /**
         * Pipeline event, indicates ICP bad input stream.
         * @note Only eNotifType & uIndex are valid in @ref NotificationData for this event.
         */
        NOTIF_ERROR_ICP_BAD_INPUT_STREAM = 200,

        /**
         * Pipeline event, indicates ICP capture failure.
         * @note Only eNotifType & uIndex are valid in @ref NotificationData for this event.
         */
        NOTIF_ERROR_ICP_CAPTURE_FAILURE = 201,

        /**
         * Pipeline event, indicates embedded data parsing failure.
         * @note Only eNotifType & uIndex are valid in @ref NotificationData for this event.
         */
        NOTIF_ERROR_ICP_EMB_DATA_PARSE_FAILURE = 202,

        /**
         * Pipeline event, indicates ISP processing failure.
         * @note Only eNotifType & uIndex are valid in @ref NotificationData for this event.
         */
        NOTIF_ERROR_ISP_PROCESSING_FAILURE = 203,

        /**
         * Pipeline event, indicates auto control processing failure.
         * @note Only eNotifType & uIndex are valid in @ref NotificationData for this event.
         */
        NOTIF_ERROR_ACP_PROCESSING_FAILURE = 204,

        /**
         * Pipeline event, indicates CDI set sensor control failure.
         * @note Only eNotifType & uIndex are valid in @ref NotificationData for this event.
         */
        NOTIF_ERROR_CDI_SET_SENSOR_CTRL_FAILURE = 205,

        /**
         * Device block event, indicates a deserializer failure.
         * @note Only eNotifType, intrCode, intrData, gpioIdxs & numGpioIdxs valid in
         * @ref NotificationData for this event.
         */
        NOTIF_ERROR_DESERIALIZER_FAILURE = 207,

        /**
         * Device block event, indicates a serializer failure.
         * @note Only eNotifType, uLinkMask, intrCode, intrData, gpioIdxs & numGpioIdxs valid in
         * @ref NotificationData for this event.
         */
        NOTIF_ERROR_SERIALIZER_FAILURE = 208,

        /**
         * Device block event, indicates a sensor failure.
         * @note Only eNotifType, uLinkMask, intrCode, intrData, gpioIdxs & numGpioIdxs valid in
         * @ref NotificationData for this event.
         */
        NOTIF_ERROR_SENSOR_FAILURE = 209,

        /**
         * Pipeline event, indicates isp process failure due to recoverable errors.
         * @note Only eNotifType & uIndex are valid in @ref NotificationData for this event.
         */
        NOTIF_ERROR_ISP_PROCESSING_FAILURE_RECOVERABLE = 210,

        /**
         * Pipeline event, indicates image authentication failure.
         * @note Only eNotifType, uIndex, frameSeqNumber & frameCaptureTSC are valid in
         * @ref NotificationData for this event.
         */
        NOTIF_ERROR_ICP_AUTH_FAILURE = 211,

        /**
         * Pipeline event, indicates out of order image is detected.
         * @note Only eNotifType, uIndex, frameSeqNumber & frameCaptureTSC are valid in
         * @ref NotificationData for this event.
         */
        NOTIF_ERROR_ICP_AUTH_OUT_OF_ORDER = 212,

        /**
         * Pipeline event, indicates ACP settings discontinuity is detected.
         * @note Only eNotifType, uIndex are valid in
         * @ref NotificationData for this event.
         */
        NOTIF_ERROR_ACP_SETTINGS_DISCONTINUITY = 213,

        /**
         * Device block event, indicates interrupt localization failure (i.e.
         * could not find source of interrupt)
         * @note Only eNotifType, intrCode, gpioIdxs & numGpioIdxs are valid in
         * @ref NotificationData for this event.
         */
        NOTIF_ERROR_INTR_LOCALIZATION_FAILURE = 220,

        /**
         * Device block event, indicates interrupt localization timeout.
         * @note Only eNotifType, intrCode, gpioIdxs & numGpioIdxs are valid in
         * @ref NotificationData for this event.
         */
        NOTIF_ERROR_INTR_LOCALIZATION_TIMEOUT = 221,

        /**
         * Pipeline event, indicates isp process failure due to ISP status timeout.
         * @note Only eNotifType, uIndex are valid in
         * @ref NotificationData for this event.
         */
        NOTIF_ERROR_ISP_PREFENCE_TIMEOUT = 222,

        /**
         * Pipeline Event, indicates that an init failure occured due to external camera device.
         * @note Only eNotifType, uIndex and initErrCode are valid in
         * @ref NotificationData for this event.
         */
        NOTIF_INIT_ERROR_FAILURE = 223,

        /**
         * Device block event, indicates interrupt error recovery.
         * @note Only eNotifType, intrCode, gpioIdxs & numGpioIdxs are valid in
         * @ref NotificationData for this event.
         */
        NOTIF_ERROR_INTR_RECOVERY = 224,


        /**
         * Device block event, indicates an error interrupt was detected on a device block
         * error GPIO pin.
         * This is an early notification; specific device and error details
         * will be provided in a subsequent notification after interrupt localization completes.
         * @note Only eNotifType, uIndex, uLinkMask, gpioIdxs & numGpioIdxs are valid in
         * @ref NotificationData for this event.
         */
        NOTIF_WARN_DEVICE_BLOCK_ERROR_INTR_DETECTED = 225,

        /**
         * Pipeline or device block event, indicates power load switch failure or error detected
         * on power load switch.
         * @note For pipeline event, only eNotifType & uIndex are valid in
         * @ref NotificationData for this event.
         * @note For device block event, only eNotifType, uIndex, uLinkMask, intrCode,
         * intrData, gpioIdxs & numGpioIdxs are valid in @ref NotificationData for this event.
         */
        NOTIF_ERROR_POWER_LOAD_SWITCH_FAILURE = 226,

        /**
         * Pipeline and device block event, indicates an unexpected internal failure.
         * @note For pipeline event, only eNotifType & uIndex are valid in
         * @ref NotificationData for this event.
         * @note For device block event, only eNotifType is valid in
         * @ref NotificationData for this event.
         */
        NOTIF_ERROR_INTERNAL_FAILURE = 300,
    };

    /**
     * @brief Defines the notification data.
     * @note A few members are not valid for certain events, please see @ref NotificationType.
     */
    struct NotificationData
    {
        /** Holds the @ref NotificationType event type. */
        NotificationType eNotifType;
        /** Holds the ID of the pipeline. This is the same as the Sensor ID in PlatformCfg. */
        uint32_t uIndex;
        /** Holds the virtual channel index */
        uint32_t uVcIdx;
        /** Holds the device block link mask. */
        uint8_t uLinkMask;
        /** Holds a sequence number of a captured frame. */
        uint64_t frameSeqNumber;
        /** Holds the TSC timestamp of the end of frame for capture. */
        uint64_t frameCaptureTSC;
        /** Holds the TSC timestamp of the start of frame for capture. */
        uint64_t frameCaptureStartTSC;
        /** Holds the Interrupt Code */
        InterruptCode intrCode;
        /** Holds the Interrupt Data */
        uint64_t intrData;
        /** Holds the GPIO indices. */
        uint32_t gpioIdxs[MAX_DEVICE_GPIOS];
        /** Holds the number of GPIO indices in the array. */
        uint32_t numGpioIdxs;
        /** Holds the Init Error Code */
        InitErrorCode initErrCode;
    };

    /** @brief Default Constructor. */
    NvSIPLPipelineNotifier() = default;

    /** @brief Delete copy constructor. */
    NvSIPLPipelineNotifier(NvSIPLPipelineNotifier &) = delete;

    /** @brief Delete move constructor. */
    NvSIPLPipelineNotifier(NvSIPLPipelineNotifier &&) = delete;

    /** @brief Delete copy assignment operator. */
    NvSIPLPipelineNotifier& operator=(NvSIPLPipelineNotifier &) & = delete;

    /** @brief Delete move assignment operator. */
    NvSIPLPipelineNotifier& operator=(NvSIPLPipelineNotifier &&) & = delete;

    /** @brief Default destructor. */
    virtual ~NvSIPLPipelineNotifier(void) = default;
};

/**
 *
 * @class NvSIPLImageGroupWriter
 *
 * @brief Describes the interfaces of SIPL pipeline feeder.
 *
 * This class defines data structures and interfaces that must be implemented by
 * the SIPL pipeline feeder in case of ISP reprocess mode.
 *
 * In ISP reprocess mode, the user can feed unprocessed sensor output captured
 * during the data collection process and then process it through HW ISP.
 *
 * The user must have configured @ref NvSIPLCamera_API using an appropriate
 * PlatformCfg to be able to use this mode.
 */
class NvSIPLImageGroupWriter
{
public:
    /** @brief Describes an unprocessed sensor output buffer. */
    struct RawBuffer
    {
        /** Holds an @ref NvSciBufObj. */
        NvSciBufObj image;
        /** Holds the ID of the sensor in @ref PlatformCfg. */
        uint32_t uIndex;
        /** Holds the virtual channel index */
        uint32_t uVcIdx;
        /** Holds a flag to signal discontinuity for the current raw buffer from the previous one. */
        bool discontinuity;
        /** Holds a flag to signal that the pipeline should drop the current buffer. */
        bool dropBuffer;
        /** Holds the TSC timestamp of the end of frame for capture. */
        uint64_t frameCaptureTSC;
        /** Holds the TSC timestamp of the start of frame for capture. */
        uint64_t frameCaptureStartTSC;
    };

    /** @brief Populates the buffer with RAW data.
     *
     * The consumer's implementation overrides this method.
     * The method is called by SIPL pipeline thread in runtime only.
     *
     * The feeder must populate the @ref RawBuffer with appropriate RAW data.
     *
     * @pre Pipeline must be initialized and operational.
     *
     * @param[out] oRawBuffer   A reference to the @ref RawBuffer that the function is to populate.
     *
     * @returns::SIPLStatus The completion status of this operation (implementation defined)
     * @retval NVSIPL_STATUS_OK             Success
     * @retval NVSIPL_STATUS_ERROR          Operation failed
     * @retval (SIPLStatus)                 Other implementation-defined status
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
     *   - Implementation ASIL: N/A
     * \endif
     *   - Maturity: GA
     */
    virtual SIPLStatus FillRawBuffer(RawBuffer &oRawBuffer) = 0;

    /** @brief Default Constructor. */
    NvSIPLImageGroupWriter() = default;

    /** @brief Delete copy constructor. */
    NvSIPLImageGroupWriter(NvSIPLImageGroupWriter &) = delete;

    /** @brief Delete move constructor. */
    NvSIPLImageGroupWriter(NvSIPLImageGroupWriter &&) = delete;

    /** @brief Delete copy assignment operator. */
    NvSIPLImageGroupWriter& operator=(NvSIPLImageGroupWriter &) & = delete;

    /** @brief Delete move assignment operator. */
    NvSIPLImageGroupWriter& operator=(NvSIPLImageGroupWriter &&) & = delete;

    /** @brief Default destructor. */
    virtual ~NvSIPLImageGroupWriter(void) = default;
};

/**
 * @brief Data structure to define the camera pipeline buffer configuration.
 */
struct NvSIPLPipelineBufferCfg
{
    /**
     * Max capture output buffers that client wants to register with the pipeline.
     * Valid range: [3, UINT16_MAX] for ICP output (live capture)
     *              [1, UINT16_MAX] for ICP output (reprocess mode) : Non-Safety
    */
    uint16_t maxCaptureBufferCount {DEFAULT_SET_CAPTURE_BUFFER_COUNT};

    /**
     * Max ISP0 output buffers that client wants to register with the pipeline.
     * Valid range: [1, UINT16_MAX]
    */
    uint16_t maxIsp0BufferCount {DEFAULT_SET_ISP_BUFFER_COUNT};

    /**
     * Max ISP1 output buffers that client wants to register with the pipeline.
     * Valid range: [1, UINT16_MAX]
    */
    uint16_t maxIsp1BufferCount {DEFAULT_SET_ISP_BUFFER_COUNT};

    /**
     * Max ISP2 output buffers that client wants to register with the pipeline.
     * Valid range: [1, UINT16_MAX]
    */
    uint16_t maxIsp2BufferCount {DEFAULT_SET_ISP_BUFFER_COUNT};
};

/**
 * @brief Defines the camera pipeline configuration.
 *
 */
struct NvSIPLPipelineConfiguration
{
    /** <tt>true</tt> if the client wants capture output frames to be delivered */
    bool captureOutputRequested {false};

    /** <tt>true</tt> if the client wants frames to be delivered from the first ISP output */
    bool isp0OutputRequested {false};

    /** <tt>true</tt> if the client wants frames to be delivered from the second ISP output */
    bool isp1OutputRequested {false};

    /** <tt>true</tt> if the client wants frames to be delivered from the third ISP output */
    bool isp2OutputRequested {false};

    /** Holds a downscale and crop configuration. */
    NvSIPLDownscaleCropCfg downscaleCropCfg {};

    /**
     * Holds ISP statistics override parameters. ISP statistcis settings enabled
     * in @ref NvSIPLIspStatsOverrideSetting will override the statistics settings
     * provided in NITO
     */
    NvSIPLIspStatsOverrideSetting statsOverrideSettings {};

    /**
     * Holds a pointer to an @ref NvSIPLImageGroupWriter.
     *
     * @note In non-safety mode, this pointer should be set to a valid @ref NvSIPLImageGroupWriter instance or nullptr value.
     *       In safety mode, this pointer must be nullptr. Setting a non-nullptr value in safety mode will result in a runtime error
     */
    NvSIPLImageGroupWriter* imageGroupWriter {nullptr};

    /**
     * Subframe pipeline feature processes frames of pixel data in a number of
     * slices determined by sliceCount, allowing for latency savings through
     * optimized scheduling.
     * <tt>true</tt> if the client wants to disable the subframe feature
     * Note: This feature is disabled by default when using YUV cameras or
     *       TPG configurations.
     */
    bool disableSubframe {false};

    /**
     * Holds number of slices for ISP slicing configuration.
     * Valid range: [0, 16]
     * Note: If sliceCount is 0, Default ISP slicing configuration will be used.
     */
    uint16_t sliceCount {0};

    /**
     * Holds the capture path type (VI or COE).
     * Default is VI path.
     */
    CapturePathType capturePathType {CapturePathType::VI};

    /**
     * Holds maximum buffer pool size for each output channel configuration.
    */
    NvSIPLPipelineBufferCfg bufferCfg {};

    /** Flag to bypass sensor control for the pipeline */
    bool bypassSensorCtrl {false};

    /**
     * Holds the number of capture output buffer to be released early.
     * Valid range: [0, 1]
     * Note: If numEarlyOutputs is 0, early outputs are disabled.
     */
    uint16_t numEarlyOutputs {0U};
};

/**
 * @brief The interface to the frame completion queue.
 */
class INvSIPLFrameCompletionQueue
{
public:

    /**
     * @brief Retrieve the next item from the queue.
     *
     * The buffer returned will have a single reference that must be released by the client
     * when it has finished with the buffer. This is done by calling item->Release().
     *
     * @pre This function must be called after @ref INvSIPLCamera::Init() and before @ref INvSIPLCamera::Deinit().
     *
     * @param[out] item The item retrieved from the queue.
     * @param[in] timeoutUsec The timeout of the request, in microseconds.
     * If the queue is empty at the time of the call,
     * this method will wait up to @c timeoutUsec microseconds
     * for a new item to arrive in the queue and be returned.
     *
     * @retval NVSIPL_STATUS_OK        if @c item has been successfully retrieved from the queue.
     * @retval NVSIPL_STATUS_TIMED_OUT if an item was not available within the timeout interval.
     * @retval NVSIPL_STATUS_EOF       if the queue has been shut down.
     *                                 In this case, no further calls can be made on the queue object.
     * @retval NVSIPL_STATUS_ERROR     if a system error occurred.
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
    virtual SIPLStatus Get(INvSIPLClient::INvSIPLBuffer*& item,
                           size_t const timeoutUsec) = 0;

    /**
     * @brief Return the current queue length.
     *
     * @pre This function must be called after @ref INvSIPLCamera::Init() and before @ref INvSIPLCamera::Deinit().
     *
     * @returns the number of elements currently in the queue.
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
    virtual size_t GetCount() const = 0;

protected:

    /** @brief Default Constructor. */
    INvSIPLFrameCompletionQueue() = default;

    /** @brief Default Destructor. */
    virtual ~INvSIPLFrameCompletionQueue() = default;

private:

    /** @brief Delete copy constructor. */
    INvSIPLFrameCompletionQueue(INvSIPLFrameCompletionQueue &) = delete;

    /** @brief Delete move constructor. */
    INvSIPLFrameCompletionQueue(INvSIPLFrameCompletionQueue &&) = delete;

    /** @brief Delete copy assignment operator. */
    INvSIPLFrameCompletionQueue& operator=(INvSIPLFrameCompletionQueue &) & = delete;

    /** @brief Delete move assignment operator. */
    INvSIPLFrameCompletionQueue& operator=(INvSIPLFrameCompletionQueue &&) & = delete;
};

/**
 * @brief The interface to the notification queue.
 */
class INvSIPLNotificationQueue
{
public:

    /**
     * @brief Retrieve the next item from the queue.
     *
     * @note If the queue is empty at the time of the call, this method will wait
     * up to @c timeoutUsec microseconds for a new item to arrive in the queue and be returned.
     *
     * @pre This function must be called after @ref INvSIPLCamera::Init() and before @ref INvSIPLCamera::Deinit().
     *
     * @param[out] item The item retrieved from the queue.
     * @param[in] timeoutUsec The timeout of the request, in microseconds.
     *
     * @retval NVSIPL_STATUS_OK        if @c item has been successfully retrieved from the queue.
     * @retval NVSIPL_STATUS_TIMED_OUT if an item was not available within the timeout interval.
     * @retval NVSIPL_STATUS_EOF       if the queue has been shut down.
     *                                 In this case, no further calls can be made on the queue object.
     * @retval NVSIPL_STATUS_ERROR     if a system error occurred.
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
    virtual SIPLStatus Get(NvSIPLPipelineNotifier::NotificationData& item,
                           size_t const timeoutUsec) = 0;

    /**
     * @brief Return the current queue length.
     *
     * @pre This function must be called after @ref INvSIPLCamera::Init() and before @ref INvSIPLCamera::Deinit().
     *
     * @returns the number of elements currently in the queue.
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
    virtual size_t GetCount() const = 0;

protected:

    /** @brief Default Constructor. */
    INvSIPLNotificationQueue() = default;

    /** @brief Default Destructor. */
    virtual ~INvSIPLNotificationQueue() = default;

private:

    /** @brief Delete copy constructor. */
    INvSIPLNotificationQueue(INvSIPLNotificationQueue &) = delete;

    /** @brief Delete move constructor. */
    INvSIPLNotificationQueue(INvSIPLNotificationQueue &&) = delete;

    /** @brief Delete copy assignment operator. */
    INvSIPLNotificationQueue& operator=(INvSIPLNotificationQueue &) & = delete;

    /** @brief Delete move assignment operator. */
    INvSIPLNotificationQueue& operator=(INvSIPLNotificationQueue &&) & = delete;
};

/**
 * @brief This is the output structure for @ref INvSIPLCamera::SetPipelineCfg().
 * It contains the queues used by the client to receive completed frames
 * and event notifications.
 */
struct NvSIPLPipelineQueues
{
    /**
     * The queue for completed capture frames.
     * Will be null if capture output was not requested.
     */
    INvSIPLFrameCompletionQueue* captureCompletionQueue {nullptr};

    /**
     * The queue for completed frames from the first ISP output.
     * Will be null if the first ISP output was not requested.
     */
    INvSIPLFrameCompletionQueue* isp0CompletionQueue {nullptr};

    /**
     * The queue for completed frames from the second ISP output.
     * Will be null if the second ISP output was not requested.
     */
    INvSIPLFrameCompletionQueue* isp1CompletionQueue {nullptr};

    /**
     * The queue for completed frames from the third ISP output.
     * Will be null if the third ISP output was not requested.
     */
    INvSIPLFrameCompletionQueue* isp2CompletionQueue {nullptr};

    /** The queue for event notifications. */
    INvSIPLNotificationQueue* notificationQueue {nullptr};
};

/**
 * @brief Holds the queues used by the client to receive device block event notifications.
 */
struct NvSIPLDeviceBlockQueues
{
    /** Queues for event notifications for each device block. */
    INvSIPLNotificationQueue* notificationQueue[MAX_DEVICEBLOCKS_PER_PLATFORM];
};

/** @} */

}  // namespace nvsipl


#endif // NVSIPLPIPELINEMGR_HPP
