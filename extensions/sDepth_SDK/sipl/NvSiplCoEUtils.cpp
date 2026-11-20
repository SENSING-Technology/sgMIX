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

#include <cstring>
#include <unistd.h>
#include <chrono>
#include <algorithm>
#include <cstdio>
#include <iostream>
#include <string>

#include "NvSiplCoEUtils.hpp"

SIPLStatus LoadNITOFile(std::string moduleName, std::vector<uint8_t>& nito)
{
    // Set up blob file path
    std::string nitoFilePath;
    char const* nitoFilePathFromEnv = getenv("NITO_PATH");
    if (nitoFilePathFromEnv == NULL) {
        nitoFilePath = "/var/nvidia/nvcam/settings/sipl/";
    } else {
        nitoFilePath = std::string(nitoFilePathFromEnv);
    }

    std::string moduleNameLower{};
    for (auto& c : moduleName) {
        moduleNameLower.push_back(std::tolower(c));
    }

    std::string nitoFile[] = {
        nitoFilePath + moduleName + ".nito",
        nitoFilePath + moduleNameLower + ".nito"
    };

    FILE *fp = nullptr;
    std::string openedFilePath;

    for (uint32_t i = 0U; i < sizeof(nitoFile)/sizeof(nitoFile[0]); i++) {
        fp = fopen(nitoFile[i].c_str(), "rb");
        if (fp == NULL) {
            // File not found, try next variant
            std::cout << "INFO: NITO file \"" << nitoFile[i] << "\" not found, trying next variant" << std::endl;
        } else {
            openedFilePath = nitoFile[i];
            std::cout << "INFO: Opened NITO file: \"" << nitoFile[i] << "\" for module: \"" << moduleName << "\"" << std::endl;
            break;
        }
    }

    if (fp == NULL) {
        std::cerr << "ERROR: Unable to open NITO file for module \"" << moduleName
                  << "\" in path \"" << nitoFilePath << "\"" << std::endl;
        return NVSIPL_STATUS_ERROR;
    }

    // Check file size
    if (fseek(fp, 0, SEEK_END) != 0) {
        std::cerr << "ERROR: Failed to seek to end of NITO file for module \"" << moduleName << "\"" << std::endl;
        fclose(fp);
        return NVSIPL_STATUS_ERROR;
    }

    auto fsize = ftell(fp);
    if (fsize <= 0) {
        std::cerr << "ERROR: NITO file for module \"" << moduleName << "\" is of invalid size: " << fsize << std::endl;
        fclose(fp);
        return NVSIPL_STATUS_ERROR;
    }

    rewind(fp);

    // Allocate blob memory
    nito.resize(fsize);

    // Load NITO data
    auto result = static_cast<long int>(fread(nito.data(), 1, fsize, fp));
    if (result != fsize) {
        std::cerr << "ERROR: Failed to read NITO file data. Expected: " << fsize
                  << " bytes, read: " << result << " bytes" << std::endl;
        nito.resize(0);
        fclose(fp);
        return NVSIPL_STATUS_ERROR;
    }

    // Close file
    fclose(fp);

    std::cout << "INFO: Successfully loaded " << fsize << " bytes from NITO file for module \""
              << moduleName << "\"" << std::endl;

    return NVSIPL_STATUS_OK;
}

SIPLStatus GetEventName(const NvSIPLPipelineNotifier::NotificationData &event, const char *&eventName)
{
    static const EventMap eventNameTable[] = {
        {NvSIPLPipelineNotifier::NotificationType::NOTIF_INFO_ICP_PROCESSING_DONE,
         "NOTIF_INFO_ICP_PROCESSING_DONE"},
        {(nvsipl::NvSIPLPipelineNotifier::NotificationType)NOTIF_INFO_ISP_PROCESSING_DONE,
         "NOTIF_INFO_ISP_PROCESSING_DONE"},
        {(nvsipl::NvSIPLPipelineNotifier::NotificationType)NOTIF_INFO_ACP_PROCESSING_DONE,
         "NOTIF_INFO_ACP_PROCESSING_DONE"},
        {NvSIPLPipelineNotifier::NotificationType::NOTIF_INFO_CDI_PROCESSING_DONE,
         "NOTIF_INFO_CDI_PROCESSING_DONE"},
        {NvSIPLPipelineNotifier::NotificationType::NOTIF_INFO_ICP_AUTH_SUCCESS,
         "NOTIF_INFO_ICP_AUTH_SUCCESS"},
        {NvSIPLPipelineNotifier::NotificationType::NOTIF_WARN_ICP_FRAME_DROP,
         "NOTIF_WARN_ICP_FRAME_DROP"},
        {NvSIPLPipelineNotifier::NotificationType::NOTIF_WARN_ICP_FRAME_DISCONTINUITY,
         "NOTIF_WARN_ICP_FRAME_DISCONTINUITY"},
        {NvSIPLPipelineNotifier::NotificationType::NOTIF_WARN_ICP_CAPTURE_TIMEOUT,
         "NOTIF_WARN_ICP_CAPTURE_TIMEOUT"},
        {NvSIPLPipelineNotifier::NotificationType::NOTIF_WARN_ICP_APP_BACK_PRESSURE,
         "NOTIF_WARN_ICP_APP_BACK_PRESSURE"},
        {NvSIPLPipelineNotifier::NotificationType::NOTIF_ERROR_ICP_BAD_INPUT_STREAM,
         "NOTIF_ERROR_ICP_BAD_INPUT_STREAM"},
        {NvSIPLPipelineNotifier::NotificationType::NOTIF_ERROR_ICP_CAPTURE_FAILURE,
         "NOTIF_ERROR_ICP_CAPTURE_FAILURE"},
        {NvSIPLPipelineNotifier::NotificationType::NOTIF_ERROR_ICP_EMB_DATA_PARSE_FAILURE,
         "NOTIF_ERROR_ICP_EMB_DATA_PARSE_FAILURE"},
        {NvSIPLPipelineNotifier::NotificationType::NOTIF_ERROR_ISP_PROCESSING_FAILURE,
         "NOTIF_ERROR_ISP_PROCESSING_FAILURE"},
        {NvSIPLPipelineNotifier::NotificationType::NOTIF_ERROR_ACP_PROCESSING_FAILURE,
         "NOTIF_ERROR_ACP_PROCESSING_FAILURE"},
        {NvSIPLPipelineNotifier::NotificationType::NOTIF_ERROR_ACP_SETTINGS_DISCONTINUITY,
         "NOTIF_ERROR_ACP_SETTINGS_DISCONTINUITY"},
        {NvSIPLPipelineNotifier::NotificationType::NOTIF_ERROR_CDI_SET_SENSOR_CTRL_FAILURE,
         "NOTIF_ERROR_CDI_SET_SENSOR_CTRL_FAILURE"},
        {NvSIPLPipelineNotifier::NotificationType::NOTIF_ERROR_DESERIALIZER_FAILURE,
         "NOTIF_ERROR_DESERIALIZER_FAILURE"},
        {NvSIPLPipelineNotifier::NotificationType::NOTIF_ERROR_SERIALIZER_FAILURE,
         "NOTIF_ERROR_SERIALIZER_FAILURE"},
        {NvSIPLPipelineNotifier::NotificationType::NOTIF_ERROR_SENSOR_FAILURE,
         "NOTIF_ERROR_SENSOR_FAILURE"},
        {NvSIPLPipelineNotifier::NotificationType::NOTIF_ERROR_INTR_LOCALIZATION_FAILURE,
         "NOTIF_ERROR_INTR_LOCALIZATION_FAILURE"},
        {NvSIPLPipelineNotifier::NotificationType::NOTIF_ERROR_INTR_LOCALIZATION_TIMEOUT,
         "NOTIF_ERROR_INTR_LOCALIZATION_TIMEOUT"},
        {NvSIPLPipelineNotifier::NotificationType::NOTIF_ERROR_ICP_AUTH_FAILURE,
         "NOTIF_ERROR_ICP_AUTH_FAILURE"},
        {NvSIPLPipelineNotifier::NotificationType::NOTIF_ERROR_ICP_AUTH_OUT_OF_ORDER,
         "NOTIF_ERROR_ICP_AUTH_OUT_OF_ORDER"},
        {NvSIPLPipelineNotifier::NotificationType::NOTIF_ERROR_INTERNAL_FAILURE,
         "NOTIF_ERROR_INTERNAL_FAILURE"},
        {NvSIPLPipelineNotifier::NotificationType::NOTIF_ERROR_ISP_PREFENCE_TIMEOUT,
         "NOTIF_ERROR_ISP_PREFENCE_TIMEOUT"},
        {NvSIPLPipelineNotifier::NotificationType::NOTIF_INIT_ERROR_FAILURE,
         "NOTIF_INIT_ERROR_FAILURE"}};
    for (uint32_t i = 0U; i < ARRAY_SIZE(eventNameTable); i++)
    {
        if (event.eNotifType == eventNameTable[i].eventType)
        {
            eventName = eventNameTable[i].eventName;
            return NVSIPL_STATUS_OK;
        }
    }
    std::cerr << "ERROR: Unknown event type" << std::endl;
    return NVSIPL_STATUS_BAD_ARGUMENT;
}
