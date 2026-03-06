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
 * @file Event.h
 *
 * Functions and structures for dealing with VPI events.
 */

#ifndef NV_VPI_EVENT_H
#define NV_VPI_EVENT_H

#include "Export.h"
#include "Status.h"
#include "Types.h"

#include <stdint.h>

/**
 * A representation of events used in stream synchronization and timing.
 *
 * Each compute function in the API is executed asynchronously with respect to the calling thread,
 * i.e., returns immediately without waiting for the completion. There are two ways of
 * synchronizing with the backend. One is to wait until all the commands in the \ref VPIStream queue
 * are finished using the \ref vpiStreamSync call. This approach, while simple, doesn't allow for
 * fine-grained ("wait for until function X is completed") or inter-device ("before running
 * function A in device B, wait until function C in device D finishes") synchronization. That's
 * where \ref VPIEvent objects come in. Conceptually, they correspond to binary semaphores and are
 * designed to closely mimic events in CUDA API:
 *
 * - Users can capture all commands submitted to a \ref VPIStream instance in an event instance (see
 *   \ref vpiEventRecord). The event is considered completed when all captured commands have been
 *   processed and removed from \ref VPIStream command queue.
 * - Inter-device synchronization is possible with \ref vpiStreamWaitEvent call that pushes a command
 *   to \ref VPIStream queue that blocks processing of future queued commands until given event is
 *   completed.
 * - Host threads can query the event's state with \ref vpiEventQuery
 * - Host threads can block until event is completed with \ref vpiEventSync.
 * - Events can be time-stamped when completed.
 * - Users can compute time-stamp difference between completed events in the same device as well as
 *   between different devices. This implies that internally, the API has to have a notion of
 *   unified time source.
 *
 * @defgroup VPI_Event Event
 * @ingroup VPI_API_Core
 * @{
 *
 */

#ifdef __cplusplus
extern "C" {
#endif

/** @anchor event_flags @name Event-specific flags. */
/**@{*/

/** Disable time-stamping of event signaling.
 * It allows for better performance in operations involving events. */
#define VPI_EVENT_DISABLE_TIMESTAMP (1ULL << 63)
/**@}*/

/**
 * Create an event instance.
 *
 * @param[in] flags Bit field specifying the desired characteristics of the event.
 *                  + The field must be a combination of zero or more of the following flags:
 *                    - \ref VPIBackend flags.
 *                      This even can be used in streams that have these backends enabled.
 *                      If no backend flags are passed and \ref VPI_REQUIRE_BACKENDS flag isn't present,
 *                      it'll consider all backends supported by the active context.
 *                    - \ref event_flags "Event-specific flags".
 *                    - \ref common_flags "Common object flags".
 *                  + If flag \ref VPI_REQUIRE_BACKENDS is given, user must pass at least one valid backend, and
 *                    they all must be enabled in current context.
 * 
 * @param[out] event Pointer to memory that will receive the created event handle.
 *                   + Mandatory parameter, can't be NULL.
 *
 * @retval #VPI_ERROR_INVALID_ARGUMENT  Output \p event handle is NULL.
 * @retval #VPI_ERROR_INVALID_ARGUMENT  \p flags is invalid.
 * @retval #VPI_ERROR_INVALID_ARGUMENT  No backend were given and \ref VPI_REQUIRE_BACKENDS is set.
 * @retval #VPI_ERROR_OUT_OF_MEMORY     Not enough resources to allocate event.
 * @retval #VPI_ERROR_INVALID_CONTEXT   Current context is invalid.
 * @retval #VPI_ERROR_INVALID_OPERATION Requested backend isn't enabled in current context.
 * @retval #VPI_SUCCESS                 Operation executed successfully.
 */
VPI_PUBLIC VPIStatus vpiEventCreate(uint64_t flags, VPIEvent *event);

/**
 * Destroy an event instance as well as all resources it owns.
 *
 * @param[in] event Event handle to be destroyed.
 *                  It can be NULL. In this case, the function doesn't do anything.
 *                  + All streams referencing the event must be idle, or else undefined behavior will ensue.
 */
VPI_PUBLIC void vpiEventDestroy(VPIEvent event);

/**
 * Captures in the event the contents of the stream command queue at the time of this call.
 *
 * When all tasks recorded are finished, the event will be signaled and calls that are waiting
 * for it will be unblocked.
 *
 * This function can be called multiple times on the same event, however existing
 * \ref vpiEventSync / \ref vpiStreamWaitEvent calls are not affected by later calls to \ref vpiEventRecord.
 * This means they will wait for the event completion of the command queue tasks described by the
 * event at the time of the previous \ref vpiEventSync / \ref vpiStreamWaitEvent call.
 *
 * The operation will implicitly flush the stream.
 *
 * @param[in] event An event handle.
 *                  + Mandatory, it can't be NULL.
 * 
 * @param[in] stream A stream handle whose command queue with tasks yet to be
 *                   executed will be recorded in the event.
 *                   + Mandatory, it can't be NULL.
 *
 * @retval #VPI_ERROR_INVALID_ARGUMENT \p event is NULL or doesn't represent an \ref VPIEvent instance.
 * @retval #VPI_ERROR_INVALID_ARGUMENT \p stream is NULL or not valid or doesn't represent \ref VPIStream instance.
 * @retval #VPI_ERROR_INVALID_ARGUMENT Event doesn't have the backend required by the stream enabled.
 * @retval #VPI_ERROR_INTERNAL         Event doesn't have any backends enabled for recording.
 * @retval #VPI_ERROR_INVALID_CONTEXT  Current context is invalid.
 * @retval #VPI_SUCCESS                Operation executed successfully.
 */
VPI_PUBLIC VPIStatus vpiEventRecord(VPIEvent event, VPIStream stream);

/**
 * Blocks the calling thread until the event is signaled.
 *
 * The event is considered signaled when all the tasks captured by
 * \ref vpiEventRecord are completed or when no tasks were captured.
 *
 * @param[in] event An event handle.
 *                  + Mandatory, it can't be NULL.
 *
 * @retval #VPI_ERROR_INVALID_ARGUMENT \p event is NULL or doesn't represent an \ref VPIEvent instance.
 * @retval #VPI_SUCCESS                Operation executed successfully.
 */
VPI_PUBLIC VPIStatus vpiEventSync(VPIEvent event);

/**
 * Queries the status of all work currently captured by the event.
 *
 * @param[in] event An event handle.
 *                  + Mandatory, it can't be NULL.
 * 
 * @param[out] state Pointer to memory that will receive the event state.
 *                   The returned state is only valid when the status returned is \ref VPI_SUCCESS.
 * 
 * @retval #VPI_ERROR_INVALID_ARGUMENT Output \p state pointer is NULL.
 * @retval #VPI_SUCCESS                Operation executed successfully.
 */
VPI_PUBLIC VPIStatus vpiEventQuery(VPIEvent event, VPIEventState *state);

/**
 * Computes the elapsed time in milliseconds between two completed events.
 *
 * @param[in] start An event handle marking the start of the time interval.
   *                + Mandatory, cannot be NULL.
 *                  + Must have been completed/signaled.
 *                  + Must not have been created with \ref VPI_EVENT_DISABLE_TIMESTAMP flag.
 * 
 * @param[in] end An event handle marking the end of the time interval
 *                + Mandatory, cannot be NULL.
 *                + Must have been completed/signaled.
 *                + Must not have been created with \ref VPI_EVENT_DISABLE_TIMESTAMP flag.
 * 
 * @param[out] msec A pointer to a variable which will be set to the time difference between the
 *                  events signaling.
 *                  + Mandatory, cannot be NULL.
 *
 * @retval #VPI_ERROR_INVALID_ARGUMENT  \p start or \p end is NULL or don't represent an \ref VPIEvent instance.
 * @retval #VPI_ERROR_INVALID_ARGUMENT  Output \p msec pointer is NULL.
 * @retval #VPI_ERROR_INVALID_OPERATION At least one event was created with timestamps disabled.
 * @retval #VPI_ERROR_NOT_READY         At least one event is not completed/signaled.
 * @retval #VPI_SUCCESS                 Operation executed successfully.
 */
VPI_PUBLIC VPIStatus vpiEventElapsedTimeMillis(VPIEvent start, VPIEvent end, float *msec);

/**
 * Returns the event flags passed during event creation.
 *
 * @param[in] event An event handle.
 *                  + Mandatory, cannot be NULL.
 * 
 * @param[out] flags Pointer to memory that will hold the event flags.
 *                   + It can't be NULL.
 *
 * @retval #VPI_ERROR_INVALID_ARGUMENT \p event is NULL or doesn't represent an \ref VPIEvent instance.
 * @retval #VPI_ERROR_INVALID_ARGUMENT \p flags is NULL.
 * @retval #VPI_SUCCESS                Operation executed successfully.
 */
VPI_PUBLIC VPIStatus vpiEventGetFlags(VPIEvent event, uint64_t *flags);

#ifdef __cplusplus
}
#endif

/** @} end of VPI_Event */

#endif /* NV_VPI_EVENT_H */
