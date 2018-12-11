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

     p->SetExitIndex(17);
     p->SetExitLine(0);

     return 17;
}
