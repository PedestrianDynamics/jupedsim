/// Copyright © 2012-2022 Forschungszentrum Jülich GmbH
/// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "AgentExitSystem.hpp"
#include "Events.hpp"
#include "GenericAgent.hpp"
#include "Geometry.hpp"
#include "Journey.hpp"
#include "Logger.hpp"
#include "NeighborhoodSearch.hpp"
#include "OperationalDecisionSystem.hpp"
#include "OperationalModel.hpp"
#include "Point.hpp"
#include "Polygon.hpp"
#include "SimulationClock.hpp"
#include "SimulationError.hpp"
#include "StageDescription.hpp"
#include "StrategicalDesicionSystem.hpp"
#include "TacticalDecisionSystem.hpp"
#include "TemplateHelper.hpp"
#include "Tracing.hpp"

#include <boost/iterator/zip_iterator.hpp>

#include <chrono>
#include <exception>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <tuple>
#include <unordered_map>
#include <vector>

class Simulation
{
public:
    virtual ~Simulation() = default;
    virtual const SimulationClock& Clock() const = 0;
    virtual void SetTracing(bool on) = 0;
    virtual PerfStats GetLastStats() const = 0;
    /// Advances the simulation by one time step.
    virtual void Iterate() = 0;
    // TODO(kkratz): doc
    virtual Journey::ID AddJourney(const std::vector<StageDescription>& journeyDescription) = 0;
    virtual void RemoveAgent(GenericAgent::ID id) = 0;
    virtual const std::vector<GenericAgent::ID>& RemovedAgents() const = 0;
    virtual size_t AgentCount() const = 0;
    virtual double ElapsedTime() const = 0;
    virtual double DT() const = 0;
    virtual void
    SwitchAgentProfile(GenericAgent::ID agent_id, OperationalModel::ParametersID profile_id) = 0;
    virtual void
    SwitchAgentJourney(GenericAgent::ID agent_id, Journey::ID journey_id, size_t stage_idx) = 0;
    virtual uint64_t Iteration() const = 0;
    virtual std::vector<GenericAgent::ID> AgentsInRange(Point p, double distance) = 0;
    /// Returns IDs of all agents inside the defined polygon
    /// @param polygon Required to be a simple convex polygon with CCW ordering.
    virtual std::vector<GenericAgent::ID> AgentsInPolygon(const std::vector<Point>& polygon) = 0;
    virtual void Notify(Event evt) = 0;
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
    PerfStats _perfStats{};

public:
    TypedSimulation(
        std::unique_ptr<ModelType>&& operationalModel,
        std::unique_ptr<CollisionGeometry>&& geometry,
        std::unique_ptr<RoutingEngine>&& routingEngine,
        double dT);

    ~TypedSimulation() override = default;

    TypedSimulation(const TypedSimulation& other) = delete;

    TypedSimulation& operator=(const TypedSimulation& other) = delete;

    TypedSimulation(TypedSimulation&& other) = delete;

    TypedSimulation& operator=(TypedSimulation&& other) = delete;

    const SimulationClock& Clock() const override { return _clock; }

    void SetTracing(bool status) override { _perfStats.SetEnabled(status); };

    PerfStats GetLastStats() const override { return _perfStats; };

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

    double ElapsedTime() const override;

    double DT() const override;

    void SwitchAgentProfile(GenericAgent::ID agent_id, OperationalModel::ParametersID profile_id)
        override;

    void SwitchAgentJourney(GenericAgent::ID agent_id, Journey::ID journey_id, size_t stage_idx)
        override;

    uint64_t Iteration() const override { return _clock.Iteration(); }

    std::vector<GenericAgent::ID> AgentsInRange(Point p, double distance) override;

    std::vector<GenericAgent::ID> AgentsInPolygon(const std::vector<Point>& polygon) override;

    void Notify(Event evt) override;

    const std::vector<AgentType>& Agents() const { return _agents; };
};

template <typename T>
TypedSimulation<T>::TypedSimulation(
    std::unique_ptr<T>&& operationalModel,
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
    auto t = _perfStats.TraceIterate();
    _agentExitSystem.Run(_agents, _removedAgentsInLastIteration);
    _neighborhoodSearch.Update(_agents);

    for(auto& [_, j] : _journeys) {
        j->Update(_neighborhoodSearch);
    }

    _stategicalDecisionSystem.Run(_journeys, _agents);
    _tacticalDecisionSystem.Run(*_routingEngine, _agents);
    {
        auto t2 = _perfStats.TraceOperationalDecisionSystemRun();
        _operationalDecisionSystem.Run(
            _clock.dT(), _clock.ElapsedTime(), _neighborhoodSearch, *_geometry, _agents);
    }
    _clock.Advance();
}

template <typename T>
Journey::ID TypedSimulation<T>::AddJourney(const std::vector<StageDescription>& journeyDescription)
{
    auto journey = std::make_unique<Journey>(
        journeyDescription, _removedAgentsInLastIteration, *_routingEngine);
    const auto id = journey->Id();
    _journeys.emplace(id, std::move(journey));
    return id;
}

template <typename T>
GenericAgent::ID TypedSimulation<T>::AddAgent(AgentType&& agent)
{
    agent.orientation = agent.orientation.Normalized();
    _operationalDecisionSystem.ValidateAgent(agent, _neighborhoodSearch);

    if(_journeys.count(agent.journeyId) == 0) {
        throw SimulationError("Unknown journey id: {}", agent.journeyId);
    }

    _agents.emplace_back(std::move(agent));
    _neighborhoodSearch.AddAgent(_agents.back());

    return _agents.back().id.getID();
}

template <typename T>
void TypedSimulation<T>::RemoveAgent(GenericAgent::ID id)
{
    const auto iter = std::find_if(
        std::begin(_agents), std::end(_agents), [id](auto& agent) { return agent.id == id; });
    if(iter == std::end(_agents)) {
        throw SimulationError("Unknown agent id {}", id);
    }
    _agents.erase(iter);
    _neighborhoodSearch.RemoveAgent(*iter);
}

template <typename T>
const typename TypedSimulation<T>::AgentType& TypedSimulation<T>::Agent(GenericAgent::ID id) const
{
    const auto iter =
        std::find_if(_agents.begin(), _agents.end(), [id](auto& ped) { return id == ped.id; });
    if(iter == _agents.end()) {
        throw SimulationError("Trying to access unknown Agent {}", id);
    }
    return *iter;
}

template <typename T>
typename TypedSimulation<T>::AgentType& TypedSimulation<T>::Agent(GenericAgent::ID id)
{
    const auto iter =
        std::find_if(_agents.begin(), _agents.end(), [id](auto& ped) { return id == ped.id; });
    if(iter == _agents.end()) {
        throw SimulationError("Trying to access unknown Agent {}", id);
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
double TypedSimulation<T>::ElapsedTime() const
{
    return _clock.ElapsedTime();
}

template <typename T>
double TypedSimulation<T>::DT() const
{
    return _clock.dT();
}

template <typename T>
void TypedSimulation<T>::SwitchAgentProfile(
    GenericAgent::ID agent_id,
    OperationalModel::ParametersID profile_id)
{
    _operationalDecisionSystem.ValidateAgentParameterProfileId(profile_id);
    Agent(agent_id).parameterProfileId = profile_id;
}

template <typename T>
void TypedSimulation<T>::SwitchAgentJourney(
    GenericAgent::ID agent_id,
    Journey::ID journey_id,
    size_t stage_idx)
{
    const auto find_iter = _journeys.find(journey_id);
    if(find_iter == std::end(_journeys)) {
        throw SimulationError("Unknown Journey id {}", journey_id);
    }
    auto& journey = find_iter->second;
    if(stage_idx >= journey->CountStages()) {
        throw SimulationError("Stage index {} for journey {} out of range", stage_idx, journey_id);
    }
    auto& agent = Agent(agent_id);
    agent.journeyId = journey_id;
    agent.currentJourneyStage = stage_idx;
}

template <typename T>
std::vector<GenericAgent::ID> TypedSimulation<T>::AgentsInRange(Point p, double distance)
{
    const auto neighbors = _neighborhoodSearch.GetNeighboringAgents(p, distance);

    std::vector<GenericAgent::ID> neighborIds{};
    neighborIds.reserve(neighbors.size());
    std::transform(
        std::begin(neighbors),
        std::end(neighbors),
        std::back_inserter(neighborIds),
        [](const auto& agent) { return agent.id; });
    return neighborIds;
}

template <typename T>
std::vector<GenericAgent::ID> TypedSimulation<T>::AgentsInPolygon(const std::vector<Point>& polygon)
{
    const Polygon poly{polygon};
    if(!poly.IsConvex()) {
        throw SimulationError("Polygon needs to be simple and convex");
    }
    const auto [p, dist] = poly.ContainingCircle();

    const auto candidates = _neighborhoodSearch.GetNeighboringAgents(p, dist);
    std::vector<GenericAgent::ID> result{};
    result.reserve(candidates.size());
    std::for_each(
        std::begin(candidates), std::end(candidates), [&result, &poly](const auto& agent) {
            if(poly.IsInside(agent.pos)) {
                result.push_back(agent.id);
            }
        });
    return result;
}

template <typename T>
void TypedSimulation<T>::Notify(Event evt)
{
    std::visit(
        [this](auto&& evt) {
            using EvtT = std::decay_t<decltype(evt)>;
            if constexpr(std::is_same_v<EvtT, NotifyWaitingSet>) {
                auto journey = _journeys.find(evt.journeyId);
                if(journey == std::end(_journeys)) {
                    throw SimulationError(
                        "Cannot send event to unknown journey {}", evt.journeyId.getID());
                }
                LOG_DEBUG("Received 'NotifyWaitingSet' evt: {}", evt);
                journey->second->HandleNofifyWaitingSetEvent(evt);
            } else if constexpr(std::is_same_v<EvtT, NotifyQueue>) {
                auto journey = _journeys.find(evt.journeyId);
                if(journey == std::end(_journeys)) {
                    throw SimulationError(
                        "Cannot send event to unknown journey {}", evt.journeyId.getID());
                }
                LOG_DEBUG("Received 'NotifyQueue' evt: {}", evt);
                journey->second->HandleNofifyQueueEvent(evt);
            } else {
                static_assert(always_false_v<EvtT>, "non-exhaustive visitor!");
            }
        },
        evt);
}
