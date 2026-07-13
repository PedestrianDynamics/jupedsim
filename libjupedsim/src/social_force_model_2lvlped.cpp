// SPDX-License-Identifier: LGPL-3.0-or-later
#include "jupedsim/social_force_model_2lvlped.h"
#include "jupedsim/error.h"

#include "Conversion.hpp"
#include "ErrorMessage.hpp"

#include <SocialForceModel2LvlPed.hpp>
#include <SocialForceModel2LvlPedBuilder.hpp>
#include <SocialForceModel2LvlPedData.hpp>

using jupedsim::detail::intoJPS_Point;
using jupedsim::detail::intoPoint;
using jupedsim::detail::intoTuple;

////////////////////////////////////////////////////////////////////////////////////////////////////
/// SocialForceModel2LvlPed Model Builder
////////////////////////////////////////////////////////////////////////////////////////////////////
JUPEDSIM_API JPS_SocialForceModel2LvlPedBuilder
JPS_SocialForceModel2LvlPedBuilder_Create()
{
    return reinterpret_cast<JPS_SocialForceModel2LvlPedBuilder>(
        new SocialForceModel2LvlPedBuilder());
}

JUPEDSIM_API JPS_OperationalModel JPS_SocialForceModel2LvlPedBuilder_Build(
    JPS_SocialForceModel2LvlPedBuilder handle,
    JPS_ErrorMessage* errorMessage)
{
    assert(handle != nullptr);
    auto builder = reinterpret_cast<SocialForceModel2LvlPedBuilder*>(handle);
    JPS_OperationalModel result{};
    try {
        result = reinterpret_cast<JPS_OperationalModel>(new SocialForceModel2LvlPed(builder->Build()));
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

JUPEDSIM_API void JPS_SocialForceModel2LvlPedBuilder_Free(JPS_SocialForceModel2LvlPedBuilder handle)
{
    delete reinterpret_cast<SocialForceModel2LvlPedBuilder*>(handle);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// SocialForceModel2LvlPedState
////////////////////////////////////////////////////////////////////////////////////////////////////
JUPEDSIM_API JPS_Point JPS_SocialForceModel2LvlPedState_GetVelocity(JPS_SocialForceModel2LvlPedState handle)
{
    assert(handle);
    const auto state = reinterpret_cast<const SocialForceModel2LvlPedData*>(handle);
    return intoJPS_Point(state->velocity);
}

JUPEDSIM_API void
JPS_SocialForceModel2LvlPedState_SetVelocity(JPS_SocialForceModel2LvlPedState handle, JPS_Point velocity)
{
    assert(handle);
    const auto state = reinterpret_cast<SocialForceModel2LvlPedData*>(handle);
    state->velocity = intoPoint(velocity);
}

JUPEDSIM_API JPS_Point JPS_SocialForceModel2LvlPedState_GetGroundSupportPosition(JPS_SocialForceModel2LvlPedState handle)
{
    assert(handle);
    const auto state = reinterpret_cast<const SocialForceModel2LvlPedData*>(handle);
    return intoJPS_Point(state->ground_support_position);
}

JUPEDSIM_API void
JPS_SocialForceModel2LvlPedState_SetGroundSupportPosition(JPS_SocialForceModel2LvlPedState handle, JPS_Point ground_support_position)
{
    assert(handle);
    const auto state = reinterpret_cast<SocialForceModel2LvlPedData*>(handle);
    state->ground_support_position = intoPoint(ground_support_position);
}

JUPEDSIM_API JPS_Point JPS_SocialForceModel2LvlPedState_GetGroundSupportVelocity(JPS_SocialForceModel2LvlPedState handle)
{
    assert(handle);
    const auto state = reinterpret_cast<const SocialForceModel2LvlPedData*>(handle);
    return intoJPS_Point(state->ground_support_velocity);
}

JUPEDSIM_API void
JPS_SocialForceModel2LvlPedState_SetGroundSupportVelocity(JPS_SocialForceModel2LvlPedState handle, JPS_Point ground_support_velocity)
{
    assert(handle);
    const auto state = reinterpret_cast<SocialForceModel2LvlPedData*>(handle);
    state->ground_support_velocity = intoPoint(ground_support_velocity);
}

JUPEDSIM_API double JPS_SocialForceModel2LvlPedState_GetHeight(JPS_SocialForceModel2LvlPedState handle)
{
    assert(handle);
    const auto state = reinterpret_cast<const SocialForceModel2LvlPedData*>(handle);
    return state->height;
}

JUPEDSIM_API void JPS_SocialForceModel2LvlPedState_SetHeight(JPS_SocialForceModel2LvlPedState handle, double height)
{
    assert(handle);
    const auto state = reinterpret_cast<SocialForceModel2LvlPedData*>(handle);
    state->height = height;
}

JUPEDSIM_API double JPS_SocialForceModel2LvlPedState_GetDesiredSpeed(JPS_SocialForceModel2LvlPedState handle)
{
    assert(handle);
    const auto state = reinterpret_cast<const SocialForceModel2LvlPedData*>(handle);
    return state->desiredSpeed;
}

JUPEDSIM_API void
JPS_SocialForceModel2LvlPedState_SetDesiredSpeed(JPS_SocialForceModel2LvlPedState handle, double desiredSpeed)
{
    assert(handle);
    const auto state = reinterpret_cast<SocialForceModel2LvlPedData*>(handle);
    state->desiredSpeed = desiredSpeed;
}

JUPEDSIM_API double JPS_SocialForceModel2LvlPedState_GetReactionTime(JPS_SocialForceModel2LvlPedState handle)
{
    assert(handle);
    const auto state = reinterpret_cast<const SocialForceModel2LvlPedData*>(handle);
    return state->reactionTime;
}

JUPEDSIM_API void
JPS_SocialForceModel2LvlPedState_SetReactionTime(JPS_SocialForceModel2LvlPedState handle, double reactionTime)
{
    assert(handle);
    const auto state = reinterpret_cast<SocialForceModel2LvlPedData*>(handle);
    state->reactionTime = reactionTime;
}

JUPEDSIM_API double JPS_SocialForceModel2LvlPedState_GetLambdaU(JPS_SocialForceModel2LvlPedState handle)
{
    assert(handle);
    const auto state = reinterpret_cast<const SocialForceModel2LvlPedData*>(handle);
    return state->lambdaU;
}

JUPEDSIM_API void
JPS_SocialForceModel2LvlPedState_SetLambdaU(JPS_SocialForceModel2LvlPedState handle, double lambdaU)
{
    assert(handle);
    const auto state = reinterpret_cast<SocialForceModel2LvlPedData*>(handle);
    state->lambdaU = lambdaU;
}

JUPEDSIM_API double JPS_SocialForceModel2LvlPedState_GetLambdaB(JPS_SocialForceModel2LvlPedState handle)
{
    assert(handle);
    const auto state = reinterpret_cast<const SocialForceModel2LvlPedData*>(handle);
    return state->lambdaB;
}

JUPEDSIM_API void
JPS_SocialForceModel2LvlPedState_SetLambdaB(JPS_SocialForceModel2LvlPedState handle, double lambdaB)
{
    assert(handle);
    const auto state = reinterpret_cast<SocialForceModel2LvlPedData*>(handle);
    state->lambdaB = lambdaB;
}

JUPEDSIM_API double JPS_SocialForceModel2LvlPedState_GetBalanceSpeed(JPS_SocialForceModel2LvlPedState handle)
{
    assert(handle);
    const auto state = reinterpret_cast<const SocialForceModel2LvlPedData*>(handle);
    return state->balanceSpeed;
}

JUPEDSIM_API void
JPS_SocialForceModel2LvlPedState_SetBalanceSpeed(JPS_SocialForceModel2LvlPedState handle, double balanceSpeed)
{
    assert(handle);
    const auto state = reinterpret_cast<SocialForceModel2LvlPedData*>(handle);
    state->balanceSpeed = balanceSpeed;
}

JUPEDSIM_API double JPS_SocialForceModel2LvlPedState_GetDamping(JPS_SocialForceModel2LvlPedState handle)
{
    assert(handle);
    const auto state = reinterpret_cast<const SocialForceModel2LvlPedData*>(handle);
    return state->damping;
}

JUPEDSIM_API void
JPS_SocialForceModel2LvlPedState_SetDamping(JPS_SocialForceModel2LvlPedState handle, double damping)
{
    assert(handle);
    const auto state = reinterpret_cast<SocialForceModel2LvlPedData*>(handle);
    state->damping = damping;
}

JUPEDSIM_API double JPS_SocialForceModel2LvlPedState_GetAgentScale(JPS_SocialForceModel2LvlPedState handle)
{
    assert(handle);
    const auto state = reinterpret_cast<const SocialForceModel2LvlPedData*>(handle);
    return state->agentScale;
}

JUPEDSIM_API void
JPS_SocialForceModel2LvlPedState_SetAgentScale(JPS_SocialForceModel2LvlPedState handle, double agentScale)
{
    assert(handle);
    const auto state = reinterpret_cast<SocialForceModel2LvlPedData*>(handle);
    state->agentScale = agentScale;
}

JUPEDSIM_API double JPS_SocialForceModel2LvlPedState_GetObstacleScale(JPS_SocialForceModel2LvlPedState handle)
{
    assert(handle);
    const auto state = reinterpret_cast<const SocialForceModel2LvlPedData*>(handle);
    return state->obstacleScale;
}

JUPEDSIM_API void
JPS_SocialForceModel2LvlPedState_SetObstacleScale(JPS_SocialForceModel2LvlPedState handle, double obstacleScale)
{
    assert(handle);
    const auto state = reinterpret_cast<SocialForceModel2LvlPedData*>(handle);
    state->obstacleScale = obstacleScale;
}

JUPEDSIM_API double JPS_SocialForceModel2LvlPedState_GetLegScale(JPS_SocialForceModel2LvlPedState handle)
{
    assert(handle);
    const auto state = reinterpret_cast<const SocialForceModel2LvlPedData*>(handle);
    return state->legScale;
}

JUPEDSIM_API void
JPS_SocialForceModel2LvlPedState_SetLegScale(JPS_SocialForceModel2LvlPedState handle, double legScale)
{
    assert(handle);
    const auto state = reinterpret_cast<SocialForceModel2LvlPedData*>(handle);
    state->legScale = legScale;
}

JUPEDSIM_API double JPS_SocialForceModel2LvlPedState_GetForceDistance(JPS_SocialForceModel2LvlPedState handle)
{
    assert(handle);
    const auto state = reinterpret_cast<const SocialForceModel2LvlPedData*>(handle);
    return state->forceDistance;
}

JUPEDSIM_API void
JPS_SocialForceModel2LvlPedState_SetForceDistance(JPS_SocialForceModel2LvlPedState handle, double forceDistance)
{
    assert(handle);
    const auto state = reinterpret_cast<SocialForceModel2LvlPedData*>(handle);
    state->forceDistance = forceDistance;
}

JUPEDSIM_API double JPS_SocialForceModel2LvlPedState_GetObstacleForceDistance(JPS_SocialForceModel2LvlPedState handle)
{
    assert(handle);
    const auto state = reinterpret_cast<const SocialForceModel2LvlPedData*>(handle);
    return state->obstacleForceDistance;
}

JUPEDSIM_API void
JPS_SocialForceModel2LvlPedState_SetObstacleForceDistance(JPS_SocialForceModel2LvlPedState handle, double obstacleForceDistance)
{
    assert(handle);
    const auto state = reinterpret_cast<SocialForceModel2LvlPedData*>(handle);
    state->obstacleForceDistance = obstacleForceDistance;
}

JUPEDSIM_API double JPS_SocialForceModel2LvlPedState_GetLegForceDistance(JPS_SocialForceModel2LvlPedState handle)
{
    assert(handle);
    const auto state = reinterpret_cast<const SocialForceModel2LvlPedData*>(handle);
    return state->legForceDistance;
}

JUPEDSIM_API void
JPS_SocialForceModel2LvlPedState_SetLegForceDistance(JPS_SocialForceModel2LvlPedState handle, double legForceDistance)
{
    assert(handle);
    const auto state = reinterpret_cast<SocialForceModel2LvlPedData*>(handle);
    state->legForceDistance = legForceDistance;
}

JUPEDSIM_API double JPS_SocialForceModel2LvlPedState_GetRadius(JPS_SocialForceModel2LvlPedState handle)
{
    assert(handle);
    const auto state = reinterpret_cast<const SocialForceModel2LvlPedData*>(handle);
    return state->radius;
}

JUPEDSIM_API void
JPS_SocialForceModel2LvlPedState_SetRadius(JPS_SocialForceModel2LvlPedState handle, double radius)
{
    assert(handle);
    const auto state = reinterpret_cast<SocialForceModel2LvlPedData*>(handle);
    state->radius = radius;
}

JUPEDSIM_API double
JPS_SocialForceModel2LvlPedState_GetBodyForce(JPS_SocialForceModel2LvlPedState handle)
{
    assert(handle);
    const auto state = reinterpret_cast<const SocialForceModel2LvlPedData*>(handle);
    return state->bodyForce;
}

JUPEDSIM_API void
JPS_SocialForceModel2LvlPedState_SetBodyForce(JPS_SocialForceModel2LvlPedState handle, double bodyForce)
{
    assert(handle);
    const auto state = reinterpret_cast<SocialForceModel2LvlPedData*>(handle);
    state->bodyForce = bodyForce;
}

JUPEDSIM_API double
JPS_SocialForceModel2LvlPedState_GetFriction(JPS_SocialForceModel2LvlPedState handle)
{
    assert(handle);
    const auto state = reinterpret_cast<const SocialForceModel2LvlPedData*>(handle);
    return state->friction;
}

JUPEDSIM_API void
JPS_SocialForceModel2LvlPedState_SetFriction(JPS_SocialForceModel2LvlPedState handle, double friction)
{
    assert(handle);
    const auto state = reinterpret_cast<SocialForceModel2LvlPedData*>(handle);
    state->friction = friction;
}
