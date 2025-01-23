// SPDX-License-Identifier: LGPL-3.0-or-later
#include "jupedsim/stage.h"

#include <Stage.hpp>
#include <Unreachable.hpp>

#include <cassert>

////////////////////////////////////////////////////////////////////////////////////////////////////
/// NotifiableQueueProxy
////////////////////////////////////////////////////////////////////////////////////////////////////
size_t JPS_NotifiableQueueProxy_GetCountTargeting(JPS_NotifiableQueueProxy handle)
{
    assert(handle);
    auto proxy = reinterpret_cast<NotifiableQueueProxy*>(handle);
    return proxy->CountTargeting();
}

size_t JPS_NotifiableQueueProxy_GetCountEnqueued(JPS_NotifiableQueueProxy handle)
{
    assert(handle);
    auto proxy = reinterpret_cast<NotifiableQueueProxy*>(handle);
    return proxy->CountEnqueued();
}

void JPS_NotifiableQueueProxy_Pop(JPS_NotifiableQueueProxy handle, size_t count)
{
    assert(handle);
    auto proxy = reinterpret_cast<NotifiableQueueProxy*>(handle);
    proxy->Pop(count);
}

size_t
JPS_NotifiableQueueProxy_GetEnqueued(JPS_NotifiableQueueProxy handle, const JPS_AgentId** data)
{
    assert(handle);
    auto proxy = reinterpret_cast<NotifiableQueueProxy*>(handle);
    const auto& agents = proxy->Enqueued();
    static_assert(
        std::is_same<JPS_AgentId, GenericAgent::ID::underlying_type>::value,
        "GenericAgentIDs cannot be casted in JPS_AgentId");
    *data = reinterpret_cast<const JPS_AgentId*>(agents.data());
    return agents.size();
}

void JPS_NotifiableQueueProxy_Free(JPS_NotifiableQueueProxy handle)
{
    delete reinterpret_cast<NotifiableQueueProxy*>(handle);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// WaitingSetProxy
////////////////////////////////////////////////////////////////////////////////////////////////////
size_t JPS_WaitingSetProxy_GetCountTargeting(JPS_WaitingSetProxy handle)
{
    assert(handle);
    auto proxy = reinterpret_cast<NotifiableWaitingSetProxy*>(handle);
    return proxy->CountTargeting();
}

size_t JPS_WaitingSetProxy_GetCountWaiting(JPS_WaitingSetProxy handle)
{
    assert(handle);
    auto proxy = reinterpret_cast<NotifiableWaitingSetProxy*>(handle);
    return proxy->CountWaiting();
}

size_t JPS_WaitingSetProxy_GetWaiting(JPS_WaitingSetProxy handle, const JPS_AgentId** data)
{
    assert(handle);
    auto proxy = reinterpret_cast<NotifiableWaitingSetProxy*>(handle);
    const auto& agents = proxy->Waiting();
    static_assert(
        std::is_same<JPS_AgentId, GenericAgent::ID::underlying_type>::value,
        "GenericAgentIDs cannot be casted in JPS_AgentId");
    *data = reinterpret_cast<const JPS_AgentId*>(agents.data());
    return agents.size();
}

void JPS_WaitingSetProxy_SetWaitingSetState(
    JPS_WaitingSetProxy handle,
    JPS_WaitingSetState newState)
{
    const auto convert = [](const auto s) {
        switch(s) {
            case JPS_WaitingSet_Active:
                return WaitingSetState::Active;
            case JPS_WaitingSet_Inactive:
                return WaitingSetState::Inactive;
        }
        UNREACHABLE();
    };
    assert(handle);
    auto proxy = reinterpret_cast<NotifiableWaitingSetProxy*>(handle);
    proxy->State(convert(newState));
}

JPS_WaitingSetState JPS_WaitingSetProxy_GetWaitingSetState(JPS_WaitingSetProxy handle)
{
    const auto convert = [](const auto s) {
        switch(s) {
            case WaitingSetState::Active:
                return JPS_WaitingSet_Active;
            case WaitingSetState::Inactive:
                return JPS_WaitingSet_Inactive;
        }
        UNREACHABLE();
    };
    assert(handle);
    auto proxy = reinterpret_cast<NotifiableWaitingSetProxy*>(handle);
    return convert(proxy->State());
}

void JPS_WaitingSetProxy_Free(JPS_WaitingSetProxy handle)
{
    delete reinterpret_cast<NotifiableWaitingSetProxy*>(handle);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// DirectSteeringProxy
////////////////////////////////////////////////////////////////////////////////////////////////////
void JPS_DirectSteeringProxy_Free(JPS_DirectSteeringProxy handle)
{
    delete reinterpret_cast<DirectSteeringProxy*>(handle);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// WaitPointProxy
////////////////////////////////////////////////////////////////////////////////////////////////////
size_t JPS_WaypointProxy_GetCountTargeting(JPS_WaypointProxy handle)
{
    assert(handle);
    auto proxy = reinterpret_cast<WaypointProxy*>(handle);
    return proxy->CountTargeting();
}

void JPS_WaypointProxy_Free(JPS_WaypointProxy handle)
{
    delete reinterpret_cast<WaypointProxy*>(handle);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// ExitProxy
////////////////////////////////////////////////////////////////////////////////////////////////////
size_t JPS_ExitProxy_GetCountTargeting(JPS_ExitProxy handle)
{
    assert(handle);
    auto proxy = reinterpret_cast<ExitProxy*>(handle);
    return proxy->CountTargeting();
}

void JPS_ExitProxy_Free(JPS_ExitProxy handle)
{
    delete reinterpret_cast<ExitProxy*>(handle);
}
