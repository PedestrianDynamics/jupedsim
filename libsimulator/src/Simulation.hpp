/**
 * \file        Simulation.h
 * \date        Dec 15, 2010
 * \version     v0.7
 * \copyright   <2009-2015> Forschungszentrum J?lich GmbH. All rights reserved.
 *
 * \section License
 * This file is part of JuPedSim.
 *
 * JuPedSim is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * JuPedSim is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with JuPedSim. If not, see <http://www.gnu.org/licenses/>.
 *
 * \section Description
 * The Simulation class represents a simulation of pedestrians
 * based on a certain model in a specific scenario. A simulation is defined by
 * various parameters and functions.
 *
 *
 **/
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
        double v0,
        Journey::ID journeyId,
        OperationalModel::ParametersID profileId);

    void RemoveAgent(uint64_t id);

    Agent* AgentPtr(Agent::UID id) const;

    const std::vector<uint64_t>& RemovedAgents() const;

    size_t AgentCount() const;

    uint64_t Iteration() const { return _clock.Iteration(); }

    const std::vector<std::unique_ptr<Agent>>& Agents() const { return _agents; };
};
