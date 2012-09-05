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
    //TODO check if the next destination is a hline!
    Hline * hline = dynamic_cast<Hline*>(graph->GetVertex(p->GetLastDestination())->nav_line);
    if(hline) {
      if(hline->DistTo(p->GetPos()) < EPS) {
	//std::cout << "new route from HLINE" << std::endl; 
	NavLine * next_dest = graph->GetNextDestination(p->GetLastDestination());
	p->SetExitIndex(next_dest->GetUniqueID());
      	p->SetExitLine(next_dest);
      	return 1;
      }
    }
    return 1;
  } else {
    int last_dest = p->GetLastDestination();
    //If there was a previous destination the routing starts from there!
    if(last_dest != -1) {
      SubRoom * sub = building->GetRoom(p->GetRoomID())->GetSubRoom(p->GetSubRoomID());
      NavLine * next_dest = (*graph).GetNextDestination(last_dest);
      //std::cout << "new route for "<< p->GetPedIndex() << std::endl;
      	p->SetExitIndex(next_dest->GetUniqueID());
      	p->SetExitLine(next_dest);
      	return 1;
    } else {
      // get next destination for person in subroom (in the subroom not next to a crossing) 
      NavLine * exit = graph->GetNextDestination(p);
      p->SetExitIndex(exit->GetUniqueID());
      p->SetExitLine(exit);
      return 1;
    }
  }
}

void GraphRouter::Init(Building* b) 
{
  Log->write("ERROR: Router is not ready to use yet");
  building = b;
  graph = new RoutingGraph(b);

}
