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

#include "IO/OutputHandler.hpp"
#include "IO/Trajectories.hpp"
#include "SimulationClock.hpp"
#include "direction/walking/DirectionStrategy.hpp"
#include "events/Event.hpp"
#include "events/EventManager.hpp"
#include "events/OldEventManager.hpp"
#include "general/Configuration.hpp"
#include "geometry/Building.hpp"
#include "geometry/GoalManager.hpp"
#include "geometry/SubRoom.hpp"
#include "math/OperationalModel.hpp"
#include "pedestrian/AgentsSourcesManager.hpp"
#include "pedestrian/PedDistributor.hpp"
#include "pedestrian/Pedestrian.hpp"
#include "routing/RoutingEngine.hpp"
#include "routing/global_shortest/GlobalRouter.hpp"

#include <chrono>
#include <cstddef>
#include <memory>

class Simulation
{
private:
    Configuration * _config;
    SimulationClock _clock;
    /// frame rate for the trajectories
    double _fps{1.0};
    unsigned int _seed{8091983};
    std::unique_ptr<Building> _building;
    /// Manage all route choices algorithms
    std::unique_ptr<RoutingEngine> _routingEngine;
    /// Force model to use
    std::unique_ptr<OperationalModel> _operationalModel;
    /// writing the trajectories to file
    std::unique_ptr<OldEventManager> _old_em{};
    fs::path _currentTrajectoriesFile;
    std::vector<std::unique_ptr<Pedestrian>> _agents;
    EventManager _em;

public:
    explicit Simulation(
        Configuration * args,
        std::unique_ptr<Building> && building,
        std::unique_ptr<RoutingEngine> && routingEngine,
        std::unique_ptr<OperationalModel> && operationalModel);

    ~Simulation() = default;

    Simulation(const Simulation & other) = delete;

    Simulation & operator=(const Simulation & other) = delete;

    Simulation(Simulation && other) = delete;

    Simulation & operator=(Simulation && other) = delete;

    const SimulationClock & Clock() const { return _clock; }
    double Fps() const { return _fps; }

    /// Advances the simulation by one time step.
    void Iterate();

    void AddAgent(std::unique_ptr<Pedestrian> && agent);

    void AddAgents(std::vector<std::unique_ptr<Pedestrian>> && agents);

    void RemoveAgents(std::vector<Pedestrian::UID> ids);

    Pedestrian & Agent(Pedestrian::UID id) const;

    const std::vector<std::unique_ptr<Pedestrian>> & Agents() const;

    size_t GetPedsNumber() const;

    void AddEvent(Event event);
    void AddEvents(std::vector<Event> events);


    /**
     * Read parameters from config.
     */
    bool InitArgs();

    /**
     * Update the route of the pedestrians and reassign rooms, in the case a room change happens
     */
    void UpdateLocations();

    /**
     * Update the routes (intermediate destinations) of the pedestrians.
     * Based on the route choice algorithm used, the next doors or the next decision points is set.
     */
    void UpdateRoutes();

    /**
     * Perform some initialisation for the simulation.
     * such as writing the headers for the trajectories.
     * @param the maximal number of pedestrian
     */
    void RunHeader(long nPed, TrajectoryWriter & writer);

    /**
     * Copy all Input Files used to the output path.
     *
     * This backs up the input files and makes reproducible results possible.
     */
    void CopyInputFilesToOutPath();

    /**
     * Helper function to copy a specific file to the output path.
     *
     * @param file File to be copied.
     */
    void CopyInputFileToOutPath(fs::path file);

    void UpdateOutputIniFile();
    void UpdateOutputGeometryFile();
    /**
     * Updates the paths to external files in the ini and geometry file in output path.
     */
    void UpdateOutputFiles();

    /**
     * print some statistics about the simulation
     */
    void PrintStatistics(double time);
};
