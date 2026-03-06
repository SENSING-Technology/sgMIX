/*
 * Copyright 2019-2021 NVIDIA Corporation. All rights reserved.
 *
 * NOTICE TO LICENSEE:
 *
 * This source code and/or documentation ("Licensed Deliverables") are
 * subject to NVIDIA intellectual property rights under U.S. and
 * international Copyright laws.
 *
 * These Licensed Deliverables contained herein is PROPRIETARY and
 * CONFIDENTIAL to NVIDIA and is being provided under the terms and
 * conditions of a form of NVIDIA software license agreement by and
 * between NVIDIA and Licensee ("License Agreement") or electronically
 * accepted by Licensee. Notwithstanding any terms or conditions to
 * the contrary in the License Agreement, reproduction or disclosure
 * of the Licensed Deliverables to any third party without the express
 * written consent of NVIDIA is prohibited.
 *
 * NOTWITHSTANDING ANY TERMS OR CONDITIONS TO THE CONTRARY IN THE
 * LICENSE AGREEMENT, NVIDIA MAKES NO REPRESENTATION ABOUT THE
 * SUITABILITY OF THESE LICENSED DELIVERABLES FOR ANY PURPOSE. IT IS
 * PROVIDED "AS IS" WITHOUT EXPRESS OR IMPLIED WARRANTY OF ANY KIND.
 * NVIDIA DISCLAIMS ALL WARRANTIES WITH REGARD TO THESE LICENSED
 * DELIVERABLES, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY,
 * NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE.
 * NOTWITHSTANDING ANY TERMS OR CONDITIONS TO THE CONTRARY IN THE
 * LICENSE AGREEMENT, IN NO EVENT SHALL NVIDIA BE LIABLE FOR ANY
 * SPECIAL, INDIRECT, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, OR ANY
 * DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS
 * ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THESE LICENSED DELIVERABLES.
 *
 * U.S. Government End Users. These Licensed Deliverables are a
 * "commercial item" as that term is defined at 48 C.F.R. 2.101 (OCT
 * 1995), consisting of "commercial computer software" and "commercial
 * computer software documentation" as such terms are used in 48
 * C.F.R. 12.212 (SEPT 1995) and is provided to the U.S. Government
 * only as a commercial end item. Consistent with 48 C.F.R.12.212 and
 * 48 C.F.R. 227.7202-1 through 227.7202-4 (JUNE 1995), all
 * U.S. Government End Users acquire the Licensed Deliverables with
 * only those rights set forth herein.
 *
 * Any use of the Licensed Deliverables in individual and commercial
 * software must include, in the user documentation and internal
 * comments to the code, the above Disclaimer and U.S. Government End
 * Users Notice.
 */

/**
 * @file Status.h
 *
 * Declaration of VPI status codes handling functions.
 */

#ifndef NV_VPI_STATUS_H
#define NV_VPI_STATUS_H

#include "Export.h"

#include <stdint.h>

/**
 * Declares entities to handle return status codes used in VPI.
 *
 * VPI functions uses status codes to return if they succeeded or not.
 *
 * @defgroup VPI_Status Status Codes
 * @ingroup VPI_API_Misc
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Status codes.
 */
typedef enum
{
    VPI_SUCCESS = 0,                /**< Operation completed successfully. */
    VPI_ERROR_NOT_IMPLEMENTED,      /**< Operation isn't implemented. */
    VPI_ERROR_INVALID_ARGUMENT,     /**< Invalid argument, either wrong range or value not accepted. */
    VPI_ERROR_INVALID_IMAGE_FORMAT, /**< Image type not accepted. */
    VPI_ERROR_INVALID_ARRAY_TYPE,   /**< Array type not accepted. */
    VPI_ERROR_INVALID_PAYLOAD_TYPE, /**< Payload not created for this algorithm. */
    VPI_ERROR_INVALID_OPERATION,    /**< Operation isn't valid in this context. */
    VPI_ERROR_INVALID_CONTEXT,      /**< Context is invalid or is already destroyed. */
    VPI_ERROR_DEVICE,               /**< Device backend error. */
    VPI_ERROR_NOT_READY,            /**< Operation not completed yet, try again later. */
    VPI_ERROR_BUFFER_LOCKED,        /**< Invalid operation on a locked buffer. */
    VPI_ERROR_OUT_OF_MEMORY,        /**< Not enough free memory to allocate object. */
    VPI_ERROR_INTERNAL              /**< Internal, non specific error. */
} VPIStatus;

/** Maximum status message length in bytes.
 *
 * This is the maximum number of bytes that will be written by \ref
 * vpiGetLastStatusMessage and \ref vpiPeekAtLastStatusMessage to the status
 * message output buffer.
 */
#define VPI_MAX_STATUS_MESSAGE_LENGTH 256

/**
 * Returns a string representation of the status code.
 *
 * @param [in] code Status code whose string representation is to be returned.
 *
 * @returns The string representation of the status code.
 *          Returned string is valid until next call of this function from the same calling thread.
 *          Returned pointer must not be freed.
 */
VPI_PUBLIC const char *vpiStatusGetName(VPIStatus code);

/** Returns and resets the status of the last VPI function call in current thread.
 *
 * A new call to this function will return \ref VPI_SUCCESS, as the thread-specific
 * status was reset. This operation doesn't affect the statuses in other threads.
 *
 * @returns The status of the last VPI function call in current thread.
 */
VPI_PUBLIC VPIStatus vpiGetLastStatus();

/** Returns and resets the status code and message of the last VPI function call in current thread.
 *
 * A new call to this function will return \ref VPI_SUCCESS, as the thread-specific
 * status was reset. This operation doesn't affect the status in other threads.
 *
 * It's guaranteed that the message is never larger than
 * \ref VPI_MAX_STATUS_MESSAGE_LENGTH bytes, including the '\0' string terminator.
 *
 * @param[out] msgBuffer Pointer to memory where the status message will be written to.
 *                       If NULL, no message is returned.
 * 
 * @param[in] lenBuffer Size in bytes of msgBuffer.
 *                      + If less than zero, \p lenBuffer is assumed to be 0.
 *
 * @returns The status of the last VPI function call in current thread.
 */
VPI_PUBLIC VPIStatus vpiGetLastStatusMessage(char *msgBuffer, int32_t lenBuffer);

/** Returns the status of the last VPI function call in current thread.
 *
 * The status code won't be reset.
 *
 * @returns The status of the last VPI function call in current thread.
 */
VPI_PUBLIC VPIStatus vpiPeekAtLastStatus();

/** Returns and status code and message of the last VPI function call in current thread.
 *
 * The status code and message won't be reset.
 *
 * It's guaranteed that the message is never larger than
 * \ref VPI_MAX_STATUS_MESSAGE_LENGTH bytes, including the '\0' string terminator.
 *
 * @param[out] msgBuffer Pointer to memory where the status message will be written to.
 *                       If NULL, no message is returned.
 * 
 * @param[in] lenBuffer Size in bytes of msgBuffer.
 *                      + If less than zero, lenBuffer is assumed to be 0.
 *
 * @returns The status of the last VPI function call in current thread.
 */
VPI_PUBLIC VPIStatus vpiPeekAtLastStatusMessage(char *msgBuffer, int32_t lenBuffer);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* NV_VPI_STATUS_H */
