/**
 * @file    CircleRouter.cpp
 * @author  Ulrich Kemloh <kemlohulrich@gmail.com>
 * @version 0.1
 * Created on: Jan 29, 2012
 * Copyright (C) <2009-2012>
 *
 * @section LICENSE
 * This file is part of OpenPedSim.
 *
 * OpenPedSim is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * OpenPedSim is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenPedSim. If not, see <http://www.gnu.org/licenses/>.
 *
 * @section DESCRIPTION
 *
 *
 *
 */

#include "CircleRouter.h"

CircleRouter::CircleRouter():GlobalRouter(){

}

CircleRouter::~CircleRouter(){

}

void CircleRouter::Init(Building* building){

	Log->write("INFO:\tInit the Circle Router Engine");
	pBuilding=building;

	//check and repair hlines ( which might be outside the geometry)
	CheckInconsistencies();

	const  int exitsCnt=GetAllGoals().size()/* +1*/;

	// init the access points
	for (int door = 0; door < exitsCnt; door++) {

		Crossing *cross=GetAllGoals()[door];

		double x1=cross->GetPoint1().GetX();
		double y1=cross->GetPoint1().GetY();

		double x2=cross->GetPoint2().GetX();
		double y2=cross->GetPoint2().GetY();


		double center[2]={0 ,0};
		center[0] = 0.5*(x1+x2);
		center[1] = 0.5*(y1+y2);

		AccessPoint* ap = new AccessPoint(door, center);

		// save the connecting sub/rooms IDs
		int id1 = -1;
		if(cross->GetSubRoom1()) {
			id1=cross->GetSubRoom1()->GetUID();
		}

		int id2 = -1;
		if(cross->GetSubRoom2()) {
			id2=cross->GetSubRoom2()->GetUID();
		}

		ap->setConnectingRooms(id1,id2);

		pAccessPoints.push_back(ap);
	}

	// loop over the rooms
	// loop over the subrooms
	// get the transitions in the subrooms
	// and compute the distances


	for (int i = 0; i < pBuilding->GetAnzRooms(); i++) {
		Room* room = pBuilding->GetRoom(i);


		for (int j = 0; j < room->GetAnzSubRooms(); j++) {

			SubRoom* sub = room->GetSubRoom(j);
			const vector<int>& exitsInSubroom = sub->GetAllGoalIDs();

			// take one access point in that room and connect to all others

			for(unsigned int k=0;k< exitsInSubroom.size();k++){

				int from_door=exitsInSubroom[k];
				AccessPoint* from_AP=pAccessPoints[from_door];
				if(from_AP->GetID()!=from_door){
					Log->write(" an AP was not correct assigned");
					exit(EXIT_FAILURE);
				}

				Crossing* from_crossing=GetAllGoals()[from_door];

				if(from_crossing->IsOpen()==false){
					string caption=((Transition*)from_crossing)->GetCaption();
					Log->write("INFO:\t Transition closed : "+caption);
					continue;
				}


				for(unsigned int l=0;l<exitsInSubroom.size();l++){
					// the entry is already 0 in the diagonal of the matrix
					int to_door=exitsInSubroom[l];
					Crossing* to_crossing=GetAllGoals()[to_door];

					//avoid connecting to myself
					if(from_door==to_door) continue;

					//skip closed doors
					if((to_crossing->IsOpen()==false)){
						continue;
					}

					if(from_crossing->IsOpen()==false){
						string caption=from_crossing->GetCaption();
						Log->write("INFO:\t Transition closed : "+caption);
						continue;
					}

					if(CanSeeEachOther(from_crossing,to_crossing)==false) continue;

					//add only if this is not pointing back to me
					// making the graph directed
					const vector<AccessPoint*>& to_aps = pAccessPoints[to_door]->GetConnectingAPs();
					const vector<AccessPoint*>& from_aps = from_AP->GetConnectingAPs();
					if(IsElementInVector(to_aps,from_AP)==false)
						if(from_aps.size()==0)
							from_AP->AddConnectingAP(pAccessPoints[to_door]);

				}
			}
		}
	}


	//dumping the complete system
	//DumpAccessPoints(1357);

	//WriteGraphGV("routing_graph.gv");
	//exit(0);

	Log->write("INFO:\tDone with the Circle Router Engine!");
}


//actually just return the local shortest path

int CircleRouter::FindExit(Pedestrian* ped){
	int nextDestination=ped->GetNextDestination();
	//get the room and find the corresponding node

	SubRoom* sub = pBuilding->GetRoom(ped->GetRoomID())->GetSubRoom(ped->GetSubRoomID());

	// get the opened exits
	const vector<int>& accessPointsInSubRoom =sub->GetAllGoalIDs();

	// first check if we are near an AP
	for(unsigned int i=0;i<accessPointsInSubRoom.size();i++){

		int apID=accessPointsInSubRoom[i];

		const Point& pt3=ped->GetPos();
		Crossing* cross = GetAllGoals()[apID];
		double distToExit=cross->DistTo(pt3);

		if(distToExit >EPS_AP_DIST) continue;

		//there is only one possible connected AP
		nextDestination=pAccessPoints[apID]->GetConnectingAPs()[0]->GetID();

		//check that the next destination is in the actual room of the pedestrian
		if(pAccessPoints[nextDestination]->isInRange(ped->GetUniqueRoomID())==false) {
			//return the last destination if defined
			int previousDestination=ped->GetNextDestination();

			//we are still somewhere in the initialization phase
			// and we are aright near a final exit to outside
			if(previousDestination==-1){

				ped->SetExitIndex(apID);
				ped->SetExitLine(GetAllGoals()[apID]);
				ped->SetSmoothTurning(true);
				return apID;
			}
			else // we are still having a valid destination, don't change
			{
				return previousDestination;
			}
		}
		else  // we have reached the new room
		{

			ped->SetExitIndex(nextDestination);
			ped->SetExitLine(GetAllGoals()[nextDestination]);
			ped->SetSmoothTurning(true);
			return nextDestination;
		}
	}

	// second: check if we are having a valid destination
	if(nextDestination!=-1)
		return nextDestination;

	// third:
	//we get here if we are still initializing
	//get the connected APs that I can see
	// and choose the directed one

	vector<AccessPoint*> APs;

	for(unsigned int i=0;i<accessPointsInSubRoom.size();i++){

		AccessPoint* ap=pAccessPoints[accessPointsInSubRoom[i]];

		//check if I can see the exit
		SubRoom* sub = pBuilding->GetRoom(ped->GetRoomID())->GetSubRoom(ped->GetSubRoomID());

		// segment connecting the two APs/goals
		Line segment = Line(ped->GetPos(),ap->GetCentre());

		// check if this in intersected by any other connections/walls/doors/trans/cross in the room
		bool isVisible=true;

		//first walls
		const vector<Wall>& walls= sub->GetAllWalls();

		for(unsigned int b=0;b<walls.size();b++){
			if(segment.IntersectionWith(walls[b])==true) {
				isVisible=false;
				//cout<<"failed: walls "<<b<<" in subroom " << sub->GetSubRoomID()<<endl;
				break;
			}
		}
		if(isVisible==false) continue;

		// then all goals
		for(unsigned int g=0;g<accessPointsInSubRoom.size();g++){
			int gID=accessPointsInSubRoom[g];
			if(gID==ap->GetID()) continue;
			// skip the concerned exits door and d
			if(segment.IntersectionWith(*GetAllGoals()[gID])==true) {
				isVisible=false;
				//cout<<"failed: goal "<<gID<<endl;
				break;
			}
		}

		if(isVisible ==true){
			APs.push_back(ap);
		}

	}
	if(APs.size()!=2){
		Log->write("WARNING: more than 2 APs in visibility");
		cout<<"size: "<<APs.size()<<endl;
	}

	//get the ap which is not pointing to the other
	AccessPoint* ap1=APs[0];
	AccessPoint* ap2=APs[1];

	if(IsElementInVector(ap1->GetConnectingAPs(),ap2)){
		nextDestination=ap2->GetID();
	}else{
		nextDestination=ap1->GetID();
	}

	ped->SetExitIndex(nextDestination);
	ped->SetExitLine(GetAllGoals()[nextDestination]);
	return nextDestination;
}
