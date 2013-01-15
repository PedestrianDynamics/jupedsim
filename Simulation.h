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

#ifndef ANALYSIS_H_
#define ANALYSIS_H_

#include "general/ArgumentParser.h"
#include "geometry/Building.h"
#include "IO/OutputHandler.h"
#include "IO/IODispatcher.h"


extern OutputHandler* Log;

class Analysis {
private:
    int pActionPt;
    int pNPeds;
    double pTmax;
    double pDt;
    double fps;
    Building* pBuilding;
    IODispatcher* iod;
    Trajectories* pTrajectories;

public:

    Analysis();
    virtual ~Analysis();

    int SetNPeds(int i);
    int GetNPeds() const;
    int GetNPedsGlobal() const;
    Building* GetBuilding() const;

    void InitArgs(ArgumentParser *args);
    int InitAnalysis();
    int RunAnalysis();

};

#endif /*ANALYSIS_H_*/
