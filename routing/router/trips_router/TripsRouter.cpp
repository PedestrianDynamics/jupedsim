//
// Created by Tobias Schrödter on 2018-12-04.
//

#include "TripsRouter.h"
#include <cfloat>
#include <algorithm>

TripsRouter::TripsRouter()
{
     Log->Write("TripsRouter!");
}

TripsRouter::TripsRouter(int id, RoutingStrategy s, Configuration* config) : Router(id, s)
{

}

TripsRouter::~TripsRouter()
{

}

bool TripsRouter::Init(Building* building)
{
     Log->Write("TripsRouter::Init");


     // Create Floorfield for each subroom
          // If goal inside room, start wave from each edge to the outside
     return true;
}

bool TripsRouter::ReInit()
{
     return true;
}

int TripsRouter::FindExit(Pedestrian* p)
{
     // Check if current position is already waiting area
     // yes: set next goal and return findExit(p)


     return 0;
}
