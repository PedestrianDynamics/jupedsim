// Copyright © 2012-2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "jupedsim/jupedsim.h"

#include "AgentIterator.hpp"
#include "CollisionFreeSpeedModelData.hpp"
#include "ErrorMessage.hpp"
#include "GeneralizedCentrifugalForceModelData.hpp"
#include "Journey.hpp"
#include "Stage.hpp"

#include <BuildInfo.hpp>
#include <CollisionFreeSpeedModel.hpp>
#include <CollisionFreeSpeedModelBuilder.hpp>
#include <CollisionFreeSpeedModelV2.hpp>
#include <CollisionFreeSpeedModelV2Builder.hpp>
#include <CollisionFreeSpeedModelV2Data.hpp>
#include <CollisionGeometry.hpp>
#include <Conversion.hpp>
#include <GeneralizedCentrifugalForceModel.hpp>
#include <GeneralizedCentrifugalForceModelBuilder.hpp>
#include <GenericAgent.hpp>
#include <Geometry.hpp>
#include <GeometryBuilder.hpp>
#include <GeometrySwitchError.hpp>
#include <Logger.hpp>
#include <OperationalModel.hpp>
#include <OperationalModelType.hpp>
#include <Point.hpp>
#include <RoutingEngine.hpp>
#include <Simulation.hpp>
#include <StageDescription.hpp>
#include <Unreachable.hpp>

#include <algorithm>
#include <cassert>
#include <exception>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>

////////////////////////////////////////////////////////////////////////////////////////////////////
/// BuildInfo
////////////////////////////////////////////////////////////////////////////////////////////////////
JPS_BuildInfo JPS_GetBuildInfo()
{
    return JPS_BuildInfo{
        GIT_COMMIT_HASH.c_str(),
        GIT_COMMIT_DATE.c_str(),
        GIT_BRANCH.c_str(),
        COMPILER.c_str(),
        COMPILER_VERSION.c_str(),
        LIBRARY_VERSION.c_str()};
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Logging
////////////////////////////////////////////////////////////////////////////////////////////////////
JUPEDSIM_API void JPS_Logging_SetDebugCallback(JPS_LoggingCallBack callback, void* userdata)
{
    if(callback) {
        Logging::Logger::Instance().SetDebugCallback(
            [callback, userdata](const std::string& msg) { callback(msg.c_str(), userdata); });
    } else {
        Logging::Logger::Instance().ClearDebugCallback();
    }
}

JUPEDSIM_API void JPS_Logging_SetInfoCallback(JPS_LoggingCallBack callback, void* userdata)
{
    if(callback) {
        Logging::Logger::Instance().SetInfoCallback(
            [callback, userdata](const std::string& msg) { callback(msg.c_str(), userdata); });
    } else {
        Logging::Logger::Instance().ClearInfoCallback();
    }
}

JUPEDSIM_API void JPS_Logging_SetWarningCallback(JPS_LoggingCallBack callback, void* userdata)
{
    if(callback) {
        Logging::Logger::Instance().SetWarningCallback(
            [callback, userdata](const std::string& msg) { callback(msg.c_str(), userdata); });
    } else {
        Logging::Logger::Instance().ClearWarningCallback();
    }
}

JUPEDSIM_API void JPS_Logging_SetErrorCallback(JPS_LoggingCallBack callback, void* userdata)
{
    if(callback) {
        Logging::Logger::Instance().SetErrorCallback(
            [callback, userdata](const std::string& msg) { callback(msg.c_str(), userdata); });
    } else {
        Logging::Logger::Instance().ClearErrorCallback();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// ErrorMessage
////////////////////////////////////////////////////////////////////////////////////////////////////
const char* JPS_ErrorMessage_GetMessage(JPS_ErrorMessage handle)
{
    JPS_ErrorMessage_t* msg = reinterpret_cast<JPS_ErrorMessage_t*>(handle);
    return msg->message.c_str();
}

void JPS_ErrorMessage_Free(JPS_ErrorMessage handle)
{
    delete reinterpret_cast<JPS_ErrorMessage_t*>(handle);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Operational Model
////////////////////////////////////////////////////////////////////////////////////////////////////
void JPS_OperationalModel_Free(JPS_OperationalModel handle)
{
    delete reinterpret_cast<OperationalModel*>(handle);
}

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
/// GeometryBuilder
////////////////////////////////////////////////////////////////////////////////////////////////////

JPS_GeometryBuilder JPS_GeometryBuilder_Create()
{
    return reinterpret_cast<JPS_GeometryBuilder>(new GeometryBuilder{});
}

void JPS_GeometryBuilder_AddAccessibleArea(
    JPS_GeometryBuilder handle,
    const JPS_Point* polygon,
    size_t lenPolygon)
{
    assert(handle != nullptr);
    auto builder = reinterpret_cast<GeometryBuilder*>(handle);
    std::vector<Point> loop{};
    loop.reserve(lenPolygon);
    std::transform(polygon, polygon + lenPolygon, std::back_inserter(loop), intoPoint);
    builder->AddAccessibleArea(loop);
}

void JPS_GeometryBuilder_ExcludeFromAccessibleArea(
    JPS_GeometryBuilder handle,
    const JPS_Point* polygon,
    size_t lenPolygon)
{
    assert(handle != nullptr);
    auto builder = reinterpret_cast<GeometryBuilder*>(handle);
    std::vector<Point> loop{};
    loop.reserve(lenPolygon);
    std::transform(polygon, polygon + lenPolygon, std::back_inserter(loop), intoPoint);
    builder->ExcludeFromAccessibleArea(loop);
}

JPS_Geometry JPS_GeometryBuilder_Build(JPS_GeometryBuilder handle, JPS_ErrorMessage* errorMessage)
{
    assert(handle != nullptr);
    auto builder = reinterpret_cast<GeometryBuilder*>(handle);
    JPS_Geometry result{};
    try {
        result = reinterpret_cast<JPS_Geometry>(new Geometry(builder->Build()));
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

void JPS_GeometryBuilder_Free(JPS_GeometryBuilder handle)
{
    delete reinterpret_cast<GeometryBuilder*>(handle);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Geometry
////////////////////////////////////////////////////////////////////////////////////////////////////
size_t JPS_Geometry_GetBoundarySize(JPS_Geometry handle)
{
    assert(handle);
    const auto geo = reinterpret_cast<Geometry const*>(handle);
    return std::get<0>(geo->collisionGeometry->AccessibleArea()).size();
}

const JPS_Point* JPS_Geometry_GetBoundaryData(JPS_Geometry handle)
{
    assert(handle);
    const auto geo = reinterpret_cast<Geometry const*>(handle);
    return reinterpret_cast<const JPS_Point*>(
        std::get<0>(geo->collisionGeometry->AccessibleArea()).data());
}

size_t JPS_Geometry_GetHoleCount(JPS_Geometry handle)
{
    assert(handle);
    const auto geo = reinterpret_cast<Geometry const*>(handle);
    return std::get<1>(geo->collisionGeometry->AccessibleArea()).size();
}

size_t
JPS_Geometry_GetHoleSize(JPS_Geometry handle, size_t hole_index, JPS_ErrorMessage* errorMessage)
{
    assert(handle);
    const auto geo = reinterpret_cast<Geometry const*>(handle);
    try {
        return std::get<1>(geo->collisionGeometry->AccessibleArea()).at(hole_index).size();
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
    return 0;
}

const JPS_Point*
JPS_Geometry_GetHoleData(JPS_Geometry handle, size_t hole_index, JPS_ErrorMessage* errorMessage)
{
    assert(handle);
    const auto geo = reinterpret_cast<Geometry const*>(handle);
    try {
        return reinterpret_cast<const JPS_Point*>(
            std::get<1>(geo->collisionGeometry->AccessibleArea()).at(hole_index).data());
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
    return nullptr;
}

void JPS_Geometry_Free(JPS_Geometry handle)
{
    delete reinterpret_cast<Geometry const*>(handle);
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
    state->strengthGeometryRepulsion = strengthNeighborRepulsion;
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

////////////////////////////////////////////////////////////////////////////////////////////////////
/// NotifiableQueueProxy
////////////////////////////////////////////////////////////////////////////////////////////////////
size_t JPS_NotifiableQueueProxy_GetCountTargeting(JPS_NotifiableQueueProxy handle)
{
    assert(handle);
    auto proxy = reinterpret_cast<NotifiableQueueProxy*>(handle);
    return proxy->CountTargeting();
}

size_t JPS_NotifiableQueueProxy_GetCountEnqueued(JPS_NotifiableQueueProxy handle)
{
    assert(handle);
    auto proxy = reinterpret_cast<NotifiableQueueProxy*>(handle);
    return proxy->CountEnqueued();
}

void JPS_NotifiableQueueProxy_Pop(JPS_NotifiableQueueProxy handle, size_t count)
{
    assert(handle);
    auto proxy = reinterpret_cast<NotifiableQueueProxy*>(handle);
    proxy->Pop(count);
}

size_t
JPS_NotifiableQueueProxy_GetEnqueued(JPS_NotifiableQueueProxy handle, const JPS_AgentId** data)
{
    assert(handle);
    auto proxy = reinterpret_cast<NotifiableQueueProxy*>(handle);
    const auto& agents = proxy->Enqueued();
    static_assert(
        std::is_same<JPS_AgentId, GenericAgent::ID::underlying_type>::value,
        "GenericAgentIDs cannot be casted in JPS_AgentId");
    *data = reinterpret_cast<const JPS_AgentId*>(agents.data());
    return agents.size();
}

void JPS_NotifiableQueueProxy_Free(JPS_NotifiableQueueProxy handle)
{
    delete reinterpret_cast<NotifiableQueueProxy*>(handle);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// WaitingSetProxy
////////////////////////////////////////////////////////////////////////////////////////////////////
size_t JPS_WaitingSetProxy_GetCountTargeting(JPS_WaitingSetProxy handle)
{
    assert(handle);
    auto proxy = reinterpret_cast<NotifiableWaitingSetProxy*>(handle);
    return proxy->CountTargeting();
}

size_t JPS_WaitingSetProxy_GetCountWaiting(JPS_WaitingSetProxy handle)
{
    assert(handle);
    auto proxy = reinterpret_cast<NotifiableWaitingSetProxy*>(handle);
    return proxy->CountWaiting();
}

size_t JPS_WaitingSetProxy_GetWaiting(JPS_WaitingSetProxy handle, const JPS_AgentId** data)
{
    assert(handle);
    auto proxy = reinterpret_cast<NotifiableWaitingSetProxy*>(handle);
    const auto& agents = proxy->Waiting();
    static_assert(
        std::is_same<JPS_AgentId, GenericAgent::ID::underlying_type>::value,
        "GenericAgentIDs cannot be casted in JPS_AgentId");
    *data = reinterpret_cast<const JPS_AgentId*>(agents.data());
    return agents.size();
}

void JPS_WaitingSetProxy_SetWaitingSetState(
    JPS_WaitingSetProxy handle,
    JPS_WaitingSetState newState)
{
    const auto convert = [](const auto s) {
        switch(s) {
            case JPS_WaitingSet_Active:
                return WaitingSetState::Active;
            case JPS_WaitingSet_Inactive:
                return WaitingSetState::Inactive;
        }
        UNREACHABLE();
    };
    assert(handle);
    auto proxy = reinterpret_cast<NotifiableWaitingSetProxy*>(handle);
    proxy->State(convert(newState));
}

JPS_WaitingSetState JPS_WaitingSetProxy_GetWaitingSetState(JPS_WaitingSetProxy handle)
{
    const auto convert = [](const auto s) {
        switch(s) {
            case WaitingSetState::Active:
                return JPS_WaitingSet_Active;
            case WaitingSetState::Inactive:
                return JPS_WaitingSet_Inactive;
        }
        UNREACHABLE();
    };
    assert(handle);
    auto proxy = reinterpret_cast<NotifiableWaitingSetProxy*>(handle);
    return convert(proxy->State());
}

void JPS_WaitingSetProxy_Free(JPS_WaitingSetProxy handle)
{
    delete reinterpret_cast<NotifiableWaitingSetProxy*>(handle);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// WaypointProxy
////////////////////////////////////////////////////////////////////////////////////////////////////
size_t JPS_WaypointProxy_GetCountTargeting(JPS_WaypointProxy handle)
{
    assert(handle);
    auto proxy = reinterpret_cast<WaypointProxy*>(handle);
    return proxy->CountTargeting();
}

void JPS_WaypointProxy_Free(JPS_WaypointProxy handle)
{
    delete reinterpret_cast<WaypointProxy*>(handle);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// WaitingSetProxy
////////////////////////////////////////////////////////////////////////////////////////////////////
size_t JPS_ExitProxy_GetCountTargeting(JPS_ExitProxy handle)
{
    assert(handle);
    auto proxy = reinterpret_cast<ExitProxy*>(handle);
    return proxy->CountTargeting();
}

void JPS_ExitProxy_Free(JPS_ExitProxy handle)
{
    delete reinterpret_cast<ExitProxy*>(handle);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Agent
////////////////////////////////////////////////////////////////////////////////////////////////////
JPS_AgentId JPS_Agent_GetId(JPS_Agent handle)
{
    assert(handle);
    const auto agent = reinterpret_cast<const GenericAgent*>(handle);
    return agent->id.getID();
}

JPS_JourneyId JPS_Agent_GetJourneyId(JPS_Agent handle)
{
    assert(handle);
    const auto agent = reinterpret_cast<const GenericAgent*>(handle);
    return agent->journeyId.getID();
}

JPS_StageId JPS_Agent_GetStageId(JPS_Agent handle)
{
    assert(handle);
    const auto agent = reinterpret_cast<const GenericAgent*>(handle);
    return agent->stageId.getID();
}

JPS_Point JPS_Agent_GetPosition(JPS_Agent handle)
{
    assert(handle);
    const auto agent = reinterpret_cast<const GenericAgent*>(handle);
    return intoJPS_Point(agent->pos);
}

JPS_Point JPS_Agent_GetOrientation(JPS_Agent handle)
{
    assert(handle);
    const auto agent = reinterpret_cast<const GenericAgent*>(handle);
    return intoJPS_Point(agent->orientation);
}

JPS_ModelType JPS_Agent_GetModelType(JPS_Agent handle)
{
    assert(handle);
    const auto agent = reinterpret_cast<const GenericAgent*>(handle);
    switch(agent->model.index()) {
        case 0:
            return JPS_GeneralizedCentrifugalForceModel;
        case 1:
            return JPS_CollisionFreeSpeedModel;
        case 2:
            return JPS_CollisionFreeSpeedModelV2;
    }
    UNREACHABLE();
}

JPS_GeneralizedCentrifugalForceModelState
JPS_Agent_GetGeneralizedCentrifugalForceModelState(JPS_Agent handle, JPS_ErrorMessage* errorMessage)
{
    assert(handle);
    const auto agent = reinterpret_cast<GenericAgent*>(handle);
    try {
        auto& model = std::get<GeneralizedCentrifugalForceModelData>(agent->model);
        return reinterpret_cast<JPS_GeneralizedCentrifugalForceModelState>(&model);
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
    return nullptr;
}

JPS_CollisionFreeSpeedModelState
JPS_Agent_GetCollisionFreeSpeedModelState(JPS_Agent handle, JPS_ErrorMessage* errorMessage)
{
    assert(handle);
    const auto agent = reinterpret_cast<GenericAgent*>(handle);
    try {
        auto& model = std::get<CollisionFreeSpeedModelData>(agent->model);
        return reinterpret_cast<JPS_CollisionFreeSpeedModelState>(&model);
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
    return nullptr;
}

JPS_CollisionFreeSpeedModelV2State
JPS_Agent_GetCollisionFreeSpeedModelV2State(JPS_Agent handle, JPS_ErrorMessage* errorMessage)
{
    assert(handle);
    const auto agent = reinterpret_cast<GenericAgent*>(handle);
    try {
        auto& model = std::get<CollisionFreeSpeedModelV2Data>(agent->model);
        return reinterpret_cast<JPS_CollisionFreeSpeedModelV2State>(&model);
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
    return nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// AgentIterator
////////////////////////////////////////////////////////////////////////////////////////////////////
JPS_Agent JPS_AgentIterator_Next(JPS_AgentIterator handle)
{
    assert(handle);
    auto iterator = reinterpret_cast<AgentIterator<GenericAgent>*>(handle);
    return reinterpret_cast<JPS_Agent>(iterator->Next());
}

void JPS_AgentIterator_Free(JPS_AgentIterator handle)
{
    delete reinterpret_cast<AgentIterator<GenericAgent>*>(handle);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// AgentIdIterator
////////////////////////////////////////////////////////////////////////////////////////////////////
struct AgentIdIterator {
    using AgentIds = std::vector<GenericAgent::ID>;
    AgentIds ids;
    AgentIds::const_iterator iter;

    AgentIdIterator(AgentIds&& ids_) : ids(std::move(ids_)) { iter = std::begin(ids); }
};

JPS_AgentId JPS_AgentIdIterator_Next(JPS_AgentIdIterator handle)
{
    assert(handle);
    auto iterator = reinterpret_cast<AgentIdIterator*>(handle);
    auto& [vec, iter] = *iterator;
    if(iter == std::end(vec)) {
        return GenericAgent::ID::Invalid.getID();
    }
    const auto id = *iter;
    ++iter;
    return id.getID();
}

void JPS_AgentIdIterator_Free(JPS_AgentIdIterator handle)
{
    delete reinterpret_cast<AgentIdIterator*>(handle);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// JourneyDescription
////////////////////////////////////////////////////////////////////////////////////////////////////
using JourneyDesc = std::map<BaseStage::ID, TransitionDescription>;

JPS_JourneyDescription JPS_JourneyDescription_Create()
{
    return reinterpret_cast<JPS_JourneyDescription>(new JourneyDesc{});
}

void JPS_JourneyDescription_AddStage(JPS_JourneyDescription handle, JPS_StageId id)
{
    assert(handle);
    auto journeyDesc = reinterpret_cast<JourneyDesc*>(handle);
    (*journeyDesc)[BaseStage::ID{id}] = NonTransitionDescription{};
}

bool JPS_JourneyDescription_SetTransitionForStage(
    JPS_JourneyDescription handle,
    JPS_StageId id,
    JPS_Transition transition,
    JPS_ErrorMessage* errorMessage)
{
    assert(handle);
    assert(transition);
    auto journeyDesc = reinterpret_cast<JourneyDesc*>(handle);

    auto iter = journeyDesc->find(BaseStage::ID{id});
    if(iter != std::end(*journeyDesc)) {
        iter->second = *reinterpret_cast<TransitionDescription*>(transition);
        return true;
    }

    if(errorMessage) {
        *errorMessage = reinterpret_cast<JPS_ErrorMessage>(new JPS_ErrorMessage_t{
            fmt::format("Could not set transition for given stage id {}. Stage not found.", id)});
    }
    return false;
}

void JPS_JourneyDescription_Free(JPS_JourneyDescription handle)
{
    delete reinterpret_cast<JourneyDesc*>(handle);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
/// JourneyTransition
////////////////////////////////////////////////////////////////////////////////////////////////////
JPS_Transition
JPS_Transition_CreateFixedTransition(JPS_StageId stageId, JPS_ErrorMessage* errorMessage)
{
    JPS_Transition result{};
    try {
        result = reinterpret_cast<JPS_Transition>(
            new TransitionDescription(FixedTransitionDescription{stageId}));

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

JPS_Transition JPS_Transition_CreateRoundRobinTransition(
    JPS_StageId* stages,
    uint64_t* weights,
    size_t len,
    JPS_ErrorMessage* errorMessage)
{
    JPS_Transition result{};
    std::vector<std::tuple<BaseStage::ID, uint64_t>> stageWeights;
    stageWeights.reserve(len);
    for(size_t i = 0; i < len; ++i) {
        stageWeights.emplace_back(std::make_tuple(stages[i], weights[i]));
    }

    try {
        result = reinterpret_cast<JPS_Transition>(
            new TransitionDescription(RoundRobinTransitionDescription{stageWeights}));
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

JPS_Transition JPS_Transition_CreateLeastTargetedTransition(
    JPS_StageId* stages,
    size_t len,
    JPS_ErrorMessage* errorMessage)
{
    JPS_Transition result{};
    std::vector<BaseStage::ID> stageIds(stages, stages + len);
    try {
        result = reinterpret_cast<JPS_Transition>(
            new TransitionDescription(LeastTargetedTransitionDescription(std::move(stageIds))));
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

void JPS_Transition_Free(JPS_Transition handle)
{
    delete reinterpret_cast<TransitionDescription*>(handle);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Simulation
////////////////////////////////////////////////////////////////////////////////////////////////////
JPS_Simulation JPS_Simulation_Create(
    JPS_OperationalModel model,
    JPS_Geometry geometry,
    double dT,
    JPS_ErrorMessage* errorMessage)
{
    assert(model);
    assert(geometry);
    JPS_Simulation result{};
    try {
        auto geometryInternal = reinterpret_cast<const Geometry*>(geometry);
        auto collisionGeometry =
            std::make_unique<CollisionGeometry>(*geometryInternal->collisionGeometry);
        auto routingEngine = geometryInternal->routingEngine->Clone();

        auto modelInternal = reinterpret_cast<OperationalModel*>(model);
        auto model = modelInternal->Clone();
        result = reinterpret_cast<JPS_Simulation>(new Simulation(
            std::move(model), std::move(collisionGeometry), std::move(routingEngine), dT));
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

JPS_JourneyId JPS_Simulation_AddJourney(
    JPS_Simulation handle,
    JPS_JourneyDescription journey,
    JPS_ErrorMessage* errorMessage)
{
    assert(handle);
    assert(journey);

    auto simulation = reinterpret_cast<Simulation*>(handle);
    auto journeyInternal = reinterpret_cast<JourneyDesc*>(journey);
    auto result = Journey::ID::Invalid.getID();
    try {
        result = simulation->AddJourney(*journeyInternal).getID();
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

static JPS_StageId add_stage(
    JPS_Simulation handle,
    const StageDescription& stageDescription,
    JPS_ErrorMessage* errorMessage)
{
    assert(handle);

    auto simulation = reinterpret_cast<Simulation*>(handle);
    auto result = Journey::ID::Invalid.getID();
    try {

        result = simulation->AddStage(stageDescription).getID();
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

JPS_StageId JPS_Simulation_AddStageWaypoint(
    JPS_Simulation handle,
    JPS_Point position,
    double distance,
    JPS_ErrorMessage* errorMessage)
{
    return add_stage(handle, WaypointDescription{intoPoint(position), distance}, errorMessage);
}

JPS_StageId JPS_Simulation_AddStageExit(
    JPS_Simulation handle,
    const JPS_Point* polygon,
    size_t len_polygon,
    JPS_ErrorMessage* errorMessage)
{
    std::vector<Point> loop{};
    loop.reserve(len_polygon);
    std::transform(polygon, polygon + len_polygon, std::back_inserter(loop), intoPoint);
    return add_stage(handle, ExitDescription{Polygon(loop)}, errorMessage);
}

JPS_StageId JPS_Simulation_AddStageNotifiableQueue(
    JPS_Simulation handle,
    const JPS_Point* waiting_positions,
    size_t len_waiting_positions,
    JPS_ErrorMessage* errorMessage)
{
    std::vector<Point> positions{};
    positions.reserve(len_waiting_positions);
    std::transform(
        waiting_positions,
        waiting_positions + len_waiting_positions,
        std::back_inserter(positions),
        intoPoint);
    return add_stage(handle, NotifiableQueueDescription{positions}, errorMessage);
}

JPS_StageId JPS_Simulation_AddStageWaitingSet(
    JPS_Simulation handle,
    const JPS_Point* waiting_positions,
    size_t len_waiting_positions,
    JPS_ErrorMessage* errorMessage)
{
    std::vector<Point> positions{};
    positions.reserve(len_waiting_positions);
    std::transform(
        waiting_positions,
        waiting_positions + len_waiting_positions,
        std::back_inserter(positions),
        intoPoint);
    return add_stage(handle, NotifiableWaitingSetDescription{positions}, errorMessage);
}

JPS_AgentId JPS_Simulation_AddGeneralizedCentrifugalForceModelAgent(
    JPS_Simulation handle,
    JPS_GeneralizedCentrifugalForceModelAgentParameters parameters,
    JPS_ErrorMessage* errorMessage)
{
    assert(handle);
    auto result = GenericAgent::ID::Invalid;
    auto simulation = reinterpret_cast<Simulation*>(handle);
    try {
        if(simulation->ModelType() != OperationalModelType::GENERALIZED_CENTRIFUGAL_FORCE) {
            throw std::runtime_error(
                "Simulation is not configured to use Generalized Centrifugal Force Model");
        }
        GenericAgent agent{
            GenericAgent::ID::Invalid,
            Journey::ID(parameters.journeyId),
            BaseStage::ID(parameters.stageId),
            intoPoint(parameters.position),
            intoPoint(parameters.orientation),
            GeneralizedCentrifugalForceModelData{
                parameters.speed,
                intoPoint(parameters.e0),
                0,
                parameters.mass,
                parameters.tau,
                parameters.v0,
                parameters.a_v,
                parameters.a_min,
                parameters.b_min,
                parameters.b_max}};
        result = simulation->AddAgent(std::move(agent));
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
    return result.getID();
}

JPS_AgentId JPS_Simulation_AddCollisionFreeSpeedModelAgent(
    JPS_Simulation handle,
    JPS_CollisionFreeSpeedModelAgentParameters parameters,
    JPS_ErrorMessage* errorMessage)
{
    assert(handle);
    auto result = GenericAgent::ID::Invalid;
    auto simulation = reinterpret_cast<Simulation*>(handle);
    try {
        if(simulation->ModelType() != OperationalModelType::COLLISION_FREE_SPEED) {
            throw std::runtime_error(
                "Simulation is not configured to use Collision Free Speed Model");
        }
        GenericAgent agent(
            GenericAgent::ID::Invalid,
            Journey::ID(parameters.journeyId),
            BaseStage::ID(parameters.stageId),
            intoPoint(parameters.position),
            {},
            CollisionFreeSpeedModelData{parameters.time_gap, parameters.v0, parameters.radius});
        result = simulation->AddAgent(std::move(agent));
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
    return result.getID();
}

JPS_AgentId JPS_Simulation_AddCollisionFreeSpeedModelV2Agent(
    JPS_Simulation handle,
    JPS_CollisionFreeSpeedModelV2AgentParameters parameters,
    JPS_ErrorMessage* errorMessage)
{
    assert(handle);
    auto result = GenericAgent::ID::Invalid;
    auto simulation = reinterpret_cast<Simulation*>(handle);
    try {
        if(simulation->ModelType() != OperationalModelType::COLLISION_FREE_SPEED_V2) {
            throw std::runtime_error(
                "Simulation is not configured to use Collision Free Speed Model V2");
        }
        GenericAgent agent(
            GenericAgent::ID::Invalid,
            Journey::ID(parameters.journeyId),
            BaseStage::ID(parameters.stageId),
            intoPoint(parameters.position),
            {},
            CollisionFreeSpeedModelV2Data{
                parameters.strengthNeighborRepulsion,
                parameters.rangeNeighborRepulsion,
                parameters.strengthGeometryRepulsion,
                parameters.rangeGeometryRepulsion,
                parameters.time_gap,
                parameters.v0,
                parameters.radius});
        result = simulation->AddAgent(std::move(agent));
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
    return result.getID();
}

bool JPS_Simulation_MarkAgentForRemoval(
    JPS_Simulation handle,
    JPS_AgentId agentId,
    JPS_ErrorMessage* errorMessage)
{
    assert(handle);
    auto simulation = reinterpret_cast<Simulation*>(handle);
    bool result{false};
    try {
        simulation->MarkAgentForRemoval(agentId);
        result = true;
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

size_t JPS_Simulation_RemovedAgents(JPS_Simulation handle, const JPS_AgentId** data)
{
    assert(handle);
    auto simulation = reinterpret_cast<Simulation*>(handle);
    const auto& agents = simulation->RemovedAgents();
    static_assert(
        std::is_same<JPS_AgentId, GenericAgent::ID::underlying_type>::value,
        "GenericAgentIDs cannot be casted in JPS_AgentId");
    *data = reinterpret_cast<const JPS_AgentId*>(agents.data());
    return agents.size();
}

bool JPS_Simulation_Iterate(JPS_Simulation handle, JPS_ErrorMessage* errorMessage)
{
    assert(handle);
    auto simulation = reinterpret_cast<Simulation*>(handle);
    bool result = false;
    try {
        simulation->Iterate();
        result = true;
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

size_t JPS_Simulation_AgentCount(JPS_Simulation handle)
{
    assert(handle);
    auto simulation = reinterpret_cast<Simulation*>(handle);
    return simulation->AgentCount();
}

double JPS_Simulation_ElapsedTime(JPS_Simulation handle)
{
    assert(handle);
    auto simulation = reinterpret_cast<Simulation*>(handle);
    return simulation->ElapsedTime();
}

double JPS_Simulation_DeltaTime(JPS_Simulation handle)
{
    assert(handle);
    auto simulation = reinterpret_cast<Simulation*>(handle);
    return simulation->DT();
}

uint64_t JPS_Simulation_IterationCount(JPS_Simulation handle)
{
    assert(handle);
    auto simulation = reinterpret_cast<Simulation*>(handle);
    return simulation->Iteration();
}

JPS_AgentIterator JPS_Simulation_AgentIterator(JPS_Simulation handle)
{
    assert(handle);
    auto simulation = reinterpret_cast<Simulation*>(handle);
    return reinterpret_cast<JPS_AgentIterator>(
        new AgentIterator<GenericAgent>(simulation->Agents()));
}

JPS_Agent
JPS_Simulation_GetAgent(JPS_Simulation handle, JPS_AgentId agentId, JPS_ErrorMessage* errorMessage)
{
    assert(handle);
    const auto simulation = reinterpret_cast<Simulation*>(handle);

    try {
        const auto agent = &simulation->Agent(agentId);
        return reinterpret_cast<JPS_Agent>(agent);
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
    return nullptr;
}

bool JPS_Simulation_SwitchAgentJourney(
    JPS_Simulation handle,
    JPS_AgentId agentId,
    JPS_JourneyId journeyId,
    JPS_StageId stageId,
    JPS_ErrorMessage* errorMessage)
{
    assert(handle);
    const auto simulation = reinterpret_cast<Simulation*>(handle);
    bool result = false;
    try {
        simulation->SwitchAgentJourney(agentId, journeyId, stageId);
        result = true;
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

JPS_ModelType JPS_Simulation_ModelType(JPS_Simulation handle)
{
    assert(handle);
    const auto simulation = reinterpret_cast<Simulation*>(handle);
    const auto type = simulation->ModelType();
    switch(type) {
        case OperationalModelType::COLLISION_FREE_SPEED:
            return JPS_CollisionFreeSpeedModel;
        case OperationalModelType::GENERALIZED_CENTRIFUGAL_FORCE:
            return JPS_GeneralizedCentrifugalForceModel;
        case OperationalModelType::COLLISION_FREE_SPEED_V2:
            return JPS_CollisionFreeSpeedModelV2;
    }
    UNREACHABLE();
}

JPS_AgentIdIterator
JPS_Simulation_AgentsInRange(JPS_Simulation handle, JPS_Point position, double distance)
{
    assert(handle);
    auto simulation = reinterpret_cast<Simulation*>(handle);
    return reinterpret_cast<JPS_AgentIdIterator>(
        new AgentIdIterator(simulation->AgentsInRange(intoPoint(position), distance)));
}

JPS_AgentIdIterator
JPS_Simulation_AgentsInPolygon(JPS_Simulation handle, const JPS_Point* polygon, size_t len_polygon)
{
    assert(handle);
    auto simulation = reinterpret_cast<Simulation*>(handle);
    std::vector<Point> poly{};
    poly.reserve(len_polygon);
    std::transform(polygon, polygon + len_polygon, std::back_inserter(poly), intoPoint);
    return reinterpret_cast<JPS_AgentIdIterator>(
        new AgentIdIterator(simulation->AgentsInPolygon(poly)));
}

JPS_StageType JPS_Simulation_GetStageType(JPS_Simulation handle, JPS_StageId id)
{
    assert(handle);
    auto simulation = reinterpret_cast<Simulation*>(handle);
    const auto convert = [](const auto t) {
        switch(t) {
            case 0:
                return JPS_WaypointType;
            case 1:
                return JPS_WaitingSetType;
            case 2:
                return JPS_NotifiableQueueType;
            case 3:
                return JPS_ExitType;
        }
        UNREACHABLE();
    };
    return convert(simulation->Stage(id).index());
}

JPS_NotifiableQueueProxy JPS_Simulation_GetNotifiableQueueProxy(
    JPS_Simulation handle,
    JPS_StageId stageId,
    JPS_ErrorMessage* errorMessage)
{
    assert(handle);
    auto simulation = reinterpret_cast<Simulation*>(handle);
    try {
        return reinterpret_cast<JPS_NotifiableQueueProxy>(
            new NotifiableQueueProxy(std::get<NotifiableQueueProxy>(simulation->Stage(stageId))));
    } catch(const std::exception& ex) {
        if(errorMessage) {
            *errorMessage = reinterpret_cast<JPS_ErrorMessage>(new JPS_ErrorMessage_t{ex.what()});
        }
        return nullptr;
    }
}
JUPEDSIM_API JPS_WaitingSetProxy JPS_Simulation_GetWaitingSetProxy(
    JPS_Simulation handle,
    JPS_StageId stageId,
    JPS_ErrorMessage* errorMessage)
{
    assert(handle);
    auto simulation = reinterpret_cast<Simulation*>(handle);
    try {
        return reinterpret_cast<JPS_WaitingSetProxy>(new NotifiableWaitingSetProxy(
            std::get<NotifiableWaitingSetProxy>(simulation->Stage(stageId))));
    } catch(const std::exception& ex) {
        if(errorMessage) {
            *errorMessage = reinterpret_cast<JPS_ErrorMessage>(new JPS_ErrorMessage_t{ex.what()});
        }
        return nullptr;
    }
}

JUPEDSIM_API JPS_WaypointProxy JPS_Simulation_GetWaypointProxy(
    JPS_Simulation handle,
    JPS_StageId stageId,
    JPS_ErrorMessage* errorMessage)
{
    assert(handle);
    auto simulation = reinterpret_cast<Simulation*>(handle);
    try {
        return reinterpret_cast<JPS_WaypointProxy>(
            new WaypointProxy(std::get<WaypointProxy>(simulation->Stage(stageId))));
    } catch(const std::exception& ex) {
        if(errorMessage) {
            *errorMessage = reinterpret_cast<JPS_ErrorMessage>(new JPS_ErrorMessage_t{ex.what()});
        }
        return nullptr;
    }
}

JUPEDSIM_API JPS_ExitProxy JPS_Simulation_GetExitProxy(
    JPS_Simulation handle,
    JPS_StageId stageId,
    JPS_ErrorMessage* errorMessage)
{
    assert(handle);
    auto simulation = reinterpret_cast<Simulation*>(handle);
    try {
        return reinterpret_cast<JPS_ExitProxy>(
            new ExitProxy(std::get<ExitProxy>(simulation->Stage(stageId))));
    } catch(const std::exception& ex) {
        if(errorMessage) {
            *errorMessage = reinterpret_cast<JPS_ErrorMessage>(new JPS_ErrorMessage_t{ex.what()});
        }
        return nullptr;
    }
}

void JPS_Simulation_SetTracing(JPS_Simulation handle, bool status)
{
    assert(handle);
    auto simuation = reinterpret_cast<Simulation*>(handle);
    simuation->SetTracing(status);
}

JPS_Trace JPS_Simulation_GetTrace(JPS_Simulation handle)
{
    assert(handle);
    auto simuation = reinterpret_cast<Simulation*>(handle);
    const auto stats = simuation->GetLastStats();
    return JPS_Trace{stats.IterationDuration(), stats.OpDecSystemRunDuration()};
}

JPS_Geometry JPS_Simulation_GetGeometry(JPS_Simulation handle)
{
    assert(handle);
    const auto simulation = reinterpret_cast<const Simulation*>(handle);
    return reinterpret_cast<JPS_Geometry>(new Geometry(simulation->Geo()));
}

bool JPS_Simulation_SwitchGeometry(
    JPS_Simulation handle,
    JPS_Geometry geometry,
    JPS_AgentIdIterator* faultyAgents,
    JPS_ErrorMessage* errorMessage)
{
    assert(handle);
    assert(geometry);

    auto simulation = reinterpret_cast<Simulation*>(handle);
    auto geometryInternal = reinterpret_cast<const Geometry*>(geometry);
    auto collisionGeometry =
        std::make_unique<CollisionGeometry>(*geometryInternal->collisionGeometry);
    auto routingEngine = geometryInternal->routingEngine->Clone();

    bool result = false;
    try {
        simulation->SwitchGeometry(std::move(collisionGeometry), std::move(routingEngine));
        result = true;
    } catch(const GeometrySwitchError& ex) {
        if(errorMessage) {
            *errorMessage = reinterpret_cast<JPS_ErrorMessage>(new JPS_ErrorMessage_t{ex.what()});
        }
        if(faultyAgents) {
            *faultyAgents =
                reinterpret_cast<JPS_AgentIdIterator>(new AgentIdIterator(ex.FaultyAgents()));
        }
    } catch(...) {
        if(errorMessage) {
            *errorMessage = reinterpret_cast<JPS_ErrorMessage>(
                new JPS_ErrorMessage_t{"Unknown internal error."});
        }
    }
    return result;
}

void JPS_Simulation_Free(JPS_Simulation handle)
{
    delete reinterpret_cast<Simulation*>(handle);
}

////////////////////////////////////////////////////////////////////////////////
/// JPS_Path
////////////////////////////////////////////////////////////////////////////////
JUPEDSIM_API void JPS_Path_Free(JPS_Path* path)
{
    delete[] path->points;
    path->points = nullptr;
    path->len = 0;
}

////////////////////////////////////////////////////////////////////////////////
/// JPS_TriangleMesh
////////////////////////////////////////////////////////////////////////////////
JUPEDSIM_API void JPS_TriangleMesh_Free(JPS_TriangleMesh* mesh)
{
    delete[] mesh->triangles;
    mesh->triangles = nullptr;
    mesh->len = 0;
}

////////////////////////////////////////////////////////////////////////////////
/// JPS_Lines
////////////////////////////////////////////////////////////////////////////////
JUPEDSIM_API void JPS_Lines_Free(JPS_Lines* lines)
{
    delete[] lines->lines;
    lines->lines = nullptr;
    lines->len = 0;
}

////////////////////////////////////////////////////////////////////////////////
/// JPS_RoutingEngine
////////////////////////////////////////////////////////////////////////////////
JUPEDSIM_API JPS_RoutingEngine JPS_RoutingEngine_Create(JPS_Geometry geometry)
{
    auto* geo = reinterpret_cast<const Geometry*>(geometry);
    return reinterpret_cast<JPS_RoutingEngine>(geo->routingEngine->Clone().release());
}

JUPEDSIM_API JPS_Path
JPS_RoutingEngine_ComputeWaypoint(JPS_RoutingEngine handle, JPS_Point from, JPS_Point to)
{
    auto* engine = reinterpret_cast<NavMeshRoutingEngine*>(handle);
    const auto path = engine->ComputeAllWaypoints(intoPoint(from), intoPoint(to));
    auto points = new JPS_Point[path.size()];
    JPS_Path p{path.size(), points};
    std::transform(
        std::begin(path), std::end(path), points, [](const auto& p) { return intoJPS_Point(p); });
    return p;
}

JUPEDSIM_API bool JPS_RoutingEngine_IsRoutable(JPS_RoutingEngine handle, JPS_Point p)
{
    const auto* engine = reinterpret_cast<NavMeshRoutingEngine*>(handle);
    return engine->IsRoutable(intoPoint(p));
}

JUPEDSIM_API JPS_TriangleMesh JPS_RoutingEngine_Mesh(JPS_RoutingEngine handle)
{
    const auto* engine = reinterpret_cast<NavMeshRoutingEngine*>(handle);
    const auto res = engine->Mesh();
    JPS_TriangleMesh mesh{res.size(), new JPS_Triangle[res.size()]};
    std::transform(std::begin(res), std::end(res), mesh.triangles, [](const auto& t) {
        auto tri = JPS_Triangle{};
        tri.points[0] = intoJPS_Point(t.points[0]);
        tri.points[1] = intoJPS_Point(t.points[1]);
        tri.points[2] = intoJPS_Point(t.points[2]);
        return tri;
    });
    return mesh;
}

JUPEDSIM_API JPS_Lines JPS_RoutingEngine_EdgesFor(JPS_RoutingEngine handle, uint32_t id)
{
    const auto* engine = reinterpret_cast<NavMeshRoutingEngine*>(handle);
    const auto res = engine->EdgesFor(id);

    JPS_Lines lines{};
    lines.lines = new JPS_Line[res.size()];
    lines.len = res.size();
    std::transform(std::begin(res), std::end(res), lines.lines, [](const auto& edge) {
        JPS_Line line{};
        line.points[0] = intoJPS_Point(edge.edge.p1);
        line.points[1] = intoJPS_Point(edge.edge.p2);
        return line;
    });
    return lines;
}

JUPEDSIM_API void JPS_RoutingEngine_Free(JPS_RoutingEngine handle)
{
    delete reinterpret_cast<NavMeshRoutingEngine*>(handle);
}
