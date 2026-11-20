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
 * @file NvSiplCameraInterface.cpp
 * @brief Implementation of SIPL Camera Interface for Production Applications
 *
 * This implementation provides a comprehensive high-level interface for SIPL camera operations,
 * The interface abstracts complex SIPL API interactions while maintaining functionality and performance.
 *
 * Key architectural decisions:
 * - Thread-safe design with proper mutex protection for all shared resources
 * - Multiple callback mechanisms to support different application requirements
 * - Comprehensive error handling with graceful degradation
 * - Memory management with automatic cleanup and resource tracking
 * - COE (Camera Over Ethernet) integration with JSON configuration support
 */

#include "NvSiplCameraInterface.h"
#include <chrono>
#include <algorithm>
#include <iomanip>
#include <fstream>

static const uint32_t DEFAULT_NUM_BUFFERS = 4;        // Balanced buffer count for latency vs memory
static const uint32_t FENCE_FRAME_TIMEOUT_MS = 100UL; // Conservative timeout for fence operations

bool g_verboseDebug = false; // Set to false for production release

/**
 * @brief Constructor - Initialize camera interface with safe defaults
 */
NvSiplCameraInterface::NvSiplCameraInterface()
    : m_bInitialized(false), m_bCapturing(false), m_bThreadsRunning(false)
    , m_sciBufModule(nullptr), m_sciSyncModule(nullptr), m_cpuWaitContext(nullptr) {

    InitializeNvSciModules();
    SIPL_COE_LOG_DEBUG("SIPL_INTERFACE", "Camera interface instance created successfully");
}

/**
 * @brief Destructor - Ensure all resources are properly released
 */
NvSiplCameraInterface::~NvSiplCameraInterface() {
    Cleanup();
    SIPL_COE_LOG_DEBUG("SIPL_INTERFACE", "Camera interface instance destroyed");
}

SIPLStatus NvSiplCameraInterface::SetSiplCameraConfig(const NvSiplCameraConfig& config)
{
    m_config = config;
    return NVSIPL_STATUS_OK;
}

void NvSiplCameraInterface::SetReadyFrameCallback(uint32_t sensorIdx, SiplReadyFrameCallback callback) {
    if (sensorIdx < m_numSensors) {
        m_sensorPipelines[sensorIdx].readyFrameCallback = callback;
    }
}

void NvSiplCameraInterface::SetEventCallback(uint32_t sensorIdx, SiplEventCallback callback) {
    if (sensorIdx < m_numSensors) {
        m_sensorPipelines[sensorIdx].eventCallback = callback;
    }
}

void NvSiplCameraInterface::SetCaptureFrameCallback(uint32_t sensorIdx, SiplReadyFrameCallback callback) {
    if (sensorIdx < m_numSensors) {
        m_sensorPipelines[sensorIdx].captureFrameCallback = callback;
    }
}

const NvSIPLPipelineQueues& NvSiplCameraInterface::GetPipelineQueues(uint32_t sensorIdx) const {
    return m_sensorPipelines[sensorIdx].pipelineQueues;
}

/**
 * @brief Initialize the camera interface for all discovered sensors
 *
 * Iterates modules → sensorConfigs to call SetPipelineCfg per sensor.
 * One SetPlatformCfg and one Init call covers all sensors.
 * Start() will start all sensors together.
 */
SIPLStatus NvSiplCameraInterface::Initialize(const NvSiplCameraConfig& config) {
    SIPL_COE_LOG_INFO("SIPL_INIT", "Initializing SIPL Camera Interface (multi-sensor)");

    if (m_bInitialized) {
        SIPL_COE_LOG_ERROR("Camera interface already initialized - cleanup required before re-initialization");
        return NVSIPL_STATUS_ERROR;
    }

    m_config = config;

    m_upCamera = INvSIPLCamera::GetInstance();
    if (!m_upCamera) {
        SIPL_COE_LOG_ERROR("Failed to obtain SIPL camera instance");
        return NVSIPL_STATUS_ERROR;
    }

    if (m_config.forceYuv422Input) {
        ForceYuv422InputFormat();
    }

    auto status = m_upCamera->SetPlatformCfg(*m_sensorSystemConfig);
    if (status != NVSIPL_STATUS_OK) {
        SIPL_COE_LOG_ERROR("Platform configuration failed - verify camera hardware connectivity");
        return status;
    }

    // Count total sensors across all modules (modules → sensorConfigs)
    m_numSensors = 0;
    for (const auto& module : m_sensorSystemConfig->modules) {
        std::visit([&](const auto& modType) {
            m_numSensors += static_cast<uint32_t>(modType.sensorConfigs.size());
        }, module.moduleType);
    }

    if (m_numSensors == 0) {
        SIPL_COE_LOG_ERROR("No sensors found in system config");
        return NVSIPL_STATUS_ERROR;
    }

    m_sensorPipelines.resize(m_numSensors);
    SIPL_COE_LOG_INFO("SIPL_INIT", "Configuring " << m_numSensors << " sensor pipelines");

    // SetPipelineCfg for EACH sensor (not each module)
    uint32_t pipelineIdx = 0;
    for (const auto& module : m_sensorSystemConfig->modules) {
        std::visit([&](const auto& modType) {
            for (size_t s = 0; s < modType.sensorConfigs.size(); s++) {
                uint32_t sensorId = std::visit([](const auto& cfg) -> uint32_t {
                    return cfg.id;
                }, modType.sensorConfigs[s]);

                auto& sp = m_sensorPipelines[pipelineIdx];
                sp.sensorId = sensorId;
                sp.sensorName = module.name;

                // 实验7解耦：capture 输出请求独立于 ICP buffer 注册
                // （对齐 nvsipl_camera：不带 -R → captureRequested=false，但 ICP buffer 恒注册）
                sp.pipelineConfig.captureOutputRequested = config.captureOutputRequested;
                sp.pipelineConfig.isp0OutputRequested = config.enableISP0;
                sp.pipelineConfig.isp1OutputRequested = config.enableISP1;
                sp.pipelineConfig.isp2OutputRequested = config.enableISP2;
                sp.pipelineConfig.disableSubframe = !config.enableSubframe;
                sp.pipelineConfig.bufferCfg.maxCaptureBufferCount = config.maxCaptureBuffers;
                sp.pipelineConfig.bufferCfg.maxIsp0BufferCount = config.maxISP0Buffers;
                sp.pipelineConfig.bufferCfg.maxIsp1BufferCount = config.maxISP1Buffers;
                sp.pipelineConfig.bufferCfg.maxIsp2BufferCount = config.maxISP2Buffers;

                auto s2 = m_upCamera->SetPipelineCfg(sensorId, sp.pipelineConfig, sp.pipelineQueues);
                if (s2 != NVSIPL_STATUS_OK) {
                    SIPL_COE_LOG_ERROR("SetPipelineCfg failed for sensorId " << sensorId);
                    status = s2;
                } else {
                    SIPL_COE_LOG_INFO("SIPL_INIT", "Pipeline configured for sensor " << sensorId
                                  << " (" << module.name << ")");
                }
                pipelineIdx++;
            }
        }, module.moduleType);
    }
    if (status != NVSIPL_STATUS_OK) {
        return status;
    }

    // 实验10：对齐 nvsipl_camera 真机序列——GetImageAttributes + buffer 分配必须在
    // Init() 之前（main.cpp AllocateBuffers@3137 < Init@3203；Init@3096 为 EMU 仿真分支，
    // 真机不编译）。vendor 原在 Init 后才做 GetImageAttributes，导致 sensor 13 返回
    // INVALID_STATE(6)。
    // YUV 模组（enableISP0=false）无 ISP0 输出，跳过 ISP0 属性获取（对 YUV pipeline
    // 请求 ISP0 属性会失败）。
    for (uint32_t i = 0; i < m_numSensors; i++) {
        if (m_config.enableICP) {
            auto ps = PrepareICPBuffers(i, m_config.maxCaptureBuffers);
            if (ps != NVSIPL_STATUS_OK) {
                SIPL_COE_LOG_ERROR("PrepareICPBuffers failed for sensor " << i);
                return ps;
            }
        }
        if (m_config.enableISP0) {
            auto ps = PrepareISP0Buffers(i, true /*BL*/, m_config.maxISP0Buffers);
            if (ps != NVSIPL_STATUS_OK) {
                SIPL_COE_LOG_ERROR("PrepareISP0Buffers failed for sensor " << i);
                return ps;
            }
        }
    }

    status = m_upCamera->Init();
    if (status != NVSIPL_STATUS_OK) {
        SIPL_COE_LOG_ERROR("Camera Init failed - verify sensor power and connectivity");
        return status;
    }

    m_bInitialized = true;
    ResetStatistics();

    SIPL_COE_LOG_SUCCESS("SIPL_INIT", "Camera interface initialized with " << m_numSensors << " sensors");
    return NVSIPL_STATUS_OK;
}

SIPLStatus NvSiplCameraInterface::SetupISPBuffers(uint32_t sensorIdx, uint32_t bufferCount, bool bl_output)
{
    if (sensorIdx >= m_numSensors) return NVSIPL_STATUS_BAD_ARGUMENT;

    if (!m_config.enableISP0) {
        SIPL_COE_LOG_ERROR("ISP buffer setup requested but ISP0 not enabled");
        return NVSIPL_STATUS_ERROR;
    }

    auto& sp = m_sensorPipelines[sensorIdx];
    // 实验10：Init 前已由 PrepareISP0Buffers 预分配（attr+buffer），跳过 attr 获取
    if (sp.bufAttrList == nullptr) {
        auto status0 = SetupBufferAttributes(sensorIdx, bl_output);
        if (status0 != NVSIPL_STATUS_OK) return status0;
    }

    auto status = RegisterBuffersWithSIPL(sensorIdx, bufferCount);
    if (status != NVSIPL_STATUS_OK) return status;

    status = SetupSyncObjects(sensorIdx);
    if (status != NVSIPL_STATUS_OK) return status;

    SIPL_COE_LOG_SUCCESS("BUFFERS", "ISP buffers setup for sensor " << sensorIdx);
    return NVSIPL_STATUS_OK;
}

SIPLStatus NvSiplCameraInterface::PrepareISPBuffers(uint32_t sensorIdx, bool bl_output)
{
    if (sensorIdx >= m_numSensors) return NVSIPL_STATUS_BAD_ARGUMENT;

    if (!m_config.enableISP0) {
        SIPL_COE_LOG_ERROR("ISP buffer prep requested but ISP0 not enabled");
        return NVSIPL_STATUS_ERROR;
    }

    auto status = SetupBufferAttributes(sensorIdx, bl_output);
    if (status != NVSIPL_STATUS_OK) return status;

    status = SetupSyncObjects(sensorIdx);
    if (status != NVSIPL_STATUS_OK) return status;

    SIPL_COE_LOG_SUCCESS("BUFFERS", "ISP buffers prepared for sensor " << sensorIdx);
    return NVSIPL_STATUS_OK;
}

SIPLStatus NvSiplCameraInterface::SetupICPBuffers(uint32_t sensorIdx, uint32_t bufferCount)
{
    if (sensorIdx >= m_numSensors) return NVSIPL_STATUS_BAD_ARGUMENT;

    if (!m_config.enableICP) {
        SIPL_COE_LOG_ERROR("ICP buffer setup requested but ICP not enabled");
        return NVSIPL_STATUS_ERROR;
    }

    auto& sp = m_sensorPipelines[sensorIdx];
    // 实验10：Init 前已由 PrepareICPBuffers 预分配（attr+buffer），跳过 attr 获取
    if (sp.icpBufAttrList == nullptr) {
        auto status = SetupIcpBufferAttributes(sensorIdx);
        if (status != NVSIPL_STATUS_OK) return status;
    }

    auto status = RegisterIcpBuffersWithSIPL(sensorIdx, bufferCount);
    if (status != NVSIPL_STATUS_OK) return status;

    status = SetupIcpSyncObjects(sensorIdx);
    if (status != NVSIPL_STATUS_OK) return status;

    SIPL_COE_LOG_SUCCESS("ICP", "ICP buffers setup for sensor " << sensorIdx);
    return NVSIPL_STATUS_OK;
}

/**
 * @brief Start camera capture with full thread and resource management
 */
SIPLStatus NvSiplCameraInterface::StartCapture() {
    if (!m_bInitialized) {
        SIPL_COE_LOG_ERROR("Cannot start capture - camera interface not initialized");
        return NVSIPL_STATUS_ERROR;
    }

    if (m_bCapturing) {
        SIPL_COE_LOG_ERROR("Capture already in progress - stop current session before starting new one");
        return NVSIPL_STATUS_ERROR;
    }

    SIPL_COE_LOG_INFO("CAPTURE", "Starting camera capture session");

    auto status = StartThreads();
    if (status != NVSIPL_STATUS_OK) {
        SIPL_COE_LOG_ERROR("Failed to start processing threads - insufficient system resources");
        return status;
    }

    status = m_upCamera->Start();
    if (status != NVSIPL_STATUS_OK) {
        SIPL_COE_LOG_ERROR("Camera streaming start failed - check sensor connectivity and power");
        StopThreads();
        return status;
    }

    m_bCapturing = true;
    ResetStatistics();

    SIPL_COE_LOG_SUCCESS("CAPTURE", "Camera capture session started successfully");
    return NVSIPL_STATUS_OK;
}

SIPLStatus NvSiplCameraInterface::StopCapture() {
    if (!m_bCapturing) {
        return NVSIPL_STATUS_OK;
    }

    SIPL_COE_LOG_INFO("CAPTURE", "Stopping camera capture session");
    m_bCapturing = false;

    auto status = m_upCamera->Stop();
    if (status != NVSIPL_STATUS_OK) {
        SIPL_COE_LOG_ERROR("Camera Stop encountered error - continuing with cleanup");
    }

    StopThreads();

    SIPL_COE_LOG_SUCCESS("CAPTURE", "Camera capture session stopped");
    return status;
}

/**
 * @brief Release a processed frame's resources back to SIPL
 *
 * Returns the SIPL buffer to the completion queue pool and marks the
 * interop buffer as available. Uses per-sensor mutex (sp.interopMutex)
 * to avoid cross-sensor contention.
 *
 * Called automatically after readyFrameCallback returns — users don't
 * need to call this manually.
 */
void NvSiplCameraInterface::ReleaseFrame(uint32_t sensorIdx, NvSiplFrame& frame) {
    if (!frame._internal_sipl_buffer) return;

    INvSIPLClient::INvSIPLBuffer* siplBuffer =
        static_cast<INvSIPLClient::INvSIPLBuffer*>(frame._internal_sipl_buffer);
    siplBuffer->Release();

    if (frame._internal_interop_buffer && sensorIdx < m_numSensors) {
        auto& sp = m_sensorPipelines[sensorIdx];
        InternalInteropBuffer* interopBuffer =
            static_cast<InternalInteropBuffer*>(frame._internal_interop_buffer);
        std::lock_guard<std::mutex> lock(*sp.interopMutex);
        interopBuffer->in_use = false;
        interopBuffer->sipl_buffer = nullptr;
    }

    frame._internal_sipl_buffer = nullptr;
    frame._internal_interop_buffer = nullptr;
}

NvSiplCameraStats NvSiplCameraInterface::GetStatistics() const {
    std::lock_guard<std::mutex> lock(m_statsMutex);
    NvSiplCameraStats stats = m_stats;

    if (stats.totalFramesReceived > 1 && stats.lastFrameTimestamp > stats.firstFrameTimestamp) {
        double timeDiff = (stats.lastFrameTimestamp - stats.firstFrameTimestamp) / 1000000.0;
        stats.averageFrameRate = (stats.totalFramesReceived - 1) / timeDiff;
    }

    return stats;
}

void NvSiplCameraInterface::ResetStatistics() {
    std::lock_guard<std::mutex> lock(m_statsMutex);
    m_stats = NvSiplCameraStats();
    SIPL_COE_LOG_DEBUG("STATS", "Performance statistics reset");
}

void NvSiplCameraInterface::PrintStatistics() const {
    auto stats = GetStatistics();

    SIPL_COE_LOG_CRITICAL("\n=== Camera Interface Performance Statistics ===");
    SIPL_COE_LOG_CRITICAL("Configuration: " << m_config.platformConfigName);
    SIPL_COE_LOG_CRITICAL("Sensor: " << m_config.sensorName << " (ID: " << m_config.sensorId << ")");
    SIPL_COE_LOG_CRITICAL("Total frames received: " << stats.totalFramesReceived);
    SIPL_COE_LOG_CRITICAL("Total frames processed: " << stats.totalFramesProcessed);
    SIPL_COE_LOG_CRITICAL("Total frames dropped: " << stats.totalFramesDropped);
    SIPL_COE_LOG_CRITICAL("Total events: " << stats.totalEvents);
    SIPL_COE_LOG_CRITICAL("Current queue depth: " << stats.currentQueueDepth);
    SIPL_COE_LOG_CRITICAL("Average frame rate: " << std::fixed << std::setprecision(2) << stats.averageFrameRate << " FPS");
    SIPL_COE_LOG_CRITICAL("================================================");
}

/**
 * @brief Save current frame to file
 */
SIPLStatus NvSiplCameraInterface::SaveFrameToFile(const NvSiplFrame& frame, std::ofstream* output_stream) {
    int ret = 0;
    if (!frame.nvbufSurface && frame.nvbufSurfaceFd == -1) {
        SIPL_COE_LOG_ERROR("No NvBufSurface data to save");
        return NVSIPL_STATUS_ERROR;
    }

    if (m_config.dumpPitchNvbufSurf && m_config.NvBufWriteSurf == nullptr)
    {
        NvBufSurfaceAllocateParams input_params = {{0}};
        input_params.params.width = frame.nvbufSurface->surfaceList[0].planeParams.width[0];
        input_params.params.height = frame.nvbufSurface->surfaceList[0].planeParams.height[0];
        input_params.params.layout = NVBUF_LAYOUT_PITCH;
        input_params.params.colorFormat = NVBUF_COLOR_FORMAT_YUV420;
        input_params.params.memType = NVBUF_MEM_SURFACE_ARRAY;
        input_params.memtag = NvBufSurfaceTag_VIDEO_CONVERT;

        NvBufSurfaceAllocate(&m_config.NvBufWriteSurf, 1, &input_params);
        m_config.NvBufWriteSurf->numFilled = 1;
        m_config.NvBufWriteFd = m_config.NvBufWriteSurf->surfaceList[0].bufferDesc;
    }

    {
        NvBufSurfTransformParams transform_params = {0};
        NvBufSurfTransformRect src_rect = {0}, dest_rect = {0};
        NvBufSurfTransformConfigParams config_params = {NvBufSurfTransformCompute_VIC, 0, NULL};

        src_rect.top = 0;
        src_rect.left = 0;
        src_rect.width = frame.nvbufSurface->surfaceList[0].planeParams.width[0];
        src_rect.height = frame.nvbufSurface->surfaceList[0].planeParams.height[0];
        dest_rect.top = 0;
        dest_rect.left = 0;
        dest_rect.width = m_config.NvBufWriteSurf->surfaceList[0].planeParams.width[0];
        dest_rect.height = m_config.NvBufWriteSurf->surfaceList[0].planeParams.height[0];
        memset(&transform_params, 0, sizeof(transform_params));
        transform_params.src_rect = &src_rect;
        transform_params.dst_rect = &dest_rect;
        NvBufSurfTransformSetSessionParams (&config_params);
        NvBufSurfTransform(frame.nvbufSurface, m_config.NvBufWriteSurf, &transform_params);
    }

    void* virtualop_data_addr = nullptr;
    unsigned int plane = 0;

    for (plane = 0; plane < m_config.NvBufWriteSurf->surfaceList[0].planeParams.num_planes; ++plane)
    {
        ret = NvBufSurfaceMap(m_config.NvBufWriteSurf, 0, plane, NVBUF_MAP_READ_WRITE);
        if (ret == 0)
        {
            unsigned int i = 0;
            virtualop_data_addr = (void *)m_config.NvBufWriteSurf->surfaceList[0].mappedAddr.addr[plane];

            NvBufSurfaceSyncForCpu(m_config.NvBufWriteSurf, 0, plane);
            for (i = 0; i < m_config.NvBufWriteSurf->surfaceList[0].planeParams.height[plane]; ++i)
            {
                std::streamsize bytes_to_write = m_config.NvBufWriteSurf->surfaceList[0].planeParams.width[plane];
                output_stream->write((char *)virtualop_data_addr + i * m_config.NvBufWriteSurf->surfaceList[0].planeParams.pitch[plane],
                                     bytes_to_write);
                if (!output_stream->good())
                {
                    std::cerr << "File write failure" << std::endl;
                    return NVSIPL_STATUS_ERROR;
                }
            }
        }
        NvBufSurfaceUnMap(m_config.NvBufWriteSurf, 0, plane);
    }

    return NVSIPL_STATUS_OK;
}

/**
 * @brief Release all resources for a single sensor pipeline
 *
 * Cleanup order: interop buffers → sync objects → NvSciBuf objects → attr lists
 * Must be called after all threads for this sensor have been joined.
 */
void NvSiplCameraInterface::CleanupSensorPipeline(SensorPipeline& sp) {
    // Release interop buffers
    {
        std::lock_guard<std::mutex> lock(*sp.interopMutex);
        for (auto& buffer : sp.interopBuffers) {
            if (buffer.is_valid) {
                if (buffer.sipl_buffer) {
                    static_cast<INvSIPLClient::INvSIPLBuffer*>(buffer.sipl_buffer)->Release();
                    buffer.sipl_buffer = nullptr;
                }
                if (buffer.nvbuf_surf) {
                    NvBufSurfaceDestroy(buffer.nvbuf_surf);
                    buffer.nvbuf_surf = nullptr;
                }
                buffer.is_valid = false;
            }
        }
        sp.interopBuffers.clear();
    }

    if (sp.syncObj) { NvSciSyncObjFree(sp.syncObj); sp.syncObj = nullptr; }
    if (sp.icpSyncObj) { NvSciSyncObjFree(sp.icpSyncObj); sp.icpSyncObj = nullptr; }

    for (auto& bufObj : sp.bufObjects) {
        if (bufObj) { NvSciBufObjFree(bufObj); bufObj = nullptr; }
    }
    sp.bufObjects.clear();

    for (auto& bufObj : sp.icpBufObjects) {
        if (bufObj) { NvSciBufObjFree(bufObj); bufObj = nullptr; }
    }
    sp.icpBufObjects.clear();

    if (sp.bufAttrList) { NvSciBufAttrListFree(sp.bufAttrList); sp.bufAttrList = nullptr; }
    if (sp.icpBufAttrList) { NvSciBufAttrListFree(sp.icpBufAttrList); sp.icpBufAttrList = nullptr; }
}

void NvSiplCameraInterface::Cleanup() {
    if (!m_bInitialized) return;
    SIPL_COE_LOG_DEBUG("CLEANUP", "Cleaning up SIPL Camera Interface...");

    if (m_bCapturing) {
        StopCapture();
    }

    for (auto& sp : m_sensorPipelines) {
        CleanupSensorPipeline(sp);
    }
    m_sensorPipelines.clear();
    m_numSensors = 0;

    (void)m_sensorSystemConfig.release();

    if (m_upCamera) { m_upCamera->Deinit(); m_upCamera.reset(); }
    if (m_upQuery) { m_upQuery.reset(); }

    if (m_config.NvBufWriteSurf) {
        NvBufSurfaceDestroy(m_config.NvBufWriteSurf);
        m_config.NvBufWriteSurf = nullptr;
        m_config.NvBufWriteFd = -1;
    }

    if (m_cpuWaitContext) { NvSciSyncCpuWaitContextFree(m_cpuWaitContext); m_cpuWaitContext = nullptr; }
    if (m_sciBufModule) { NvSciBufModuleClose(m_sciBufModule); m_sciBufModule = nullptr; }
    if (m_sciSyncModule) { NvSciSyncModuleClose(m_sciSyncModule); m_sciSyncModule = nullptr; }

    m_bInitialized = false;
    SIPL_COE_LOG_SUCCESS("CLEANUP", "SIPL Camera Interface cleanup completed");
}

// **Private method implementations**

/**
 * @brief Setup camera system configuration (unified CoE + GMSL flow)
 *
 * Query sequence:
 *   1. ParseDatabase()              — loads UDDF driver libs (required for GMSL, safe for CoE)
 *   2. ParseJsonFile(path)          — parses JSON config if provided (works for both CoE and GMSL)
 *   3. GetSensorSystemConfig(name)  — selects config by platform name
 *   4. ApplyMask(config, masks)     — filters GMSL links (mandatory for GMSL, skipped for CoE)
 */
SIPLStatus NvSiplCameraInterface::SetupCameraSystemConfig() {
    SIPL_COE_LOG_DEBUG("CONFIG", "Setting up camera system configuration...");

    if (m_upQuery) {
        SIPL_COE_LOG_DEBUG("CONFIG", "Query already initialized, skipping");
        return NVSIPL_STATUS_OK;
    }

    m_upQuery = INvSIPLCameraQuery::GetInstance();
    if (!m_upQuery) {
        SIPL_COE_LOG_ERROR("Failed to get INvSIPLCameraQuery instance");
        return NVSIPL_STATUS_ERROR;
    }

    // Step 1: Parse database — loads UDDF driver .so libs from /usr/lib/nvsipl_drv/
    auto status = m_upQuery->ParseDatabase();
    if (status != NVSIPL_STATUS_OK) {
        SIPL_COE_LOG_ERROR("ParseDatabase failed: " << status);
        return status;
    }
    SIPL_COE_LOG_SUCCESS("CONFIG", "ParseDatabase completed");

    // Step 2: Parse JSON file if provided (works for both CoE and GMSL JSON configs)
    if (!m_config.jsonFilePath.empty()) {
        status = m_upQuery->ParseJsonFile(m_config.jsonFilePath);
        if (status != NVSIPL_STATUS_OK) {
            SIPL_COE_LOG_ERROR("ParseJsonFile failed for: " << m_config.jsonFilePath);
            return status;
        }
        SIPL_COE_LOG_SUCCESS("CONFIG", "Parsed JSON file: " << m_config.jsonFilePath);
    }

    // Step 3: Get sensor system config by platform name
    if (m_config.platformConfigName.empty()) {
        SIPL_COE_LOG_ERROR("No config-name specified — set the 'config-name' property");
        return NVSIPL_STATUS_BAD_ARGUMENT;
    }

    m_sensorSystemConfig = std::make_unique<sensorconfig::SensorSystemConfig>();
    status = m_upQuery->GetSensorSystemConfig(m_config.platformConfigName, *m_sensorSystemConfig);
    if (status != NVSIPL_STATUS_OK) {
        SIPL_COE_LOG_ERROR("GetSensorSystemConfig failed for: " << m_config.platformConfigName);
        return status;
    }
    SIPL_COE_LOG_INFO("CONFIG", "Got sensor system config '" << m_config.platformConfigName
                  << "': " << m_sensorSystemConfig->modules.size() << " modules, "
                  << m_sensorSystemConfig->transports.size() << " transports");

    // Step 4: Apply link masks — mandatory for GMSL, skipped for CoE
    bool isGmsl = !m_sensorSystemConfig->modules.empty() &&
                  std::holds_alternative<sensorconfig::GmslModule>(
                      m_sensorSystemConfig->modules[0].moduleType);

    if (isGmsl && m_config.linkMasks.empty()) {
        SIPL_COE_LOG_ERROR("link-mask is required for GMSL configs (e.g., link-mask=0001)");
        return NVSIPL_STATUS_BAD_ARGUMENT;
    }

    if (!m_config.linkMasks.empty()) {
        status = m_upQuery->ApplyMask(*m_sensorSystemConfig, m_config.linkMasks);
        if (status != NVSIPL_STATUS_OK) {
            SIPL_COE_LOG_ERROR("ApplyMask failed");
            return status;
        }
        SIPL_COE_LOG_INFO("CONFIG", "Applied link mask — " << m_sensorSystemConfig->modules.size()
                      << " modules after filtering");
    }

    if (m_sensorSystemConfig->modules.empty()) {
        SIPL_COE_LOG_ERROR("No modules found in system config after setup");
        return NVSIPL_STATUS_ERROR;
    }

    SIPL_COE_LOG_SUCCESS("CONFIG", "Camera system configuration loaded successfully");
    return NVSIPL_STATUS_OK;
}

/**
 * @brief Force all sensor VCs to YUV422/UYVY (vendor CQueryAdapter equivalent)
 *
 * 标准 libnvsipl_query 不识别 s36.json 里的 "inputFormat":"yuv422"
 * （仅支持 uyvy/vyuy/yuyv/yvyu/rawXX 字符串），解析后 vcInfo 保持默认
 * inputFormat=RAW12(fmt=8)、cfa=0(Unknown)，导致 YUV 模组
 * GetImageAttributes(ICP) 返回 GENERIC_ERROR。
 * 厂商 nvsipl_camera 用 CCameraHalQueryAdapter 在解析后强制改写为
 * YUV422/uyvy（见厂商二进制字符串 "forcing sensor %s vc to YUV422/%s"），
 * 此处做同样的修正。必须在 SetPlatformCfg 之前调用。
 */
void NvSiplCameraInterface::ForceYuv422InputFormat() {
    if (!m_sensorSystemConfig) {
        return;
    }
    for (auto& module : m_sensorSystemConfig->modules) {
        std::visit([](auto& modType) {
            for (auto& sensorCfgVar : modType.sensorConfigs) {
                std::visit([](auto& sensorCfg) {
                    for (auto& vc : sensorCfg.vcInfoList) {
                        if (vc.inputFormat != NVSIPL_CAP_INPUT_FORMAT_TYPE_YUV422 ||
                            vc.cfa != NVSIPL_PIXEL_ORDER_UYVY) {
                            vc.inputFormat = NVSIPL_CAP_INPUT_FORMAT_TYPE_YUV422;
                            vc.cfa = NVSIPL_PIXEL_ORDER_UYVY;
                        }
                    }
                }, sensorCfgVar);
            }
        }, module.moduleType);
    }
}

/**
 * @brief Initialize NvSci modules
 */
SIPLStatus NvSiplCameraInterface::InitializeNvSciModules() {
    SIPL_COE_LOG_DEBUG("NVSCI", "Initializing NvSci modules...");

    // Initialize NvSciBuf module
    NvSciError sciErr = NvSciBufModuleOpen(&m_sciBufModule);
    if (sciErr != NvSciError_Success) {
        return NVSIPL_STATUS_ERROR;
    }

    // Initialize NvSciSync module
    sciErr = NvSciSyncModuleOpen(&m_sciSyncModule);
    if (sciErr != NvSciError_Success) {
        SIPL_COE_LOG_ERROR("Failed to open NvSciSync module: " << sciErr);
        if (m_sciBufModule) {
            NvSciBufModuleClose(m_sciBufModule);
            m_sciBufModule = nullptr;
        }
        return NVSIPL_STATUS_ERROR;
    }

    // Create CPU wait context
    if (m_cpuWaitContext == nullptr) {
        sciErr = NvSciSyncCpuWaitContextAlloc(m_sciSyncModule, &m_cpuWaitContext);
        if (sciErr != NvSciError_Success)
        {
            SIPL_COE_LOG_ERROR("Failed to allocate CPU wait context: " << sciErr);
            if (m_sciSyncModule)
            {
                NvSciSyncModuleClose(m_sciSyncModule);
                m_sciSyncModule = nullptr;
            }
            if (m_sciBufModule)
            {
                NvSciBufModuleClose(m_sciBufModule);
                m_sciBufModule = nullptr;
            }
            return NVSIPL_STATUS_ERROR;
        }
    }

    SIPL_COE_LOG_SUCCESS("NVSCI", "NvSci modules initialized successfully");
    return NVSIPL_STATUS_OK;
}

SIPLStatus NvSiplCameraInterface::SetupBufferAttributes(uint32_t sensorIdx, bool bl_output) {
    auto& sp = m_sensorPipelines[sensorIdx];
    SIPL_COE_LOG_DEBUG("BUFFERS", "Setting up ISP0 buffer attributes for sensor " << sensorIdx);

    NvSciError sciErr = NvSciError_Success;
    NvSciBufAttrList siplAttrList = NULL, nvbufAttrList = NULL;
    NvSciBufAttrList unreconciledAttrLists[2];
    NvSciBufAttrList conflictAttrList = NULL;

    sciErr = NvSciBufAttrListCreate(m_sciBufModule, &nvbufAttrList);
    if (sciErr != NvSciError_Success) {
        SIPL_COE_LOG_ERROR("Failed to create nvbuf attribute list for sensor " << sensorIdx << ": " << sciErr);
        return NVSIPL_STATUS_ERROR;
    }

    sciErr = NvSciBufAttrListCreate(m_sciBufModule, &siplAttrList);
    if (sciErr != NvSciError_Success) {
        SIPL_COE_LOG_ERROR("Failed to create SIPL attribute list for sensor " << sensorIdx << ": " << sciErr);
        NvSciBufAttrListFree(nvbufAttrList);
        return NVSIPL_STATUS_ERROR;
    }

    sciErr = SetNvSciBufAttributes(&siplAttrList, bl_output);
    if (sciErr != NvSciError_Success) {
        SIPL_COE_LOG_ERROR("SetNvSciBufAttributes failed for sensor " << sensorIdx << ": " << sciErr);
        NvSciBufAttrListFree(nvbufAttrList);
        NvSciBufAttrListFree(siplAttrList);
        return NVSIPL_STATUS_ERROR;
    }

    auto status = m_upCamera->GetImageAttributes(sp.sensorId,
                                                 INvSIPLClient::ConsumerDesc::OutputType::ISP0,
                                                 siplAttrList);
    if (status != NVSIPL_STATUS_OK) {
        SIPL_COE_LOG_ERROR("GetImageAttributes failed for sensor " << sensorIdx << ": " << status);
        goto cleanup;
    }

    sciErr = SetNvBufNvSciBufAttributes(&nvbufAttrList, siplAttrList);
    if (sciErr != NvSciError_Success) {
        SIPL_COE_LOG_ERROR("SetNvBufNvSciBufAttributes failed for sensor " << sensorIdx << ": " << sciErr);
        NvSciBufAttrListFree(nvbufAttrList);
        NvSciBufAttrListFree(siplAttrList);
        return NVSIPL_STATUS_ERROR;
    }

    unreconciledAttrLists[0] = siplAttrList;
    unreconciledAttrLists[1] = nvbufAttrList;
    sciErr = NvSciBufAttrListReconcile(unreconciledAttrLists, 2U, &sp.bufAttrList, &conflictAttrList);
    if (sciErr != NvSciError_Success) {
        SIPL_COE_LOG_ERROR("NvSciBufAttrListReconcile failed for sensor " << sensorIdx << ": " << sciErr);
        goto cleanup;
    }

    NvSciBufAttrListFree(nvbufAttrList);
    NvSciBufAttrListFree(siplAttrList);
    if (conflictAttrList) NvSciBufAttrListFree(conflictAttrList);

    SIPL_COE_LOG_SUCCESS("BUFFERS", "ISP0 buffer attributes setup complete for sensor " << sensorIdx);
    return NVSIPL_STATUS_OK;

cleanup:
    if (nvbufAttrList) NvSciBufAttrListFree(nvbufAttrList);
    if (siplAttrList) NvSciBufAttrListFree(siplAttrList);
    if (conflictAttrList) NvSciBufAttrListFree(conflictAttrList);
    return NVSIPL_STATUS_ERROR;
}

SIPLStatus NvSiplCameraInterface::RegisterBuffersWithSIPL(uint32_t sensorIdx, uint32_t bufferCount) {
    auto& sp = m_sensorPipelines[sensorIdx];
    SIPL_COE_LOG_DEBUG("BUFFERS", "Registering ISP0 buffers for sensor " << sensorIdx);

    // 实验10：补齐式分配（Init 前已由 PrepareISP0Buffers 预分配，仅补差额）
    while (sp.bufObjects.size() < bufferCount) {
        NvSciBufObj bufObj = {0};
        InternalInteropBuffer interopBuffer;

        NvSciError sciErr = NvSciBufObjAlloc(sp.bufAttrList, &bufObj);
        if (sciErr != NvSciError_Success) {
            SIPL_COE_LOG_ERROR("Failed to allocate buffer " << sp.bufObjects.size()
                           << " for sensor " << sensorIdx);
            goto cleanup;
        }
        sp.bufObjects.push_back(bufObj);
        interopBuffer.sci_buf_obj = bufObj;

        int ret = NvmmNvSciBufToNvBufSurface(bufObj, &interopBuffer.nvbuf_surf);
        if (ret != 0 || !interopBuffer.nvbuf_surf) {
            SIPL_COE_LOG_ERROR("NvmmNvSciBufToNvBufSurface failed for sensor " << sensorIdx);
            goto cleanup;
        }

        interopBuffer.nvbuf_surf->numFilled = 1;
        interopBuffer.nvbufsurface_fd = interopBuffer.nvbuf_surf->surfaceList[0].bufferDesc;
        interopBuffer.is_valid = true;
        interopBuffer.in_use = false;
        sp.interopBuffers.push_back(interopBuffer);
    }

    {
        auto status = m_upCamera->RegisterImages(sp.sensorId,
                                                 INvSIPLClient::ConsumerDesc::OutputType::ISP0,
                                                 sp.bufObjects);
        if (status != NVSIPL_STATUS_OK) {
            SIPL_COE_LOG_ERROR("RegisterImages failed for sensor " << sensorIdx);
            goto cleanup;
        }
    }

    SIPL_COE_LOG_SUCCESS("BUFFERS", "ISP0 buffers registered for sensor " << sensorIdx
                      << " (" << bufferCount << " buffers)");
    return NVSIPL_STATUS_OK;

cleanup:
    for (auto& buf : sp.interopBuffers) {
        if (buf.nvbuf_surf) { NvBufSurfaceDestroy(buf.nvbuf_surf); }
    }
    sp.interopBuffers.clear();
    for (auto& obj : sp.bufObjects) { NvSciBufObjFree(obj); }
    sp.bufObjects.clear();
    return NVSIPL_STATUS_ERROR;
}

SIPLStatus NvSiplCameraInterface::SetupSyncObjects(uint32_t sensorIdx) {
    auto& sp = m_sensorPipelines[sensorIdx];
    SIPL_COE_LOG_DEBUG("SYNC", "Setting up ISP0 sync objects for sensor " << sensorIdx);

    NvSciSyncAttrList signalerAttrList = nullptr, waiterAttrList = nullptr;
    NvSciSyncAttrList reconciledList = nullptr, newConflictList = nullptr;
    NvSciSyncAttrList unreconciledList[2];

    NvSciError sciErr = NvSciSyncAttrListCreate(m_sciSyncModule, &waiterAttrList);
    if (sciErr != NvSciError_Success) {
        SIPL_COE_LOG_ERROR("Failed to create waiter attr list for sensor " << sensorIdx << ": " << sciErr);
        return NVSIPL_STATUS_ERROR;
    }

    sciErr = NvSciSyncAttrListCreate(m_sciSyncModule, &signalerAttrList);
    if (sciErr != NvSciError_Success) {
        SIPL_COE_LOG_ERROR("Failed to create signaler attr list for sensor " << sensorIdx << ": " << sciErr);
        NvSciSyncAttrListFree(waiterAttrList);
        return NVSIPL_STATUS_ERROR;
    }

    bool cpu_waiter = true;
    NvSciSyncAccessPerm cpuPerm = NvSciSyncAccessPerm_WaitOnly;
    NvSciSyncAttrKeyValuePair keyValue[2] = {
        {NvSciSyncAttrKey_NeedCpuAccess, &cpu_waiter, sizeof(cpu_waiter)},
        {NvSciSyncAttrKey_RequiredPerm, &cpuPerm, sizeof(cpuPerm)}
    };

    sciErr = NvSciSyncAttrListSetAttrs(waiterAttrList, keyValue, 2);
    if (sciErr != NvSciError_Success) {
        SIPL_COE_LOG_ERROR("Failed to set waiter attributes for sensor " << sensorIdx << ": " << sciErr);
        goto cleanup;
    }

    {
        auto status = m_upCamera->FillNvSciSyncAttrList(sp.sensorId,
                                                   INvSIPLClient::ConsumerDesc::OutputType::ISP0,
                                                   signalerAttrList, SIPL_SIGNALER);
        if (status != NVSIPL_STATUS_OK) {
            SIPL_COE_LOG_ERROR("FillNvSciSyncAttrList failed for sensor " << sensorIdx << ": " << status);
            goto cleanup;
        }
    }

    unreconciledList[0] = signalerAttrList;
    unreconciledList[1] = waiterAttrList;
    sciErr = NvSciSyncAttrListReconcile(unreconciledList, 2U, &reconciledList, &newConflictList);
    if (sciErr != NvSciError_Success) {
        SIPL_COE_LOG_ERROR("Failed to reconcile sync attributes for sensor " << sensorIdx << ": " << sciErr);
        goto cleanup;
    }

    sciErr = NvSciSyncObjAlloc(reconciledList, &sp.syncObj);
    if (sciErr != NvSciError_Success) {
        SIPL_COE_LOG_ERROR("Failed to allocate sync object for sensor " << sensorIdx << ": " << sciErr);
        goto cleanup;
    }

    {
        auto status = m_upCamera->RegisterNvSciSyncObj(sp.sensorId,
                                                  INvSIPLClient::ConsumerDesc::OutputType::ISP0,
                                                  NVSIPL_EOFSYNCOBJ, sp.syncObj);
        if (status != NVSIPL_STATUS_OK) {
            SIPL_COE_LOG_ERROR("RegisterNvSciSyncObj failed for sensor " << sensorIdx << ": " << status);
            goto cleanup;
        }
    }

    NvSciSyncAttrListFree(signalerAttrList);
    NvSciSyncAttrListFree(waiterAttrList);
    if (reconciledList) NvSciSyncAttrListFree(reconciledList);
    if (newConflictList) NvSciSyncAttrListFree(newConflictList);

    SIPL_COE_LOG_SUCCESS("SYNC", "ISP0 sync objects setup complete for sensor " << sensorIdx);
    return NVSIPL_STATUS_OK;

cleanup:
    if (signalerAttrList) NvSciSyncAttrListFree(signalerAttrList);
    if (waiterAttrList) NvSciSyncAttrListFree(waiterAttrList);
    if (reconciledList) NvSciSyncAttrListFree(reconciledList);
    if (newConflictList) NvSciSyncAttrListFree(newConflictList);
    if (sp.syncObj) { NvSciSyncObjFree(sp.syncObj); sp.syncObj = nullptr; }
    return NVSIPL_STATUS_ERROR;
}

SIPLStatus NvSiplCameraInterface::SetupIcpBufferAttributes(uint32_t sensorIdx) {
    auto& sp = m_sensorPipelines[sensorIdx];
    SIPL_COE_LOG_DEBUG("ICP", "Setting up ICP buffer attributes for sensor " << sensorIdx);

    NvSciBufAttrList icpCaptureAttrList = NULL;
    NvSciBufAttrList conflictAttrList_icp = NULL;

    NvSciError sciErr = NvSciBufAttrListCreate(m_sciBufModule, &icpCaptureAttrList);
    if (sciErr != NvSciError_Success) {
        SIPL_COE_LOG_ERROR("Failed to create ICP attr list for sensor " << sensorIdx << ": " << sciErr);
        return NVSIPL_STATUS_ERROR;
    }

    NvSciBufType bufType = NvSciBufType_Image;
    // 实验9修正：对齐 CNvSIPLMaster::CaptureFillAttributesAllocateSciBuf（ICP buffer
    // 用 ReadWrite 权限——nvsipl_camera 验证过的路径）。vendor 原 Readonly 会在
    // captureOutputRequested=false 时导致 secondary sensor(13) 的 GetImageAttributes
    // 返回 INVALID_STATE。
    NvSciBufAttrValAccessPerm accessPerm = NvSciBufAccessPerm_ReadWrite;
    NvSciBufAttrKeyValuePair attrKvp[] = {
        {NvSciBufGeneralAttrKey_Types, &bufType, sizeof(bufType)},
        {NvSciBufGeneralAttrKey_RequiredPerm, &accessPerm, sizeof(accessPerm)}};

    sciErr = NvSciBufAttrListSetAttrs(icpCaptureAttrList, attrKvp, 2);
    if (sciErr != NvSciError_Success) {
        SIPL_COE_LOG_ERROR("Failed to set ICP buffer attributes for sensor " << sensorIdx << ": " << sciErr);
        NvSciBufAttrListFree(icpCaptureAttrList);
        return NVSIPL_STATUS_ERROR;
    }

    auto status = m_upCamera->GetImageAttributes(sp.sensorId,
                                                 INvSIPLClient::ConsumerDesc::OutputType::ICP,
                                                 icpCaptureAttrList);
    if (status != NVSIPL_STATUS_OK) {
        SIPL_COE_LOG_ERROR("Failed to get ICP image attributes for sensor " << sensorIdx
                        << ": " << static_cast<unsigned>(status));
        NvSciBufAttrListFree(icpCaptureAttrList);
        return status;
    }

    sciErr = NvSciBufAttrListReconcile(&icpCaptureAttrList, 1U, &sp.icpBufAttrList, &conflictAttrList_icp);
    NvSciBufAttrListFree(icpCaptureAttrList);
    if (conflictAttrList_icp) NvSciBufAttrListFree(conflictAttrList_icp);

    if (sciErr != NvSciError_Success) {
        SIPL_COE_LOG_ERROR("ICP NvSciBufAttrListReconcile failed for sensor " << sensorIdx << ": " << sciErr);
        return NVSIPL_STATUS_ERROR;
    }

    SIPL_COE_LOG_SUCCESS("ICP", "ICP buffer attributes setup complete for sensor " << sensorIdx);
    return NVSIPL_STATUS_OK;
}

SIPLStatus NvSiplCameraInterface::RegisterIcpBuffersWithSIPL(uint32_t sensorIdx, uint32_t bufferCount) {
    auto& sp = m_sensorPipelines[sensorIdx];

    // 实验10：补齐式分配（Init 前预分配过的 buffer 直接复用）
    while (sp.icpBufObjects.size() < bufferCount) {
        NvSciBufObj bufObj;
        NvSciError sciErr = NvSciBufObjAlloc(sp.icpBufAttrList, &bufObj);
        if (sciErr != NvSciError_Success) {
            SIPL_COE_LOG_ERROR("ICP buffer alloc failed for sensor " << sensorIdx);
            for (auto& obj : sp.icpBufObjects) { NvSciBufObjFree(obj); }
            sp.icpBufObjects.clear();
            return NVSIPL_STATUS_ERROR;
        }
        sp.icpBufObjects.push_back(bufObj);
    }

    return m_upCamera->RegisterImages(sp.sensorId,
                                      INvSIPLClient::ConsumerDesc::OutputType::ICP,
                                      sp.icpBufObjects);
}

// 实验10：Init 前预分配 ICP buffer（attr 获取 + NvSciBufObj 分配，不注册）
// 对齐 nvsipl_camera 真机序列（AllocateBuffers 在 Init 之前）
SIPLStatus NvSiplCameraInterface::PrepareICPBuffers(uint32_t sensorIdx, uint32_t bufferCount) {
    if (sensorIdx >= m_numSensors) return NVSIPL_STATUS_BAD_ARGUMENT;
    auto status = SetupIcpBufferAttributes(sensorIdx);
    if (status != NVSIPL_STATUS_OK) return status;

    auto& sp = m_sensorPipelines[sensorIdx];
    while (sp.icpBufObjects.size() < bufferCount) {
        NvSciBufObj bufObj;
        NvSciError sciErr = NvSciBufObjAlloc(sp.icpBufAttrList, &bufObj);
        if (sciErr != NvSciError_Success) {
            SIPL_COE_LOG_ERROR("ICP pre-alloc failed for sensor " << sensorIdx);
            for (auto& obj : sp.icpBufObjects) { NvSciBufObjFree(obj); }
            sp.icpBufObjects.clear();
            return NVSIPL_STATUS_ERROR;
        }
        sp.icpBufObjects.push_back(bufObj);
    }
    SIPL_COE_LOG_SUCCESS("ICP", "ICP buffers pre-allocated (" << sp.icpBufObjects.size()
                      << ") for sensor " << sensorIdx);
    return NVSIPL_STATUS_OK;
}

// 实验10：Init 前预分配 ISP0 buffer（attr 获取 + NvSciBufObj 分配 + interop 转换，不注册）
// 对齐 nvsipl_camera 真机序列（AllocateBuffers 在 Init 之前）
SIPLStatus NvSiplCameraInterface::PrepareISP0Buffers(uint32_t sensorIdx, bool bl_output, uint32_t bufferCount) {
    if (sensorIdx >= m_numSensors) return NVSIPL_STATUS_BAD_ARGUMENT;

    if (!m_config.enableISP0) {
        SIPL_COE_LOG_ERROR("ISP buffer prep requested but ISP0 not enabled");
        return NVSIPL_STATUS_ERROR;
    }

    auto status = SetupBufferAttributes(sensorIdx, bl_output);
    if (status != NVSIPL_STATUS_OK) return status;

    auto& sp = m_sensorPipelines[sensorIdx];
    while (sp.bufObjects.size() < bufferCount) {
        NvSciBufObj bufObj = {0};
        InternalInteropBuffer interopBuffer;

        NvSciError sciErr = NvSciBufObjAlloc(sp.bufAttrList, &bufObj);
        if (sciErr != NvSciError_Success) {
            SIPL_COE_LOG_ERROR("ISP0 pre-alloc failed for sensor " << sensorIdx);
            goto cleanup;
        }
        sp.bufObjects.push_back(bufObj);
        interopBuffer.sci_buf_obj = bufObj;

        int ret = NvmmNvSciBufToNvBufSurface(bufObj, &interopBuffer.nvbuf_surf);
        if (ret != 0 || !interopBuffer.nvbuf_surf) {
            SIPL_COE_LOG_ERROR("NvmmNvSciBufToNvBufSurface failed for sensor " << sensorIdx);
            goto cleanup;
        }

        interopBuffer.nvbuf_surf->numFilled = 1;
        interopBuffer.nvbufsurface_fd = interopBuffer.nvbuf_surf->surfaceList[0].bufferDesc;
        interopBuffer.is_valid = true;
        interopBuffer.in_use = false;
        sp.interopBuffers.push_back(interopBuffer);
    }

    SIPL_COE_LOG_SUCCESS("BUFFERS", "ISP0 buffers pre-allocated (" << sp.bufObjects.size()
                      << ") for sensor " << sensorIdx);
    return NVSIPL_STATUS_OK;

cleanup:
    for (auto& buf : sp.interopBuffers) {
        if (buf.nvbuf_surf) { NvBufSurfaceDestroy(buf.nvbuf_surf); }
    }
    sp.interopBuffers.clear();
    for (auto& obj : sp.bufObjects) { NvSciBufObjFree(obj); }
    sp.bufObjects.clear();
    return NVSIPL_STATUS_ERROR;
}

SIPLStatus NvSiplCameraInterface::SetupIcpSyncObjects(uint32_t sensorIdx) {
    (void)sensorIdx;
    return NVSIPL_STATUS_OK;
}

SIPLStatus NvSiplCameraInterface::RegisterAutoControlPlugin(uint32_t sensorIdx) {
    if (sensorIdx >= m_numSensors) return NVSIPL_STATUS_BAD_ARGUMENT;
    auto& sp = m_sensorPipelines[sensorIdx];

    std::vector<uint8_t> nitoBlob;
    auto status = LoadNITOFile(sp.sensorName, nitoBlob);
    if (status != NVSIPL_STATUS_OK) {
        SIPL_COE_LOG_ERROR("Failed to load NITO file for sensor " << sensorIdx
                        << " (" << sp.sensorName << ")");
        return status;
    }

    ISiplControlAuto* autoControl = nullptr;
    status = m_upCamera->RegisterAutoControlPlugin(sp.sensorId, NV_PLUGIN, autoControl, nitoBlob);
    if (status != NVSIPL_STATUS_OK) {
        SIPL_COE_LOG_ERROR("RegisterAutoControlPlugin failed for sensor " << sensorIdx);
        return status;
    }

    SIPL_COE_LOG_SUCCESS("AUTOCTL", "Auto control registered for sensor " << sensorIdx);
    return NVSIPL_STATUS_OK;
}

/**
 * @brief Start per-sensor processing threads
 *
 * Each sensor gets 3 threads:
 *   - ISP0 thread: reads from isp0CompletionQueue, calls ProcessISP0Frame
 *   - ICP thread: reads from captureCompletionQueue, consumes raw frames
 *   - Event thread: reads from notificationQueue, forwards to event callback
 *
 * All threads are fully isolated per-sensor — no shared mutable state.
 */
SIPLStatus NvSiplCameraInterface::StartThreads() {
    if (!m_config.useInternalThreads) {
        SIPL_COE_LOG_INFO("THREADS", "Internal threads disabled — external threads will handle processing");
        m_bThreadsRunning = false;
        return NVSIPL_STATUS_OK;
    }

    m_bThreadsRunning = true;

    for (uint32_t i = 0; i < m_numSensors; i++) {
        auto& sp = m_sensorPipelines[i];
        if (m_config.enableISP0) {
            sp.isp0Thread = std::thread(&NvSiplCameraInterface::ISP0ThreadFunc, this, i);
        }
        // 实验10修复：captureOutputRequested=false 时 SIPL 不创建 captureCompletionQueue
        // （null），ICP 分发线程只服务于用户取 RAW 帧，此路径下不启动。
        // ICP buffer 注册仍由 SetupICPBuffers 完成（ISP 输入用，与本线程无关）。
        if (m_config.enableICP && m_config.captureOutputRequested) {
            sp.icpThread = std::thread(&NvSiplCameraInterface::ICPThreadFunc, this, i);
        }
        sp.eventThread = std::thread(&NvSiplCameraInterface::EventThreadFunc, this, i);
    }

    SIPL_COE_LOG_SUCCESS("THREADS", "Started threads for " << m_numSensors << " sensors ("
                      << (m_config.enableISP0 ? "ISP0+" : "") << (m_config.enableICP ? "ICP+" : "")
                      << "Event per sensor)");
    return NVSIPL_STATUS_OK;
}

/**
 * @brief Stop all per-sensor threads
 *
 * Sets m_bThreadsRunning=false, then joins all threads.
 * SIPL completion queues return NVSIPL_STATUS_EOF after Stop(),
 * which causes thread loops to exit.
 */
void NvSiplCameraInterface::StopThreads() {
    SIPL_COE_LOG_DEBUG("THREADS", "Stopping all sensor threads...");
    m_bThreadsRunning = false;

    for (uint32_t i = 0; i < m_numSensors; i++) {
        auto& sp = m_sensorPipelines[i];
        if (sp.isp0Thread.joinable()) sp.isp0Thread.join();
        if (sp.icpThread.joinable()) sp.icpThread.join();
        if (sp.eventThread.joinable()) sp.eventThread.join();
    }

    SIPL_COE_LOG_SUCCESS("THREADS", "All sensor threads stopped");
}

/**
 * @brief ISP0 processing thread — one per sensor
 *
 * Reads from this sensor's pipelineQueues.isp0CompletionQueue.
 * Each sensor's queue is independent (populated by SIPL internally).
 */
void NvSiplCameraInterface::ISP0ThreadFunc(uint32_t sensorIdx) {
    auto& sp = m_sensorPipelines[sensorIdx];
    SIPLStatus status = NVSIPL_STATUS_OK;
    INvSIPLClient::INvSIPLBuffer* pBuffer = nullptr;

    SIPL_COE_LOG_DEBUG("ISP0_THREAD", "ISP0 thread started for sensor " << sensorIdx);

    while (m_bThreadsRunning && status != NVSIPL_STATUS_EOF) {
        status = sp.pipelineQueues.isp0CompletionQueue->Get(pBuffer, m_config.captureTimeoutMs * 1000);
        if (status == NVSIPL_STATUS_OK && pBuffer) {
            ProcessISP0Frame(sensorIdx, pBuffer);
        } else if (status == NVSIPL_STATUS_TIMED_OUT) {
            continue;
        }
    }

    SIPL_COE_LOG_DEBUG("ISP0_THREAD", "ISP0 thread finished for sensor " << sensorIdx);
}

/**
 * @brief ICP (raw capture) thread — one per sensor
 * Consumes raw frames from captureCompletionQueue and releases them.
 */
void NvSiplCameraInterface::ICPThreadFunc(uint32_t sensorIdx) {
    auto& sp = m_sensorPipelines[sensorIdx];
    SIPLStatus status = NVSIPL_STATUS_OK;
    INvSIPLClient::INvSIPLBuffer* pBuffer = nullptr;

    // 防御：队列不存在（captureOutputRequested=false）时直接退出，避免空指针解引用
    if (sp.pipelineQueues.captureCompletionQueue == nullptr) {
        SIPL_COE_LOG_WARNING("ICP_THREAD", "captureCompletionQueue is null for sensor "
                         << sensorIdx << " — ICP thread exiting");
        return;
    }

    while (m_bThreadsRunning && status != NVSIPL_STATUS_EOF) {
        status = sp.pipelineQueues.captureCompletionQueue->Get(pBuffer, m_config.captureTimeoutMs * 1000);
        if (status == NVSIPL_STATUS_OK && pBuffer) {
            // ProcessICPFrame 内部负责全部释放路径（无回调时直接 Release；
            // 有回调时经 ReleaseFrame 在回调返回后 Release）
            ProcessICPFrame(sensorIdx, pBuffer);
        } else if (status == NVSIPL_STATUS_TIMED_OUT) {
            continue;
        }
    }
}

/**
 * @brief Event notification thread — one per sensor
 * Each sensor's SetPipelineCfg creates a separate notificationQueue.
 * Per-sensor thread avoids latency from round-robin polling.
 */
void NvSiplCameraInterface::EventThreadFunc(uint32_t sensorIdx) {
    auto& sp = m_sensorPipelines[sensorIdx];
    SIPLStatus status = NVSIPL_STATUS_OK;
    NvSIPLPipelineNotifier::NotificationData notificationData;

    while (m_bThreadsRunning && status != NVSIPL_STATUS_EOF) {
        status = sp.pipelineQueues.notificationQueue->Get(notificationData, m_config.captureTimeoutMs * 1000);
        if (status == NVSIPL_STATUS_OK) {
            {
                std::lock_guard<std::mutex> lock(m_statsMutex);
                m_stats.totalEvents++;
            }
            if (sp.eventCallback) {
                sp.eventCallback(notificationData);
            }
        } else if (status == NVSIPL_STATUS_TIMED_OUT) {
            continue;
        }
    }
}

/**
 * @brief Process a completed ISP0 frame for a specific sensor
 *
 * Called from per-sensor ISP0 thread. Each sensor has its own thread, interop
 * buffers, and callback — no cross-sensor state access.
 *
 * Flow: SIPL buffer → fence wait → find interop buffer → build NvSiplFrame
 *       → user callback (GStreamer copies to NVMM) → auto-release
 */
void NvSiplCameraInterface::ProcessISP0Frame(uint32_t sensorIdx, INvSIPLClient::INvSIPLBuffer* pBuffer) {
    if (!pBuffer) return;

    auto& sp = m_sensorPipelines[sensorIdx];

    // Update global frame count (shared stat, mutex-protected)
    {
        std::lock_guard<std::mutex> lock(m_statsMutex);
        m_stats.totalFramesReceived++;
    }

    // No callback registered — release buffer immediately
    if (!sp.readyFrameCallback) {
        pBuffer->Release();
        return;
    }

    // Cast to NvM buffer to access NvSciBuf and NvSciSync objects
    INvSIPLClient::INvSIPLNvMBuffer* pNvMBuffer =
        dynamic_cast<INvSIPLClient::INvSIPLNvMBuffer*>(pBuffer);
    if (!pNvMBuffer) {
        SIPL_COE_LOG_ERROR("Failed to get NvM buffer for sensor " << sensorIdx);
        pBuffer->Release();
        return;
    }

    // Wait on EOF fence to ensure ISP processing is complete before CPU access
    NvSciSyncFence fence = NvSciSyncFenceInitializer;
    if (pNvMBuffer->GetEOFNvSciSyncFence(&fence) != NVSIPL_STATUS_OK) {
        SIPL_COE_LOG_ERROR("GetEOFNvSciSyncFence failed for sensor " << sensorIdx);
        pBuffer->Release();
        return;
    }

    NvSciError sciErr = NvSciSyncFenceWait(&fence, m_cpuWaitContext, FENCE_FRAME_TIMEOUT_MS * 2000UL);
    NvSciSyncFenceClear(&fence);
    if (sciErr != NvSciError_Success) {
        SIPL_COE_LOG_ERROR("Frame fence wait failed for sensor " << sensorIdx << ": " << sciErr);
        pBuffer->Release();
        return;
    }

    // Find the matching interop buffer by NvSciBufObj handle
    // Per-sensor mutex — no contention with other sensors
    NvSciBufObj sciBufObj = pNvMBuffer->GetNvSciBufImage();
    InternalInteropBuffer* interopBuffer = nullptr;

    {
        std::lock_guard<std::mutex> lock(*sp.interopMutex);
        for (auto& buffer : sp.interopBuffers) {
            if (buffer.is_valid && buffer.sci_buf_obj == sciBufObj && !buffer.in_use) {
                interopBuffer = &buffer;
                buffer.in_use = true;
                buffer.sipl_buffer = pNvMBuffer;
                break;
            }
        }
    }

    if (!interopBuffer) {
        SIPL_COE_LOG_ERROR("No matching interop buffer found for sensor " << sensorIdx);
        pBuffer->Release();
        return;
    }

    // Build NvSiplFrame with NvBufSurface ready for the callback consumer
    NvSiplFrame frame;
    // 硬同步改造：时间戳改用 SIPL 硬件 SOF TSC（ImageMetaData::frameCaptureStartTSC），
    // 替代 steady_clock::now()（帧到达用户态时刻，受线程调度/VIC 抖动污染）。
    // TSC 为全系统统一硬件时钟，sensorGroup 硬触发下左右目 SOF 直接可比，
    // 配对依据从"软件到达时间"升级为"硬件曝光开始时间"。
    const auto& imgMeta = pNvMBuffer->GetImageData();
    frame.frameNumber = m_stats.totalFramesReceived;
    frame.timestamp = imgMeta.frameCaptureStartTSC / 1000ULL;  // TSC(ns) → µs
    frame.sensorId = sp.sensorId;

    if (interopBuffer->nvbuf_surf && interopBuffer->nvbuf_surf->numFilled > 0) {
        const auto& surfaceDesc = interopBuffer->nvbuf_surf->surfaceList[0];
        frame.nvbufSurface = interopBuffer->nvbuf_surf;
        frame.nvbufSurfaceFd = interopBuffer->nvbufsurface_fd;
        frame.width = surfaceDesc.width;
        frame.height = surfaceDesc.height;
        frame.pitch = surfaceDesc.pitch;
        frame.colorFormat = surfaceDesc.colorFormat;
    }
    frame._internal_sipl_buffer = pNvMBuffer;
    frame._internal_interop_buffer = interopBuffer;

    UpdateStatistics(frame);

    // Invoke the per-sensor callback (GStreamer layer copies SIPL→NVMM and pushes to pad queue)
    SIPL_COE_LOG_DEBUG("ISP0_PROC", "Calling ready frame callback for sensor "
                    << sensorIdx << " frame " << frame.frameNumber);

    try {
        sp.readyFrameCallback(frame);
    } catch (const std::exception& e) {
        SIPL_COE_LOG_ERROR("Ready frame callback failed for sensor " << sensorIdx << ": " << e.what());
    }

    // Auto-release: return SIPL buffer to pool + mark interop buffer available
    ReleaseFrame(sensorIdx, frame);

    SIPL_COE_LOG_DEBUG("ISP0_PROC", "Sensor " << sensorIdx << " frame "
                    << frame.frameNumber << " processed and released");
}

/**
 * @brief Process a raw ICP frame for a specific sensor
 *
 * 两条路径：
 *  - 无 captureFrameCallback（RAW 模组，capture 不向用户输出）：仅计数并释放。
 *  - 有 captureFrameCallback（YUV 模组直出）：CPU 直读 NvSciBuf（packed 格式
 *    无 EOF fence、不可 Nvmm 转换）→ 构建 NvSiplFrame(rawData) → 回调 → ReleaseFrame。
 * 本函数接管 pBuffer 的全部释放路径（含各错误分支）。
 */
void NvSiplCameraInterface::ProcessICPFrame(uint32_t sensorIdx, INvSIPLClient::INvSIPLBuffer* pBuffer) {
    if (!pBuffer) return;

    auto& sp = m_sensorPipelines[sensorIdx];
    {
        std::lock_guard<std::mutex> lock(m_statsMutex);
        m_stats.totalFramesProcessed++;
    }

    if (!sp.captureFrameCallback) {
        SIPL_COE_LOG_DEBUG("ICP_PROC", "ICP frame captured for sensor " << sensorIdx);
        pBuffer->Release();
        return;
    }

    INvSIPLClient::INvSIPLNvMBuffer* pNvMBuffer =
        dynamic_cast<INvSIPLClient::INvSIPLNvMBuffer*>(pBuffer);
    if (!pNvMBuffer) {
        SIPL_COE_LOG_ERROR("ICP: failed to get NvM buffer for sensor " << sensorIdx);
        pBuffer->Release();
        return;
    }

    // 厂商示例（CNvSIPLConsumer::DisplayFrameEglYuv422）确认：
    // ICP capture buffer 在回调时已完整，没有 EOF fence 可等（ISP 未启用时
    // 不建 CPU waiter），且 packed 格式无法转 NvBufSurface —— CPU 直读。
    NvSciBufObj sciBufObj = pNvMBuffer->GetNvSciBufImage();
    if (!sciBufObj) {
        SIPL_COE_LOG_ERROR("ICP: GetNvSciBufImage failed for sensor " << sensorIdx);
        pBuffer->Release();
        return;
    }

    // 几何只查一次（raw ICP buffer 只有 per-plane key 可靠，Surf* key 不填）
    if (sp.icpFrameWidth == 0) {
        NvSciBufAttrList bufAttrList = nullptr;
        if (NvSciBufObjGetAttrList(sciBufObj, &bufAttrList) != NvSciError_Success) {
            SIPL_COE_LOG_ERROR("ICP: NvSciBufObjGetAttrList failed for sensor " << sensorIdx);
            pBuffer->Release();
            return;
        }
        NvSciBufAttrKeyValuePair queryAttrs[] = {
            { NvSciBufImageAttrKey_PlaneWidth,  NULL, 0 },
            { NvSciBufImageAttrKey_PlaneHeight, NULL, 0 },
            { NvSciBufImageAttrKey_PlanePitch,  NULL, 0 },
        };
        if (NvSciBufAttrListGetAttrs(bufAttrList, queryAttrs, 3) != NvSciError_Success ||
            !queryAttrs[0].value || !queryAttrs[1].value || !queryAttrs[2].value) {
            SIPL_COE_LOG_ERROR("ICP: buffer geometry attrs missing for sensor " << sensorIdx);
            pBuffer->Release();
            return;
        }
        sp.icpFrameWidth  = ((const uint32_t*)queryAttrs[0].value)[0];
        sp.icpFrameHeight = ((const uint32_t*)queryAttrs[1].value)[0];
        sp.icpFramePitch  = ((const uint32_t*)queryAttrs[2].value)[0];
        SIPL_COE_LOG_CRITICAL("[SIPL] ICP capture 几何: sensor " << sensorIdx << " "
                              << sp.icpFrameWidth << "x" << sp.icpFrameHeight
                              << " pitch=" << sp.icpFramePitch);
    }

    // 采集硬件直写该 buffer，CPU 读前必须 invalidate cache，否则出现横条纹
    const uint32_t bufSize = sp.icpFramePitch * sp.icpFrameHeight;
    NvSciBufObjFlushCpuCacheRange(sciBufObj, 0U, bufSize);

    const void* pSrc = nullptr;
    if (NvSciBufObjGetConstCpuPtr(sciBufObj, &pSrc) != NvSciError_Success || !pSrc) {
        SIPL_COE_LOG_ERROR("ICP: GetConstCpuPtr failed for sensor " << sensorIdx);
        pBuffer->Release();
        return;
    }

    NvSiplFrame frame;
    const auto& imgMeta = pNvMBuffer->GetImageData();
    frame.frameNumber = m_stats.totalFramesProcessed;
    frame.timestamp = imgMeta.frameCaptureStartTSC / 1000ULL;  // TSC(ns) → µs
    frame.sensorId = sp.sensorId;
    frame.rawData = const_cast<void*>(pSrc);
    frame.rawDataSize = bufSize;
    frame.width = sp.icpFrameWidth;
    frame.height = sp.icpFrameHeight;
    frame.pitch = sp.icpFramePitch;
    frame._internal_sipl_buffer = pNvMBuffer;

    try {
        sp.captureFrameCallback(frame);
    } catch (const std::exception& e) {
        SIPL_COE_LOG_ERROR("Capture frame callback failed for sensor " << sensorIdx << ": " << e.what());
    }

    ReleaseFrame(sensorIdx, frame);
}

/**
 * @brief Update statistics
 */
void NvSiplCameraInterface::UpdateStatistics(const NvSiplFrame& frame) {
    std::lock_guard<std::mutex> lock(m_statsMutex);

    if (m_stats.firstFrameTimestamp == 0) {
        m_stats.firstFrameTimestamp = frame.timestamp;
    }
    m_stats.lastFrameTimestamp = frame.timestamp;
    m_stats.totalFramesProcessed++;
}

SIPLStatus NvSiplCameraInterface::CreateInteropBuffer(uint32_t sensorIdx, NvSciBufObj* bufObj, NvBufSurface** nvbufSurface) {
    if (sensorIdx >= m_numSensors || !bufObj || !nvbufSurface) {
        return NVSIPL_STATUS_BAD_ARGUMENT;
    }
    auto& sp = m_sensorPipelines[sensorIdx];

    NvSciError sciErr = NvSciBufObjAlloc(sp.bufAttrList, bufObj);
    if (sciErr != NvSciError_Success) {
        SIPL_COE_LOG_ERROR("Failed to allocate NvSciBuf for sensor " << sensorIdx);
        return NVSIPL_STATUS_ERROR;
    }

    int ret = NvmmNvSciBufToNvBufSurface(*bufObj, nvbufSurface);
    if (ret != 0 || !(*nvbufSurface)) {
        NvSciBufObjFree(*bufObj);
        return NVSIPL_STATUS_ERROR;
    }

    (*nvbufSurface)->numFilled = 1;
    return NVSIPL_STATUS_OK;
}

SIPLStatus NvSiplCameraInterface::RegisterInteropBuffer(uint32_t sensorIdx, NvSciBufObj bufObj) {
    if (sensorIdx >= m_numSensors || !bufObj) {
        return NVSIPL_STATUS_BAD_ARGUMENT;
    }
    m_sensorPipelines[sensorIdx].bufObjects.push_back(bufObj);
    return NVSIPL_STATUS_OK;
}

SIPLStatus NvSiplCameraInterface::FinalizeBufferRegistration(uint32_t sensorIdx) {
    if (sensorIdx >= m_numSensors) {
        return NVSIPL_STATUS_BAD_ARGUMENT;
    }
    auto& sp = m_sensorPipelines[sensorIdx];

    if (sp.bufObjects.empty()) {
        SIPL_COE_LOG_ERROR("No buffers to register for sensor " << sensorIdx);
        return NVSIPL_STATUS_BAD_ARGUMENT;
    }

    auto status = m_upCamera->RegisterImages(sp.sensorId,
                                             INvSIPLClient::ConsumerDesc::OutputType::ISP0,
                                             sp.bufObjects);
    if (status != NVSIPL_STATUS_OK) {
        SIPL_COE_LOG_ERROR("RegisterImages failed for sensor " << sensorIdx);
        return status;
    }

    SIPL_COE_LOG_SUCCESS("INTEROP", "Finalized " << sp.bufObjects.size()
                      << " buffers for sensor " << sensorIdx);
    return NVSIPL_STATUS_OK;
}
