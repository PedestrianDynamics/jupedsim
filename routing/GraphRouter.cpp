/*
 * GraphRouter.cpp
 *
 *  Created on: Aug 20, 2012
 *      Author: David Haensel
 */

#include "GraphRouter.h"
#include "../pedestrian/Pedestrian.h"

using namespace std;

/******************************
 * GraphGrouter Methods
 *****************************/


GraphRouter::GraphRouter()
{
	_building=NULL;
}

GraphRouter::~GraphRouter()
{

}

int GraphRouter::FindExit(Pedestrian* p)
{
    //std::cout << p->Getdt() << "\n";

	set<int> closed_doors = p->GetKnownClosedDoors();

	if(p->GetLastDestination() == -1) {
		//this is needed for initialisation
		p->ChangedSubRoom();
		//Set Initial Route at the beginning
		// get next destination for person in subroom (in the subroom not next to a crossing)
		ExitDistance ed = g.GetGraph(closed_doors)->GetNextDestination(p);
		if(!ed.GetDest())
		{
			// std::cout << "DELETE " << p->GetPedIndex() << std::endl;
			// building->DeletePedFromSim(p);
			return -1;
		}
		p->SetExitIndex(ed.GetDest()->id);
		p->SetExitLine(ed.GetDest()->nav_line);
		return 1;
	} else {
		//the pedestrian at least had a route, now check if he needs a new one
		//if the pedestrian changed the subroom he needs a new route
		if(p->ChangedSubRoom()) {
			ExitDistance ed = g.GetGraph(closed_doors)->GetNextDestination(p->GetLastDestination(), p);

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
		if(p->GetNextDestination() != -1 && !g.GetGraph(closed_doors)->GetVertex(p->GetLastDestination())) {
			ExitDistance ed = g.GetGraph(closed_doors)->GetNextDestination(p);
			p->SetExitIndex(ed.GetDest()->id);
			p->SetExitLine(ed.GetDest()->nav_line);
		}
		//check if the pedestrian reached an hline
		Hline * hline = dynamic_cast<Hline*>(g.GetGraph(closed_doors)->GetVertex(p->GetNextDestination())->nav_line);
		if(hline) {
			// check if the pedestrian is near the Line or In LIne
			if(g.GetGraph(closed_doors)->GetVertex(p->GetNextDestination())->nav_line->DistTo(p->GetPos()) < J_EPS*10 || g.GetGraph(closed_doors)->GetVertex(p->GetNextDestination())->nav_line->IsInLineSegment(p->GetPos()))  {
				//std::cout << "new route from HLINE" << std::endl;
				ExitDistance ed = g.GetGraph(closed_doors)->GetNextDestination(p->GetLastDestination(),p);
				p->SetExitIndex(ed.GetDest()->id);
				p->SetExitLine(ed.GetDest()->nav_line);
				return 1;
			}
		}
		Transition * transition = dynamic_cast<Transition*>(g.GetGraph(closed_doors)->GetVertex(p->GetNextDestination())->nav_line);
		if(transition) {
			if(!transition->IsOpen() && transition->DistTo(p->GetPos()) < 0.5) {
				p->AddKnownClosedDoor(transition->GetUniqueID());
				ExitDistance ed = g.GetGraph(closed_doors)->GetNextDestination(p);
				//std::cout << p->GetPedIndex() << " -- " << ed.GetDest()->id << std::endl;
				p->SetExitIndex(ed.GetDest()->id);
				p->SetExitLine(ed.GetDest()->nav_line);
			}
		}

		// share Information about closed Doors
#pragma omp critical
		if(p->DoorKnowledgeCount() != 0) {
			// std::cout << "ped" << p->GetPedIndex() << std::endl;
			SubRoom * sub  = _building->GetRoom(p->GetRoomID())->GetSubRoom(p->GetSubRoomID());
			const vector<Pedestrian*> ps = sub->GetAllPedestrians();
			for(unsigned int i = 0; i < ps.size(); i++) {
				if((p->GetPos() - ps[i]->GetPos()).Norm() < J_EPS_INFO_DIST) {
					if(ps[i]->GetKnownClosedDoors() != closed_doors)
					{
						ps[i]->MergeKnownClosedDoors(p->GetKnownDoors());
						//maybe the other pedestrian needs a new route
						ExitDistance ed = g.GetGraph(ps[i]->GetKnownClosedDoors())->GetNextDestination(ps[i]);
						if(!ed.GetDest())
						{
							std::cout << "DELETE " << ps[i]->GetID() << std::endl;

							_building->DeletePedFromSim(ps[i]);
						} else {
							// FIXME: ps[i] changedsubroom has to be called to avoid to give a new route twice!
							// sometimes the outher pedestrian changed the subroom and gets a new route here. after this he is looking for a new route but there is no need for.
							ps[i]->ChangedSubRoom();
							ps[i]->SetExitIndex(ed.GetDest()->id);
							ps[i]->SetExitLine(ed.GetDest()->nav_line);
						}
					}
				}
			}
		}
		return 1;
	}
}

void GraphRouter::Init(Building* b)
{
    GlobalRouter::Init(b);


    Log->Write("ERROR: GRAPHROUTER  is not ready to use yet.");
    _building = b;
    g.init(b);

    //std::cout <<  b->GetTransition("200E Normal Exit E3")->IsOpen() << std::endl;
    //        b->GetTransition("200E Normal Exit E3")->Close();

    //std::cout <<  b->GetTransition("200E Normal Exit E3")->IsOpen() << std::endl;
}
