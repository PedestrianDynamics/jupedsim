// Copyright © 2012-2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "jupedsim/collision_free_speed_model.h"
#include "jupedsim/error.h"

#include "Conversion.hpp"
#include "ErrorMessage.hpp"

#include <CollisionFreeSpeedModelv2.hpp>
#include <CollisionFreeSpeedModelv2Builder.hpp>
#include <CollisionFreeSpeedModelv2Data.hpp>

using jupedsim::detail::intoJPS_Point;
using jupedsim::detail::intoPoint;
using jupedsim::detail::intoTuple;

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Collision Free Speed Model v2 Builder
////////////////////////////////////////////////////////////////////////////////////////////////////
JUPEDSIM_API JPS_CollisionFreeSpeedModelv2Builder JPS_CollisionFreeSpeedModelv2Builder_Create()
{
    return reinterpret_cast<JPS_CollisionFreeSpeedModelv2Builder>(
        new CollisionFreeSpeedModelv2Builder());
}

JUPEDSIM_API JPS_OperationalModel JPS_CollisionFreeSpeedModelv2Builder_Build(
    JPS_CollisionFreeSpeedModelv2Builder handle,
    JPS_ErrorMessage* errorMessage)
{
    assert(handle != nullptr);
    auto builder = reinterpret_cast<CollisionFreeSpeedModelv2Builder*>(handle);
    JPS_OperationalModel result{};
    try {
        result =
            reinterpret_cast<JPS_OperationalModel>(new CollisionFreeSpeedModelv2(builder->Build()));
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
JPS_CollisionFreeSpeedModelv2Builder_Free(JPS_CollisionFreeSpeedModelv2Builder handle)
{
    delete reinterpret_cast<CollisionFreeSpeedModelv2Builder*>(handle);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// CollisionFreeSpeedModelv2State
////////////////////////////////////////////////////////////////////////////////////////////////////
double JPS_CollisionFreeSpeedModelv2State_GetStrengthNeighborRepulsion(
    JPS_CollisionFreeSpeedModelv2State handle)
{
    assert(handle);
    const auto state = reinterpret_cast<const CollisionFreeSpeedModelv2Data*>(handle);
    return state->strengthNeighborRepulsion;
}

void JPS_CollisionFreeSpeedModelv2State_SetStrengthNeighborRepulsion(
    JPS_CollisionFreeSpeedModelv2State handle,
    double strengthNeighborRepulsion)
{
    assert(handle);
    auto state = reinterpret_cast<CollisionFreeSpeedModelv2Data*>(handle);
    state->strengthGeometryRepulsion = strengthNeighborRepulsion;
}

double JPS_CollisionFreeSpeedModelv2State_GetRangeNeighborRepulsion(
    JPS_CollisionFreeSpeedModelv2State handle)
{
    assert(handle);
    const auto state = reinterpret_cast<const CollisionFreeSpeedModelv2Data*>(handle);
    return state->rangeNeighborRepulsion;
}

void JPS_CollisionFreeSpeedModelv2State_SetRangeNeighborRepulsion(
    JPS_CollisionFreeSpeedModelv2State handle,
    double rangeNeighborRepulsion)
{
    assert(handle);
    auto state = reinterpret_cast<CollisionFreeSpeedModelv2Data*>(handle);
    state->rangeNeighborRepulsion = rangeNeighborRepulsion;
}

double JPS_CollisionFreeSpeedModelv2State_GetStrengthGeometryRepulsion(
    JPS_CollisionFreeSpeedModelv2State handle)
{
    assert(handle);
    const auto state = reinterpret_cast<const CollisionFreeSpeedModelv2Data*>(handle);
    return state->strengthGeometryRepulsion;
}

void JPS_CollisionFreeSpeedModelv2State_SetStrengthGeometryRepulsion(
    JPS_CollisionFreeSpeedModelv2State handle,
    double strengthGeometryRepulsion)
{
    assert(handle);
    auto state = reinterpret_cast<CollisionFreeSpeedModelv2Data*>(handle);
    state->strengthGeometryRepulsion = strengthGeometryRepulsion;
}

double JPS_CollisionFreeSpeedModelv2State_GetRangeGeometryRepulsion(
    JPS_CollisionFreeSpeedModelv2State handle)
{
    assert(handle);
    const auto state = reinterpret_cast<const CollisionFreeSpeedModelv2Data*>(handle);
    return state->rangeNeighborRepulsion;
}

void JPS_CollisionFreeSpeedModelv2State_SetRangeGeometryRepulsion(
    JPS_CollisionFreeSpeedModelv2State handle,
    double rangeNeighborRepulsion)
{
    assert(handle);
    auto state = reinterpret_cast<CollisionFreeSpeedModelv2Data*>(handle);
    state->rangeNeighborRepulsion = rangeNeighborRepulsion;
}

double JPS_CollisionFreeSpeedModelv2State_GetTimeGap(JPS_CollisionFreeSpeedModelv2State handle)
{
    assert(handle);
    const auto state = reinterpret_cast<const CollisionFreeSpeedModelv2Data*>(handle);
    return state->timeGap;
}

void JPS_CollisionFreeSpeedModelv2State_SetTimeGap(
    JPS_CollisionFreeSpeedModelv2State handle,
    double time_gap)
{
    assert(handle);
    auto state = reinterpret_cast<CollisionFreeSpeedModelv2Data*>(handle);
    state->timeGap = time_gap;
}

double JPS_CollisionFreeSpeedModelv2State_GetV0(JPS_CollisionFreeSpeedModelv2State handle)
{
    assert(handle);
    const auto state = reinterpret_cast<const CollisionFreeSpeedModelv2Data*>(handle);
    return state->v0;
}

void JPS_CollisionFreeSpeedModelv2State_SetV0(JPS_CollisionFreeSpeedModelv2State handle, double v0)
{
    assert(handle);
    auto state = reinterpret_cast<CollisionFreeSpeedModelv2Data*>(handle);
    state->v0 = v0;
}
double JPS_CollisionFreeSpeedModelv2State_GetRadius(JPS_CollisionFreeSpeedModelv2State handle)
{
    assert(handle);
    const auto state = reinterpret_cast<const CollisionFreeSpeedModelv2Data*>(handle);
    return state->radius;
}

void JPS_CollisionFreeSpeedModelv2State_SetRadius(
    JPS_CollisionFreeSpeedModelv2State handle,
    double radius)
{
    assert(handle);
    auto state = reinterpret_cast<CollisionFreeSpeedModelv2Data*>(handle);
    state->radius = radius;
}
