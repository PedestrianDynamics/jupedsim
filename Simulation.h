/**
 * @file    Simulation.h
 * @date Created on: Dec 15, 2010
 * Copyright (C) <2009-2011>
 *
 * @section LICENSE
 * This file is part of JuPedSim.
 *
 * JuPedSim is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * JuPedSim is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with JuPedSim. If not, see <http://www.gnu.org/licenses/>.
 *
 * \section description
 * The Simulation class represents a simulation of pedestrians
 * based on a certain model in a specific scenario. A simulation is defined by
 * various parameters and functions.
 *
 */

#ifndef SIMULATION_H_
#define SIMULATION_H_

#include "general/ArgumentParser.h"
#include "geometry/Building.h"
#include "IO/OutputHandler.h"
#include "IO/IODispatcher.h"
#include "math/ForceModel.h"
#include "math/ODESolver.h"
#include "routing/GlobalRouter.h"
#include "routing/QuickestPathRouter.h"
#include "routing/DirectionStrategy.h"
#include "routing/DummyRouter.h"
#include "routing/GraphRouter.h"
#include "routing/NavMesh.h"
#include "routing/MeshRouter.h"
#include "routing/RoutingEngine.h"
#include "routing/SafestPathRouter.h"
#include "pedestrian/PedDistributor.h"
#include "events/EventManager.h" //neuer Header fuer die Events


extern OutputHandler* Log;

class Simulation {
private:
	///Number of pedestrians in the simulation
    int _nPeds;
    ///Maximum simulation time
    double _tmax;
    /// time step
    double _deltaT;
    /// frame rate for the trajectories
    double _fps;
    ///seed using for the random number generator
    unsigned int _seed;
    /// building object
    Building* _building;
    ///initial distribution of the pedestrians
    PedDistributor* _distribution;
    /// door crossing strategy for the pedestrians
    DirectionStrategy* _direction;
    /// Force model to use
    ForceModel* _model;
    /// differential equation solver
    ODESolver* _solver;
    /// writing the trajectories to file
    IODispatcher* _iod;
    ///new: EventManager
    EventManager* _em;


public:
    Simulation();
    virtual ~Simulation();

    /**
     * Initialize the number of agents in the simulation
     */
    void SetPedsNumber(int i);

    /**
     * Initialize the number of agents in the simulation
     */
    int GetPedsNumber() const;

    /**
     * Returns the number of agents when running on a distributed system (MPI)
     * NOT IMPLEMENTED
     */
    int GetNPedsGlobal() const;

    /**
     * @return the building object containing all geometry elements
     */
    Building* GetBuilding() const;

    /**
     * Read parameters from the argument parser class.
     */
    void InitArgs(ArgumentParser *args);

    /**
     *
     * @return the total simulated/evacuation time
     */
    int RunSimulation();

    /**
     * Update the pedestrians states: positions, velocity, route
     */
    void Update();

};

#endif /*SIMULATION_H_*/
