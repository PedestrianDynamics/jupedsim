// SPDX-License-Identifier: LGPL-3.0-or-later
#include "jupedsim/collision_free_speed_model.h"
#include "jupedsim/error.h"

#include "Conversion.hpp"
#include "ErrorMessage.hpp"

#include <CollisionFreeSpeedModelV2.hpp>
#include <CollisionFreeSpeedModelV2Builder.hpp>
#include <CollisionFreeSpeedModelV2Data.hpp>

using jupedsim::detail::intoJPS_Point;
using jupedsim::detail::intoPoint;
using jupedsim::detail::intoTuple;

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Collision Free Speed Model V2 Builder
////////////////////////////////////////////////////////////////////////////////////////////////////
JUPEDSIM_API JPS_CollisionFreeSpeedModelV2Builder JPS_CollisionFreeSpeedModelV2Builder_Create()
{
    return reinterpret_cast<JPS_CollisionFreeSpeedModelV2Builder>(
        new CollisionFreeSpeedModelV2Builder());
}

JUPEDSIM_API JPS_OperationalModel JPS_CollisionFreeSpeedModelV2Builder_Build(
    JPS_CollisionFreeSpeedModelV2Builder handle,
    JPS_ErrorMessage* errorMessage)
{
    assert(handle != nullptr);
    auto builder = reinterpret_cast<CollisionFreeSpeedModelV2Builder*>(handle);
    JPS_OperationalModel result{};
    try {
        result =
            reinterpret_cast<JPS_OperationalModel>(new CollisionFreeSpeedModelV2(builder->Build()));
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
JPS_CollisionFreeSpeedModelV2Builder_Free(JPS_CollisionFreeSpeedModelV2Builder handle)
{
    delete reinterpret_cast<CollisionFreeSpeedModelV2Builder*>(handle);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// CollisionFreeSpeedModelV2State
////////////////////////////////////////////////////////////////////////////////////////////////////
double JPS_CollisionFreeSpeedModelV2State_GetStrengthNeighborRepulsion(
    JPS_CollisionFreeSpeedModelV2State handle)
{
    assert(handle);
    const auto state = reinterpret_cast<const CollisionFreeSpeedModelV2Data*>(handle);
    return state->strengthNeighborRepulsion;
}

void JPS_CollisionFreeSpeedModelV2State_SetStrengthNeighborRepulsion(
    JPS_CollisionFreeSpeedModelV2State handle,
    double strengthNeighborRepulsion)
{
    assert(handle);
    auto state = reinterpret_cast<CollisionFreeSpeedModelV2Data*>(handle);
    state->strengthNeighborRepulsion = strengthNeighborRepulsion;
}

double JPS_CollisionFreeSpeedModelV2State_GetRangeNeighborRepulsion(
    JPS_CollisionFreeSpeedModelV2State handle)
{
    assert(handle);
    const auto state = reinterpret_cast<const CollisionFreeSpeedModelV2Data*>(handle);
    return state->rangeNeighborRepulsion;
}

void JPS_CollisionFreeSpeedModelV2State_SetRangeNeighborRepulsion(
    JPS_CollisionFreeSpeedModelV2State handle,
    double rangeNeighborRepulsion)
{
    assert(handle);
    auto state = reinterpret_cast<CollisionFreeSpeedModelV2Data*>(handle);
    state->rangeNeighborRepulsion = rangeNeighborRepulsion;
}

double JPS_CollisionFreeSpeedModelV2State_GetStrengthGeometryRepulsion(
    JPS_CollisionFreeSpeedModelV2State handle)
{
    assert(handle);
    const auto state = reinterpret_cast<const CollisionFreeSpeedModelV2Data*>(handle);
    return state->strengthGeometryRepulsion;
}

void JPS_CollisionFreeSpeedModelV2State_SetStrengthGeometryRepulsion(
    JPS_CollisionFreeSpeedModelV2State handle,
    double strengthGeometryRepulsion)
{
    assert(handle);
    auto state = reinterpret_cast<CollisionFreeSpeedModelV2Data*>(handle);
    state->strengthGeometryRepulsion = strengthGeometryRepulsion;
}

double JPS_CollisionFreeSpeedModelV2State_GetRangeGeometryRepulsion(
    JPS_CollisionFreeSpeedModelV2State handle)
{
    assert(handle);
    const auto state = reinterpret_cast<const CollisionFreeSpeedModelV2Data*>(handle);
    return state->rangeNeighborRepulsion;
}

void JPS_CollisionFreeSpeedModelV2State_SetRangeGeometryRepulsion(
    JPS_CollisionFreeSpeedModelV2State handle,
    double rangeNeighborRepulsion)
{
    assert(handle);
    auto state = reinterpret_cast<CollisionFreeSpeedModelV2Data*>(handle);
    state->rangeNeighborRepulsion = rangeNeighborRepulsion;
}

double JPS_CollisionFreeSpeedModelV2State_GetTimeGap(JPS_CollisionFreeSpeedModelV2State handle)
{
    assert(handle);
    const auto state = reinterpret_cast<const CollisionFreeSpeedModelV2Data*>(handle);
    return state->timeGap;
}

void JPS_CollisionFreeSpeedModelV2State_SetTimeGap(
    JPS_CollisionFreeSpeedModelV2State handle,
    double time_gap)
{
    assert(handle);
    auto state = reinterpret_cast<CollisionFreeSpeedModelV2Data*>(handle);
    state->timeGap = time_gap;
}

double JPS_CollisionFreeSpeedModelV2State_GetV0(JPS_CollisionFreeSpeedModelV2State handle)
{
    assert(handle);
    const auto state = reinterpret_cast<const CollisionFreeSpeedModelV2Data*>(handle);
    return state->v0;
}

void JPS_CollisionFreeSpeedModelV2State_SetV0(JPS_CollisionFreeSpeedModelV2State handle, double v0)
{
    assert(handle);
    auto state = reinterpret_cast<CollisionFreeSpeedModelV2Data*>(handle);
    state->v0 = v0;
}
double JPS_CollisionFreeSpeedModelV2State_GetRadius(JPS_CollisionFreeSpeedModelV2State handle)
{
    assert(handle);
    const auto state = reinterpret_cast<const CollisionFreeSpeedModelV2Data*>(handle);
    return state->radius;
}

void JPS_CollisionFreeSpeedModelV2State_SetRadius(
    JPS_CollisionFreeSpeedModelV2State handle,
    double radius)
{
    assert(handle);
    auto state = reinterpret_cast<CollisionFreeSpeedModelV2Data*>(handle);
    state->radius = radius;
}
