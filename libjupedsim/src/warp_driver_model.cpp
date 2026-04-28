// SPDX-License-Identifier: LGPL-3.0-or-later
#include "jupedsim/warp_driver_model.h"
#include "jupedsim/error.h"

#include "Conversion.hpp"
#include "ErrorMessage.hpp"

#include <WarpDriverModel.hpp>
#include <WarpDriverModelBuilder.hpp>
#include <WarpDriverModelData.hpp>

using jupedsim::detail::intoJPS_Point;
using jupedsim::detail::intoPoint;
using jupedsim::detail::intoTuple;

////////////////////////////////////////////////////////////////////////////////////////////////////
/// WarpDriverModel Model Builder
////////////////////////////////////////////////////////////////////////////////////////////////////
JUPEDSIM_API JPS_WarpDriverModelBuilder
JPS_WarpDriverModelBuilder_Create(double timeHorizon,
        double stepSize,
        double sigma,
        double timeUncertainty,
        double velocityUncertaintyX,
        double velocityUncertaintyY)
{
    return reinterpret_cast<JPS_WarpDriverModelBuilder>(
        new WarpDriverModelBuilder(timeHorizon, stepSize, sigma, timeUncertainty, velocityUncertaintyX, velocityUncertaintyY));
}

JUPEDSIM_API JPS_OperationalModel JPS_WarpDriverModelBuilder_Build(
    JPS_WarpDriverModelBuilder handle,
    JPS_ErrorMessage* errorMessage)
{
    assert(handle != nullptr);
    auto builder = reinterpret_cast<WarpDriverModelBuilder*>(handle);
    JPS_OperationalModel result{};
    try {
        result = reinterpret_cast<JPS_OperationalModel>(new WarpDriverModel(builder->Build()));
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

JUPEDSIM_API void JPS_WarpDriverModelBuilder_Free(JPS_WarpDriverModelBuilder handle)
{
    delete reinterpret_cast<WarpDriverModelBuilder*>(handle);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// WarpDriverModelState
////////////////////////////////////////////////////////////////////////////////////////////////////
JUPEDSIM_API double JPS_WarpDriverModelState_GetRadius(JPS_WarpDriverModelState handle)
{
    assert(handle);
    const auto state = reinterpret_cast<const WarpDriverModelData*>(handle);
    return state->radius;
}

JUPEDSIM_API void
JPS_WarpDriverModelState_SetRadius(JPS_WarpDriverModelState handle, double radius)
{
    assert(handle);
    const auto state = reinterpret_cast<WarpDriverModelData*>(handle);
    state->radius = radius;
}

JUPEDSIM_API double JPS_WarpDriverModelState_GetV0(JPS_WarpDriverModelState handle)
{
    assert(handle);
    const auto state = reinterpret_cast<const WarpDriverModelData*>(handle);
    return state->v0;
}

JUPEDSIM_API void JPS_WarpDriverModelState_SetV0(JPS_WarpDriverModelState handle, double v0)
{
    assert(handle);
    const auto state = reinterpret_cast<WarpDriverModelData*>(handle);
    state->v0 = v0;
}
