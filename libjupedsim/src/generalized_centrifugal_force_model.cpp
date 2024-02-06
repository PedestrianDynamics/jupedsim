// Copyright © 2012-2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "jupedsim/generalized_centrifugal_force_model.h"
#include "jupedsim/error.h"

#include "Conversion.hpp"
#include "ErrorMessage.hpp"

#include <GeneralizedCentrifugalForceModel.hpp>
#include <GeneralizedCentrifugalForceModelBuilder.hpp>
#include <GeneralizedCentrifugalForceModelData.hpp>

using jupedsim::detail::intoJPS_Point;
using jupedsim::detail::intoPoint;
using jupedsim::detail::intoTuple;

////////////////////////////////////////////////////////////////////////////////////////////////////
/// GeneralizedCentrifugalForceModel Model Builder
////////////////////////////////////////////////////////////////////////////////////////////////////
JPS_GeneralizedCentrifugalForceModelBuilder JPS_GeneralizedCentrifugalForceModelBuilder_Create(
    double strengthNeighborRepulsion,
    double strengthGeometryRepulsion,
    double maxNeighborInteractionDistance,
    double maxGeometryInteractionDistance,
    double maxNeighborInterpolationDistance,
    double maxGeometryInterpolationDistance,
    double maxNeighborRepulsionForce,
    double maxGeometryRepulsionForce)
{
    return reinterpret_cast<JPS_GeneralizedCentrifugalForceModelBuilder>(
        new GeneralizedCentrifugalForceModelBuilder(
            strengthNeighborRepulsion,
            strengthGeometryRepulsion,
            maxNeighborInteractionDistance,
            maxGeometryInteractionDistance,
            maxNeighborInterpolationDistance,
            maxGeometryInterpolationDistance,
            maxNeighborRepulsionForce,
            maxGeometryRepulsionForce));
}

JPS_OperationalModel JPS_GeneralizedCentrifugalForceModelBuilder_Build(
    JPS_GeneralizedCentrifugalForceModelBuilder handle,
    JPS_ErrorMessage* errorMessage)
{
    assert(handle != nullptr);
    auto builder = reinterpret_cast<GeneralizedCentrifugalForceModelBuilder*>(handle);
    JPS_OperationalModel result{};
    try {
        result = reinterpret_cast<JPS_OperationalModel>(
            new GeneralizedCentrifugalForceModel(builder->Build()));
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

void JPS_GeneralizedCentrifugalForceModelBuilder_Free(
    JPS_GeneralizedCentrifugalForceModelBuilder handle)
{
    delete reinterpret_cast<GeneralizedCentrifugalForceModelBuilder*>(handle);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// GeneralizedCentrifugalForceModelState
////////////////////////////////////////////////////////////////////////////////////////////////////
double
JPS_GeneralizedCentrifugalForceModelState_GetSpeed(JPS_GeneralizedCentrifugalForceModelState handle)
{
    assert(handle);
    const auto state = reinterpret_cast<const GeneralizedCentrifugalForceModelData*>(handle);
    return state->speed;
}
void JPS_GeneralizedCentrifugalForceModelState_SetSpeed(
    JPS_GeneralizedCentrifugalForceModelState handle,
    double speed)
{
    assert(handle);
    const auto state = reinterpret_cast<GeneralizedCentrifugalForceModelData*>(handle);
    state->speed = speed;
}

JPS_Point
JPS_GeneralizedCentrifugalForceModelState_GetE0(JPS_GeneralizedCentrifugalForceModelState handle)
{
    assert(handle);
    const auto state = reinterpret_cast<const GeneralizedCentrifugalForceModelData*>(handle);
    return intoJPS_Point(state->e0);
}

void JPS_GeneralizedCentrifugalForceModelState_SetE0(
    JPS_GeneralizedCentrifugalForceModelState handle,
    JPS_Point e0)
{
    assert(handle);
    const auto state = reinterpret_cast<GeneralizedCentrifugalForceModelData*>(handle);
    state->e0 = intoPoint(e0);
}

double
JPS_GeneralizedCentrifugalForceModelState_GetMass(JPS_GeneralizedCentrifugalForceModelState handle)
{
    assert(handle);
    const auto state = reinterpret_cast<const GeneralizedCentrifugalForceModelData*>(handle);
    return state->mass;
}

void JPS_GeneralizedCentrifugalForceModelState_SetMass(
    JPS_GeneralizedCentrifugalForceModelState handle,
    double mass)
{
    assert(handle);
    const auto state = reinterpret_cast<GeneralizedCentrifugalForceModelData*>(handle);
    state->mass = mass;
}

double
JPS_GeneralizedCentrifugalForceModelState_GetTau(JPS_GeneralizedCentrifugalForceModelState handle)
{
    assert(handle);
    const auto state = reinterpret_cast<const GeneralizedCentrifugalForceModelData*>(handle);
    return state->tau;
}

void JPS_GeneralizedCentrifugalForceModelState_SetTau(
    JPS_GeneralizedCentrifugalForceModelState handle,
    double tau)
{
    assert(handle);
    const auto state = reinterpret_cast<GeneralizedCentrifugalForceModelData*>(handle);
    state->tau = tau;
}

double
JPS_GeneralizedCentrifugalForceModelState_GetV0(JPS_GeneralizedCentrifugalForceModelState handle)
{
    assert(handle);
    const auto state = reinterpret_cast<const GeneralizedCentrifugalForceModelData*>(handle);
    return state->v0;
}

void JPS_GeneralizedCentrifugalForceModelState_SetV0(
    JPS_GeneralizedCentrifugalForceModelState handle,
    double v0)
{
    assert(handle);
    const auto state = reinterpret_cast<GeneralizedCentrifugalForceModelData*>(handle);
    state->v0 = v0;
}

double
JPS_GeneralizedCentrifugalForceModelState_GetAV(JPS_GeneralizedCentrifugalForceModelState handle)
{
    assert(handle);
    const auto state = reinterpret_cast<const GeneralizedCentrifugalForceModelData*>(handle);
    return state->Av;
}

void JPS_GeneralizedCentrifugalForceModelState_SetAV(
    JPS_GeneralizedCentrifugalForceModelState handle,
    double a_v)
{
    assert(handle);
    const auto state = reinterpret_cast<GeneralizedCentrifugalForceModelData*>(handle);
    state->Av = a_v;
}

double
JPS_GeneralizedCentrifugalForceModelState_GetAMin(JPS_GeneralizedCentrifugalForceModelState handle)
{
    assert(handle);
    const auto state = reinterpret_cast<const GeneralizedCentrifugalForceModelData*>(handle);
    return state->AMin;
}

void JPS_GeneralizedCentrifugalForceModelState_SetAMin(
    JPS_GeneralizedCentrifugalForceModelState handle,
    double a_min)
{
    assert(handle);
    const auto state = reinterpret_cast<GeneralizedCentrifugalForceModelData*>(handle);
    state->AMin = a_min;
}

double
JPS_GeneralizedCentrifugalForceModelState_GetBMin(JPS_GeneralizedCentrifugalForceModelState handle)
{
    assert(handle);
    const auto state = reinterpret_cast<const GeneralizedCentrifugalForceModelData*>(handle);
    return state->BMin;
}

void JPS_GeneralizedCentrifugalForceModelState_SetBMin(
    JPS_GeneralizedCentrifugalForceModelState handle,
    double b_min)
{
    assert(handle);
    const auto state = reinterpret_cast<GeneralizedCentrifugalForceModelData*>(handle);
    state->BMin = b_min;
}

double
JPS_GeneralizedCentrifugalForceModelState_GetBMax(JPS_GeneralizedCentrifugalForceModelState handle)
{
    assert(handle);
    const auto state = reinterpret_cast<const GeneralizedCentrifugalForceModelData*>(handle);
    return state->BMax;
}

void JPS_GeneralizedCentrifugalForceModelState_SetBMax(
    JPS_GeneralizedCentrifugalForceModelState handle,
    double b_max)
{
    assert(handle);
    const auto state = reinterpret_cast<GeneralizedCentrifugalForceModelData*>(handle);
    state->BMax = b_max;
}
