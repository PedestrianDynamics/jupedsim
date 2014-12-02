/**
 * \file        DummyRouter.h
 * \date        Aug 7, 2012
 * \version     v0.6
 * \copyright   <2009-2014> Forschungszentrum Jülich GmbH. All rights reserved.
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


#ifndef DUMMYROUTER_H_
#define DUMMYROUTER_H_

#include "Router.h"

class DummyRouter: public Router {
private:
     Building* _building;
public:
     DummyRouter();
     DummyRouter(int id, RoutingStrategy s);
     virtual ~DummyRouter();

     virtual int FindExit(Pedestrian* p);
     virtual bool Init(Building* b);

};

#endif /* DUMMYROUTER_H_ */
