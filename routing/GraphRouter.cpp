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


    if(p->GetLastDestination() == -1) {
	//this is needed for initialisation
	p->ChangedSubRoom();
	//Set Initial Route at the beginning
	// get next destination for person in subroom (in the subroom not next to a crossing) 
	ExitDistance ed = graph->GetNextDestination(p);
	p->SetExitIndex(ed.GetDest()->id);
	p->SetExitLine(ed.GetDest()->nav_line);
	return 1;
    } else {
	//the pedestrian at least had a route, now check if he needs a new one
	//if the pedestrian changed the subroom he needs a new route
	if(p->ChangedSubRoom()) {
	    ExitDistance ed = graph->GetNextDestination(p->GetLastDestination(), p);
	    // check if the next destination is in the right subroom
	    // if the routing graph changes, it could happen, that the pedestrian has to turn.
	    if(ed.GetSubRoom()->GetRoomID() != p->GetRoomID() || ed.GetSubRoom()->GetSubRoomID() != p->GetSubRoomID()) {
		p->SetExitIndex(p->GetLastDestination());
		p->SetExitLine(ed.GetSrc()->nav_line);
		return 1;
	    }
	    p->SetExitIndex(ed.GetDest()->id);
	    p->SetExitLine(ed.GetDest()->nav_line);
	    return 1;
	}
	//check if the pedestrian reached an hline
	Hline * hline = dynamic_cast<Hline*>(graph->GetVertex(p->GetLastDestination())->nav_line);
	if(hline) {
	    if(graph->GetVertex(p->GetLastDestination())->nav_line->DistTo(p->GetPos()) < EPS * 10) {
		//std::cout << "new route from HLINE" << std::endl; 
		ExitDistance ed = graph->GetNextDestination(p->GetLastDestination(),p);
		p->SetExitIndex(ed.GetDest()->id);
		p->SetExitLine(ed.GetDest()->nav_line);
		return 1;
	    }
	}
	Transition * transition = dynamic_cast<Transition*>(graph->GetVertex(p->GetLastDestination())->nav_line);
	if(transition) {
      
	    if(!transition->IsOpen()) {
		std::cout << "transition" << transition->GetUniqueID() << std::endl;

		graph->closeDoor(transition->GetUniqueID());
		//graph->print();
		ExitDistance ed = graph->GetNextDestination(p);

		p->SetExitIndex(ed.GetDest()->id);
		p->SetExitLine(ed.GetDest()->nav_line);
		return 1;

	    } 
	}
	return 1;
	
    }
}


void GraphRouter::Init(Building* b) 
{
  Log->write("ERROR: Router is not ready to use yet");
  building = b;
  graph = new RoutingGraph(b);
  std::cout <<  b->GetTransition("200E Normal Exit E3")->IsOpen() << std::endl; 
  b->GetTransition("200E Normal Exit E3")->Close();

  std::cout <<  b->GetTransition("200E Normal Exit E3")->IsOpen() << std::endl; 

}
