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
    //
    //if the pedestrian has a destination in the actual room, there is nothing to do
    if(p->GetNextDestination() != -1) {
      //std::cout << "route ok"<< std::endl;
      if(p->GetNextDestination() != p->GetExitIndex()){
	p->SetExitIndex(p->GetNextDestination());
	
	p->SetExitLine(graph->GetVertexCrossing(p->GetNextDestination()));
      }
	return 1;
    } else {
      int last_dest = p->GetLastDestination();
      if(last_dest != -1) {

	SubRoom * sub = building->GetRoom(p->GetRoomID())->GetSubRoom(p->GetSubRoomID());
	Crossing * next_dest = (*graph).GetNextDestination(last_dest, sub);
	if(next_dest->IsInRoom(sub->GetRoomID()) && next_dest->IsInSubRoom(sub->GetSubRoomID())) {
	  std::cout << "new route for "<< p->GetPedIndex() << std::endl;
	  p->SetExitIndex(next_dest->GetIndex());
	
	  p->SetExitLine(next_dest);
	  return 1;
	} else {
	  p->SetExitIndex(last_dest);
	  p->SetExitLine(graph->GetVertexCrossing(last_dest));
	}
      } else {
	SubRoom * sub = building->GetRoom(p->GetRoomID())->GetSubRoom(p->GetSubRoomID());
	const vector<Transition*>& crossings = sub->GetAllTransitions();
	
	
	std::cout << "new random route" << crossings.back()->GetIndex() << std::endl;
	p->SetExitIndex(crossings.front()->GetIndex());
	
	p->SetExitLine(crossings.front());
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


