// Copyright © 2012-2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "jupedsim/simulation.h"

#include "jupedsim/agent.h"
#include "jupedsim/error.h"

#include "AgentIterator.hpp"
#include "Conversion.hpp"
#include "ErrorMessage.hpp"
#include "JourneyDescription.hpp"

#include <Geometry.hpp>
#include <GeometrySwitchError.hpp>
#include <Simulation.hpp>
#include <Unreachable.hpp>

#include <cassert>

using jupedsim::detail::intoJPS_Point;
using jupedsim::detail::intoPoint;
using jupedsim::detail::intoTuple;

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

JPS_StageId
JPS_Simulation_AddStageDirectSteering(JPS_Simulation handle, JPS_ErrorMessage* errorMessage)
{
    return add_stage(handle, DirectSteeringDescription{}, errorMessage);
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

JPS_AgentId JPS_Simulation_AddSocialForceModelAgent(
    JPS_Simulation handle,
    JPS_SocialForceModelAgentParameters parameters,
    JPS_ErrorMessage* errorMessage)
{
    assert(handle);
    auto result = GenericAgent::ID::Invalid;
    auto simulation = reinterpret_cast<Simulation*>(handle);
    try {
        if(simulation->ModelType() != OperationalModelType::SOCIAL_FORCE) {
            throw std::runtime_error("Simulation is not configured to use Social Force Model");
        }
        GenericAgent agent{
            GenericAgent::ID::Invalid,
            Journey::ID(parameters.journeyId),
            BaseStage::ID(parameters.stageId),
            intoPoint(parameters.position),
            intoPoint(parameters.orientation),
            SocialForceModelData{
                intoPoint(parameters.velocity),
                parameters.mass,
                parameters.desiredSpeed,
                parameters.reactionTime,
                parameters.agentScale,
                parameters.obstacleScale,
                parameters.forceDistance,
                parameters.radius}};
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
        case OperationalModelType::SOCIAL_FORCE:
            return JPS_SocialForceModel;
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
            case 4:
                return JPS_DirectSteeringType;
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

JPS_DirectSteeringProxy JPS_Simulation_GetDirectSteeringProxy(
    JPS_Simulation handle,
    JPS_StageId stageId,
    JPS_ErrorMessage* errorMessage)
{
    assert(handle);
    auto simulation = reinterpret_cast<Simulation*>(handle);
    try {
        return reinterpret_cast<JPS_DirectSteeringProxy>(
            new DirectSteeringProxy(std::get<DirectSteeringProxy>(simulation->Stage(stageId))));
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

bool JPS_Simulation_SetDeltaTime(JPS_Simulation handle, double dt, JPS_ErrorMessage* errorMessage)
{
    assert(handle);
    auto simulation = reinterpret_cast<Simulation*>(handle);
    bool result = false;
    try {
        simulation->Clock().UpdateDT(dt);
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
