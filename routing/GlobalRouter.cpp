/**
 * @file    GlobalRouter.cpp
 * @author  Ulrich Kemloh <kemlohulrich@gmail.com>
 * @version 0.1
 * Created on: Dec 15, 2010
 * Copyright (C) <2009-2011>
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

#include "GlobalRouter.h"

#include "AccessPoint.h"
#include "Router.h"
#include "../general/xmlParser.h"
#include "../geometry/Building.h"
#include "../pedestrian/Pedestrian.h"

#include <cfloat>
#include <fstream>

using namespace std;

GlobalRouter::GlobalRouter() :
		Router() {

	pAccessPoints = map<int, AccessPoint*>();

	pMap_id_to_index = std::map<int, int>();
	pMap_index_to_id = std::map<int, int>();

	pDistMatrix = NULL;
	pPathsMatrix = NULL;
	pBuilding = NULL;

}

GlobalRouter::~GlobalRouter() {

	if (pDistMatrix && pPathsMatrix) {
		const int exitsCnt = pBuilding->GetGoalsCount();
		for (int p = 0; p < exitsCnt; ++p) {
			delete[] pDistMatrix[p];
			delete[] pPathsMatrix[p];
		}

		delete[] pDistMatrix;
		delete[] pPathsMatrix;
	}

	map<int, AccessPoint*>::const_iterator itr;
	for (itr = pAccessPoints.begin(); itr != pAccessPoints.end(); ++itr) {
		delete itr->second;
	}
	pAccessPoints.clear();
}

void GlobalRouter::Init(Building* building) {

	Log->Write("INFO:\tInit the Global Router Engine");
	pBuilding = building;

	//check the loaded routing informations (crossings, hlines,...)
	// for inconsistencies
	//TODO: enable me when the navigation lines IDs are fixed
	//	CheckInconsistencies();

	// initialize the network for the floydwarshall algo
	// initialize the distances matrix

	//		const int exitsCnt = pCrossings.size()+ pTransitions.size()+pHlines.size();
	const int exitsCnt = pBuilding->GetGoalsCount();

	pDistMatrix = new double*[exitsCnt];
	pPathsMatrix = new int*[exitsCnt];

	for (int i = 0; i < exitsCnt; ++i) {
		pDistMatrix[i] = new double[exitsCnt];
		pPathsMatrix[i] = new int[exitsCnt];
	}
	//	initializing the values
	// all nodes are disconnected
	for (int p = 0; p < exitsCnt; ++p)
		for (int r = 0; r < exitsCnt; ++r) {
			pDistMatrix[p][r] = (r == p) ? 0.0 : FLT_MAX;/*0.0*/
			pPathsMatrix[p][r] = p;/*0.0*/
		}

	// init the access points
	int index = 0;

	for (map<int, Hline*>::const_iterator itr = pBuilding->GetAllHlines().begin();
			itr != pBuilding->GetAllHlines().end(); ++itr) {

		//int door=itr->first;
		int door = itr->second->GetUniqueID();
		Hline* cross = itr->second;
		Point centre = cross->GetCentre();
		double center[2] = { centre.GetX(), centre.GetY() };

		AccessPoint* ap = new AccessPoint(door, center);
		ap->SetNavLine(cross);

		// save the connecting sub/rooms IDs
		int id1 = -1;
		if (cross->GetSubRoom()) {
			id1 = cross->GetSubRoom()->GetUID();
		}

		ap->setConnectingRooms(id1, id1);
		pAccessPoints[door] = ap;

		//very nasty
		pMap_id_to_index[door] = index;
		pMap_index_to_id[index] = door;
		index++;

	}


	for (map<int, Crossing*>::const_iterator itr = pBuilding->GetAllCrossings().begin();
			itr != pBuilding->GetAllCrossings().end(); ++itr) {

		//int door=itr->first;
		int door = itr->second->GetUniqueID();
		Crossing* cross = itr->second;
		Point centre = cross->GetCentre();
		double center[2] = { centre.GetX(), centre.GetY() };

		AccessPoint* ap = new AccessPoint(door, center);
		ap->SetNavLine(cross);

		// save the connecting sub/rooms IDs
		int id1 = -1;
		if (cross->GetSubRoom1()) {
			id1 = cross->GetSubRoom1()->GetUID();
		}

		int id2 = -1;
		if (cross->GetSubRoom2()) {
			id2 = cross->GetSubRoom2()->GetUID();
		}

		ap->setConnectingRooms(id1, id2);
		pAccessPoints[door] = ap;

		//very nasty
		pMap_id_to_index[door] = index;
		pMap_index_to_id[index] = door;
		index++;

	}

	for (map<int, Transition*>::const_iterator itr = pBuilding->GetAllTransitions().begin();
			itr != pBuilding->GetAllTransitions().end(); ++itr) {

		//int door=itr->first;
		int door = itr->second->GetUniqueID();
		Transition* cross = itr->second;
		Point centre = cross->GetCentre();
		double center[2] = { centre.GetX(), centre.GetY() };

		AccessPoint* ap = new AccessPoint(door, center);
		ap->SetNavLine(cross);

		ap->SetClosed(!cross->IsOpen());
		// save the connecting sub/rooms IDs
		int id1 = -1;
		if (cross->GetSubRoom1()) {
			id1 = cross->GetSubRoom1()->GetUID();
		}

		int id2 = -1;
		if (cross->GetSubRoom2()) {
			id2 = cross->GetSubRoom2()->GetUID();
		}

		ap->setConnectingRooms(id1, id2);
		pAccessPoints[door] = ap;

		//set the final destination
		if (cross->IsExit() && cross->IsOpen()) {
			ap->setFinalDestination(true);
		} else if ((id1 == -1) && (id2 == -1)) {
			Log->Write(" a final destination outside the geometry was found");
			ap->setFinalDestination(true);
		} else if (cross->GetRoom1()->GetCaption() == "outside") {
			ap->setFinalDestination(true);
		}

		//very nasty
		pMap_id_to_index[door] = index;
		pMap_index_to_id[index] = door;
		index++;

	}

	// loop over the rooms
	// loop over the subrooms
	// get the transitions in the subrooms
	// and compute the distances

	for (int i = 0; i < pBuilding->GetAnzRooms(); i++) {
		Room* room = pBuilding->GetRoom(i);

		for (int j = 0; j < room->GetAnzSubRooms(); j++) {

			SubRoom* sub = room->GetSubRoom(j);

			//collect all navigation objects
			vector<NavLine*> allGoals;
			const vector<Crossing*>& crossings = sub->GetAllCrossings();
			allGoals.insert(allGoals.end(), crossings.begin(), crossings.end());
			const vector<Transition*>& transitions = sub->GetAllTransitions();
			allGoals.insert(allGoals.end(), transitions.begin(),
					transitions.end());
			const vector<Hline*>& hlines = sub->GetAllHlines();
			allGoals.insert(allGoals.end(), hlines.begin(), hlines.end());

			//dump goals and APs

			//			for (map<int, AccessPoint*>::const_iterator itr = pAccessPoints.begin();
			//					itr != pAccessPoints.end(); ++itr) {
			//
			//						AccessPoint* from_AP = itr->second;
			//						cout <<" AP ID: "<<itr->first<<endl;
			//			}
			//
			//			cout<<"size: "<<allGoals.size()<<endl;
			//			for(unsigned int i =0;i<allGoals.size();i++){
			//				cout<<"ID: " <<allGoals[i]->GetUniqueID()<<endl;
			//			}
			//			exit(0);

			//process the hlines
			//process the crossings
			//process the transitions
			for (unsigned int n1 = 0; n1 < allGoals.size(); n1++) {

				NavLine* nav1 = allGoals[n1];
				AccessPoint* from_AP = pAccessPoints[nav1->GetUniqueID()];
				int from_door = pMap_id_to_index[nav1->GetUniqueID()];

				for (unsigned int n2 = 0; n2 < allGoals.size(); n2++) {
					NavLine* nav2 = allGoals[n2];

					if (n1 == n2)
						continue;
					if (nav1->operator ==(*nav2))
						continue;

					if (sub->IsVisible(nav1, nav2, true)) {
						int to_door = pMap_id_to_index[nav2->GetUniqueID()];
						pDistMatrix[from_door][to_door] = (nav1->GetCentre()
								- nav2->GetCentre()).Norm();
						from_AP->AddConnectingAP(
								pAccessPoints[nav2->GetUniqueID()]);
					}
				}
			}
		}
	}

	// Handle the extra final destination which are not in any rooms
	// check which of the actual final destinations are connected to the extra

	//	for (map<int, int>::iterator it = pMapIdToFinalDestination.begin();
	//			it != pMapIdToFinalDestination.end(); it++) {
	//
	//		int to_door = it->first;
	//		AccessPoint* to_AP = pAccessPoints[to_door];
	//		printf("checking final [%d] aka [%s]\n", to_door,
	//				GetGoal(to_door)->GetCaption().c_str());
	//
	//		for (unsigned int i = 0; i < pAccessPoints.size(); i++) {
	//			AccessPoint* from_AP = pAccessPoints[i];
	//			int from_door = from_AP->GetID();
	//			if (to_door == from_door)
	//				continue;
	//
	//			// connect only the final exits to the virtual final destinations
	//			if (from_AP->isFinalDestination() == false)
	//				continue;
	//
	//			// connect only open final exits
	//			//if(GetAllGoals()[from_door]->IsOpen()==false) continue;
	//
	//			string to_room_caption = GetGoal(to_door)->GetCaption();
	//			string from_room_caption = "room_"
	//					+ GetGoal(from_door)->GetRoom1()->GetCaption();
	//
	//			if (from_room_caption == to_room_caption) {
	//				double dist = from_AP->GetDistanceTo(to_AP);
	//				dist = GetGoal(to_door)->DistTo(from_AP->GetCentre());
	//				//they are all situated at the same distance cuz they are virtual
	//				if (dist < 18.0)
	//					pDistMatrix[from_door][to_door] = 1.0;
	//			}
	//		}
	//		Log->write("FATAL ERROR: \t you should never get here");
	//		exit(0);
	//	}

	//run the floyd warshall algorithm
	FloydWarshall();

	// set the configuration for reaching the outside
	// set the distances to all final APs

	for (map<int, AccessPoint*>::const_iterator itr = pAccessPoints.begin();
			itr != pAccessPoints.end(); ++itr) {

		AccessPoint* from_AP = itr->second;
		int from_door = pMap_id_to_index[itr->first];


		double tmpMinDist = FLT_MAX;
		int tmpFinalGlobalNearestID = from_door;

		for (map<int, AccessPoint*>::const_iterator itr1 =
				pAccessPoints.begin(); itr1 != pAccessPoints.end(); ++itr1) {

			AccessPoint* to_AP = itr1->second;

			if(from_AP->isFinalDestination()) continue;

			if (to_AP->isFinalDestination()) {

				int to_door = pMap_id_to_index[itr1->first];
				if (from_door == to_door)
					continue;

				//cout <<" checking final destination: "<< pAccessPoints[j]->GetID()<<endl;
				double dist = pDistMatrix[from_door][to_door];
				if (dist < tmpMinDist) {
					tmpFinalGlobalNearestID = to_door;
					tmpMinDist = dist;
				}
			}
		}

		// in the case it is the final APs
		if (tmpFinalGlobalNearestID == from_door)
			tmpMinDist = 0;

		if (tmpMinDist == FLT_MAX) {
			char tmp[CLENGTH];
			sprintf(tmp,
					"ERROR: GlobalRouter: There is no path from hline/crossing/transition [ %d ] to the outside\n",
					from_door);
			Log->Write(tmp);
			exit(EXIT_FAILURE);
		}

		// set the distance to the final destination ( OUT )
		from_AP->AddFinalDestination(FINAL_DEST_OUT, tmpMinDist);

		// set the intermediate path to global final destination
		GetPath(from_door, tmpFinalGlobalNearestID);

		if (pTmpPedPath.size() >= 2) {
			//pAccessPoints[i]->setGlobalNearestAP(pTmpPedPath[1]);
			//from_AP->AddIntermediateDest(FINAL_DEST_OUT, pTmpPedPath[1]);
			from_AP->AddTransitAPsTo(FINAL_DEST_OUT,
					pAccessPoints[pMap_index_to_id[pTmpPedPath[1]]]);
		} else {
			if ((from_AP->isFinalDestination() == false)
					&& (!from_AP->IsClosed())) {

				from_AP->Dump();
				Log->Write(
						"ERROR: GlobalRouter: hline/crossing/transition [ %d ] in room [%s] is out of visibility range \n");
				exit(EXIT_FAILURE);
			}
		}
		pTmpPedPath.clear();
	}

	// set the configuration to reach alternatives that were manually specified
	// in the persons file
	// set the distances to alternative destinations

	for (unsigned int p = 0; p < pFinalDestinations.size(); p++) {

		//get the uniqueID and find the corresponding index in the matrix
		//loop over all transitions

		int to_door_matrix_index=-1;
		int to_door_uid=-1;
		for (map<int, Transition*>::const_iterator itr = pBuilding->GetAllTransitions().begin();
				itr != pBuilding->GetAllTransitions().end(); ++itr) {

			int index = itr->second->GetIndex();
			if (pFinalDestinations[p]==index){
				to_door_matrix_index=pMap_id_to_index[itr->second->GetUniqueID()];
				to_door_uid=itr->second->GetUniqueID();
				break;
			}
		}

		if(to_door_uid==-1){
			char tmp[CLENGTH];
			sprintf(tmp,
					"ERROR: GlobalRouter: Final destination not found [ %d ]\n",
					pFinalDestinations[p]);
			Log->Write(tmp);
			exit(EXIT_FAILURE);
		}



		for (map<int, AccessPoint*>::const_iterator itr =
				pAccessPoints.begin(); itr != pAccessPoints.end(); ++itr) {

			AccessPoint* from_AP = itr->second;
			int from_door_matrix_index = pMap_id_to_index[itr->first];

			//comment this if you want infinite as distance to unreachable destinations
			double dist = pDistMatrix[from_door_matrix_index][to_door_matrix_index];
			from_AP->AddFinalDestination(pFinalDestinations[p], dist);

			// set the intermediate path
			// set the intermediate path to global final destination
			GetPath(from_door_matrix_index, to_door_matrix_index);
			if (pTmpPedPath.size() >= 2) {
				from_AP->AddTransitAPsTo(pFinalDestinations[p],
						pAccessPoints[pMap_index_to_id[pTmpPedPath[1]]]);
			} else {
				if ((from_AP->isFinalDestination() == false)
						&& (!from_AP->IsClosed())) {
					from_AP->Dump();
					exit(EXIT_FAILURE);
				}
			}
			pTmpPedPath.clear();

		}

	}

	//create a complete navigation graph
	//LoadNavigationGraph("./Inputfiles/120531_navigation_graph_arena.xml");

	//dumping the complete system
	//DumpAccessPoints(17);
	//DumpAccessPoints(18);
	//DumpAccessPoints(19);
	//exit(0);
	//DumpAccessPoints(826);

	//vector<string> rooms;
	//rooms.push_back("050");
	//WriteGraphGV("routing_graph.gv",FINAL_DEST_OUT,rooms);
	//WriteGraphGV("routing_graph.gv",4,rooms);
	//exit(0);
	Log->Write("INFO:\tDone with the Global Router Engine!");
}

void GlobalRouter::GetPath(int i, int j) {
	if (pDistMatrix[i][j] == FLT_MAX)
		return;
	if (i != j)
		GetPath(i, pPathsMatrix[i][j]);
	pTmpPedPath.push_back(j);
	//printf("--%d--",j);
}

/*
 floyd_warshall()

 after calling this function dist[i][j] will the the minimum distance
 between i and j if it exists (i.e. if there's a path between i and j)
 or 0, otherwise
 */
void GlobalRouter::FloydWarshall() {
	//	int i, j, k;
	const int n = pBuilding->GetGoalsCount();

	for (int k = 0; k < n; k++)
		for (int i = 0; i < n; i++)
			for (int j = 0; j < n; j++)
				if (pDistMatrix[i][k] + pDistMatrix[k][j] < pDistMatrix[i][j]) {
					pDistMatrix[i][j] = pDistMatrix[i][k] + pDistMatrix[k][j];
					pPathsMatrix[i][j] = pPathsMatrix[k][j];
				}
	return;

}

void GlobalRouter::DumpAccessPoints(int p) {

	if (p != -1) {
		pAccessPoints.at(p)->Dump();
	} else {
		for (map<int, AccessPoint*>::const_iterator itr = pAccessPoints.begin();
				itr != pAccessPoints.end(); ++itr) {
			itr->second->Dump();
		}

	}
}

int GlobalRouter::FindExit(Pedestrian* ped) {

	int nextDestination = ped->GetNextDestination();

	if (nextDestination == -1) {
		return GetBestDefaultRandomExit(ped);

	} else {

		SubRoom* sub = pBuilding->GetRoom(ped->GetRoomID())->GetSubRoom(
				ped->GetSubRoomID());

		const vector<int>& accessPointsInSubRoom = sub->GetAllGoalIDs();
		for (unsigned int i = 0; i < accessPointsInSubRoom.size(); i++) {

			int apID = accessPointsInSubRoom[i];
			AccessPoint* ap = pAccessPoints[apID];

			const Point& pt3 = ped->GetPos();
			double distToExit = ap->GetNavLine()->DistTo(pt3);

			if (distToExit > J_EPS_DIST)
				continue;

			//one AP is near actualize destination:
			nextDestination = ap->GetNearestTransitAPTO(
					ped->GetFinalDestination());

			if (nextDestination == -1) { // we are almost at the exit
				return ped->GetNextDestination();

			} else {

				//check that the next destination is in the actual room of the pedestrian
				if (pAccessPoints[nextDestination]->isInRange(
						sub->GetUID())==false) {
					//return the last destination if defined
					int previousDestination = ped->GetNextDestination();

					//we are still somewhere in the initialization phase
					if (previousDestination == -1) {
						ped->SetExitIndex(apID);
						ped->SetExitLine(pAccessPoints[apID]->GetNavLine());
						return apID;
					} else // we are still having a valid destination, don't change
					{
						return previousDestination;
					}
				} else // we have reached the new room
				{
					ped->SetExitIndex(nextDestination);
					ped->SetExitLine(
							pAccessPoints[nextDestination]->GetNavLine());
					return nextDestination;
				}
			}

		}

		return GetBestDefaultRandomExit(ped);
	}

}

int GlobalRouter::GetBestDefaultRandomExit(Pedestrian* ped) {

	// get the opened exits
	SubRoom* sub = pBuilding->GetRoom(ped->GetRoomID())->GetSubRoom(
			ped->GetSubRoomID());

	// get the opened exits
	const vector<int>& accessPointsInSubRoom = sub->GetAllGoalIDs();
	int bestAPsID = -1;
	double minDist = FLT_MAX;

	for (unsigned int i = 0; i < accessPointsInSubRoom.size(); i++) {
		int apID = accessPointsInSubRoom[i];

		AccessPoint* ap = pAccessPoints[apID];

		if (ap->isInRange(sub->GetUID()) == false)
			continue;

		//check if that exit is open.
		if (ap->IsClosed())
			continue;

		//the line from the current position to the centre of the nav line.
		// at least the line in that direction minus EPS
		const Point& posA = ped->GetPos();
		const Point& posB = ap->GetNavLine()->GetCentre();
		const Point& posC = (posB - posA).Normalized()
				* ((posA - posB).Norm() - J_EPS) + posA;

		//check if visible
		if (sub->IsVisible(posA, posC, true) == false)
			continue;

		double dist = ap->GetDistanceTo(ped->GetFinalDestination())
				+ ap->distanceTo(posA.GetX(), posA.GetY());

		if (dist < minDist) {
			bestAPsID = ap->GetID();
			minDist = dist;
		}
	}

	if (bestAPsID != -1) {
		ped->SetExitIndex(bestAPsID);
		ped->SetExitLine(pAccessPoints[bestAPsID]->GetNavLine());
		return bestAPsID;
	} else {
		cout << "ERROR:\t GlobalRouter.cpp: a valid destination could not be found" << endl;
		exit(EXIT_FAILURE);
		return -1;
	}
}

bool GlobalRouter::CanSeeEachother(const Point&pt1, const Point& pt2) {

	Line segment = Line(pt1, pt2);

	for (int i = 0; i < pBuilding->GetAnzRooms(); i++) {
		Room* room = pBuilding->GetRoom(i);

		for (int j = 0; j < room->GetAnzSubRooms(); j++) {
			SubRoom* sub = room->GetSubRoom(j);
			//  walls
			const vector<Wall>& walls = sub->GetAllWalls();

			for (unsigned int b = 0; b < walls.size(); b++) {
				if (segment.IntersectionWith(walls[b]) == true) {
					return false;
				}
			}
		}
	}

	// then all goals
	//fIXME
//	for (int door = 0; door < pBuilding->GetGoalsCount(); door++) {
//		Crossing *cross =pBuilding-> GetGoal(door);
//		if (cross->GetRoom1()->GetCaption() == "outside")
//			continue;
//		if (segment.IntersectionWith(*cross) == true) {
//			return false;
//		}
//	}

	return true;
}


bool GlobalRouter::CanSeeEachOther(Crossing* c1, Crossing* c2) {

	//do they share at least one subroom?
	//find the common subroom,
	//return false if none
	SubRoom* sb1_a = c1->GetSubRoom1();
	SubRoom* sb1_b = c1->GetSubRoom2();

	SubRoom* sb2_a = c2->GetSubRoom1();
	SubRoom* sb2_b = c2->GetSubRoom2();

	SubRoom* sub = NULL;

	if ((sb1_a != NULL) && (sb1_a == sb2_a))
		sub = sb1_a;
	else if ((sb1_a != NULL) && (sb1_a == sb2_b))
		sub = sb1_a;
	else if ((sb1_b != NULL) && (sb1_b == sb2_a))
		sub = sb1_b;
	else if ((sb1_b != NULL) && (sb1_b == sb2_b))
		sub = sb1_b;

	if (sub == NULL) {
		//char tmp[CLENGTH];
		//sprintf(tmp,"no common subroom found for transitions [%d] and [%d]",
		//		c1->GetIndex(),c2->GetIndex());
		//Log->write(tmp);
		return false;
	}

	// segment connecting the two APs/goals
	const Point& p1 = (c1->GetPoint1() + c1->GetPoint2()) * 0.5;
	const Point& p2 = (c2->GetPoint1() + c2->GetPoint2()) * 0.5;
	Line segment = Line(p1, p2);

	// check if this in intersected by any other connections/walls/doors/trans/cross in the room

	//first walls
	const vector<Wall>& walls = sub->GetAllWalls();

	for (unsigned int b = 0; b < walls.size(); b++) {
		if (segment.IntersectionWith(walls[b]) == true) {
			return false;
		}
	}

	// also take into account other crossings/transitions
	const vector<int>& exitsInSubroom = sub->GetAllGoalIDs();

	int id1 = c1->GetIndex();
	int id2 = c2->GetIndex();
	// then all goals
	for (int g = 0; g < (int) exitsInSubroom.size(); g++) {
		int gID = exitsInSubroom[g];
		// skip the concerned exits door and d
		if ((id1 == gID) || (id2 == gID))
			continue;
		if (segment.IntersectionWith(*pBuilding->GetGoal(exitsInSubroom[g])) == true) {
			return false;
		}
	}

	//last check in the case of a concav polygon
	// check if the middle of the connection line lies inside the subroom
	Point middle = (p1 + p2) * 0.5;
	bool isVisible = sub->IsInSubRoom(middle);

	if (isVisible == false) {
		return false;
	}

	return true;
}

SubRoom* GlobalRouter::GetCommonSubRoom(Crossing* c1, Crossing* c2) {
	SubRoom* sb11 = c1->GetSubRoom1();
	SubRoom* sb12 = c1->GetSubRoom2();
	SubRoom* sb21 = c2->GetSubRoom1();
	SubRoom* sb22 = c2->GetSubRoom2();

	if (sb11 == sb21)
		return sb11;
	if (sb11 == sb22)
		return sb11;
	if (sb12 == sb21)
		return sb12;
	if (sb12 == sb22)
		return sb12;

	return NULL;
}

void GlobalRouter::WriteGraphGV(string filename, int finalDestination,
		const vector<string> rooms_captions) {
	ofstream graph_file(filename.c_str());
	if (graph_file.is_open() == false) {
		Log->Write("Unable to open file" + filename);
		return;
	}

	//header
	graph_file << "## Produced by OPS_GCFM" << endl;
	//graph_file << "##comand: \" sfdp -Goverlap=prism -Gcharset=latin1"<<filename <<"| gvmap -e | neato -Ecolor=\"#55555522\" -n2 -Tpng > "<< filename<<".png \""<<endl;
	graph_file << "##Command to produce the output: \"neato -n -s -Tpng "
			<< filename << " > " << filename << ".png\"" << endl;
	graph_file << "digraph OPS_GCFM_ROUTING {" << endl;
	graph_file << "overlap=scale;" << endl;
	graph_file << "splines=false;" << endl;
	graph_file << "fontsize=20;" << endl;
	graph_file
			<< "label=\"Graph generated by the routing engine for destination: "
			<< finalDestination << "\"" << endl;

	vector<int> rooms_ids = vector<int>();

	if (rooms_captions.empty()) {
		// then all rooms should be printed
		for (int i = 0; i < pBuilding->GetAnzRooms(); i++) {
			rooms_ids.push_back(i);
		}

	} else {
		for (unsigned int i = 0; i < rooms_captions.size(); i++) {
			rooms_ids.push_back(
					pBuilding->GetRoom(rooms_captions[i])->GetRoomID());
		}
	}

	for (map<int, AccessPoint*>::const_iterator itr = pAccessPoints.begin();
			itr != pAccessPoints.end(); ++itr) {

		AccessPoint* from_AP = itr->second;

		int from_door = from_AP->GetID();

		// check for valid room
		NavLine* nav = from_AP->GetNavLine();
		int room_id = -1;

		if (dynamic_cast<Crossing*>(nav) != NULL) {
			room_id = ((Crossing*) (nav))->GetRoom1()->GetRoomID();

		} else if (dynamic_cast<Hline*>(nav) != NULL) {
			room_id = ((Hline*) (nav))->GetRoom()->GetRoomID();

		} else if (dynamic_cast<Transition*>(nav) != NULL) {
			room_id = ((Transition*) (nav))->GetRoom1()->GetRoomID();

		} else {
			cout << "WARNING: Unkown navigation line type" << endl;
			continue;
		}

		if (IsElementInVector(rooms_ids, room_id) == false)
			continue;

		double px = from_AP->GetCentre().GetX();
		double py = from_AP->GetCentre().GetY();
		//graph_file << from_door <<" [shape=ellipse, pos=\""<<px<<", "<<py<<" \"] ;"<<endl;
		//graph_file << from_door <<" [shape=ellipse, pos=\""<<px<<","<<py<<"\" ];"<<endl;

		//const vector<AccessPoint*>& from_aps = from_AP->GetConnectingAPs();
		const vector<AccessPoint*>& from_aps = from_AP->GetTransitAPsTo(
				finalDestination);

		if (from_aps.size() == 0) {

			if (from_AP->isFinalDestination()) {
				graph_file << from_door << " [pos=\"" << px << ", " << py
						<< " \", style=filled, color=green,fontsize=5] ;"
						<< endl;
				//				graph_file << from_door <<" [width=\"0.41\", height=\"0.31\",fixedsize=false,pos=\""<<px<<", "<<py<<" \", style=filled, color=green,fontsize=4] ;"<<endl;
			} else {
				graph_file << from_door << " [pos=\"" << px << ", " << py
						<< " \", style=filled, color=red,fontsize=5] ;" << endl;
				//				graph_file << from_door <<" [width=\"0.41\", height=\"0.31\",fixedsize=false,pos=\""<<px<<", "<<py<<" \", style=filled, color=red,fontsize=4] ;"<<endl;
			}
		} else {
			// check that all connecting aps are contained in the room_ids list
			// if not marked as sink.
			bool isSink = true;
			for (unsigned int j = 0; j < from_aps.size(); j++) {
				NavLine* nav = from_aps[j]->GetNavLine();
				int room_id = -1;

				if (dynamic_cast<Crossing*>(nav) != NULL) {
					room_id = ((Crossing*) (nav))->GetRoom1()->GetRoomID();

				} else if (dynamic_cast<Hline*>(nav) != NULL) {
					room_id = ((Hline*) (nav))->GetRoom()->GetRoomID();

				} else if (dynamic_cast<Transition*>(nav) != NULL) {
					room_id = ((Transition*) (nav))->GetRoom1()->GetRoomID();

				} else {
					cout << "WARNING: Unkown navigation line type" << endl;
					continue;
				}

				if (IsElementInVector(rooms_ids, room_id) == true) {
					isSink = false;
					break;
				}
			}

			if (isSink) {
				//				graph_file << from_door <<" [width=\"0.3\", height=\"0.21\",fixedsize=false,pos=\""<<px<<", "<<py<<" \" ,style=filled, color=green, fontsize=4] ;"<<endl;
				graph_file << from_door << " [pos=\"" << px << ", " << py
						<< " \" ,style=filled, color=green, fontsize=5] ;"
						<< endl;
			} else {
				//				graph_file << from_door <<" [width=\"0.3\", height=\"0.231\",fixedsize=false, pos=\""<<px<<", "<<py<<" \", fontsize=4] ;"<<endl;
				graph_file << from_door << " [pos=\"" << px << ", " << py
						<< " \", style=filled, color=yellow, fontsize=5] ;"
						<< endl;
			}
		}

	}

	//connections

	for (map<int, AccessPoint*>::const_iterator itr = pAccessPoints.begin();
			itr != pAccessPoints.end(); ++itr) {

		AccessPoint* from_AP = itr->second;
		int from_door = from_AP->GetID();

		//const vector<AccessPoint*>& aps = from_AP->GetConnectingAPs();
		const vector<AccessPoint*>& aps = from_AP->GetTransitAPsTo(
				finalDestination);

		NavLine* nav = from_AP->GetNavLine();
		int room_id = -1;

		if (dynamic_cast<Crossing*>(nav) != NULL) {
			room_id = ((Crossing*) (nav))->GetRoom1()->GetRoomID();

		} else if (dynamic_cast<Hline*>(nav) != NULL) {
			room_id = ((Hline*) (nav))->GetRoom()->GetRoomID();

		} else if (dynamic_cast<Transition*>(nav) != NULL) {
			room_id = ((Transition*) (nav))->GetRoom1()->GetRoomID();

		} else {
			cout << "WARNING: Unkown navigation line type" << endl;
			continue;
		}

		if (IsElementInVector(rooms_ids, room_id) == false)
			continue;

		for (unsigned int j = 0; j < aps.size(); j++) {
			AccessPoint* to_AP = aps[j];
			int to_door = to_AP->GetID();

			NavLine* nav = to_AP->GetNavLine();
			int room_id = -1;

			if (dynamic_cast<Crossing*>(nav) != NULL) {
				room_id = ((Crossing*) (nav))->GetRoom1()->GetRoomID();

			} else if (dynamic_cast<Hline*>(nav) != NULL) {
				room_id = ((Hline*) (nav))->GetRoom()->GetRoomID();

			} else if (dynamic_cast<Transition*>(nav) != NULL) {
				room_id = ((Transition*) (nav))->GetRoom1()->GetRoomID();

			} else {
				cout << "WARNING: Unkown navigation line type" << endl;
				continue;
			}

			if (IsElementInVector(rooms_ids, room_id) == false)
				continue;

			graph_file << from_door << " -> " << to_door << " [ label="
					<< from_AP->GetDistanceTo(to_AP)
							+ to_AP->GetDistanceTo(finalDestination)
					<< ", fontsize=10]; " << endl;
		}

	}
	//graph_file << "node [shape=box];  gy2; yr2; rg2; gy1; yr1; rg1;"<<endl;
	//graph_file << "node [shape=circle,fixedsize=true,width=0.9];  green2; yellow2; red2; safe2; safe1; green1; yellow1; red1;"<<endl;

	//graph_file << "0 -> 1 ;"<<endl;

	graph_file << "}" << endl;

	//done
	graph_file.close();
}
