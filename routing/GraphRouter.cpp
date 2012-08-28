/*
 * GraphRouter.cpp
 *
 *  Created on: Aug 20, 2012
 *      Author: David Haensel
 */

#include "GraphRouter.h"


/******************************
 * GraphGrouter Methods
 *****************************/


GraphRouter::GraphRouter() 
{

}

GraphRouter::~GraphRouter() 
{

}

int GraphRouter::FindExit(Pedestrian* p) 
{
  //if the pedestrian has a destination in the actual subroom, there is nothing to do
  if(p->GetNextDestination() != -1) {
    //TODO check if the destination ist still the right one!
    return 1;
  } else {
    int last_dest = p->GetLastDestination();
    //If there was a previous destination the routing starts from there!
    if(last_dest != -1) {
      SubRoom * sub = building->GetRoom(p->GetRoomID())->GetSubRoom(p->GetSubRoomID());
      Crossing * next_dest = (*graph).GetNextDestination(last_dest);
      //check if the next destination is in the right subroom. sometimes pedestrians are pushed back in the other subroom.
      if(next_dest->IsInRoom(sub->GetRoomID()) && next_dest->IsInSubRoom(sub->GetSubRoomID())) {
	std::cout << "new route for "<< p->GetPedIndex() << std::endl;
	p->SetExitIndex(next_dest->GetIndex());
	
	p->SetExitLine(next_dest);
	return 1;
      }
    } else {
      // get next destination for person in subroom (in the subroom not next to a crossing) 
      Crossing * exit = graph->GetNextDestination(p);

      p->SetExitIndex(exit->GetIndex());
      p->SetExitLine(exit);
      return 1;
    }
  }
}

void GraphRouter::Init(Building* b) 
{
  Log->write("ERROR: Router is not ready to use yet");
  building = b;
  graph = new RoutingGraph(GetAllTransitions(), GetAllCrossings());
}
