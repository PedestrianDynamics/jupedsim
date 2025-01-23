// SPDX-License-Identifier: LGPL-3.0-or-later
#include "jupedsim/anticipation_velocity_model.h"
#include "jupedsim/error.h"

#include "Conversion.hpp"
#include "ErrorMessage.hpp"

#include <AnticipationVelocityModel.hpp>
#include <AnticipationVelocityModelBuilder.hpp>
#include <AnticipationVelocityModelData.hpp>

#include <stdint.h>

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Anticipation Velocity Model Builder
////////////////////////////////////////////////////////////////////////////////////////////////////
JUPEDSIM_API JPS_AnticipationVelocityModelBuilder
JPS_AnticipationVelocityModelBuilder_Create(double pushoutStrength, uint64_t rng_seed)
{
    return reinterpret_cast<JPS_AnticipationVelocityModelBuilder>(
        new AnticipationVelocityModelBuilder(pushoutStrength, rng_seed));
}

JUPEDSIM_API JPS_OperationalModel JPS_AnticipationVelocityModelBuilder_Build(
    JPS_AnticipationVelocityModelBuilder handle,
    JPS_ErrorMessage* errorMessage)
{
    assert(handle != nullptr);
    auto builder = reinterpret_cast<AnticipationVelocityModelBuilder*>(handle);
    JPS_OperationalModel result{};
    try {
        result =
            reinterpret_cast<JPS_OperationalModel>(new AnticipationVelocityModel(builder->Build()));
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

JUPEDSIM_API void
JPS_AnticipationVelocityModelBuilder_Free(JPS_AnticipationVelocityModelBuilder handle)
{
    delete reinterpret_cast<AnticipationVelocityModelBuilder*>(handle);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// AnticipationVelocityModelState
////////////////////////////////////////////////////////////////////////////////////////////////////
double JPS_AnticipationVelocityModelState_GetStrengthNeighborRepulsion(
    JPS_AnticipationVelocityModelState handle)
{
    assert(handle);
    const auto state = reinterpret_cast<const AnticipationVelocityModelData*>(handle);
    return state->strengthNeighborRepulsion;
}

void JPS_AnticipationVelocityModelState_SetStrengthNeighborRepulsion(
    JPS_AnticipationVelocityModelState handle,
    double strengthNeighborRepulsion)
{
    assert(handle);
    auto state = reinterpret_cast<AnticipationVelocityModelData*>(handle);
    state->strengthNeighborRepulsion = strengthNeighborRepulsion;
}

double JPS_AnticipationVelocityModelState_GetRangeNeighborRepulsion(
    JPS_AnticipationVelocityModelState handle)
{
    assert(handle);
    const auto state = reinterpret_cast<const AnticipationVelocityModelData*>(handle);
    return state->rangeNeighborRepulsion;
}

void JPS_AnticipationVelocityModelState_SetRangeNeighborRepulsion(
    JPS_AnticipationVelocityModelState handle,
    double rangeNeighborRepulsion)
{
    assert(handle);
    auto state = reinterpret_cast<AnticipationVelocityModelData*>(handle);
    state->rangeNeighborRepulsion = rangeNeighborRepulsion;
}

double
JPS_AnticipationVelocityModelState_GetAnticipationTime(JPS_AnticipationVelocityModelState handle)
{
    assert(handle);
    const auto state = reinterpret_cast<const AnticipationVelocityModelData*>(handle);
    return state->anticipationTime;
}

void JPS_AnticipationVelocityModelState_SetAnticipationTime(
    JPS_AnticipationVelocityModelState handle,
    double anticipationTime)
{
    assert(handle);
    auto state = reinterpret_cast<AnticipationVelocityModelData*>(handle);
    state->anticipationTime = anticipationTime;
}

double JPS_AnticipationVelocityModelState_GetReactionTime(JPS_AnticipationVelocityModelState handle)
{
    assert(handle);
    const auto state = reinterpret_cast<const AnticipationVelocityModelData*>(handle);
    return state->reactionTime;
}

void JPS_AnticipationVelocityModelState_SetReactionTime(
    JPS_AnticipationVelocityModelState handle,
    double reactionTime)
{
    assert(handle);
    auto state = reinterpret_cast<AnticipationVelocityModelData*>(handle);
    state->reactionTime = reactionTime;
}

double
JPS_AnticipationVelocityModelState_GetWallBufferDistance(JPS_AnticipationVelocityModelState handle)
{
    assert(handle);
    const auto state = reinterpret_cast<const AnticipationVelocityModelData*>(handle);
    return state->wallBufferDistance;
}

void JPS_AnticipationVelocityModelState_SetWallBufferDistance(
    JPS_AnticipationVelocityModelState handle,
    double wallBufferDistance)
{
    assert(handle);
    auto state = reinterpret_cast<AnticipationVelocityModelData*>(handle);
    state->wallBufferDistance = wallBufferDistance;
}

double JPS_AnticipationVelocityModelState_GetTimeGap(JPS_AnticipationVelocityModelState handle)
{
    assert(handle);
    const auto state = reinterpret_cast<const AnticipationVelocityModelData*>(handle);
    return state->timeGap;
}

void JPS_AnticipationVelocityModelState_SetTimeGap(
    JPS_AnticipationVelocityModelState handle,
    double time_gap)
{
    assert(handle);
    auto state = reinterpret_cast<AnticipationVelocityModelData*>(handle);
    state->timeGap = time_gap;
}

double JPS_AnticipationVelocityModelState_GetV0(JPS_AnticipationVelocityModelState handle)
{
    assert(handle);
    const auto state = reinterpret_cast<const AnticipationVelocityModelData*>(handle);
    return state->v0;
}

void JPS_AnticipationVelocityModelState_SetV0(JPS_AnticipationVelocityModelState handle, double v0)
{
    assert(handle);
    auto state = reinterpret_cast<AnticipationVelocityModelData*>(handle);
    state->v0 = v0;
}
double JPS_AnticipationVelocityModelState_GetRadius(JPS_AnticipationVelocityModelState handle)
{
    assert(handle);
    const auto state = reinterpret_cast<const AnticipationVelocityModelData*>(handle);
    return state->radius;
}

void JPS_AnticipationVelocityModelState_SetRadius(
    JPS_AnticipationVelocityModelState handle,
    double radius)
{
    assert(handle);
    auto state = reinterpret_cast<AnticipationVelocityModelData*>(handle);
    state->radius = radius;
}
