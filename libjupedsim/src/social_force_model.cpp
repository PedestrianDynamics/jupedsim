// SPDX-License-Identifier: LGPL-3.0-or-later
#include "jupedsim/social_force_model.h"
#include "jupedsim/error.h"

#include "Conversion.hpp"
#include "ErrorMessage.hpp"

#include <SocialForceModel.hpp>
#include <SocialForceModelBuilder.hpp>
#include <SocialForceModelData.hpp>

using jupedsim::detail::intoJPS_Point;
using jupedsim::detail::intoPoint;
using jupedsim::detail::intoTuple;

////////////////////////////////////////////////////////////////////////////////////////////////////
/// SocialForceModel Model Builder
////////////////////////////////////////////////////////////////////////////////////////////////////
JUPEDSIM_API JPS_SocialForceModelBuilder
JPS_SocialForceModelBuilder_Create(double bodyForce, double friction)
{
    return reinterpret_cast<JPS_SocialForceModelBuilder>(
        new SocialForceModelBuilder(bodyForce, friction));
}

JUPEDSIM_API JPS_OperationalModel JPS_SocialForceModelBuilder_Build(
    JPS_SocialForceModelBuilder handle,
    JPS_ErrorMessage* errorMessage)
{
    assert(handle != nullptr);
    auto builder = reinterpret_cast<SocialForceModelBuilder*>(handle);
    JPS_OperationalModel result{};
    try {
        result = reinterpret_cast<JPS_OperationalModel>(new SocialForceModel(builder->Build()));
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
    return result;
}

JUPEDSIM_API void JPS_SocialForceModelBuilder_Free(JPS_SocialForceModelBuilder handle)
{
    delete reinterpret_cast<SocialForceModelBuilder*>(handle);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// SocialForceModelState
////////////////////////////////////////////////////////////////////////////////////////////////////
JUPEDSIM_API JPS_Point JPS_SocialForceModelState_GetVelocity(JPS_SocialForceModelState handle)
{
    assert(handle);
    const auto state = reinterpret_cast<const SocialForceModelData*>(handle);
    return intoJPS_Point(state->velocity);
}

JUPEDSIM_API void
JPS_SocialForceModelState_SetVelocity(JPS_SocialForceModelState handle, JPS_Point velocity)
{
    assert(handle);
    const auto state = reinterpret_cast<SocialForceModelData*>(handle);
    state->velocity = intoPoint(velocity);
}

JUPEDSIM_API double JPS_SocialForceModelState_GetMass(JPS_SocialForceModelState handle)
{
    assert(handle);
    const auto state = reinterpret_cast<const SocialForceModelData*>(handle);
    return state->mass;
}

JUPEDSIM_API void JPS_SocialForceModelState_SetMass(JPS_SocialForceModelState handle, double mass)
{
    assert(handle);
    const auto state = reinterpret_cast<SocialForceModelData*>(handle);
    state->mass = mass;
}

JUPEDSIM_API double JPS_SocialForceModelState_GetDesiredSpeed(JPS_SocialForceModelState handle)
{
    assert(handle);
    const auto state = reinterpret_cast<const SocialForceModelData*>(handle);
    return state->desiredSpeed;
}

JUPEDSIM_API void
JPS_SocialForceModelState_SetDesiredSpeed(JPS_SocialForceModelState handle, double desiredSpeed)
{
    assert(handle);
    const auto state = reinterpret_cast<SocialForceModelData*>(handle);
    state->desiredSpeed = desiredSpeed;
}

JUPEDSIM_API double JPS_SocialForceModelState_GetReactionTime(JPS_SocialForceModelState handle)
{
    assert(handle);
    const auto state = reinterpret_cast<const SocialForceModelData*>(handle);
    return state->reactionTime;
}

JUPEDSIM_API void
JPS_SocialForceModelState_SetReactionTime(JPS_SocialForceModelState handle, double reactionTime)
{
    assert(handle);
    const auto state = reinterpret_cast<SocialForceModelData*>(handle);
    state->reactionTime = reactionTime;
}

JUPEDSIM_API double JPS_SocialForceModelState_GetAgentScale(JPS_SocialForceModelState handle)
{
    assert(handle);
    const auto state = reinterpret_cast<const SocialForceModelData*>(handle);
    return state->agentScale;
}

JUPEDSIM_API void
JPS_SocialForceModelState_SetAgentScale(JPS_SocialForceModelState handle, double agentScale)
{
    assert(handle);
    const auto state = reinterpret_cast<SocialForceModelData*>(handle);
    state->agentScale = agentScale;
}

JUPEDSIM_API double JPS_SocialForceModelState_GetObstacleScale(JPS_SocialForceModelState handle)
{
    assert(handle);
    const auto state = reinterpret_cast<const SocialForceModelData*>(handle);
    return state->obstacleScale;
}

JUPEDSIM_API void
JPS_SocialForceModelState_SetObstacleScale(JPS_SocialForceModelState handle, double obstacleScale)
{
    assert(handle);
    const auto state = reinterpret_cast<SocialForceModelData*>(handle);
    state->obstacleScale = obstacleScale;
}

JUPEDSIM_API double JPS_SocialForceModelState_GetForceDistance(JPS_SocialForceModelState handle)
{
    assert(handle);
    const auto state = reinterpret_cast<const SocialForceModelData*>(handle);
    return state->forceDistance;
}

JUPEDSIM_API void
JPS_SocialForceModelState_SetForceDistance(JPS_SocialForceModelState handle, double forceDistance)
{
    assert(handle);
    const auto state = reinterpret_cast<SocialForceModelData*>(handle);
    state->forceDistance = forceDistance;
}

JUPEDSIM_API double JPS_SocialForceModelState_GetRadius(JPS_SocialForceModelState handle)
{
    assert(handle);
    const auto state = reinterpret_cast<const SocialForceModelData*>(handle);
    return state->radius;
}

JUPEDSIM_API void
JPS_SocialForceModelState_SetRadius(JPS_SocialForceModelState handle, double radius)
{
    assert(handle);
    const auto state = reinterpret_cast<SocialForceModelData*>(handle);
    state->radius = radius;
}
