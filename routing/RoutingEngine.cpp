/*
 * RoutingEngine.cpp
 *
 *  Created on: Jan 10, 2013
 *      Author: Ulrich Kemloh
 */

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
                  index, _tripsCollection.size());
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
