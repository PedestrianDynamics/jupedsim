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
#ifndef SIMULATION_H_
#define SIMULATION_H_

#include "general/ArgumentParser.h"
#include "geometry/Building.h"
#include "geometry/SubRoom.h"
#include "IO/OutputHandler.h"
#include "IO/IODispatcher.h"
#include "math/OperationalModel.h"
#include "math/ODESolver.h"
#include "router/global_shortest/GlobalRouter.h"
#include "router/quickest/QuickestPathRouter.h"
#include "direction/walking/DirectionStrategy.h"
#include "router/RoutingEngine.h"
#include "pedestrian/PedDistributor.h"
#include "router/smoke_router/SmokeRouter.h"
#include "events/EventManager.h"
#include "pedestrian/AgentsSourcesManager.h"
#include "general/Configuration.h"

class EventManager;
class GoalManager;

class Simulation {
private:
    ///Number of pedestrians in the simulation
    long _nPeds;
    ///Maximum simulation time
    //double _tmax;
    /// time step
    double _deltaT;
    /// frame rate for the trajectories
    double _fps;
    ///seed using for the random number generator
    unsigned int _seed;
    /// building object
    std::shared_ptr<Building> _building;
    /// Force model to use
    std::shared_ptr<OperationalModel> _operationalModel;
    /// Manage all route choices algorithms
    std::shared_ptr<RoutingEngine> _routingEngine;
    /// differential equation solver
    ODESolver* _solver;
    /// writing the trajectories to file
    IODispatcher* _iod;
    /// EventManager
    EventManager* _em;
    /// config
    Configuration* _config;
    /// Agents sources manager
    AgentsSourcesManager _agentSrcManager;
    /// hybrid simulation manager
    //HybridSimulationManager
    int _periodic;
     int _maxSimTime;

    bool _gotSources; // is true if we got some sources. Otherwise, false.
     bool _trainConstraints; // true if inifile has some train constraints

    // bool _printPB; // print progressbar

    ///
    GoalManager* _goalManager;

public:
    /**
     * Constructor
     */
    Simulation(Configuration* args);

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
     * Perform some finalization like writing the
     * footers for the trajectories.
     */
    void RunFooter();

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
     Transition*  correctDoorStatistics(const Pedestrian& ped, double distance, int trans_id) const;

    /**
     * @return the agents source manager
     */
    AgentsSourcesManager& GetAgentSrcManager();

    /**
     * Check if any agents are waiting to enter the simulation
     */
    void ProcessAgentsQueue();

    /**
     * @return a pointer to the building object
     */
    Building* GetBuilding();

    /**
     * Update the flow for the door that the pedestrian just crossed
     * @param ped
     */
    void UpdateFlowAtDoors(const Pedestrian& ped) const;

     /**
     * Update the refresh ticks for all doors. they count up and measure the age of the tickvalue (ffRouter, quickest)
     *
     */
     void UpdateDoorticks() const;
     int GetMaxSimTime() const;
     void  incrementCountTraj();

     bool correctGeometry(std::shared_ptr<Building> building,  std::shared_ptr<TrainTimeTable>);
     void WriteTrajectories();
     bool TrainTraffic();

     int _countTraj=0; // count number of TXT trajectories to produce
     double _maxFileSize; // in MB
};

#endif /*SIMULATION_H_*/
