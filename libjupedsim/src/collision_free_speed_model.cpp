// SPDX-License-Identifier: LGPL-3.0-or-later
#include "jupedsim/collision_free_speed_model.h"
#include "jupedsim/error.h"

#include "Conversion.hpp"
#include "ErrorMessage.hpp"

#include <CollisionFreeSpeedModel.hpp>
#include <CollisionFreeSpeedModelBuilder.hpp>
#include <CollisionFreeSpeedModelData.hpp>

using jupedsim::detail::intoJPS_Point;
using jupedsim::detail::intoPoint;
using jupedsim::detail::intoTuple;

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Collision Free Speed Model Builder
////////////////////////////////////////////////////////////////////////////////////////////////////
JUPEDSIM_API JPS_CollisionFreeSpeedModelBuilder JPS_CollisionFreeSpeedModelBuilder_Create(
    double strengthNeighborRepulsion,
    double rangeNeighborRepulsion,
    double strengthGeometryRepulsion,
    double rangeGeometryRepulsion)
{
    return reinterpret_cast<JPS_CollisionFreeSpeedModelBuilder>(new CollisionFreeSpeedModelBuilder(
        strengthNeighborRepulsion,
        rangeNeighborRepulsion,
        strengthGeometryRepulsion,
        rangeGeometryRepulsion));
}

JUPEDSIM_API JPS_OperationalModel JPS_CollisionFreeSpeedModelBuilder_Build(
    JPS_CollisionFreeSpeedModelBuilder handle,
    JPS_ErrorMessage* errorMessage)
{
    assert(handle != nullptr);
    auto builder = reinterpret_cast<CollisionFreeSpeedModelBuilder*>(handle);
    JPS_OperationalModel result{};
    try {
        result =
            reinterpret_cast<JPS_OperationalModel>(new CollisionFreeSpeedModel(builder->Build()));
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

JUPEDSIM_API void JPS_CollisionFreeSpeedModelBuilder_Free(JPS_CollisionFreeSpeedModelBuilder handle)
{
    delete reinterpret_cast<CollisionFreeSpeedModelBuilder*>(handle);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// CollisionFreeSpeedModelState
////////////////////////////////////////////////////////////////////////////////////////////////////
double JPS_CollisionFreeSpeedModelState_GetTimeGap(JPS_CollisionFreeSpeedModelState handle)
{
    assert(handle);
    const auto state = reinterpret_cast<const CollisionFreeSpeedModelData*>(handle);
    return state->timeGap;
}

void JPS_CollisionFreeSpeedModelState_SetTimeGap(
    JPS_CollisionFreeSpeedModelState handle,
    double time_gap)
{
    assert(handle);
    auto state = reinterpret_cast<CollisionFreeSpeedModelData*>(handle);
    state->timeGap = time_gap;
}

double JPS_CollisionFreeSpeedModelState_GetV0(JPS_CollisionFreeSpeedModelState handle)
{
    assert(handle);
    const auto state = reinterpret_cast<const CollisionFreeSpeedModelData*>(handle);
    return state->v0;
}

void JPS_CollisionFreeSpeedModelState_SetV0(JPS_CollisionFreeSpeedModelState handle, double v0)
{
    assert(handle);
    auto state = reinterpret_cast<CollisionFreeSpeedModelData*>(handle);
    state->v0 = v0;
}
double JPS_CollisionFreeSpeedModelState_GetRadius(JPS_CollisionFreeSpeedModelState handle)
{
    assert(handle);
    const auto state = reinterpret_cast<const CollisionFreeSpeedModelData*>(handle);
    return state->radius;
}

void JPS_CollisionFreeSpeedModelState_SetRadius(
    JPS_CollisionFreeSpeedModelState handle,
    double radius)
{
    assert(handle);
    auto state = reinterpret_cast<CollisionFreeSpeedModelData*>(handle);
    state->radius = radius;
}
