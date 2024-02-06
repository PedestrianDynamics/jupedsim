/* Copyright © 2012-2024 Forschungszentrum Jülich GmbH */
/* SPDX-License-Identifier: LGPL-3.0-or-later */
#pragma once

#include "error.h"
#include "export.h"
#include "types.h"

#include <stddef.h> /*NOLINT(modernize-deprecated-headers)*/

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Identifies the type of stage
 */
enum JPS_StageType {
    JPS_NotifiableQueueType,
    JPS_WaitingSetType,
    JPS_WaypointType,
    JPS_ExitType,
    JPS_DirectSteeringType
};

/**
 * Opaque type of an NotifiableQueueProxy
 */
typedef struct JPS_NotifiableQueueProxy_t* JPS_NotifiableQueueProxy;

JUPEDSIM_API size_t JPS_NotifiableQueueProxy_GetCountTargeting(JPS_NotifiableQueueProxy handle);

JUPEDSIM_API size_t JPS_NotifiableQueueProxy_GetCountEnqueued(JPS_NotifiableQueueProxy handle);

JUPEDSIM_API void JPS_NotifiableQueueProxy_Pop(JPS_NotifiableQueueProxy handle, size_t count);

JUPEDSIM_API size_t
JPS_NotifiableQueueProxy_GetEnqueued(JPS_NotifiableQueueProxy handle, const JPS_AgentId** data);

JUPEDSIM_API void JPS_NotifiableQueueProxy_Free(JPS_NotifiableQueueProxy handle);

enum JPS_WaitingSetState { JPS_WaitingSet_Active, JPS_WaitingSet_Inactive };
typedef struct JPS_WaitingSetProxy_t* JPS_WaitingSetProxy;

JUPEDSIM_API size_t JPS_WaitingSetProxy_GetCountTargeting(JPS_WaitingSetProxy handle);

JUPEDSIM_API size_t JPS_WaitingSetProxy_GetCountWaiting(JPS_WaitingSetProxy handle);

JUPEDSIM_API size_t
JPS_WaitingSetProxy_GetWaiting(JPS_WaitingSetProxy handle, const JPS_AgentId** data);

JUPEDSIM_API void
JPS_WaitingSetProxy_SetWaitingSetState(JPS_WaitingSetProxy handle, JPS_WaitingSetState newState);

JUPEDSIM_API JPS_WaitingSetState JPS_WaitingSetProxy_GetWaitingSetState(JPS_WaitingSetProxy handle);

JUPEDSIM_API void JPS_WaitingSetProxy_Free(JPS_WaitingSetProxy handle);

typedef struct JPS_WaypointProxy_t* JPS_WaypointProxy;

JUPEDSIM_API size_t JPS_WaypointProxy_GetCountTargeting(JPS_WaypointProxy handle);

JUPEDSIM_API void JPS_WaypointProxy_Free(JPS_WaypointProxy handle);

typedef struct JPS_ExitProxy_t* JPS_ExitProxy;

JUPEDSIM_API size_t JPS_ExitProxy_GetCountTargeting(JPS_ExitProxy handle);

JUPEDSIM_API void JPS_ExitProxy_Free(JPS_ExitProxy handle);

typedef struct JPS_DirectSteeringProxy_t* JPS_DirectSteeringProxy;

JUPEDSIM_API void JPS_DirectSteeringProxy_Free(JPS_DirectSteeringProxy handle);

#ifdef __cplusplus
}
#endif
