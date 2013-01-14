/*Simulation.h:
  The Simulation class represents a simulation of pedestrians
  based on a certain model in a specific scenario. A simulation is defined by
  various parameters and functions.
  Copyright (C) <2009-2010>  <Jonas Mehlich and Mohcine Chraibi>

  This file is part of OpenPedSim.

  OpenPedSim is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  any later version.

  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  OpenPedSim is distributed in the hope that it will be useful,
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Foobar. If not, see <http://www.gnu.org/licenses/>.
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
#include "routing/RoutingEngine.h"
#include "pedestrian/PedDistributor.h"


extern OutputHandler* Log;

class Simulation {
private:
    int pActionPt; // on or off
    int pNPeds; // number of pedestrians
    double pTmax; // Maximale Simulationszeit
    double pDt; // Zeitschritt
    double fps; // framerate
    Building* pBuilding; // Geometrie mit Rooms, SubRooms und Wänden
    PedDistributor* pDistribution; // verteilt die Fußgänger zu Beginn der Simulation
    DirectionStrategy* pDirection; // gibt die Richtungswahl zum Ziel an
    ForceModel* pModel; // das verwendete Kraftmodell (im Moment nur GCFM)
    ODESolver* pSolver; // Löser für die ODE
    IODispatcher* iod;
    Trajectories* pTrajectories;

public:
    // Konstruktor
    Simulation();
    virtual ~Simulation();
    // Setter-Funktionen
    int SetNPeds(int i);
    // Getter-Funktionen
    int GetNPeds() const;
    int GetNPedsGlobal() const;
    Building* GetBuilding() const;
    // Sonstige-Funktionen
    void InitArgs(ArgumentParser *args);
    int InitSimulation();
    int RunSimulation();
    void Update(); // update the complete system
    void DistributeDestinations(); //assign the pedestrians their final destinations
    void InitRoutineClearing(); // set some parameters specific to routine clearing

};

#endif /*SIMULATION_H_*/
