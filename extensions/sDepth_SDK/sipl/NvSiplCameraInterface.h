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

/**
 * @file NvSiplCameraInterface.h
 * @brief SIPL Camera Interface for CoE and GMSL Applications
 *
 * This header provides a high-level interface for SIPL camera operations,
 * abstracting away the complexity of direct SIPL API usage.
 *
 * **CORE FEATURES:**
 * - Unified query path supporting both CoE (Camera over Ethernet) and GMSL cameras
 * - JSON-based and database-based camera configuration
 * - Comprehensive buffer and synchronization object management
 * - Built-in NvBufSurface interoperability support
 * - Thread-safe frame delivery with configurable callbacks
 * - Robust error handling and performance monitoring
 * - Sample-specific logging system (SIPL_COE_LOG_*)
 *
 * **CALLBACK ARCHITECTURE (per-sensor):**
 *
 * 1. **SetReadyFrameCallback(sensorIdx, cb) [RECOMMENDED]**
 *    - Receives fully prepared NvSiplFrame with synchronized NvBufSurface
 *    - Automatic buffer management (no manual release required)
 *    - One callback per sensor — routes to the correct output pad
 *
 * 2. **SetEventCallback(sensorIdx, cb)**
 *    - Handles per-sensor pipeline notifications and errors
 *
 * **MULTI-SENSOR SUPPORT:**
 * - Supports N sensors across all modules (CoE or GMSL)
 * - Hawk stereo: 1 module with 2 AR0234 sensors → 2 pipelines
 * - Each sensor has its own pipeline, buffers, threads, and callbacks
 * - Single Init() + Start() call covers all sensors
 *
 * **IMPORTANT LIMITATIONS:**
 * - For CoE: MAC/IP addresses in JSON files must match your camera setup
 * - For GMSL: link-mask must be provided to select active deserializer links
 * - Default JSON templates provided in /var/nvidia/nvcam/settings/sipl/
 */

#ifndef NV_SIPL_CAMERA_INTERFACE_H
#define NV_SIPL_CAMERA_INTERFACE_H

#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <functional>
#include <thread>
#include <atomic>
#include <mutex>
#include <chrono>
#include <iomanip>

// SIPL Core includes
#include "NvSIPLCamera.hpp"
#include "NvSIPLCameraQuery.hpp"
#include "NvSIPLCameraTypes.hpp"
#include "NvSIPLClient.hpp"

// NvSci includes
#include "nvscibuf.h"
#include "nvscisync.h"

// NvBufSurface support
#include "nvbufsurface.h"
#include "nvbufsurftransform.h"
#include "nvbufsurface_nvscibuf.h"

#include "NvNvSciBufHelper.h"
#include "NvSiplCoEUtils.hpp"

using namespace nvsipl;

// **Forward declarations**
struct NvSiplFrame;
struct NvSiplCameraConfig;
class NvSiplCameraInterface;

// **Sample-specific logging system**
extern bool g_verboseDebug;

// SIPL CoE Unit Sample specific logging macros
#define SIPL_COE_LOG_DEBUG(tag, msg) \
    do { if (g_verboseDebug) { \
        std::cout << "[DEBUG][" << tag << "] " << msg << std::endl; \
    } } while(0)

#define SIPL_COE_LOG_INFO(tag, msg) \
    do { if (g_verboseDebug) { \
        std::cout << "[INFO][" << tag << "] " << msg << std::endl; \
    } } while(0)

#define SIPL_COE_LOG_SUCCESS(tag, msg) \
    do { if (g_verboseDebug) { \
        std::cout << "[SUCCESS][" << tag << "] " << msg << std::endl; \
    } } while(0)

#define SIPL_COE_LOG_WARNING(tag, msg) \
    do { \
        std::cout << "[WARNING][" << tag << "] " << msg << std::endl; \
    } while(0)

#define SIPL_COE_LOG_ERROR(msg) \
    do { \
        std::cerr << "[ERROR] " << msg << std::endl; \
    } while(0)

#define SIPL_COE_LOG_CRITICAL(msg) \
    do { \
        std::cout << msg << std::endl; \
    } while(0)

/**
 * @brief Frame data structure providing simplified access to camera frame information
 */
struct NvSiplFrame {
    // Frame metadata
    uint32_t frameNumber;
    uint64_t timestamp;
    uint32_t sensorId;

    // NvBufSurface data (for processing)
    NvBufSurface* nvbufSurface;
    int nvbufSurfaceFd;

    // Raw buffer data (if needed)
    void* rawData;
    size_t rawDataSize;

    // Frame properties
    uint32_t width;
    uint32_t height;
    uint32_t pitch;
    uint32_t colorFormat;

    // Internal tracking (do not modify)
    void* _internal_sipl_buffer;
    void* _internal_interop_buffer;

    NvSiplFrame() : frameNumber(0), timestamp(0), sensorId(0),
                  nvbufSurface(nullptr), nvbufSurfaceFd(-1),
                  rawData(nullptr), rawDataSize(0),
                  width(0), height(0), pitch(0), colorFormat(0),
                  _internal_sipl_buffer(nullptr), _internal_interop_buffer(nullptr) {}
};

/**
 * @brief Camera configuration structure for CoE and GMSL applications
 */
struct NvSiplCameraConfig {
    // Platform configuration
    std::string platformConfigName; // Config name for GetSensorSystemConfig (e.g., "NOVA0", "VB1940")
    std::string jsonFilePath;       // Path to JSON configuration file (CoE or GMSL)
    std::string sensorName;         // Sensor name resolved during caps negotiation
    uint32_t sensorId;

    // GMSL-specific configuration
    std::vector<uint32_t> linkMasks; // Link enable masks per transport — required for GMSL
    bool disableAuth;                // Disable sensor authentication (equivalent to -Z)

    // Pipeline configuration
    bool enableISP0;
    bool enableISP1;
    bool enableISP2;
    bool enableICP;
    bool enableSubframe;
    // 解耦开关（实验7）：captureOutputRequested 控制 pipeline 是否向用户输出 RAW，
    // enableICP 仅控制 ICP buffer 注册/线程。nvsipl_camera 不带 -R 时为
    // captureRequested=false + ICP buffer 恒注册（CNvSIPLMaster registerWithSipl[0]=true）。
    bool captureOutputRequested;

    // YUV 模组修正（对齐厂商 nvsipl_camera 的 CQueryAdapter 行为）：
    // 标准 libnvsipl_query 不识别 s36.json 的 "inputFormat":"yuv422"，
    // 解析后 vcInfo 保持默认 inputFormat=RAW12 / cfa=0（Unknown），
    // 导致 GetImageAttributes(ICP) 返回 GENERIC_ERROR。
    // 置 true 时在 SetPlatformCfg 前强制 vcInfo 为 YUV422/UYVY。
    bool forceYuv422Input;

    // Buffer configuration
    uint32_t maxCaptureBuffers;
    uint32_t maxISP0Buffers;
    uint32_t maxISP1Buffers;
    uint32_t maxISP2Buffers;

    // Advanced options
    bool enableAutoControl;
    uint32_t captureTimeoutMs;
    bool dumpPitchNvbufSurf;
    int32_t NvBufWriteFd;
    NvBufSurface* NvBufWriteSurf;

    // Threading options
    bool useInternalThreads;

    NvSiplCameraConfig() : sensorId(0),
                         disableAuth(false),
                         enableISP0(true), enableISP1(false), enableISP2(false),
                         enableICP(true), enableSubframe(false),
                         captureOutputRequested(true),
                         forceYuv422Input(false),
                         maxCaptureBuffers(4), maxISP0Buffers(64),
                         maxISP1Buffers(64), maxISP2Buffers(64),
                         enableAutoControl(true),
                         captureTimeoutMs(5000),
                         dumpPitchNvbufSurf(false),
                         NvBufWriteFd(-1),
                         NvBufWriteSurf(nullptr),
                         useInternalThreads(true) {}
};

// **Callback function type definitions**
typedef std::function<void(const NvSiplFrame& frame)> SiplReadyFrameCallback;
typedef std::function<void(const NvSIPLPipelineNotifier::NotificationData& notificationData)> SiplEventCallback;

/**
 * @brief Camera statistics structure for performance monitoring
 */
struct NvSiplCameraStats {
    uint32_t totalFramesReceived;
    uint32_t totalFramesProcessed;
    uint32_t totalFramesDropped;
    uint32_t totalEvents;
    uint64_t firstFrameTimestamp;
    uint64_t lastFrameTimestamp;
    double averageFrameRate;
    uint32_t currentQueueDepth;

    NvSiplCameraStats() : totalFramesReceived(0), totalFramesProcessed(0),
                        totalFramesDropped(0), totalEvents(0),
                        firstFrameTimestamp(0), lastFrameTimestamp(0),
                        averageFrameRate(0.0), currentQueueDepth(0) {}
};

/**
 * @brief SIPL Camera Interface for CoE and GMSL Applications
 *
 * This class provides a comprehensive high-level interface for SIPL camera operations,
 * abstracting away the complexity of direct SIPL API usage. It manages:
 * - Unified Query API and configuration management (CoE + GMSL)
 * - Buffer and synchronization object lifecycle
 * - NvBufSurface interoperability for seamless integration
 * - Thread-safe frame delivery via callbacks
 * - Comprehensive error handling and performance statistics
 *
 * **Usage Example:**
 * ```cpp
 * NvSiplCameraInterface camera;
 * NvSiplCameraConfig config;
 * config.platformConfigName = "NOVA0";
 * config.linkMasks = {0x0001};
 *
 * camera.SetSiplCameraConfig(config);
 * camera.SetupCameraSystemConfig();
 * camera.Initialize(config);
 *
 * // Setup buffers and callbacks for each discovered sensor
 * for (uint32_t i = 0; i < camera.GetNumSensors(); i++) {
 *     camera.SetupICPBuffers(i, 4);
 *     camera.SetupISPBuffers(i, 16, true);
 *     camera.RegisterAutoControlPlugin(i);
 *     camera.SetReadyFrameCallback(i, [](const NvSiplFrame& frame) {
 *         // Process frame — NvBufSurface ready, auto-released after callback
 *     });
 * }
 *
 * camera.StartCapture();   // starts ALL sensors together
 * // ... frames delivered via per-sensor callbacks ...
 * camera.StopCapture();    // stops ALL sensors together
 * ```
 */
class NvSiplCameraInterface {
public:
    NvSiplCameraInterface();
    ~NvSiplCameraInterface();

    // **Initialization and Configuration**

    /**
     * @brief Initialize the camera interface with comprehensive error checking
     *
     * This method performs the complete initialization sequence including:
     * - SIPL camera instance creation and validation
     * - Platform configuration with COE system integration
     * - Pipeline configuration for requested output types
     * - Camera hardware initialization and readiness verification
     *
     * The initialization is designed to be idempotent and will return an error
     * if called multiple times without intervening cleanup.
     *
     * @param config Camera configuration parameters (optional, uses stored config if not provided)
     * @return NVSIPL_STATUS_OK on success, appropriate error code on failure
     */
    SIPLStatus Initialize(const NvSiplCameraConfig& config = NvSiplCameraConfig());

    /**
     * @brief Check if the camera interface is initialized
     * @return true if initialized, false otherwise
     */
    bool IsInitialized() const { return m_bInitialized; }

    /**
     * @brief Set camera configuration parameters
     *
     * This method allows configuration to be set before initialization.
     * It provides a way to configure the camera without immediately initializing it,
     * which is useful for applications that need to set up multiple cameras.
     *
     * @param config Complete camera configuration structure
     * @return NVSIPL_STATUS_OK on success
     */
    SIPLStatus SetSiplCameraConfig(const NvSiplCameraConfig& config);

    /**
     * @brief Setup ISP (Image Signal Processor) buffers with comprehensive validation
     *
     * This method configures the buffer infrastructure for ISP output processing.
     * It handles the complex NvSci buffer attribute negotiation and ensures
     * compatibility between SIPL requirements and application needs.
     *
     * The setup process includes:
     * - Buffer attribute list creation and reconciliation
     * - Buffer object allocation and registration with SIPL
     * - Synchronization object setup for frame-level coordination
     *
     * @param bufferCount Number of buffers to allocate in the pool
     * @param bl_output True if output is in block linear format, false if in pitch linear format
     * @return NVSIPL_STATUS_OK on success, error code on failure
     */
    SIPLStatus SetupISPBuffers(uint32_t sensorIdx, uint32_t bufferCount, bool bl_output);

    /**
     * @brief Prepare ISP buffer attributes and sync objects for a specific sensor
     *
     * @param sensorIdx Index into the sensor pipeline array (0-based)
     * @param bl_output True if output is in block linear format
     * @return NVSIPL_STATUS_OK on success, error code on failure
     */
    SIPLStatus PrepareISPBuffers(uint32_t sensorIdx, bool bl_output);

    /**
     * @brief Setup ICP buffers for a specific sensor
     *
     * @param sensorIdx Index into the sensor pipeline array (0-based)
     * @param bufferCount Number of ICP buffers to allocate
     * @return NVSIPL_STATUS_OK on success, error code on failure
     */
    SIPLStatus SetupICPBuffers(uint32_t sensorIdx, uint32_t bufferCount);

    /**
     * @brief Pre-Init buffer preparation (实验10)
     *
     * 对齐 nvsipl_camera 真机序列：ICP/ISP0 的 GetImageAttributes + NvSciBufObj
     * 分配必须在 INvSIPLCamera::Init() 之前（nvsipl_camera main.cpp 的
     * AllocateBuffers@3137 循环在 Init@3203 之前；Init@3096 是 NVMEDIA_HAS_EMU
     * 仿真分支真机不编译）。Init 后再 GetImageAttributes 会导致 secondary
     * sensor 返回 INVALID_STATE。
     *
     * 用法：Initialize() 内部会在 Init() 前自动调用（读 maxCaptureBuffers/
     * maxISP0Buffers 配置）。SetupICPBuffers/SetupISPBuffers 检测到已预分配
     * 的 buffer 时直接复用并只做注册。
     */
    SIPLStatus PrepareICPBuffers(uint32_t sensorIdx, uint32_t bufferCount);
    SIPLStatus PrepareISP0Buffers(uint32_t sensorIdx, bool bl_output, uint32_t bufferCount);

    /**
     * @brief Setup camera system configuration using COE Query API
     *
     * This method initializes the COE Query API with JSON configuration and
     * populates the camera system configuration with discovered devices.
     * It handles the complete device discovery and configuration process.
     *
     * @return NVSIPL_STATUS_OK on success, error code otherwise
     */
    SIPLStatus SetupCameraSystemConfig();

    /**
     * @brief Force all sensor VCs to YUV422/UYVY input format
     *
     * Replicates the vendor nvsipl_camera CQueryAdapter fixup for YUV modules
     * (e.g. S36/ISX031): the standard query parser does not understand
     * "inputFormat":"yuv422" and leaves vcInfo at the RAW12 default.
     * Called from Initialize() before SetPlatformCfg when
     * NvSiplCameraConfig::forceYuv422Input is true.
     */
    void ForceYuv422InputFormat();

    /**
     * @brief Get sensor system configuration with discovered modules
     *
     * @pre SetupCameraSystemConfig() must have completed successfully.
     *      Calling before that results in undefined behavior.
     *
     * @return Reference to the sensor system configuration.
     */
    const sensorconfig::SensorSystemConfig& GetSensorSystemConfig() const { return *m_sensorSystemConfig; }

    /**
     * @brief Register auto control plugin for a specific sensor
     *
     * @param sensorIdx Index into the sensor pipeline array (0-based)
     * @return NVSIPL_STATUS_OK on success, error code otherwise
     */
    SIPLStatus RegisterAutoControlPlugin(uint32_t sensorIdx);

    /**
     * @brief Get the number of discovered sensors
     * @return Number of sensors (available after Initialize)
     */
    uint32_t GetNumSensors() const { return m_numSensors; }

    // **Callback Registration**

    /**
     * @brief Set ready-to-use frame callback for a specific sensor (RECOMMENDED)
     *
     * The callback receives a fully prepared NvSiplFrame with NvBufSurface ready,
     * sync objects waited on, and automatic buffer management.
     *
     * @param sensorIdx Index into the sensor pipeline array (0-based)
     * @param callback Function to call for each ISP0 frame
     */
    void SetReadyFrameCallback(uint32_t sensorIdx, SiplReadyFrameCallback callback);

    /**
     * @brief Set capture (ICP) frame callback for a specific sensor
     *
     * YUV 模组（如 S36/ISX031）不走 ISP，帧从 captureCompletionQueue 直出。
     * 仅在 captureOutputRequested=true 时有效；回调语义与 SetReadyFrameCallback 一致
     * （NvBufSurface 已就绪，回调返回后自动释放）。
     *
     * @param sensorIdx Index into the sensor pipeline array (0-based)
     * @param callback Function to call for each ICP capture frame
     */
    void SetCaptureFrameCallback(uint32_t sensorIdx, SiplReadyFrameCallback callback);

    /**
     * @brief Set callback for camera events for a specific sensor
     *
     * @param sensorIdx Index into the sensor pipeline array (0-based)
     * @param callback Function to call when events occur
     */
    void SetEventCallback(uint32_t sensorIdx, SiplEventCallback callback);

    // **Camera Operations**

    /**
     * @brief Start camera capture with full thread and resource management
     *
     * This method initiates the camera capture process by:
     * - Starting all processing threads for concurrent frame handling
     * - Beginning camera streaming from the sensor
     * - Initializing performance monitoring and statistics collection
     *
     * The capture process is designed to be non-blocking, with frame processing
     * handled asynchronously through registered callbacks or queuing mechanisms.
     *
     * @return NVSIPL_STATUS_OK on success, error code on failure
     */
    SIPLStatus StartCapture();

    /**
     * @brief Stop camera capture with comprehensive cleanup
     *
     * This method performs an orderly shutdown of the capture process:
     * - Stops camera streaming to halt new frame generation
     * - Terminates all processing threads gracefully
     * - Clears any pending frames from processing queues
     * - Resets capture state for potential restart
     *
     * @return NVSIPL_STATUS_OK on success, error code on failure
     */
    SIPLStatus StopCapture();

    /**
     * @brief Check if camera is currently capturing
     * @return true if capturing, false otherwise
     */
    bool IsCapturing() const { return m_bCapturing; }

    // **Statistics and Monitoring**

    /**
     * @brief Get comprehensive camera statistics for performance monitoring
     *
     * This method provides detailed performance metrics including:
     * - Frame count statistics (received, processed, dropped)
     * - Timing information and frame rate calculations
     * - Queue depth and system resource utilization
     * - Event count for system monitoring
     *
     * @return Complete statistics structure with current performance data
     */
    NvSiplCameraStats GetStatistics() const;

    /**
     * @brief Reset all performance statistics counters
     *
     * This method clears all accumulated performance data and starts fresh measurement.
     * It's typically called at the beginning of capture sessions or when resetting
     * performance monitoring for specific test scenarios.
     */
    void ResetStatistics();

    /**
     * @brief Print comprehensive statistics to console for debugging
     *
     * This method outputs formatted performance statistics to the console,
     * providing detailed information about camera operation for debugging
     * and performance analysis purposes.
     */
    void PrintStatistics() const;

    /**
     * @brief Save current frame to file with format conversion
     *
     * This method saves frame data to a file stream with optional format conversion.
     * It handles the complete pipeline including color space conversion and
     * proper file format output.
     *
     * @param frame Frame to save (must contain valid NvBufSurface data)
     * @param output_stream Pre-opened output file stream for writing
     * @return NVSIPL_STATUS_OK on success, error code on failure
     */
    SIPLStatus SaveFrameToFile(const NvSiplFrame& frame, std::ofstream* output_stream);

    /**
     * @brief Get raw SIPL camera object (for advanced users)
     * @return Pointer to SIPL camera interface (use with caution)
     */
    INvSIPLCamera* GetRawSiplCamera() { return m_upCamera.get(); }

    /**
     * @brief Get pipeline queues for a specific sensor
     * @param sensorIdx Index into the sensor pipeline array (0-based)
     * @return Pipeline queues structure
     */
    const NvSIPLPipelineQueues& GetPipelineQueues(uint32_t sensorIdx) const;

    /**
     * @brief Get CPU wait context for fence synchronization
     * @return CPU wait context for external thread usage
     */
    NvSciSyncCpuWaitContext GetCpuWaitContext() const { return m_cpuWaitContext; }

    /**
     * @brief Create a new NvSci buffer compatible with SIPL for a specific sensor
     *
     * Used with PrepareISPBuffers() for on-demand buffer allocation by external
     * allocators (e.g., GStreamer buffer pool).
     *
     * @param sensorIdx Index into the sensor pipeline array (0-based)
     * @param bufObj Output buffer object
     * @param nvbufSurface Output NvBufSurface for interop
     * @return NVSIPL_STATUS_OK on success
     */
    SIPLStatus CreateInteropBuffer(uint32_t sensorIdx, NvSciBufObj* bufObj, NvBufSurface** nvbufSurface);

    /**
     * @brief Register a created buffer object with SIPL for a specific sensor
     * @param sensorIdx Index into the sensor pipeline array (0-based)
     * @param bufObj Buffer object to register
     * @return NVSIPL_STATUS_OK on success
     */
    SIPLStatus RegisterInteropBuffer(uint32_t sensorIdx, NvSciBufObj bufObj);

    /**
     * @brief Finalize buffer registration for a specific sensor
     * Must be called after all buffers are created and before starting capture
     * @param sensorIdx Index into the sensor pipeline array (0-based)
     * @return NVSIPL_STATUS_OK on success
     */
    SIPLStatus FinalizeBufferRegistration(uint32_t sensorIdx);

    /**
     * @brief Clean up all resources and stop capture
     */
    void Cleanup();

private:
    // **Per-sensor interop buffer**
    struct InternalInteropBuffer {
        NvSciBufObj sci_buf_obj;
        NvBufSurface* nvbuf_surf;
        int nvbufsurface_fd;
        bool is_valid;
        bool in_use;
        INvSIPLClient::INvSIPLBuffer* sipl_buffer;

        InternalInteropBuffer() : sci_buf_obj(nullptr), nvbuf_surf(nullptr),
                                  nvbufsurface_fd(-1), is_valid(false),
                                  in_use(false), sipl_buffer(nullptr) {}
    };

    // **Per-sensor pipeline state — fully isolated, no cross-sensor mutable state**
    struct SensorPipeline {
        uint32_t sensorId = 0;
        std::string sensorName;
        NvSIPLPipelineConfiguration pipelineConfig {};
        NvSIPLPipelineQueues pipelineQueues {};

        std::vector<NvSciBufObj> bufObjects;
        std::vector<NvSciBufObj> icpBufObjects;
        NvSciBufAttrList bufAttrList = nullptr;
        NvSciBufAttrList icpBufAttrList = nullptr;
        std::vector<InternalInteropBuffer> interopBuffers;
        // ICP 直出帧几何缓存（YUV 模组）。raw ICP buffer 是 packed 格式，
        // NvmmNvSciBufToNvBufSurface 无法转换（Thor r39.2 getColorFormatInfo
        // 拒绝所有 packed enum），按厂商示例改为 CPU 直读 + 几何按 plane key 查询
        uint32_t icpFrameWidth = 0;
        uint32_t icpFrameHeight = 0;
        uint32_t icpFramePitch = 0;

        NvSciSyncObj syncObj = nullptr;
        NvSciSyncObj icpSyncObj = nullptr;

        std::thread isp0Thread;
        std::thread icpThread;
        std::thread eventThread;
        std::unique_ptr<std::mutex> interopMutex;

        SensorPipeline() : interopMutex(std::make_unique<std::mutex>()) {}
        SensorPipeline(SensorPipeline&&) = default;
        SensorPipeline& operator=(SensorPipeline&&) = default;

        SiplReadyFrameCallback readyFrameCallback;
        SiplReadyFrameCallback captureFrameCallback;  // ICP 直出帧（YUV 模组，captureOutputRequested=true）
        SiplEventCallback eventCallback;
    };

    // **Internal state**
    bool m_bInitialized;
    std::atomic<bool> m_bCapturing;
    std::atomic<bool> m_bThreadsRunning;
    NvSiplCameraConfig m_config;
    NvSiplCameraStats m_stats;

    // **SIPL objects (shared — read-only after Init)**
    std::unique_ptr<INvSIPLCamera> m_upCamera;
    std::unique_ptr<INvSIPLCameraQuery> m_upQuery;
    std::unique_ptr<sensorconfig::SensorSystemConfig> m_sensorSystemConfig;

    // **Per-sensor pipelines**
    std::vector<SensorPipeline> m_sensorPipelines;
    uint32_t m_numSensors = 0;

    // **NvSci modules (shared — read-only after Init)**
    NvSciBufModule m_sciBufModule;
    NvSciSyncModule m_sciSyncModule;
    NvSciSyncCpuWaitContext m_cpuWaitContext;

    // **Mutexes**
    mutable std::mutex m_statsMutex;

    // **Internal methods**
    SIPLStatus InitializeNvSciModules();
    SIPLStatus SetupBufferAttributes(uint32_t sensorIdx, bool bl_output);
    SIPLStatus SetupSyncObjects(uint32_t sensorIdx);
    SIPLStatus SetupIcpBufferAttributes(uint32_t sensorIdx);
    SIPLStatus SetupIcpSyncObjects(uint32_t sensorIdx);
    SIPLStatus StartThreads();
    SIPLStatus RegisterBuffersWithSIPL(uint32_t sensorIdx, uint32_t bufferCount);
    SIPLStatus RegisterIcpBuffersWithSIPL(uint32_t sensorIdx, uint32_t bufferCount);
    void StopThreads();

    // **Per-sensor thread functions**
    void ISP0ThreadFunc(uint32_t sensorIdx);
    void ICPThreadFunc(uint32_t sensorIdx);
    void EventThreadFunc(uint32_t sensorIdx);

    // **Per-sensor frame processing**
    void ProcessISP0Frame(uint32_t sensorIdx, INvSIPLClient::INvSIPLBuffer* pBuffer);
    void ProcessICPFrame(uint32_t sensorIdx, INvSIPLClient::INvSIPLBuffer* pBuffer);

    // **Utilities**
    void ReleaseFrame(uint32_t sensorIdx, NvSiplFrame& frame);
    void UpdateStatistics(const NvSiplFrame& frame);
    void CleanupSensorPipeline(SensorPipeline& sp);
};

#endif // NVSIPL_CAMERA_INTERFACE_H
