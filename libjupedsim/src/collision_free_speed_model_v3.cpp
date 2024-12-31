// Copyright © 2012-2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "jupedsim/collision_free_speed_model.h"
#include "jupedsim/error.h"

#include "Conversion.hpp"
#include "ErrorMessage.hpp"

#include <CollisionFreeSpeedModelV3.hpp>
#include <CollisionFreeSpeedModelV3Builder.hpp>
#include <CollisionFreeSpeedModelV3Data.hpp>

using jupedsim::detail::intoJPS_Point;
using jupedsim::detail::intoPoint;
using jupedsim::detail::intoTuple;

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Collision Free Speed Model V3 Builder
////////////////////////////////////////////////////////////////////////////////////////////////////
JUPEDSIM_API JPS_CollisionFreeSpeedModelV3Builder JPS_CollisionFreeSpeedModelV3Builder_Create()
{
    return reinterpret_cast<JPS_CollisionFreeSpeedModelV3Builder>(
        new CollisionFreeSpeedModelV3Builder());
}

JUPEDSIM_API JPS_OperationalModel JPS_CollisionFreeSpeedModelV3Builder_Build(
    JPS_CollisionFreeSpeedModelV3Builder handle,
    JPS_ErrorMessage* errorMessage)
{
    assert(handle != nullptr);
    auto builder = reinterpret_cast<CollisionFreeSpeedModelV3Builder*>(handle);
    JPS_OperationalModel result{};
    try {
        result =
            reinterpret_cast<JPS_OperationalModel>(new CollisionFreeSpeedModelV3(builder->Build()));
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
JPS_CollisionFreeSpeedModelV3Builder_Free(JPS_CollisionFreeSpeedModelV3Builder handle)
{
    delete reinterpret_cast<CollisionFreeSpeedModelV3Builder*>(handle);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// CollisionFreeSpeedModelV3State
////////////////////////////////////////////////////////////////////////////////////////////////////
double JPS_CollisionFreeSpeedModelV3State_GetStrengthNeighborRepulsion(
    JPS_CollisionFreeSpeedModelV3State handle)
{
    assert(handle);
    const auto state = reinterpret_cast<const CollisionFreeSpeedModelV3Data*>(handle);
    return state->strengthNeighborRepulsion;
}

void JPS_CollisionFreeSpeedModelV3State_SetStrengthNeighborRepulsion(
    JPS_CollisionFreeSpeedModelV3State handle,
    double strengthNeighborRepulsion)
{
    assert(handle);
    auto state = reinterpret_cast<CollisionFreeSpeedModelV3Data*>(handle);
    state->strengthNeighborRepulsion = strengthNeighborRepulsion;
}

double JPS_CollisionFreeSpeedModelV3State_GetRangeNeighborRepulsion(
    JPS_CollisionFreeSpeedModelV3State handle)
{
    assert(handle);
    const auto state = reinterpret_cast<const CollisionFreeSpeedModelV3Data*>(handle);
    return state->rangeNeighborRepulsion;
}

void JPS_CollisionFreeSpeedModelV3State_SetRangeNeighborRepulsion(
    JPS_CollisionFreeSpeedModelV3State handle,
    double rangeNeighborRepulsion)
{
    assert(handle);
    auto state = reinterpret_cast<CollisionFreeSpeedModelV3Data*>(handle);
    state->rangeNeighborRepulsion = rangeNeighborRepulsion;
}

double JPS_CollisionFreeSpeedModelV3State_GetStrengthGeometryRepulsion(
    JPS_CollisionFreeSpeedModelV3State handle)
{
    assert(handle);
    const auto state = reinterpret_cast<const CollisionFreeSpeedModelV3Data*>(handle);
    return state->strengthGeometryRepulsion;
}

void JPS_CollisionFreeSpeedModelV3State_SetStrengthGeometryRepulsion(
    JPS_CollisionFreeSpeedModelV3State handle,
    double strengthGeometryRepulsion)
{
    assert(handle);
    auto state = reinterpret_cast<CollisionFreeSpeedModelV3Data*>(handle);
    state->strengthGeometryRepulsion = strengthGeometryRepulsion;
}

double JPS_CollisionFreeSpeedModelV3State_GetRangeGeometryRepulsion(
    JPS_CollisionFreeSpeedModelV3State handle)
{
    assert(handle);
    const auto state = reinterpret_cast<const CollisionFreeSpeedModelV3Data*>(handle);
    return state->rangeNeighborRepulsion;
}

void JPS_CollisionFreeSpeedModelV3State_SetRangeGeometryRepulsion(
    JPS_CollisionFreeSpeedModelV3State handle,
    double rangeNeighborRepulsion)
{
    assert(handle);
    auto state = reinterpret_cast<CollisionFreeSpeedModelV3Data*>(handle);
    state->rangeNeighborRepulsion = rangeNeighborRepulsion;
}

double JPS_CollisionFreeSpeedModelV3State_GetTimeGap(JPS_CollisionFreeSpeedModelV3State handle)
{
    assert(handle);
    const auto state = reinterpret_cast<const CollisionFreeSpeedModelV3Data*>(handle);
    return state->timeGap;
}

void JPS_CollisionFreeSpeedModelV3State_SetTimeGap(
    JPS_CollisionFreeSpeedModelV3State handle,
    double time_gap)
{
    assert(handle);
    auto state = reinterpret_cast<CollisionFreeSpeedModelV3Data*>(handle);
    state->timeGap = time_gap;
}

double JPS_CollisionFreeSpeedModelV3State_GetV0(JPS_CollisionFreeSpeedModelV3State handle)
{
    assert(handle);
    const auto state = reinterpret_cast<const CollisionFreeSpeedModelV3Data*>(handle);
    return state->v0;
}

void JPS_CollisionFreeSpeedModelV3State_SetV0(JPS_CollisionFreeSpeedModelV3State handle, double v0)
{
    assert(handle);
    auto state = reinterpret_cast<CollisionFreeSpeedModelV3Data*>(handle);
    state->v0 = v0;
}
double JPS_CollisionFreeSpeedModelV3State_GetRadius(JPS_CollisionFreeSpeedModelV3State handle)
{
    assert(handle);
    const auto state = reinterpret_cast<const CollisionFreeSpeedModelV3Data*>(handle);
    return state->radius;
}

void JPS_CollisionFreeSpeedModelV3State_SetRadius(
    JPS_CollisionFreeSpeedModelV3State handle,
    double radius)
{
    assert(handle);
    auto state = reinterpret_cast<CollisionFreeSpeedModelV3Data*>(handle);
    state->radius = radius;
}
