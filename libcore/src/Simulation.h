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

#include "IO/OutputHandler.h"
#include "IO/Trajectories.h"
#include "direction/walking/DirectionStrategy.h"
#include "events/EventManager.h"
#include "general/Configuration.h"
#include "geometry/Building.h"
#include "geometry/GoalManager.h"
#include "geometry/SubRoom.h"
#include "math/OperationalModel.h"
#include "pedestrian/AgentsSourcesManager.h"
#include "pedestrian/PedDistributor.h"
#include "routing/RoutingEngine.h"
#include "routing/global_shortest/GlobalRouter.h"
#include "routing/quickest/QuickestPathRouter.h"
#include "routing/smoke_router/SmokeRouter.h"

class Simulation
{
private:
    /// Max file size 16Mb
    static const size_t _maxFileSize{1 << 24};
    Configuration * _config;
    long _nPeds;
    double _deltaT;
    /// frame rate for the trajectories
    double _fps;
    unsigned int _seed;
    std::shared_ptr<Building> _building;
    /// Force model to use
    std::shared_ptr<OperationalModel> _operationalModel;
    /// Manage all route choices algorithms
    std::shared_ptr<RoutingEngine> _routingEngine;
    /// writing the trajectories to file
    std::unique_ptr<Trajectories> _iod;
    std::unique_ptr<EventManager> _em;
    AgentsSourcesManager _agentSrcManager;
    int _periodic;
    int _maxSimTime;
    /// Will be set if pedestrian sources exist
    bool _gotSources;
    /// Will be set if the configuration contains train constraints
    bool _trainConstraints;
    GoalManager _goalManager;
    fs::path _currentTrajectoriesFile;
    int _countTraj = 0; // count number of TXT trajectories to produce
public:
    /**
     * Constructor
     */
    Simulation(Configuration * args);

    /**
     * Destructor
     */
    virtual ~Simulation();

    /**
     * Initialize the number of agents in the simulation
     */
    long GetPedsNumber() const;

    /**
     * Read parameters from config.
     */
    bool InitArgs();

    /**
     * Update the route of the pedestrians and reassign rooms, in the case a room change happens
     */
    void UpdateRoutesAndLocations();

    /**
     * Update the routes (intermediate destinations) of the pedestrians.
     * Based on the route choice algorithm used, the next doors or the next decision points is set.
     * TODO:
     */
    void UpdateRoutes();

    /**
     * Update the location of the agents and remove the agents from the simulation who have left the building.
     * Locations includes room/subrooms.
     * TODO:
     */
    void UpdateLocations();

    /**
     * Perform some initialisation for the simulation.
     * such as writing the headers for the trajectories.
     * @param the maximal number of pedestrian
     */
    void RunHeader(long nPed = -1);

    /**
     * Run the main part of the simulation
     */
    double RunBody(double maxSimTime);

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
     * Run a standard simulation
     * @return the total simulated/evacuation time
     */
    double RunStandardSimulation(double maxSimTime);

    /**
     * print some statistics about the simulation
     */
    void PrintStatistics(double time);

    /**
     * In case the distance of <ped> to its actual transition is bigger than 0.5 m
     * search in the the history of <ped> and pick up the nearest closest transition
     * @param[in]:  Pedestrian <ped>
     * @param[in]:  distance of <ped> to its actual transition
     * @param[in]:  unique id of that transition
     * @param[out]: nearest closest transition or nullptr if no correction was needed
     **/
    Transition * correctDoorStatistics(const Pedestrian & ped, double distance, int trans_id) const;

    /**
     * @return the agents source manager
     */
    AgentsSourcesManager & GetAgentSrcManager();

    /**
     * Check if any agents are waiting to enter the simulation
     */
    //TODO(KKZ) update doc
    void AddNewAgents();

    /**
     * @return a pointer to the building object
     */
    Building * GetBuilding();

    /**
     * Update the flow for the door that the pedestrian just crossed
     * @param ped
     */
    void UpdateFlowAtDoors(const Pedestrian & ped) const;

    /**
     * Update the refresh ticks for all doors. they count up and measure the age of the tickvalue (ffRouter, quickest)
     *
     */
    void UpdateDoorticks() const;
    int GetMaxSimTime() const;
    void incrementCountTraj();

    bool correctGeometry(std::shared_ptr<Building> building, std::shared_ptr<TrainTimeTable>);

    /**
     * Updates the output filename if the current file exceeds _maxFileSize.
     * Works only for FileFormat::TXT.
     */
    void RotateOutputFile();
    bool TrainTraffic();
};
