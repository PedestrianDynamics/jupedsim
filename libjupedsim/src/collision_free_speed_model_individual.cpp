// Copyright © 2012-2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "jupedsim/collision_free_speed_model.h"
#include "jupedsim/error.h"

#include "Conversion.hpp"
#include "ErrorMessage.hpp"

#include <CollisionFreeSpeedModelIndividual.hpp>
#include <CollisionFreeSpeedModelIndividualBuilder.hpp>
#include <CollisionFreeSpeedModelIndividualData.hpp>

using jupedsim::detail::intoJPS_Point;
using jupedsim::detail::intoPoint;
using jupedsim::detail::intoTuple;

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Collision Free Speed Model Builder
////////////////////////////////////////////////////////////////////////////////////////////////////
JUPEDSIM_API JPS_CollisionFreeSpeedModelIndividualBuilder
JPS_CollisionFreeSpeedModelIndividualBuilder_Create()
{
    return reinterpret_cast<JPS_CollisionFreeSpeedModelIndividualBuilder>(
        new CollisionFreeSpeedModelIndividualBuilder());
}

JUPEDSIM_API JPS_OperationalModel JPS_CollisionFreeSpeedModelIndividualBuilder_Build(
    JPS_CollisionFreeSpeedModelIndividualBuilder handle,
    JPS_ErrorMessage* errorMessage)
{
    assert(handle != nullptr);
    auto builder = reinterpret_cast<CollisionFreeSpeedModelIndividualBuilder*>(handle);
    JPS_OperationalModel result{};
    try {
        result = reinterpret_cast<JPS_OperationalModel>(
            new CollisionFreeSpeedModelIndividual(builder->Build()));
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

JUPEDSIM_API void JPS_CollisionFreeSpeedModelIndividualBuilder_Free(
    JPS_CollisionFreeSpeedModelIndividualBuilder handle)
{
    delete reinterpret_cast<CollisionFreeSpeedModelIndividualBuilder*>(handle);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// CollisionFreeSpeedModelIndividualState
////////////////////////////////////////////////////////////////////////////////////////////////////
double JPS_CollisionFreeSpeedModelIndividualState_GetStrengthNeighborRepulsion(
    JPS_CollisionFreeSpeedModelIndividualState handle)
{
    assert(handle);
    const auto state = reinterpret_cast<const CollisionFreeSpeedModelIndividualData*>(handle);
    return state->strengthNeighborRepulsion;
}

void JPS_CollisionFreeSpeedModelIndividualState_SetStrengthNeighborRepulsion(
    JPS_CollisionFreeSpeedModelIndividualState handle,
    double strengthNeighborRepulsion)
{
    assert(handle);
    auto state = reinterpret_cast<CollisionFreeSpeedModelIndividualData*>(handle);
    state->strengthGeometryRepulsion = strengthNeighborRepulsion;
}

double JPS_CollisionFreeSpeedModelIndividualState_GetRangeNeighborRepulsion(
    JPS_CollisionFreeSpeedModelIndividualState handle)
{
    assert(handle);
    const auto state = reinterpret_cast<const CollisionFreeSpeedModelIndividualData*>(handle);
    return state->rangeNeighborRepulsion;
}

void JPS_CollisionFreeSpeedModelIndividualState_SetRangeNeighborRepulsion(
    JPS_CollisionFreeSpeedModelIndividualState handle,
    double rangeNeighborRepulsion)
{
    assert(handle);
    auto state = reinterpret_cast<CollisionFreeSpeedModelIndividualData*>(handle);
    state->rangeNeighborRepulsion = rangeNeighborRepulsion;
}

double JPS_CollisionFreeSpeedModelIndividualState_GetStrengthGeometryRepulsion(
    JPS_CollisionFreeSpeedModelIndividualState handle)
{
    assert(handle);
    const auto state = reinterpret_cast<const CollisionFreeSpeedModelIndividualData*>(handle);
    return state->strengthGeometryRepulsion;
}

void JPS_CollisionFreeSpeedModelIndividualState_SetStrengthGeometryRepulsion(
    JPS_CollisionFreeSpeedModelIndividualState handle,
    double strengthGeometryRepulsion)
{
    assert(handle);
    auto state = reinterpret_cast<CollisionFreeSpeedModelIndividualData*>(handle);
    state->strengthGeometryRepulsion = strengthGeometryRepulsion;
}

double JPS_CollisionFreeSpeedModelIndividualState_GetRangeGeometryRepulsion(
    JPS_CollisionFreeSpeedModelIndividualState handle)
{
    assert(handle);
    const auto state = reinterpret_cast<const CollisionFreeSpeedModelIndividualData*>(handle);
    return state->rangeNeighborRepulsion;
}

void JPS_CollisionFreeSpeedModelIndividualState_SetRangeGeometryRepulsion(
    JPS_CollisionFreeSpeedModelIndividualState handle,
    double rangeNeighborRepulsion)
{
    assert(handle);
    auto state = reinterpret_cast<CollisionFreeSpeedModelIndividualData*>(handle);
    state->rangeNeighborRepulsion = rangeNeighborRepulsion;
}

double JPS_CollisionFreeSpeedModelIndividualState_GetTimeGap(
    JPS_CollisionFreeSpeedModelIndividualState handle)
{
    assert(handle);
    const auto state = reinterpret_cast<const CollisionFreeSpeedModelIndividualData*>(handle);
    return state->timeGap;
}

void JPS_CollisionFreeSpeedModelIndividualState_SetTimeGap(
    JPS_CollisionFreeSpeedModelIndividualState handle,
    double time_gap)
{
    assert(handle);
    auto state = reinterpret_cast<CollisionFreeSpeedModelIndividualData*>(handle);
    state->timeGap = time_gap;
}

double
JPS_CollisionFreeSpeedModelIndividualState_GetV0(JPS_CollisionFreeSpeedModelIndividualState handle)
{
    assert(handle);
    const auto state = reinterpret_cast<const CollisionFreeSpeedModelIndividualData*>(handle);
    return state->v0;
}

void JPS_CollisionFreeSpeedModelIndividualState_SetV0(
    JPS_CollisionFreeSpeedModelIndividualState handle,
    double v0)
{
    assert(handle);
    auto state = reinterpret_cast<CollisionFreeSpeedModelIndividualData*>(handle);
    state->v0 = v0;
}
double JPS_CollisionFreeSpeedModelIndividualState_GetRadius(
    JPS_CollisionFreeSpeedModelIndividualState handle)
{
    assert(handle);
    const auto state = reinterpret_cast<const CollisionFreeSpeedModelIndividualData*>(handle);
    return state->radius;
}

void JPS_CollisionFreeSpeedModelIndividualState_SetRadius(
    JPS_CollisionFreeSpeedModelIndividualState handle,
    double radius)
{
    assert(handle);
    auto state = reinterpret_cast<CollisionFreeSpeedModelIndividualData*>(handle);
    state->radius = radius;
}
