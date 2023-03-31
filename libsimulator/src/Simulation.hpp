/// Copyright © 2012-2022 Forschungszentrum Jülich GmbH
/// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "AgentExitSystem.hpp"
#include "GenericAgent.hpp"
#include "Geometry.hpp"
#include "Journey.hpp"
#include "Logger.hpp"
#include "NeighborhoodSearch.hpp"
#include "OperationalDecisionSystem.hpp"
#include "OperationalModel.hpp"
#include "Point.hpp"
#include "SimulationClock.hpp"
#include "StageDescription.hpp"
#include "StrategicalDesicionSystem.hpp"
#include "TacticalDecisionSystem.hpp"

#include <boost/iterator/zip_iterator.hpp>
#include <chrono>
#include <cstddef>
#include <iterator>
#include <memory>
#include <tuple>
#include <unordered_map>
#include <vector>

class Simulation
{
public:
    virtual ~Simulation() = default;
    virtual const SimulationClock& Clock() const = 0;
    /// Advances the simulation by one time step.
    virtual void Iterate() = 0;
    // TODO(kkratz): doc
    virtual Journey::ID AddJourney(const std::vector<StageDescription>& journeyDescription) = 0;
    virtual void RemoveAgent(GenericAgent::ID id) = 0;
    virtual const std::vector<GenericAgent::ID>& RemovedAgents() const = 0;
    virtual size_t AgentCount() const = 0;
    virtual void
    SwitchAgentProfile(GenericAgent::ID agent_id, OperationalModel::ParametersID profile_id) = 0;
    virtual uint64_t Iteration() const = 0;
};

template <typename ModelType>
class TypedSimulation : public Simulation
{
    using AgentType = typename ModelType::Data;

private:
    SimulationClock _clock;
    StrategicalDecisionSystem<AgentType> _stategicalDecisionSystem{};
    TacticalDecisionSystem<AgentType> _tacticalDecisionSystem{};
    OperationalDecisionSystem<ModelType> _operationalDecisionSystem;
    AgentExitSystem<AgentType> _agentExitSystem{};
    NeighborhoodSearch<AgentType> _neighborhoodSearch{2.2};
    std::unique_ptr<RoutingEngine> _routingEngine;
    std::unique_ptr<CollisionGeometry> _geometry;
    std::vector<AgentType> _agents;
    std::vector<GenericAgent::ID> _removedAgentsInLastIteration;
    std::unordered_map<Journey::ID, std::unique_ptr<Journey>> _journeys;

public:
    TypedSimulation(
        std::unique_ptr<ModelType> operationalModel,
        std::unique_ptr<CollisionGeometry>&& geometry,
        std::unique_ptr<RoutingEngine>&& routingEngine,
        double dT);

    ~TypedSimulation() override = default;

    TypedSimulation(const TypedSimulation& other) = delete;

    TypedSimulation& operator=(const TypedSimulation& other) = delete;

    TypedSimulation(TypedSimulation&& other) = delete;

    TypedSimulation& operator=(TypedSimulation&& other) = delete;

    const SimulationClock& Clock() const override { return _clock; }

    /// Advances the simulation by one time step.
    void Iterate() override;

    // TODO(kkratz): doc
    Journey::ID AddJourney(const std::vector<StageDescription>& journeyDescription) override;

    GenericAgent::ID AddAgent(AgentType&& agent);

    void RemoveAgent(GenericAgent::ID) override;

    const AgentType& Agent(GenericAgent::ID id) const;

    AgentType& Agent(GenericAgent::ID id);

    const std::vector<GenericAgent::ID>& RemovedAgents() const override;

    size_t AgentCount() const override;

    void SwitchAgentProfile(GenericAgent::ID agent_id, OperationalModel::ParametersID profile_id)
        override;

    uint64_t Iteration() const override { return _clock.Iteration(); }

    const std::vector<AgentType>& Agents() const { return _agents; };
};

template <typename T>
TypedSimulation<T>::TypedSimulation(
    std::unique_ptr<T> operationalModel,
    std::unique_ptr<CollisionGeometry>&& geometry,
    std::unique_ptr<RoutingEngine>&& routingEngine,
    double dT)
    : _clock(dT)
    , _operationalDecisionSystem(std::move(operationalModel))
    , _routingEngine(std::move(routingEngine))
    , _geometry(std::move(geometry))
{
    // TODO(kkratz): Ensure all areas are fully contained inside the walkable area. Otherwise an
    // agent may try to navigate to a point outside the navigation mesh, resulting in an exception.
}

template <typename T>
void TypedSimulation<T>::Iterate()
{
    _neighborhoodSearch.Update(_agents);
    _agentExitSystem.Run(_agents, _removedAgentsInLastIteration);
    _stategicalDecisionSystem.Run(_journeys, _agents);
    _tacticalDecisionSystem.Run(*_routingEngine, _agents);
    _operationalDecisionSystem.Run(
        _clock.dT(), _clock.ElapsedTime(), _neighborhoodSearch, *_geometry, _agents);

    _clock.Advance();
    LOG_DEBUG("Iteration done.");
}

template <typename T>
Journey::ID TypedSimulation<T>::AddJourney(const std::vector<StageDescription>& journeyDescription)
{
    auto journey = std::make_unique<Journey>(journeyDescription, _removedAgentsInLastIteration);
    const auto id = journey->Id();
    _journeys.emplace(id, std::move(journey));
    return id;
}

template <typename T>
GenericAgent::ID TypedSimulation<T>::AddAgent(AgentType&& agent)
{
    agent.orientation = agent.orientation.Normalized();

    if(_journeys.count(agent.journeyId) == 0) {
        throw std::runtime_error(fmt::format("Unknown journey id: {}", agent.journeyId));
    }

    _agents.emplace_back(std::move(agent));
    return _agents.back().id.getID();
}

template <typename T>
void TypedSimulation<T>::RemoveAgent(GenericAgent::ID id)
{
    const auto iter = std::find_if(
        std::begin(_agents), std::end(_agents), [id](auto& agent) { return agent.id == id; });
    if(iter == std::end(_agents)) {
        throw std::runtime_error(fmt::format("Unknown agent id {}", id));
    }
    _agents.erase(iter);
}

template <typename T>
const typename TypedSimulation<T>::AgentType& TypedSimulation<T>::Agent(GenericAgent::ID id) const
{
    const auto iter =
        std::find_if(_agents.begin(), _agents.end(), [id](auto& ped) { return id == ped.id; });
    if(iter == _agents.end()) {
        throw std::logic_error(fmt::format("Trying to access unknown Agent {}", id));
    }
    return *iter;
}

template <typename T>
typename TypedSimulation<T>::AgentType& TypedSimulation<T>::Agent(GenericAgent::ID id)
{
    const auto iter =
        std::find_if(_agents.begin(), _agents.end(), [id](auto& ped) { return id == ped.id; });
    if(iter == _agents.end()) {
        throw std::logic_error(fmt::format("Trying to access unknown Agent {}", id));
    }
    return *iter;
}

template <typename T>
const std::vector<GenericAgent::ID>& TypedSimulation<T>::RemovedAgents() const
{
    return _removedAgentsInLastIteration;
}

template <typename T>
size_t TypedSimulation<T>::AgentCount() const
{
    return _agents.size();
}

template <typename T>
void TypedSimulation<T>::SwitchAgentProfile(
    GenericAgent::ID agent_id,
    OperationalModel::ParametersID profile_id)
{
    _operationalDecisionSystem.ValidateAgentParameterProfileId(profile_id);
    Agent(agent_id).parameterProfileId = profile_id;
}
