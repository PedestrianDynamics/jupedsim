// Copyright © 2012-2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "jupedsim/agent.h"

#include "AgentIterator.hpp"
#include "Conversion.hpp"
#include "ErrorMessage.hpp"

#include <GenericAgent.hpp>
#include <Unreachable.hpp>

#include <cassert>

using jupedsim::detail::intoJPS_Point;
using jupedsim::detail::intoPoint;

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Agent
////////////////////////////////////////////////////////////////////////////////////////////////////
JPS_AgentId JPS_Agent_GetId(JPS_Agent handle)
{
    assert(handle);
    const auto agent = reinterpret_cast<const GenericAgent*>(handle);
    return agent->id.getID();
}

JPS_JourneyId JPS_Agent_GetJourneyId(JPS_Agent handle)
{
    assert(handle);
    const auto agent = reinterpret_cast<const GenericAgent*>(handle);
    return agent->journeyId.getID();
}

JPS_StageId JPS_Agent_GetStageId(JPS_Agent handle)
{
    assert(handle);
    const auto agent = reinterpret_cast<const GenericAgent*>(handle);
    return agent->stageId.getID();
}

JPS_Point JPS_Agent_GetPosition(JPS_Agent handle)
{
    assert(handle);
    const auto agent = reinterpret_cast<const GenericAgent*>(handle);
    return intoJPS_Point(agent->pos);
}

JPS_Point JPS_Agent_GetTarget(JPS_Agent handle)
{
    assert(handle);
    const auto agent = reinterpret_cast<const GenericAgent*>(handle);
    return intoJPS_Point(agent->target);
}

bool JPS_Agent_SetTarget(JPS_Agent handle, JPS_Point waypoint, JPS_ErrorMessage* errorMessage)
{
    assert(handle);
    try {
        auto agent = reinterpret_cast<GenericAgent*>(handle);
        agent->target = intoPoint(waypoint);
        return true;
    } catch(const std::exception& ex) {
        if(errorMessage) {
            *errorMessage = reinterpret_cast<JPS_ErrorMessage>(new JPS_ErrorMessage_t{ex.what()});
        }
    } catch(...) {
        if(errorMessage) {
            *errorMessage = reinterpret_cast<JPS_ErrorMessage>(
                new JPS_ErrorMessage_t{"Unknown internal error."});
        }
    }
    return false;
}

JPS_Point JPS_Agent_GetOrientation(JPS_Agent handle)
{
    assert(handle);
    const auto agent = reinterpret_cast<const GenericAgent*>(handle);
    return intoJPS_Point(agent->orientation);
}

JPS_ModelType JPS_Agent_GetModelType(JPS_Agent handle)
{
    assert(handle);
    const auto agent = reinterpret_cast<const GenericAgent*>(handle);
    switch(agent->model.index()) {
        case 0:
            return JPS_GeneralizedCentrifugalForceModel;
        case 1:
            return JPS_CollisionFreeSpeedModel;
        case 2:
            return JPS_CollisionFreeSpeedModelV2;
        case 3:
            return JPS_CollisionFreeSpeedModelV3;    
        case 4:
            return JPS_SocialForceModel;
    }
    UNREACHABLE();
}

JPS_GeneralizedCentrifugalForceModelState
JPS_Agent_GetGeneralizedCentrifugalForceModelState(JPS_Agent handle, JPS_ErrorMessage* errorMessage)
{
    assert(handle);
    const auto agent = reinterpret_cast<GenericAgent*>(handle);
    try {
        auto& model = std::get<GeneralizedCentrifugalForceModelData>(agent->model);
        return reinterpret_cast<JPS_GeneralizedCentrifugalForceModelState>(&model);
    } catch(const std::exception& ex) {
        if(errorMessage) {
            *errorMessage = reinterpret_cast<JPS_ErrorMessage>(new JPS_ErrorMessage_t{ex.what()});
        }
    } catch(...) {
        if(errorMessage) {
            *errorMessage = reinterpret_cast<JPS_ErrorMessage>(
                new JPS_ErrorMessage_t{"Unknown internal error."});
        }
    }
    return nullptr;
}

JPS_CollisionFreeSpeedModelState
JPS_Agent_GetCollisionFreeSpeedModelState(JPS_Agent handle, JPS_ErrorMessage* errorMessage)
{
    assert(handle);
    const auto agent = reinterpret_cast<GenericAgent*>(handle);
    try {
        auto& model = std::get<CollisionFreeSpeedModelData>(agent->model);
        return reinterpret_cast<JPS_CollisionFreeSpeedModelState>(&model);
    } catch(const std::exception& ex) {
        if(errorMessage) {
            *errorMessage = reinterpret_cast<JPS_ErrorMessage>(new JPS_ErrorMessage_t{ex.what()});
        }
    } catch(...) {
        if(errorMessage) {
            *errorMessage = reinterpret_cast<JPS_ErrorMessage>(
                new JPS_ErrorMessage_t{"Unknown internal error."});
        }
    }
    return nullptr;
}

JPS_CollisionFreeSpeedModelV2State
JPS_Agent_GetCollisionFreeSpeedModelV2State(JPS_Agent handle, JPS_ErrorMessage* errorMessage)
{
    assert(handle);
    const auto agent = reinterpret_cast<GenericAgent*>(handle);
    try {
        auto& model = std::get<CollisionFreeSpeedModelV2Data>(agent->model);
        return reinterpret_cast<JPS_CollisionFreeSpeedModelV2State>(&model);
    } catch(const std::exception& ex) {
        if(errorMessage) {
            *errorMessage = reinterpret_cast<JPS_ErrorMessage>(new JPS_ErrorMessage_t{ex.what()});
        }
    } catch(...) {
        if(errorMessage) {
            *errorMessage = reinterpret_cast<JPS_ErrorMessage>(
                new JPS_ErrorMessage_t{"Unknown internal error."});
        }
    }
    return nullptr;
}

JPS_SocialForceModelState
JPS_Agent_GetSocialForceModelState(JPS_Agent handle, JPS_ErrorMessage* errorMessage)
{
    assert(handle);
    const auto agent = reinterpret_cast<GenericAgent*>(handle);
    try {
        auto& model = std::get<SocialForceModelData>(agent->model);
        return reinterpret_cast<JPS_SocialForceModelState>(&model);
    } catch(const std::exception& ex) {
        if(errorMessage) {
            *errorMessage = reinterpret_cast<JPS_ErrorMessage>(new JPS_ErrorMessage_t{ex.what()});
        }
    } catch(...) {
        if(errorMessage) {
            *errorMessage = reinterpret_cast<JPS_ErrorMessage>(
                new JPS_ErrorMessage_t{"Unknown internal error."});
        }
    }
    return nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// AgentIterator
////////////////////////////////////////////////////////////////////////////////////////////////////
JPS_Agent JPS_AgentIterator_Next(JPS_AgentIterator handle)
{
    assert(handle);
    auto iterator = reinterpret_cast<AgentIterator<GenericAgent>*>(handle);
    return reinterpret_cast<JPS_Agent>(iterator->Next());
}

void JPS_AgentIterator_Free(JPS_AgentIterator handle)
{
    delete reinterpret_cast<AgentIterator<GenericAgent>*>(handle);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// AgentIdIterator
////////////////////////////////////////////////////////////////////////////////////////////////////
JPS_AgentId JPS_AgentIdIterator_Next(JPS_AgentIdIterator handle)
{
    assert(handle);
    auto iterator = reinterpret_cast<AgentIdIterator*>(handle);
    auto& [vec, iter] = *iterator;
    if(iter == std::end(vec)) {
        return GenericAgent::ID::Invalid.getID();
    }
    const auto id = *iter;
    ++iter;
    return id.getID();
}

void JPS_AgentIdIterator_Free(JPS_AgentIdIterator handle)
{
    delete reinterpret_cast<AgentIdIterator*>(handle);
}
