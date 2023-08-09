// Copyright © 2012-2023 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "jupedsim/jupedsim.h"

#include "AgentIterator.hpp"
#include "ErrorMessage.hpp"
#include "Events.hpp"
#include "Journey.hpp"
#include "Stage.hpp"

#include <BuildInfo.hpp>
#include <CollisionGeometry.hpp>
#include <Conversion.hpp>
#include <GCFMModel.hpp>
#include <GCFMModelBuilder.hpp>
#include <GenericAgent.hpp>
#include <Geometry.hpp>
#include <GeometryBuilder.hpp>
#include <Logger.hpp>
#include <OperationalModel.hpp>
#include <OperationalModelType.hpp>
#include <Point.hpp>
#include <RoutingEngine.hpp>
#include <Simulation.hpp>
#include <StageDescription.hpp>
#include <Unreachable.hpp>
#include <VelocityModel.hpp>
#include <VelocityModelBuilder.hpp>

#include <algorithm>
#include <cassert>
#include <exception>
#include <iterator>
#include <memory>
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
void JPS_Geometry_Free(JPS_Geometry handle)
{
    delete reinterpret_cast<Geometry*>(handle);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// GCFMModelAgentIterator
////////////////////////////////////////////////////////////////////////////////////////////////////
const JPS_GCFMModelAgentParameters*
JPS_GCFMModelAgentIterator_Next(JPS_GCFMModelAgentIterator handle)
{
    assert(handle);
    auto iterator =
        reinterpret_cast<AgentIterator<GCFMModel::Data, JPS_GCFMModelAgentParameters>*>(handle);
    return iterator->Next();
}

void JPS_GCFMModelAgentIterator_Free(JPS_GCFMModelAgentIterator handle)
{
    delete reinterpret_cast<AgentIterator<GCFMModel::Data, JPS_GCFMModelAgentParameters>*>(handle);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// VelocityModelAgentIterator
////////////////////////////////////////////////////////////////////////////////////////////////////
const JPS_VelocityModelAgentParameters*
JPS_VelocityModelAgentIterator_Next(JPS_VelocityModelAgentIterator handle)
{
    assert(handle);
    auto iterator =
        reinterpret_cast<AgentIterator<VelocityModel::Data, JPS_VelocityModelAgentParameters>*>(
            handle);
    return iterator->Next();
}

void JPS_VelocityModelAgentIterator_Free(JPS_VelocityModelAgentIterator handle)
{
    delete reinterpret_cast<AgentIterator<VelocityModel::Data, JPS_VelocityModelAgentParameters>*>(
        handle);
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
using JourneyDesc = std::vector<Stage::ID>;

JPS_JourneyDescription JPS_JourneyDescription_Create()
{
    return reinterpret_cast<JPS_JourneyDescription>(new JourneyDesc{});
}

void JPS_JourneyDescription_AddStage(JPS_JourneyDescription handle, JPS_StageId id)
{
    assert(handle);
    auto journeyDesc = reinterpret_cast<JourneyDesc*>(handle);
    journeyDesc->emplace_back(id);
}

void JPS_JourneyDescription_Free(JPS_JourneyDescription handle)
{
    delete reinterpret_cast<JourneyDesc*>(handle);
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
        auto geometryInternal = reinterpret_cast<Geometry*>(geometry);
        auto collisionGeometry =
            std::make_unique<CollisionGeometry>(*geometryInternal->collisionGeometry);
        auto routingEngine = geometryInternal->routingEngine->Clone();

        auto modelInternal = reinterpret_cast<OperationalModel*>(model);
        auto model = modelInternal->Clone();

        if(dynamic_cast<VelocityModel*>(model.get())) {
            auto ptr = dynamic_cast<VelocityModel*>(model.release());
            auto derived = std::unique_ptr<VelocityModel>(ptr);
            result = reinterpret_cast<JPS_Simulation>(new TypedSimulation<VelocityModel>(
                std::move(derived), std::move(collisionGeometry), std::move(routingEngine), dT));
        } else if(dynamic_cast<GCFMModel*>(model.get())) {
            auto ptr = dynamic_cast<GCFMModel*>(model.release());
            auto derived = std::unique_ptr<GCFMModel>(ptr);
            result = reinterpret_cast<JPS_Simulation>(new TypedSimulation<GCFMModel>(
                std::move(derived), std::move(collisionGeometry), std::move(routingEngine), dT));
        } else {
            throw std::runtime_error("Unknown model type encountered");
        }
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

JPS_AgentId JPS_Simulation_AddGCFMModelAgent(
    JPS_Simulation handle,
    JPS_GCFMModelAgentParameters parameters,
    JPS_ErrorMessage* errorMessage)
{
    assert(handle);
    auto result = GenericAgent::ID::Invalid;
    auto simulation_base = reinterpret_cast<Simulation*>(handle);
    auto simulation = dynamic_cast<TypedSimulation<GCFMModel>*>(simulation_base);
    if(simulation == nullptr) {
        if(errorMessage) {
            *errorMessage = reinterpret_cast<JPS_ErrorMessage>(new JPS_ErrorMessage_t{
                "Simulation is not using GCFMModel, cannot add an Agent with this model."});
        }
        return result.getID();
    }
    try {
        GCFMModel::Data agent(
            GCFMModel::Data::ID(parameters.agentId),
            Journey::ID(parameters.journeyId),
            OperationalModel::ParametersID(parameters.profileId),
            intoPoint(parameters.position),
            intoPoint(parameters.orientation),
            parameters.speed,
            intoPoint(parameters.e0));
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

JPS_AgentId JPS_Simulation_AddVelocityModelAgent(
    JPS_Simulation handle,
    JPS_VelocityModelAgentParameters parameters,
    JPS_ErrorMessage* errorMessage)
{
    assert(handle);
    auto result = GenericAgent::ID::Invalid;
    auto simulation_base = reinterpret_cast<Simulation*>(handle);
    auto simulation = dynamic_cast<TypedSimulation<VelocityModel>*>(simulation_base);
    if(simulation == nullptr) {
        if(errorMessage) {
            *errorMessage = reinterpret_cast<JPS_ErrorMessage>(new JPS_ErrorMessage_t{
                "Simulation is not using VelocityModel, cannot add an Agent with this model."});
        }
        return result.getID();
    }
    try {
        VelocityModel::Data agent(
            VelocityModel::Data::ID(parameters.agentId),
            Journey::ID(parameters.journeyId),
            OperationalModel::ParametersID(parameters.profileId),
            intoPoint(parameters.position),
            intoPoint(parameters.orientation),
            intoPoint(parameters.e0));
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

bool JPS_Simulation_RemoveAgent(
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

bool JPS_Simulation_ReadGCFMModelAgent(
    JPS_Simulation handle,
    JPS_AgentId id,
    JPS_GCFMModelAgentParameters* agent_out,
    JPS_ErrorMessage* errorMessage)
{
    assert(handle);
    auto simulation_base = reinterpret_cast<Simulation*>(handle);
    auto simulation = dynamic_cast<TypedSimulation<GCFMModel>*>(simulation_base);
    assert(simulation);
    try {
        const auto& agent = simulation->Agent(id);
        agent_out->speed = agent.speed, agent_out->e0 = intoJPS_Point(agent.e0),
        agent_out->position = intoJPS_Point(agent.pos),
        agent_out->orientation = intoJPS_Point(agent.orientation),
        agent_out->journeyId = agent.journeyId.getID();
        agent_out->profileId = agent.parameterProfileId.getID();
        return true;
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
    return false;
}

bool JPS_Simulation_ReadVelocityModelAgent(
    JPS_Simulation handle,
    JPS_AgentId id,
    JPS_VelocityModelAgentParameters* agent_out,
    JPS_ErrorMessage* errorMessage)
{
    assert(handle);
    auto simulation_base = reinterpret_cast<Simulation*>(handle);
    auto simulation = dynamic_cast<TypedSimulation<VelocityModel>*>(simulation_base);
    assert(simulation);
    try {
        const auto& agent = simulation->Agent(id);
        agent_out->position = intoJPS_Point(agent.pos),
        agent_out->orientation = intoJPS_Point(agent.orientation),
        agent_out->journeyId = agent.journeyId.getID();
        agent_out->profileId = agent.parameterProfileId.getID();
        return true;
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
    return false;
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

JPS_GCFMModelAgentIterator JPS_Simulation_GCFMModelAgentIterator(JPS_Simulation handle)
{
    assert(handle);
    auto simulation_base = reinterpret_cast<Simulation*>(handle);
    auto simulation = dynamic_cast<TypedSimulation<GCFMModel>*>(simulation_base);
    assert(simulation);
    return reinterpret_cast<JPS_GCFMModelAgentIterator>(
        new AgentIterator<GCFMModel::Data, JPS_GCFMModelAgentParameters>(
            simulation->Agents(), [](const auto& agent) {
                return JPS_GCFMModelAgentParameters{
                    agent.speed,
                    intoJPS_Point(agent.e0),
                    intoJPS_Point(agent.pos),
                    intoJPS_Point(agent.orientation),
                    agent.journeyId.getID(),
                    agent.parameterProfileId.getID(),
                    agent.id.getID()};
            }));
}

JPS_VelocityModelAgentIterator JPS_Simulation_VelocityModelAgentIterator(JPS_Simulation handle)
{
    assert(handle);
    auto simulation_base = reinterpret_cast<Simulation*>(handle);
    auto simulation = dynamic_cast<TypedSimulation<VelocityModel>*>(simulation_base);
    assert(simulation);
    return reinterpret_cast<JPS_VelocityModelAgentIterator>(
        new AgentIterator<VelocityModel::Data, JPS_VelocityModelAgentParameters>(
            simulation->Agents(), [](const auto& agent) {
                return JPS_VelocityModelAgentParameters{
                    intoJPS_Point(agent.e0),
                    intoJPS_Point(agent.pos),
                    intoJPS_Point(agent.orientation),
                    agent.journeyId.getID(),
                    agent.parameterProfileId.getID(),
                    agent.id.getID()};
            }));
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

bool JPS_Simulation_SwitchAgentJourney(
    JPS_Simulation handle,
    JPS_AgentId agentId,
    JPS_JourneyId journeyId,
    JPS_StageIndex stageIdx,
    JPS_ErrorMessage* errorMessage)
{
    assert(handle);
    const auto simulation = reinterpret_cast<Simulation*>(handle);
    bool result = false;
    try {
        simulation->SwitchAgentJourney(agentId, journeyId, stageIdx);
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
    const auto simulation_base = reinterpret_cast<Simulation*>(handle);
    if(const auto simulation = dynamic_cast<TypedSimulation<GCFMModel>*>(simulation_base);
       simulation != nullptr) {
        return JPS_GCFMModel;
    }
    if(const auto simulation = dynamic_cast<TypedSimulation<VelocityModel>*>(simulation_base);
       simulation != nullptr) {
        return JPS_VelocityModel;
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

bool JPS_Simulation_ChangeWaitingSetState(
    JPS_Simulation handle,
    JPS_StageId stageId,
    bool active,
    JPS_ErrorMessage* errorMessage)
{
    assert(handle);
    auto simuation = reinterpret_cast<Simulation*>(handle);
    try {
        simuation->Notify(NotifyWaitingSet{
            stageId,
            active ? NotifiableWaitingSet::WaitingState::Active :
                     NotifiableWaitingSet::WaitingState::Inactive});
    } catch(const std::exception& ex) {
        if(errorMessage) {
            *errorMessage = reinterpret_cast<JPS_ErrorMessage>(new JPS_ErrorMessage_t{ex.what()});
        }
        return false;
    }
    return true;
}

bool JPS_Simulation_PopAgentsFromQueue(
    JPS_Simulation handle,
    JPS_StageId stageId,
    size_t count,
    JPS_ErrorMessage* errorMessage)
{
    assert(handle);
    auto simuation = reinterpret_cast<Simulation*>(handle);
    try {
        simuation->Notify(NotifyQueue{stageId, count});
    } catch(const std::exception& ex) {
        if(errorMessage) {
            *errorMessage = reinterpret_cast<JPS_ErrorMessage>(new JPS_ErrorMessage_t{ex.what()});
        }
        return false;
    }
    return true;
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
    auto* geo = reinterpret_cast<Geometry*>(geometry);
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
