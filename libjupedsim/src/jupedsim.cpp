/// Copyright © 2012-2022 Forschungszentrum Jülich GmbH
/// SPDX-License-Identifier: LGPL-3.0-or-later
#include "jupedsim/jupedsim.h"

#include "AgentIterator.hpp"
#include "ErrorMessage.hpp"

#include <Agent.hpp>
#include <Area.hpp>
#include <CollisionGeometry.hpp>
#include <GCFMModel.hpp>
#include <GCFMModelBuilder.hpp>
#include <Geometry.hpp>
#include <GeometryBuilder.hpp>
#include <Journey.hpp>
#include <Logger.hpp>
#include <OperationalModel.hpp>
#include <OperationalModelType.hpp>
#include <Point.hpp>
#include <RoutingEngine.hpp>
#include <Simulation.hpp>
#include <VelocityModel.hpp>
#include <VelocityModelBuilder.hpp>

#include <cassert>
#include <exception>
#include <memory>
#include <string>
#include <vector>

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
/// GCFM Model Builder
////////////////////////////////////////////////////////////////////////////////////////////////////
JPS_GCFMModelBuilder JPS_GCFMModelBuilder_Create(
    double nu_Ped,
    double nu_Wall,
    double dist_eff_Ped,
    double dist_eff_Wall,
    double intp_width_Ped,
    double intp_width_Wall,
    double maxf_Ped,
    double maxf_Wall)
{
    return reinterpret_cast<JPS_GCFMModelBuilder>(new GCFMModelBuilder(
        nu_Ped,
        nu_Wall,
        dist_eff_Ped,
        dist_eff_Wall,
        intp_width_Ped,
        intp_width_Wall,
        maxf_Ped,
        maxf_Wall));
}

void JPS_GCFMModelBuilder_AddParameterProfile(
    JPS_GCFMModelBuilder handle,
    uint64_t id,
    double mass,
    double tau,
    double v0,
    double a_v,
    double a_min,
    double b_min,
    double b_max)
{
    assert(handle != nullptr);
    auto builder = reinterpret_cast<GCFMModelBuilder*>(handle);
    builder->AddAgentParameterProfile({id, mass, tau, v0, a_v, a_min, b_min, b_max});
}

JPS_OperationalModel
JPS_GCFMModelBuilder_Build(JPS_GCFMModelBuilder handle, JPS_ErrorMessage* errorMessage)
{
    assert(handle != nullptr);
    auto builder = reinterpret_cast<GCFMModelBuilder*>(handle);
    JPS_OperationalModel result{};
    try {
        result = reinterpret_cast<JPS_OperationalModel>(new GCFMModel(builder->Build()));
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

void JPS_GCFMModelBuilder_Free(JPS_GCFMModelBuilder handle)
{
    delete reinterpret_cast<GCFMModelBuilder*>(handle);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Velocity Model Builder
////////////////////////////////////////////////////////////////////////////////////////////////////
JUPEDSIM_API JPS_VelocityModelBuilder
JPS_VelocityModelBuilder_Create(double aPed, double DPed, double aWall, double DWall)
{
    return reinterpret_cast<JPS_VelocityModelBuilder>(
        new VelocityModelBuilder(aPed, DPed, aWall, DWall));
}

JUPEDSIM_API void JPS_VelocityModelBuilder_AddParameterProfile(
    JPS_VelocityModelBuilder handle,
    uint64_t id,
    double t,
    double tau,
    double v0,
    double radius)
{
    assert(handle);
    auto builder = reinterpret_cast<VelocityModelBuilder*>(handle);
    builder->AddAgentParameterProfile({id, t, tau, v0, radius});
}

JUPEDSIM_API JPS_OperationalModel
JPS_VelocityModelBuilder_Build(JPS_VelocityModelBuilder handle, JPS_ErrorMessage* errorMessage)
{
    assert(handle != nullptr);
    auto builder = reinterpret_cast<VelocityModelBuilder*>(handle);
    JPS_OperationalModel result{};
    try {
        result = reinterpret_cast<JPS_OperationalModel>(new VelocityModel(builder->Build()));
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

JUPEDSIM_API void JPS_VelocityModelBuilder_Free(JPS_VelocityModelBuilder handle)
{
    delete reinterpret_cast<VelocityModelBuilder*>(handle);
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
    double* points,
    size_t pointCount)
{
    assert(handle != nullptr);
    auto builder = reinterpret_cast<GeometryBuilder*>(handle);
    std::vector<Point> lineLoop{};
    lineLoop.reserve(pointCount);
    for(size_t pointIndex = 0; pointIndex < pointCount; ++pointIndex) {
        lineLoop.emplace_back(points[pointIndex * 2], points[pointIndex * 2 + 1]);
    }
    // TODO(kkratz): Consider adding a move version of 'AddAccessibleArea'
    builder->AddAccessibleArea(lineLoop);
}

void JPS_GeometryBuilder_ExcludeFromAccessibleArea(
    JPS_GeometryBuilder handle,
    double* points,
    size_t pointCount)
{
    assert(handle != nullptr);
    auto builder = reinterpret_cast<GeometryBuilder*>(handle);
    std::vector<Point> lineLoop{};
    lineLoop.reserve(pointCount);
    for(size_t pointIndex = 0; pointIndex < pointCount; ++pointIndex) {
        lineLoop.emplace_back(points[pointIndex * 2], points[pointIndex * 2 + 1]);
    }
    // TODO(kkratz): Consider adding a move version of 'AddAccessibleArea'
    builder->ExcludeFromAccessibleArea(lineLoop);
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
void JPS_Geometry_Free(JPS_Geometry handle)
{
    delete reinterpret_cast<Geometry*>(handle);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// AreasBuilder
////////////////////////////////////////////////////////////////////////////////////////////////////
JPS_AreasBuilder JPS_AreasBuilder_Create()
{
    return reinterpret_cast<JPS_AreasBuilder>(new AreasBuilder{});
}

void JPS_AreasBuilder_AddArea(
    JPS_AreasBuilder handle,
    uint64_t id,
    double* points,
    size_t pointCount,
    const char** tags,
    size_t tagCount)
{
    assert(handle);
    auto builder = reinterpret_cast<AreasBuilder*>(handle);
    std::vector<Point> lineLoop{};
    lineLoop.reserve(pointCount);
    for(size_t pointIndex = 0; pointIndex < pointCount; ++pointIndex) {
        lineLoop.emplace_back(points[pointIndex * 2], points[pointIndex * 2 + 1]);
    }
    std::vector<std::string> labels{};
    labels.reserve(tagCount);
    for(size_t tagIndex = 0; tagIndex < tagCount; ++tagIndex) {
        labels.emplace_back(tags[tagIndex]);
    }
    builder->AddArea(id, lineLoop, labels);
}

JPS_Areas JPS_AreasBuilder_Build(JPS_AreasBuilder handle, JPS_ErrorMessage* errorMessage)
{
    assert(handle);
    auto builder = reinterpret_cast<AreasBuilder*>(handle);
    JPS_Areas result{};
    try {
        result = reinterpret_cast<JPS_Areas>(new Areas(builder->Build()));
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

void JPS_AreasBuilder_Free(JPS_AreasBuilder handle)
{
    delete reinterpret_cast<AreasBuilder*>(handle);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Areas
////////////////////////////////////////////////////////////////////////////////////////////////////
void JPS_Areas_Free(JPS_Areas handle)
{
    delete reinterpret_cast<Areas*>(handle);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Agent
////////////////////////////////////////////////////////////////////////////////////////////////////
double JPS_Agent_PositionX(JPS_Agent handle)
{
    assert(handle);
    const auto agent = reinterpret_cast<const Agent*>(handle);
    return agent->pos.x;
}

double JPS_Agent_PositionY(JPS_Agent handle)
{
    assert(handle);
    const auto agent = reinterpret_cast<const Agent*>(handle);
    return agent->pos.y;
}

double JPS_Agent_OrientationX(JPS_Agent handle)
{
    assert(handle);
    const auto agent = reinterpret_cast<const Agent*>(handle);
    return agent->orientation.x;
}

double JPS_Agent_OrientationY(JPS_Agent handle)
{
    assert(handle);
    const auto agent = reinterpret_cast<const Agent*>(handle);
    return agent->orientation.y;
}

JPS_AgentId JPS_Agent_Id(JPS_Agent handle)
{
    assert(handle);
    const auto agent = reinterpret_cast<const Agent*>(handle);
    return agent->id.getID();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// AgentIterator
////////////////////////////////////////////////////////////////////////////////////////////////////
JPS_Agent JPS_AgentIterator_Next(JPS_AgentIterator handle)
{
    assert(handle);
    auto iterator = reinterpret_cast<AgentIterator*>(handle);
    return reinterpret_cast<JPS_Agent>(iterator->Next());
}

void JPS_AgentIterator_Free(JPS_AgentIterator handle)
{
    delete reinterpret_cast<AgentIterator*>(handle);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Journey
////////////////////////////////////////////////////////////////////////////////////////////////////
JPS_Journey JPS_Journey_Create_SimpleJourney(const JPS_Waypoint* waypoints, size_t count_waypoints)
{
    auto* journey = new SimpleJourney{};
    for(size_t index = 0; index < count_waypoints; ++index) {
        const auto& waypoint = waypoints[index];
        journey->AddWaypoint({waypoint.position.x, waypoint.position.y}, waypoint.distance);
    }
    return reinterpret_cast<JPS_Journey>(journey);
}

void JPS_Journey_Free(JPS_Journey handle)
{
    delete reinterpret_cast<Journey*>(handle);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Simulation
////////////////////////////////////////////////////////////////////////////////////////////////////
JPS_Simulation JPS_Simulation_Create(
    JPS_OperationalModel model,
    JPS_Geometry geometry,
    JPS_Areas areas,
    double dT,
    JPS_ErrorMessage* errorMessage)
{
    assert(model);
    assert(geometry);
    assert(areas);
    JPS_Simulation result{};
    try {
        auto geometryInternal = reinterpret_cast<Geometry*>(geometry);
        auto collisionGeometry =
            std::make_unique<CollisionGeometry>(*geometryInternal->collisionGeometry);
        auto routingEngine = geometryInternal->routingEngine->Clone();

        auto modelInternal = reinterpret_cast<OperationalModel*>(model);
        auto model = modelInternal->Clone();

        auto areasInternal = reinterpret_cast<Areas*>(areas);
        auto areas = std::make_unique<Areas>(*areasInternal);

        result = reinterpret_cast<JPS_Simulation>(new Simulation(
            std::move(model),
            std::move(collisionGeometry),
            std::move(routingEngine),
            std::move(areas),
            dT));
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
    JPS_Journey journey,
    JPS_ErrorMessage* errorMessage)
{
    assert(handle);
    assert(journey);

    auto simulation = reinterpret_cast<Simulation*>(handle);
    auto journeyInternal = reinterpret_cast<Journey*>(journey);
    auto result = Journey::ID::Invalid.getID();
    try {
        result = simulation->AddJourney(std::unique_ptr<Journey>(journeyInternal->Clone())).getID();
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

JPS_AgentId JPS_Simulation_AddAgent(
    JPS_Simulation handle,
    JPS_AgentParameters parameters,
    JPS_ErrorMessage* errorMessage)
{
    assert(handle);
    auto simulation = reinterpret_cast<Simulation*>(handle);
    JPS_AgentId result{};
    try {
        result = simulation->AddAgent(
            Point(parameters.positionX, parameters.positionY),
            Point(parameters.orientationX, parameters.orientationY),
            parameters.journeyId,
            parameters.profileId);
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

JUPEDSIM_API bool JPS_Simulation_RemoveAgent(
    JPS_Simulation handle,
    JPS_AgentId agentId,
    JPS_ErrorMessage* errorMessage)
{
    assert(handle);
    auto simulation = reinterpret_cast<Simulation*>(handle);
    bool result{false};
    try {
        simulation->RemoveAgent(agentId);
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

JPS_Agent
JPS_Simulation_ReadAgent(JPS_Simulation handle, JPS_AgentId id, JPS_ErrorMessage* errorMessage)
{
    assert(handle);
    auto simulation = reinterpret_cast<Simulation*>(handle);
    JPS_Agent result{};
    try {
        result = reinterpret_cast<JPS_Agent>(simulation->AgentPtr(id));
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
    *data = agents.data();
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
    return reinterpret_cast<JPS_AgentIterator>(new AgentIterator(simulation->Agents()));
}

bool JPS_Simulation_SwitchAgentProfile(
    JPS_Simulation handle,
    JPS_AgentId agentId,
    JPS_ModelParameterProfileId profileId,
    JPS_ErrorMessage* errorMessage)
{
    assert(handle);
    const auto simulation = reinterpret_cast<Simulation*>(handle);
    bool result = false;
    try {
        simulation->SwitchAgentProfile(agentId, profileId);
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

void JPS_Simulation_Free(JPS_Simulation handle)
{
    delete reinterpret_cast<Simulation*>(handle);
}
