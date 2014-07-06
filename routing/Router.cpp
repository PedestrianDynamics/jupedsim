/**
 * \file        Router.cpp
 * \date        Nov 11, 2010
 * \version     v0.5
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
 *
 *
 **/


#include "Router.h"
#include "../geometry/Building.h"
#include "../pedestrian/Pedestrian.h"
#include "../IO/OutputHandler.h"

using namespace std;


Router::Router()
{
     _trips = vector<vector<int> >();
     _finalDestinations = vector<int>();
     _id=-1;
     _strategy=ROUTING_UNDEFINED;
}

Router::~Router()
{
}

const vector<int> Router::GetTrip(int index) const
{
     if ((index >= 0) && (index < (int) _trips.size()))
          return _trips[index];
     else {
          char tmp[CLENGTH];
          sprintf(tmp, "ERROR: \tWrong 'index' [%d] > [%d] in Routing::GetTrip()",
                  index, int(_trips.size()));
          Log->Write(tmp);
          exit(EXIT_FAILURE);
     }
}

void Router::AddTrip(vector<int> trip)
{
     _trips.push_back(trip);
}

void Router::AddFinalDestinationID(int id)
{
     _finalDestinations.push_back(id);
}

const vector<int> Router::GetFinalDestinations() const
{
     return _finalDestinations;
}

void Router::SetID(int id)
{
     _id=id;
}

int Router::GetID() const
{
     return _id;
}

void Router::SetStrategy(RoutingStrategy strategy)
{
     _strategy=strategy;
}

RoutingStrategy Router::GetStrategy() const
{
     return _strategy;
}

void Router::WriteToErrorLog() const
{
     //TODO
//      for (map<int, Crossing*>::const_iterator iter = pCrossings.begin();
//                      iter != pCrossings.end(); ++iter) {
//              iter->second->WriteToErrorLog();
//      }
//      for (map<int, Transition*>::const_iterator iter = pTransitions.begin();
//                      iter != pTransitions.end(); ++iter) {
//              iter->second->WriteToErrorLog();
//      }
//      for (map<int, Hline*>::const_iterator iter = pHlines.begin();
//                      iter != pHlines.end(); ++iter) {
//              iter->second->WriteToErrorLog();
//      }
}

