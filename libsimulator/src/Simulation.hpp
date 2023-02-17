/// Copyright © 2012-2022 Forschungszentrum Jülich GmbH
/// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "AgentExitSystem.hpp"
#include "Area.hpp"
#include "GenericAgent.hpp"
#include "Geometry.hpp"
#include "Journey.hpp"
#include "Logger.hpp"
#include "NeighborhoodSearch.hpp"
#include "OperationalDecisionSystem.hpp"
#include "OperationalModel.hpp"
#include "Point.hpp"
#include "SimulationClock.hpp"
#include "StrategicalDesicionSystem.hpp"
#include "TacticalDecisionSystem.hpp"

#include <boost/iterator/zip_iterator.hpp>
#include <chrono>
#include <cstddef>
#include <iterator>
#include <memory>
#include <tuple>
#include <unordered_map>

class Simulation
{
public:
    virtual ~Simulation() = default;
    virtual const SimulationClock& Clock() const = 0;
    /// Advances the simulation by one time step.
    virtual void Iterate() = 0;
    // TODO(kkratz): doc
    virtual Journey::ID AddJourney(std::unique_ptr<Journey>&& journey) = 0;
    virtual uint64_t AddAgent(
        const Point& position,
        const Point& orientation,
        Journey::ID journeyId,
        OperationalModel::ParametersID profileId) = 0;
    virtual void RemoveAgent(uint64_t id) = 0;
    virtual const GenericAgent& Agent(GenericAgent::ID id) const = 0;
    virtual GenericAgent& Agent(GenericAgent::ID id) = 0;
    virtual const std::vector<uint64_t>& RemovedAgents() const = 0;
    virtual size_t AgentCount() const = 0;
    virtual void
    SwitchAgentProfile(GenericAgent::ID agent_id, OperationalModel::ParametersID profile_id) = 0;
    virtual uint64_t Iteration() const = 0;
    virtual const std::vector<GenericAgent>& Agents() const = 0;
};

template <typename ModelType>
class TypedSimulation : public Simulation
{
private:
    SimulationClock _clock;
    StrategicalDecisionSystem _stategicalDecisionSystem{};
    TacticalDecisionSystem _tacticalDecisionSystem{};
    OperationalDecisionSystem<ModelType> _operationalDecisionSystem;
    AgentExitSystem _agentExitSystem{};
    NeighborhoodSearch<std::tuple<GenericAgent*, typename ModelType::Data*>> _neighborhoodSearch{
        2.2};
    std::unique_ptr<RoutingEngine> _routingEngine;
    std::unique_ptr<CollisionGeometry> _geometry;
    std::vector<GenericAgent> _generic_agent_data;
    std::vector<typename ModelType::Data> _modelAgentData;
    std::unique_ptr<Areas> _areas;
    std::vector<uint64_t> _removedAgentsInLastIteration;
    std::unordered_map<Journey::ID, std::unique_ptr<Journey>> _journeys;

public:
    TypedSimulation(
        std::unique_ptr<ModelType> operationalModel,
        std::unique_ptr<CollisionGeometry>&& geometry,
        std::unique_ptr<RoutingEngine>&& routingEngine,
        std::unique_ptr<Areas>&& areas,
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
    Journey::ID AddJourney(std::unique_ptr<Journey>&& journey) override;

    uint64_t AddAgent(
        const Point& position,
        const Point& orientation,
        Journey::ID journeyId,
        OperationalModel::ParametersID profileId) override;

    void RemoveAgent(uint64_t id) override;

    const GenericAgent& Agent(GenericAgent::ID id) const override;

    GenericAgent& Agent(GenericAgent::ID id) override;

    const std::vector<uint64_t>& RemovedAgents() const override;

    size_t AgentCount() const override;

    void SwitchAgentProfile(GenericAgent::ID agent_id, OperationalModel::ParametersID profile_id)
        override;

    uint64_t Iteration() const override { return _clock.Iteration(); }

    const std::vector<GenericAgent>& Agents() const override { return _generic_agent_data; };
};

template <typename T>
TypedSimulation<T>::TypedSimulation(
    std::unique_ptr<T> operationalModel,
    std::unique_ptr<CollisionGeometry>&& geometry,
    std::unique_ptr<RoutingEngine>&& routingEngine,
    std::unique_ptr<Areas>&& areas,
    double dT)
    : _clock(dT)
    , _operationalDecisionSystem(std::move(operationalModel))
    , _routingEngine(std::move(routingEngine))
    , _geometry(std::move(geometry))
    , _areas(std::move(areas))
{
    // TODO(kkratz): Ensure all areas are fully contained inside the walkable area. Otherwise an
    // agent may try to navigate to a point outside the navigation mesh, resulting in an exception.
}

template <typename T>
void TypedSimulation<T>::Iterate()
{
    std::vector<std::tuple<Point, std::tuple<GenericAgent*, typename T::Data*>>> data{};
    data.reserve(_generic_agent_data.size());

    auto modelDataIter = std::begin(_modelAgentData);
    for(auto& agentData : _generic_agent_data) {
        data.push_back(
            std::make_tuple(agentData.pos, std::make_tuple(&agentData, &*modelDataIter)));
        ++modelDataIter;
    }

    _neighborhoodSearch.Update(data);
    _agentExitSystem.Run(*_areas, _generic_agent_data, _removedAgentsInLastIteration);
    _stategicalDecisionSystem.Run(*_areas, _generic_agent_data);
    _tacticalDecisionSystem.Run(*_areas, *_routingEngine, _generic_agent_data);
    _operationalDecisionSystem.Run(
        _clock.dT(),
        _clock.ElapsedTime(),
        _neighborhoodSearch,
        *_geometry,
        _generic_agent_data,
        _modelAgentData);

    _clock.Advance();
    LOG_DEBUG("Iteration done.");
}

template <typename T>
Journey::ID TypedSimulation<T>::AddJourney(std::unique_ptr<Journey>&& journey)
{
    const auto id = journey->Id();
    _journeys.emplace(id, std::move(journey));
    return id;
}

template <typename T>
uint64_t TypedSimulation<T>::AddAgent(
    const Point& position,
    const Point& orientation,
    Journey::ID journeyId,
    OperationalModel::ParametersID profileId)
{
    GenericAgent agent{};
    const auto orientationNormalised = orientation.Normalized();

    agent.orientation = orientationNormalised;
    agent.pos = position;
    agent.parameterProfileId = profileId;

    if(const auto& iter = _journeys.find(journeyId); iter != _journeys.end()) {
        agent.behaviour = std::make_unique<FollowWaypointsBehaviour>(
            dynamic_cast<SimpleJourney*>(iter->second.get()));
    } else {
        throw std::runtime_error(fmt::format("Unknown journey id: {}", journeyId));
    }

    _generic_agent_data.emplace_back(std::move(agent));
    _modelAgentData.emplace_back();
    return _generic_agent_data.back().id.getID();
}

template <typename T>
void TypedSimulation<T>::RemoveAgent(uint64_t id)
{
    const auto iter = std::find_if(
        std::begin(_generic_agent_data), std::end(_generic_agent_data), [id](auto& agent) {
            return agent.id.getID() == id;
        });
    if(iter == std::end(_generic_agent_data)) {
        throw std::runtime_error(fmt::format("Unknown agent id {}", id));
    }
    _generic_agent_data.erase(iter);
    const auto offset = std::distance(std::begin(_generic_agent_data), iter);
    _modelAgentData.erase(std::begin(_modelAgentData) + offset);
}

template <typename T>
const GenericAgent& TypedSimulation<T>::Agent(GenericAgent::ID id) const
{
    const auto iter =
        std::find_if(_generic_agent_data.begin(), _generic_agent_data.end(), [id](auto& ped) {
            return id == ped.id;
        });
    if(iter == _generic_agent_data.end()) {
        throw std::logic_error(fmt::format("Trying to access unknown Agent {}", id));
    }
    return *iter;
}

template <typename T>
GenericAgent& TypedSimulation<T>::Agent(GenericAgent::ID id)
{
    const auto iter =
        std::find_if(_generic_agent_data.begin(), _generic_agent_data.end(), [id](auto& ped) {
            return id == ped.id;
        });
    if(iter == _generic_agent_data.end()) {
        throw std::logic_error(fmt::format("Trying to access unknown Agent {}", id));
    }
    return *iter;
}

template <typename T>
const std::vector<uint64_t>& TypedSimulation<T>::RemovedAgents() const
{
    return _removedAgentsInLastIteration;
}

template <typename T>
size_t TypedSimulation<T>::AgentCount() const
{
    return _generic_agent_data.size();
}

template <typename T>
void TypedSimulation<T>::SwitchAgentProfile(
    GenericAgent::ID agent_id,
    OperationalModel::ParametersID profile_id)
{
    _operationalDecisionSystem.ValidateAgentParameterProfileId(profile_id);
    Agent(agent_id).parameterProfileId = profile_id;
}
