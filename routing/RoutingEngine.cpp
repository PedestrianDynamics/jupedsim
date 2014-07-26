/**
 * \file        RoutingEngine.cpp
 * \date        Jan 10, 2013
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


#include "RoutingEngine.h"
#include "../pedestrian/Pedestrian.h"

using namespace std;


RoutingEngine::RoutingEngine()
{

}

RoutingEngine::~RoutingEngine()
{
     for(unsigned int r=0; r<_routersCollection.size(); r++) {
          delete _routersCollection[r];
     }
     _routersCollection.clear();
}

void RoutingEngine::AddFinalDestinationID(int id)
{
     for(unsigned int r=0; r<_routersCollection.size(); r++) {
          _routersCollection[r]->AddFinalDestinationID(id);
     }
}

void RoutingEngine::FindRoute(Pedestrian* ped)
{
     ped->FindRoute();
}

void RoutingEngine::AddRouter(Router* router)
{
     for(unsigned int r=0; r<_routersCollection.size(); r++) {
          if(_routersCollection[r]->GetStrategy()==router->GetStrategy()) {
               Log->Write("ERROR: \tDuplicate router found with 'id' [%d].",router->GetID());
               Log->Write("ERROR: \tDouble check your configuration files");
               exit(EXIT_FAILURE);
          }
     }
     _routersCollection.push_back(router);
}

const vector<string> RoutingEngine::GetTrip(int index) const
{
     if ((index >= 0) && (index < (int) _tripsCollection.size()))
          return _tripsCollection[index];
     else {
          char tmp[CLENGTH];
          sprintf(tmp, "ERROR: \tWrong 'index' [%d] > [%d] in Routing::GetTrip()",
                  index, int(_tripsCollection.size()));
          Log->Write(tmp);
          exit(EXIT_FAILURE);
     }
}

Router* RoutingEngine::GetRouter(RoutingStrategy strategy) const
{
     for(unsigned int r=0; r<_routersCollection.size(); r++) {
          if(_routersCollection[r]->GetStrategy()==strategy)
               return _routersCollection[r];
     }
     //Log->Write("ERROR: \t Could not Find any router with ID:  [%d].",strategy);
     //exit(EXIT_FAILURE);
     return (Router*) NULL;
}

Router* RoutingEngine::GetRouter(int id) const
{
     for(unsigned int r=0; r<_routersCollection.size(); r++) {
          if(_routersCollection[r]->GetID()==id)
               return _routersCollection[r];
     }
     return (Router*) NULL;
}

void RoutingEngine::AddTrip(vector<string> trip)
{
     _tripsCollection.push_back(trip);
}

void RoutingEngine::Init(Building* building)
{
     for(unsigned int r=0; r<_routersCollection.size(); r++) {
          _routersCollection[r]->Init(building);
     }
}
