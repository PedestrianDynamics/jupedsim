/*
 * GraphRouter.cpp
 *
 *  Created on: Aug 20, 2012
 *      Author: David Haensel
 */

#include "GraphRouter.h"
GraphRouter::GraphRouter() 
{

}

GraphRouter::~GraphRouter() 
{

}

int GraphRouter::FindExit(Pedestrian* p) 
{
    SubRoom * sub =  building->GetRoom(p->GetRoomID())->GetSubRoom(p->GetSubRoomID());

    
    p->SetExitIndex(1);

    p->SetExitLine(GetAllCrossings()[1]);
    return 1;
}

void GraphRouter::Init(Building* b) 
{
    Log->write("ERROR: Router is not ready to use yet");
    building = b;
    RoutingGraph graph(GetAllTransitions(), GetAllCrossings());
  
  
}


