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

#include "AgentExitSystem.hpp"
#include "Geometry.hpp"
#include "IO/OutputHandler.hpp"
#include "IO/Trajectories.hpp"
#include "OperationalDecisionSystem.hpp"
#include "RoutingEngine.hpp"
#include "SimulationClock.hpp"
#include "StrategicalDesicionSystem.hpp"
#include "TacticalDecisionSystem.hpp"
#include "general/Configuration.hpp"
#include "geometry/Building.hpp"
#include "geometry/GoalManager.hpp"
#include "geometry/SubRoom.hpp"
#include "math/OperationalModel.hpp"
#include "neighborhood/NeighborhoodSearch.hpp"
#include "pedestrian/AgentsSourcesManager.hpp"
#include "pedestrian/PedDistributor.hpp"
#include "pedestrian/Pedestrian.hpp"

#include <chrono>
#include <cstddef>
#include <memory>

class Simulation
{
private:
    Configuration* _config;
    SimulationClock _clock;
    StrategicalDecisionSystem _stategicalDecisionSystem{};
    TacticalDecisionSystem _tacticalDecisionSystem{};
    OperationalDecisionSystem _operationalDecisionSystem;
    AgentExitSystem _agentExitSystem{};

    /// frame rate for the trajectories
    double _fps{1.0};
    unsigned int _seed{8091983};
    NeighborhoodSearch _neighborhoodSearch;
    std::unique_ptr<Building> _building;
    std::unique_ptr<RoutingEngine> _routingEngine;
    std::unique_ptr<Geometry> _geometry;
    std::vector<std::unique_ptr<Pedestrian>> _agents;
    std::map<Area::Id, Area> _areas;
    bool _eventProcessed{false};

public:
    Simulation(
        Configuration* args,
        std::unique_ptr<Building>&& building,
        std::unique_ptr<Geometry>&& geometry,
        std::unique_ptr<RoutingEngine>&& routingEngine);

    ~Simulation() = default;

    Simulation(const Simulation& other) = delete;

    Simulation& operator=(const Simulation& other) = delete;

    Simulation(Simulation&& other) = delete;

    Simulation& operator=(Simulation&& other) = delete;

    const SimulationClock& Clock() const { return _clock; }
    double Fps() const { return _fps; }

    /// Advances the simulation by one time step.
    void Iterate();

    void AddAgent(std::unique_ptr<Pedestrian>&& agent);

    void AddAgents(std::vector<std::unique_ptr<Pedestrian>>&& agents);

    void RemoveAgents(std::vector<Pedestrian::UID> ids);

    Pedestrian& Agent(Pedestrian::UID id) const;

    const std::vector<std::unique_ptr<Pedestrian>>& Agents() const;

    size_t GetPedsNumber() const;

    void OpenDoor(int doorId);

    void TempCloseDoor(int doorId);

    void CloseDoor(int doorId);

    void ResetDoor(int doorId);

    void ActivateTrain(
        int trainId,
        int trackId,
        const TrainType& type,
        double startOffset,
        bool reversed);

    void DeactivateTrain(int trainId, int trackId);

    /**
     * Read parameters from config.
     */
    bool InitArgs();

    /**
     * print some statistics about the simulation
     */
    void PrintStatistics(double time);

private:
    /**
     * Update the route of the pedestrians and reassign rooms, in the case a room change happens
     */
    void UpdateLocations();
};
