/// Copyright © 2012-2022 Forschungszentrum Jülich GmbH
/// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "Agent.hpp"
#include "AgentExitSystem.hpp"
#include "Area.hpp"
#include "Geometry.hpp"
#include "Journey.hpp"
#include "NeighborhoodSearch.hpp"
#include "OperationalDecisionSystem.hpp"
#include "OperationalModel.hpp"
#include "SimulationClock.hpp"
#include "StrategicalDesicionSystem.hpp"
#include "TacticalDecisionSystem.hpp"

#include <chrono>
#include <cstddef>
#include <memory>
#include <unordered_map>

class Simulation
{
private:
    SimulationClock _clock;
    StrategicalDecisionSystem _stategicalDecisionSystem{};
    TacticalDecisionSystem _tacticalDecisionSystem{};
    OperationalDecisionSystem _operationalDecisionSystem;
    AgentExitSystem _agentExitSystem{};
    NeighborhoodSearch _neighborhoodSearch{2.2};
    std::unique_ptr<RoutingEngine> _routingEngine;
    std::unique_ptr<CollisionGeometry> _geometry;
    std::vector<std::unique_ptr<Agent>> _agents;
    std::unique_ptr<Areas> _areas;
    std::vector<uint64_t> _removedAgentsInLastIteration;
    std::unordered_map<Journey::ID, std::unique_ptr<Journey>> _journeys;
    bool _eventProcessed{false};

public:
    Simulation(
        std::unique_ptr<OperationalModel> operationalModel,
        std::unique_ptr<CollisionGeometry>&& geometry,
        std::unique_ptr<RoutingEngine>&& routingEngine,
        std::unique_ptr<Areas>&& areas,
        double dT);

    ~Simulation() = default;

    Simulation(const Simulation& other) = delete;

    Simulation& operator=(const Simulation& other) = delete;

    Simulation(Simulation&& other) = delete;

    Simulation& operator=(Simulation&& other) = delete;

    const SimulationClock& Clock() const { return _clock; }

    /// Advances the simulation by one time step.
    void Iterate();

    // TODO(kkratz): doc
    Journey::ID AddJourney(std::unique_ptr<Journey>&& journey);

    uint64_t AddAgent(
        const Point& position,
        const Point& orientation,
        double Av,
        double AMin,
        double BMax,
        double BMin,
        Journey::ID journeyId,
        OperationalModel::ParametersID profileId);

    void RemoveAgent(uint64_t id);

    Agent* AgentPtr(Agent::ID id) const;

    const std::vector<uint64_t>& RemovedAgents() const;

    size_t AgentCount() const;

    void SwitchAgentProfile(Agent::ID agent_id, OperationalModel::ParametersID profile_id) const;

    uint64_t Iteration() const { return _clock.Iteration(); }

    const std::vector<std::unique_ptr<Agent>>& Agents() const { return _agents; };
};
