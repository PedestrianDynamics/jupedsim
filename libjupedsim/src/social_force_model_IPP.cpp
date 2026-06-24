// SPDX-License-Identifier: LGPL-3.0-or-later
#include "jupedsim/social_force_model_IPP.h"
#include "jupedsim/error.h"

#include "Conversion.hpp"
#include "ErrorMessage.hpp"

#include <SocialForceModelIPP.hpp>
#include <SocialForceModelIPPBuilder.hpp>
#include <SocialForceModelIPPData.hpp>

using jupedsim::detail::intoJPS_Point;
using jupedsim::detail::intoPoint;
using jupedsim::detail::intoTuple;

////////////////////////////////////////////////////////////////////////////////////////////////////
/// SocialForceModelIPP Model Builder
////////////////////////////////////////////////////////////////////////////////////////////////////
JUPEDSIM_API JPS_SocialForceModelIPPBuilder
JPS_SocialForceModelIPPBuilder_Create()
{
    return reinterpret_cast<JPS_SocialForceModelIPPBuilder>(
        new SocialForceModelIPPBuilder());
}

JUPEDSIM_API JPS_OperationalModel JPS_SocialForceModelIPPBuilder_Build(
    JPS_SocialForceModelIPPBuilder handle,
    JPS_ErrorMessage* errorMessage)
{
    assert(handle != nullptr);
    auto builder = reinterpret_cast<SocialForceModelIPPBuilder*>(handle);
    JPS_OperationalModel result{};
    try {
        result = reinterpret_cast<JPS_OperationalModel>(new SocialForceModelIPP(builder->Build()));
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

JUPEDSIM_API void JPS_SocialForceModelIPPBuilder_Free(JPS_SocialForceModelIPPBuilder handle)
{
    delete reinterpret_cast<SocialForceModelIPPBuilder*>(handle);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// SocialForceModelIPPState
////////////////////////////////////////////////////////////////////////////////////////////////////
JUPEDSIM_API JPS_Point JPS_SocialForceModelIPPState_GetVelocity(JPS_SocialForceModelIPPState handle)
{
    assert(handle);
    const auto state = reinterpret_cast<const SocialForceModelIPPData*>(handle);
    return intoJPS_Point(state->velocity);
}

JUPEDSIM_API void
JPS_SocialForceModelIPPState_SetVelocity(JPS_SocialForceModelIPPState handle, JPS_Point velocity)
{
    assert(handle);
    const auto state = reinterpret_cast<SocialForceModelIPPData*>(handle);
    state->velocity = intoPoint(velocity);
}

JUPEDSIM_API JPS_Point JPS_SocialForceModelIPPState_GetGroundSupportPosition(JPS_SocialForceModelIPPState handle)
{
    assert(handle);
    const auto state = reinterpret_cast<const SocialForceModelIPPData*>(handle);
    return intoJPS_Point(state->ground_support_position);
}

JUPEDSIM_API void
JPS_SocialForceModelIPPState_SetGroundSupportPosition(JPS_SocialForceModelIPPState handle, JPS_Point ground_support_position)
{
    assert(handle);
    const auto state = reinterpret_cast<SocialForceModelIPPData*>(handle);
    state->ground_support_position = intoPoint(ground_support_position);
}

JUPEDSIM_API JPS_Point JPS_SocialForceModelIPPState_GetGroundSupportVelocity(JPS_SocialForceModelIPPState handle)
{
    assert(handle);
    const auto state = reinterpret_cast<const SocialForceModelIPPData*>(handle);
    return intoJPS_Point(state->ground_support_velocity);
}

JUPEDSIM_API void
JPS_SocialForceModelIPPState_SetGroundSupportVelocity(JPS_SocialForceModelIPPState handle, JPS_Point ground_support_velocity)
{
    assert(handle);
    const auto state = reinterpret_cast<SocialForceModelIPPData*>(handle);
    state->ground_support_velocity = intoPoint(ground_support_velocity);
}

JUPEDSIM_API double JPS_SocialForceModelIPPState_GetHeight(JPS_SocialForceModelIPPState handle)
{
    assert(handle);
    const auto state = reinterpret_cast<const SocialForceModelIPPData*>(handle);
    return state->height;
}

JUPEDSIM_API void JPS_SocialForceModelIPPState_SetHeight(JPS_SocialForceModelIPPState handle, double height)
{
    assert(handle);
    const auto state = reinterpret_cast<SocialForceModelIPPData*>(handle);
    state->height = height;
}

JUPEDSIM_API double JPS_SocialForceModelIPPState_GetDesiredSpeed(JPS_SocialForceModelIPPState handle)
{
    assert(handle);
    const auto state = reinterpret_cast<const SocialForceModelIPPData*>(handle);
    return state->desiredSpeed;
}

JUPEDSIM_API void
JPS_SocialForceModelIPPState_SetDesiredSpeed(JPS_SocialForceModelIPPState handle, double desiredSpeed)
{
    assert(handle);
    const auto state = reinterpret_cast<SocialForceModelIPPData*>(handle);
    state->desiredSpeed = desiredSpeed;
}

JUPEDSIM_API double JPS_SocialForceModelIPPState_GetReactionTime(JPS_SocialForceModelIPPState handle)
{
    assert(handle);
    const auto state = reinterpret_cast<const SocialForceModelIPPData*>(handle);
    return state->reactionTime;
}

JUPEDSIM_API void
JPS_SocialForceModelIPPState_SetReactionTime(JPS_SocialForceModelIPPState handle, double reactionTime)
{
    assert(handle);
    const auto state = reinterpret_cast<SocialForceModelIPPData*>(handle);
    state->reactionTime = reactionTime;
}

JUPEDSIM_API double JPS_SocialForceModelIPPState_GetLambdaU(JPS_SocialForceModelIPPState handle)
{
    assert(handle);
    const auto state = reinterpret_cast<const SocialForceModelIPPData*>(handle);
    return state->lambdaU;
}

JUPEDSIM_API void
JPS_SocialForceModelIPPState_SetLambdaU(JPS_SocialForceModelIPPState handle, double lambdaU)
{
    assert(handle);
    const auto state = reinterpret_cast<SocialForceModelIPPData*>(handle);
    state->lambdaU = lambdaU;
}

JUPEDSIM_API double JPS_SocialForceModelIPPState_GetLambdaB(JPS_SocialForceModelIPPState handle)
{
    assert(handle);
    const auto state = reinterpret_cast<const SocialForceModelIPPData*>(handle);
    return state->lambdaB;
}

JUPEDSIM_API void
JPS_SocialForceModelIPPState_SetLambdaB(JPS_SocialForceModelIPPState handle, double lambdaB)
{
    assert(handle);
    const auto state = reinterpret_cast<SocialForceModelIPPData*>(handle);
    state->lambdaB = lambdaB;
}

JUPEDSIM_API double JPS_SocialForceModelIPPState_GetBalanceSpeed(JPS_SocialForceModelIPPState handle)
{
    assert(handle);
    const auto state = reinterpret_cast<const SocialForceModelIPPData*>(handle);
    return state->balanceSpeed;
}

JUPEDSIM_API void
JPS_SocialForceModelIPPState_SetBalanceSpeed(JPS_SocialForceModelIPPState handle, double balanceSpeed)
{
    assert(handle);
    const auto state = reinterpret_cast<SocialForceModelIPPData*>(handle);
    state->balanceSpeed = balanceSpeed;
}

JUPEDSIM_API double JPS_SocialForceModelIPPState_GetDamping(JPS_SocialForceModelIPPState handle)
{
    assert(handle);
    const auto state = reinterpret_cast<const SocialForceModelIPPData*>(handle);
    return state->damping;
}

JUPEDSIM_API void
JPS_SocialForceModelIPPState_SetDamping(JPS_SocialForceModelIPPState handle, double damping)
{
    assert(handle);
    const auto state = reinterpret_cast<SocialForceModelIPPData*>(handle);
    state->damping = damping;
}

JUPEDSIM_API double JPS_SocialForceModelIPPState_GetAgentScale(JPS_SocialForceModelIPPState handle)
{
    assert(handle);
    const auto state = reinterpret_cast<const SocialForceModelIPPData*>(handle);
    return state->agentScale;
}

JUPEDSIM_API void
JPS_SocialForceModelIPPState_SetAgentScale(JPS_SocialForceModelIPPState handle, double agentScale)
{
    assert(handle);
    const auto state = reinterpret_cast<SocialForceModelIPPData*>(handle);
    state->agentScale = agentScale;
}

JUPEDSIM_API double JPS_SocialForceModelIPPState_GetObstacleScale(JPS_SocialForceModelIPPState handle)
{
    assert(handle);
    const auto state = reinterpret_cast<const SocialForceModelIPPData*>(handle);
    return state->obstacleScale;
}

JUPEDSIM_API void
JPS_SocialForceModelIPPState_SetObstacleScale(JPS_SocialForceModelIPPState handle, double obstacleScale)
{
    assert(handle);
    const auto state = reinterpret_cast<SocialForceModelIPPData*>(handle);
    state->obstacleScale = obstacleScale;
}

JUPEDSIM_API double JPS_SocialForceModelIPPState_GetLegScale(JPS_SocialForceModelIPPState handle)
{
    assert(handle);
    const auto state = reinterpret_cast<const SocialForceModelIPPData*>(handle);
    return state->legScale;
}

JUPEDSIM_API void
JPS_SocialForceModelIPPState_SetLegScale(JPS_SocialForceModelIPPState handle, double legScale)
{
    assert(handle);
    const auto state = reinterpret_cast<SocialForceModelIPPData*>(handle);
    state->legScale = legScale;
}

JUPEDSIM_API double JPS_SocialForceModelIPPState_GetForceDistance(JPS_SocialForceModelIPPState handle)
{
    assert(handle);
    const auto state = reinterpret_cast<const SocialForceModelIPPData*>(handle);
    return state->forceDistance;
}

JUPEDSIM_API void
JPS_SocialForceModelIPPState_SetForceDistance(JPS_SocialForceModelIPPState handle, double forceDistance)
{
    assert(handle);
    const auto state = reinterpret_cast<SocialForceModelIPPData*>(handle);
    state->forceDistance = forceDistance;
}

JUPEDSIM_API double JPS_SocialForceModelIPPState_GetObstacleForceDistance(JPS_SocialForceModelIPPState handle)
{
    assert(handle);
    const auto state = reinterpret_cast<const SocialForceModelIPPData*>(handle);
    return state->obstacleForceDistance;
}

JUPEDSIM_API void
JPS_SocialForceModelIPPState_SetObstacleForceDistance(JPS_SocialForceModelIPPState handle, double obstacleForceDistance)
{
    assert(handle);
    const auto state = reinterpret_cast<SocialForceModelIPPData*>(handle);
    state->obstacleForceDistance = obstacleForceDistance;
}

JUPEDSIM_API double JPS_SocialForceModelIPPState_GetLegForceDistance(JPS_SocialForceModelIPPState handle)
{
    assert(handle);
    const auto state = reinterpret_cast<const SocialForceModelIPPData*>(handle);
    return state->legForceDistance;
}

JUPEDSIM_API void
JPS_SocialForceModelIPPState_SetLegForceDistance(JPS_SocialForceModelIPPState handle, double legForceDistance)
{
    assert(handle);
    const auto state = reinterpret_cast<SocialForceModelIPPData*>(handle);
    state->legForceDistance = legForceDistance;
}

JUPEDSIM_API double JPS_SocialForceModelIPPState_GetRadius(JPS_SocialForceModelIPPState handle)
{
    assert(handle);
    const auto state = reinterpret_cast<const SocialForceModelIPPData*>(handle);
    return state->radius;
}

JUPEDSIM_API void
JPS_SocialForceModelIPPState_SetRadius(JPS_SocialForceModelIPPState handle, double radius)
{
    assert(handle);
    const auto state = reinterpret_cast<SocialForceModelIPPData*>(handle);
    state->radius = radius;
}

JUPEDSIM_API double
JPS_SocialForceModelIPPState_GetBodyForce(JPS_SocialForceModelIPPState handle)
{
    assert(handle);
    const auto state = reinterpret_cast<const SocialForceModelIPPData*>(handle);
    return state->bodyForce;
}

JUPEDSIM_API void
JPS_SocialForceModelIPPState_SetBodyForce(JPS_SocialForceModelIPPState handle, double bodyForce)
{
    assert(handle);
    const auto state = reinterpret_cast<SocialForceModelIPPData*>(handle);
    state->bodyForce = bodyForce;
}

JUPEDSIM_API double
JPS_SocialForceModelIPPState_GetFriction(JPS_SocialForceModelIPPState handle)
{
    assert(handle);
    const auto state = reinterpret_cast<const SocialForceModelIPPData*>(handle);
    return state->friction;
}

JUPEDSIM_API void
JPS_SocialForceModelIPPState_SetFriction(JPS_SocialForceModelIPPState handle, double friction)
{
    assert(handle);
    const auto state = reinterpret_cast<SocialForceModelIPPData*>(handle);
    state->friction = friction;
}
