/**
 * \file        Simulation.h
 * \date        Dec 15, 2010
 * \version     v0.6
 * \copyright   <2009-2014> Forschungszentrum J?lich GmbH. All rights reserved.
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
#include "routing/GlobalRouter.h"
#include "routing/QuickestPathRouter.h"
#include "routing/DirectionStrategy.h"
#include "routing/DummyRouter.h"
#include "routing/MeshRouter.h"
#include "routing/RoutingEngine.h"
#include "routing/SafestPathRouter.h"
#include "pedestrian/PedDistributor.h"
#include "routing/CognitiveMapRouter.h"
#include "events/EventManager.h"
#include "pedestrian/AgentsSourcesManager.h"

//Forward declarations
//class AgentsSourcesManager;
class EventManager;
class HybridSimulationManager;

class Simulation
{
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
    std::unique_ptr<Building> _building;
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
    /// argument parser
    ArgumentParser _argsParser;
    /// Agents sources manager
    AgentsSourcesManager _agentSrcManager;
    /// hybrid simulation manager
    //HybridSimulationManager
    std::shared_ptr<HybridSimulationManager>_hybridSimManager=nullptr;

public:
    /**
     * Constructor
     */
    Simulation(const ArgumentParser& args);

    /**
     * Destructor
     */
    virtual ~Simulation();

    /**
     * Initialize the number of agents in the simulation
     */
    long GetPedsNumber() const;

    /**
     * Read parameters from the argument parser class.
     */
    bool InitArgs(const ArgumentParser& args);

    /**
     * Update the route of the pedestrians and reassign rooms, in the case a room change happens
     */
    void UpdateRoutesAndLocations();

    /**
     * Perform some initialisation for the simulation.
     * such as writing the headers for the trajectories.
     * @param the maximal number of pedestrian
     */
    void RunHeader(long nPed=-1);

    /**
     * Run the main part of the simulation
     */
    int RunBody(double maxSimTime);

    /**
     * Perform some finalization like writing the
     * footers for the trajectories.
     */
    void RunFooter();

    /**
     * Run a standard simulation
     * @return the total simulated/evacuation time
     */
    int RunStandardSimulation(double maxSimTime);

    /**
     * print some statistics about the simulation
     */
    void PrintStatistics();

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
};

#endif /*SIMULATION_H_*/
