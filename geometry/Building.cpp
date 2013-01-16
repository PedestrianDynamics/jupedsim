/**
 * File:   Building.cpp
 *
 *  Created on 1. October 2010, 09:25
 *
 * @section LICENSE
 * This file is part of JuPedSim.
 *
 * JuPedSim is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * JuPedSim is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with JuPedSim. If not, see <http://www.gnu.org/licenses/>.
 *
 * @section DESCRIPTION
 *
 *
 *
 */

#include "Building.h"
#include "../general/xmlParser.h"
#include "Obstacle.h"
#include "Room.h"
#include "Hline.h"
#include "Crossing.h"
#include "Transition.h"
#include "SubRoom.h"

using namespace std;

/************************************************************
 Konstruktoren
 ************************************************************/

Building::Building() {
	pCaption = "no_caption";
	pRooms = vector<Room*>();
}


Building::~Building() {
	for (int i = 0; i < GetAnzRooms(); i++)
		delete pRooms[i];

}

/************************************************************
 Setter-Funktionen
 ************************************************************/
void Building::SetCaption(string s) {
	pCaption = s;
}


void Building::SetAllRooms(const vector<Room*>& rooms) {
	pRooms = rooms;
}

void Building::SetRoom(Room* room, int index) {
	if ((index >= 0) && (index < (int) pRooms.size())) {
		pRooms[index] = room;
	} else {
		Log->Write("ERROR: \tWrong Index in CBuilding::SetRoom()");
		exit(0);
	}
}

/*************************************************************
 Getter-Funktionen
 ************************************************************/

string Building::GetCaption() const {
	return pCaption;
}


int Building::GetAnzRooms() const {
	return pRooms.size();
}

int Building::GetGoalsCount() const {
	return _transitions.size() + _hLines.size() + _crossings.size();
}

const vector<Room*>& Building::GetAllRooms() const {
	return pRooms;
}

Room* Building::GetRoom(int index) const {
	if ((index >= 0) && (index < (int) pRooms.size())) {
		return pRooms[index];
	} else {
		char tmp[CLENGTH];
		sprintf(tmp,
				"ERROR: Wrong 'index' in CBuiling::GetRoom() index: %d size: %d",
				index, pRooms.size());
		Log->Write(tmp);
		exit(0);
	}
}


/*************************************************************
 Sonstiges
 ************************************************************/

void Building::AddRoom(Room* room) {
	pRooms.push_back(room);
}

void Building::AddSurroundingRoom() {
	Log->Write("INFO: \tAdding the room 'outside' ");
	// first look for the geometry boundaries
	double x_min = FLT_MAX;
	double x_max = -FLT_MAX;
	double y_min = FLT_MAX;
	double y_max = -FLT_MAX;

	//finding the bounding of the grid
	// and collect the pedestrians
	for (unsigned int r = 0; r < pRooms.size(); r++) {
		Room* room = pRooms[r];
		for (int j = 0; j < room->GetAnzSubRooms(); j++) {
			SubRoom* sub = room->GetSubRoom(j);
			const vector<Wall>& allWalls = sub->GetAllWalls();

			for (unsigned int a = 0; a < allWalls.size(); a++) {
				double x1 = allWalls[a].GetPoint1().GetX();
				double y1 = allWalls[a].GetPoint1().GetY();
				double x2 = allWalls[a].GetPoint2().GetX();
				double y2 = allWalls[a].GetPoint2().GetY();

				double xmax = (x1 > x2) ? x1 : x2;
				double xmin = (x1 > x2) ? x2 : x1;
				double ymax = (y1 > y2) ? y1 : y2;
				double ymin = (y1 > y2) ? y2 : y1;

				x_min = (xmin <= x_min) ? xmin : x_min;
				x_max = (xmax >= x_max) ? xmax : x_max;
				y_max = (ymax >= y_max) ? ymax : y_max;
				y_min = (ymin <= y_min) ? ymin : y_min;
			}
		}
	}

	//make the grid slightly larger.
	x_min = x_min - 10.0;
	x_max = x_max + 10.0;
	y_min = y_min - 10.0;
	y_max = y_max + 10.0;

	SubRoom* bigSubroom = new NormalSubRoom();
	bigSubroom->SetRoomID(pRooms.size());
	bigSubroom->SetSubRoomID(0); // should be the single subroom
	bigSubroom->AddWall(Wall(Point(x_min, y_min), Point(x_min, y_max)));
	bigSubroom->AddWall(Wall(Point(x_min, y_max), Point(x_max, y_max)));
	bigSubroom->AddWall(Wall(Point(x_max, y_max), Point(x_max, y_min)));
	bigSubroom->AddWall(Wall(Point(x_max, y_min), Point(x_min, y_min)));

	Room * bigRoom = new Room();
	bigRoom->AddSubRoom(bigSubroom);
	bigRoom->SetCaption("outside");
	bigRoom->SetRoomID(pRooms.size());
	AddRoom(bigRoom);
}

void Building::InitGeometry() {
	Log->Write("INFO: \tInit Geometry");
	for (int i = 0; i < GetAnzRooms(); i++) {
		Room* room = GetRoom(i);
		// Polygone berechnen
		for (int j = 0; j < room->GetAnzSubRooms(); j++) {
			SubRoom* s = room->GetSubRoom(j);
			// Alle Übergänge in diesem Raum bestimmen
			// Übergänge müssen zu Wänden ergänzt werden
			vector<Line*> goals = vector<Line*>();

			//  crossings
			const vector<Crossing*>& crossings = s->GetAllCrossings();
			for (unsigned int i = 0; i < crossings.size(); i++) {
				goals.push_back(crossings[i]);
			}

			// and  transitions
			const vector<Transition*>& transitions = s->GetAllTransitions();
			for (unsigned int i = 0; i < transitions.size(); i++) {
				goals.push_back(transitions[i]);
			}

			// initialize the poly
			s->ConvertLineToPoly(goals);
			s->CalculateArea();
			goals.clear();

			//do the same for the obstacles that are closed
			const vector<Obstacle*>& obstacles = s->GetAllObstacles();
			for (unsigned int obs = 0; obs < obstacles.size(); ++obs) {
				if (obstacles[obs]->GetClosed() == 1)
					obstacles[obs]->ConvertLineToPoly();
			}
		}
	}
	Log->Write("INFO: \tInit Geometry successful!!!\n");
}



/*************************************************************
 Ein-Ausgabe
 ************************************************************/

void Building::LoadBuilding(string filename) {

	Log->Write("INFO: \tParsing the geometry file");

	XMLNode xMainNode = XMLNode::openFileHelper(filename.c_str(), "geometry");

	double version = xmltof(xMainNode.getAttribute("version"), -1);
	if (version < 0.4) {
		Log->Write("ERROR: \tOnly version > 0.4 supported");
		Log->Write("ERROR: \tparsing geometry file failed!");
		exit(EXIT_FAILURE);
	}
	pCaption = xmltoa(xMainNode.getAttribute("caption"), "virtual building");

	//The file has two main nodes
	//<rooms> and <transitions>

	XMLNode xRoomsNode = xMainNode.getChildNode("rooms");
	int nRooms = xRoomsNode.nChildNode("room");

	//processing the rooms node
	for (int i = 0; i < nRooms; i++) {
		XMLNode xRoom = xRoomsNode.getChildNode("room", i);
		Room* room = new Room();

		string room_id = xmltoa(xRoom.getAttribute("id"), "-1");
		room->SetRoomID(xmltoi(room_id.c_str(), -1));

		string caption = "room " + room_id;
		room->SetCaption(
				xmltoa(xRoom.getAttribute("caption"), caption.c_str()));

		double position = xmltof(xRoom.getAttribute("zpos"), 0.0);
		if(position>6.0) position+=50;
		room->SetZPos(position);

		//parsing the subrooms
		int nSubRooms = xRoom.nChildNode("subroom");

		for (int s = 0; s < nSubRooms; s++) {
			XMLNode xSubroomsNode = xRoom.getChildNode("subroom", s);

			string subroom_id = xmltoa(xSubroomsNode.getAttribute("id"), "-1");
			string closed = xmltoa(xSubroomsNode.getAttribute("closed"), "0");
			string type = xmltoa(xSubroomsNode.getAttribute("class"),
					"subroom");

			SubRoom* subroom = NULL;

			if (type == "stair") {
				double up_x = xmltof( xSubroomsNode.getChildNode("up").getAttribute("px"), 0.0);
				double up_y = xmltof( xSubroomsNode.getChildNode("up").getAttribute("py"), 0.0);
				double down_x = xmltof( xSubroomsNode.getChildNode("down").getAttribute("py"), 0.0);
				double down_y = xmltof( xSubroomsNode.getChildNode("down").getAttribute("py"), 0.0);
				subroom = new Stair();
				((Stair*)subroom)->SetUp(Point(up_x,up_y));
				((Stair*)subroom)->SetDown(Point(down_x,down_y));
			} else {
				//normal subroom or corridor
				subroom = new NormalSubRoom();
			}

			subroom->SetRoomID(room->GetRoomID());
			subroom->SetSubRoomID(xmltoi(subroom_id.c_str(), -1));

			//looking for polygons (walls)
			int nPoly = xSubroomsNode.nChildNode("polygon");
			for (int p = 0; p < nPoly; p++) {
				XMLNode xPolyVertices = xSubroomsNode.getChildNode("polygon",
						p);
				int nVertices =
						xSubroomsNode.getChildNode("polygon", p).nChildNode(
								"vertex");

				for (int v = 0; v < nVertices - 1; v++) {
					double x1 =
							xmltof(
									xPolyVertices.getChildNode("vertex", v).getAttribute(
											"px"));
					double y1 =
							xmltof(
									xPolyVertices.getChildNode("vertex", v).getAttribute(
											"py"));

					double x2 =
							xmltof(
									xPolyVertices.getChildNode("vertex", v + 1).getAttribute(
											"px"));
					double y2 =
							xmltof(
									xPolyVertices.getChildNode("vertex", v + 1).getAttribute(
											"py"));
					subroom->AddWall(Wall(Point(x1, y1), Point(x2, y2)));
				}

			}

			//looking for obstacles
			int nObst = xSubroomsNode.nChildNode("obstacle");

			for (int obst = 0; obst < nObst; obst++) {
				XMLNode xObstacle = xSubroomsNode.getChildNode("obstacle",
						obst);
				int nPoly = xObstacle.nChildNode("polygon");
				int id = xmltof(xObstacle.getAttribute("id"), -1);
				int height = xmltof(xObstacle.getAttribute("height"), 0);
				double closed = xmltof(xObstacle.getAttribute("closed"), 0);
				string caption = xmltoa(xObstacle.getAttribute("caption"),
						"-1");

				Obstacle* obstacle = new Obstacle();
				obstacle->SetId(id);
				obstacle->SetCaption(caption);
				obstacle->SetClosed(closed);
				obstacle->SetHeight(height);

				for (int p = 0; p < nPoly; p++) {
					XMLNode xPolyVertices = xObstacle.getChildNode("polygon",
							p);
					int nVertices =
							xObstacle.getChildNode("polygon", p).nChildNode(
									"vertex");
					for (int v = 0; v < nVertices - 1; v++) {
						double x1 =
								xmltof(
										xPolyVertices.getChildNode("vertex", v).getAttribute(
												"px"));
						double y1 =
								xmltof(
										xPolyVertices.getChildNode("vertex", v).getAttribute(
												"py"));

						double x2 =
								xmltof(
										xPolyVertices.getChildNode("vertex",
												v + 1).getAttribute("px"));
						double y2 =
								xmltof(
										xPolyVertices.getChildNode("vertex",
												v + 1).getAttribute("py"));
						obstacle->AddWall(Wall(Point(x1, y1), Point(x2, y2)));
					}
				}
				subroom->AddObstacle(obstacle);
			}
			room->AddSubRoom(subroom);
		}
		//parsing the crossings
		XMLNode xCrossingsNode = xRoom.getChildNode("crossings");
		int nCrossing = xCrossingsNode.nChildNode("crossing");

		//processing the rooms node
		for (int i = 0; i < nCrossing; i++) {
			XMLNode xCrossing = xCrossingsNode.getChildNode("crossing", i);

			int id = xmltoi(xCrossing.getAttribute("id"), -1);
			int sub1_id = xmltoi(xCrossing.getAttribute("subroom1_id"), -1);
			int sub2_id = xmltoi(xCrossing.getAttribute("subroom2_id"), -1);
			double x1 = xmltof(
					xCrossing.getChildNode("vertex", 0).getAttribute("px"));
			double y1 = xmltof(
					xCrossing.getChildNode("vertex", 0).getAttribute("py"));
			double x2 = xmltof(
					xCrossing.getChildNode("vertex", 1).getAttribute("px"));
			double y2 = xmltof(
					xCrossing.getChildNode("vertex", 1).getAttribute("py"));

			Crossing* c = new Crossing();
			c->SetIndex(id);
			c->SetPoint1(Point(x1, y1));
			c->SetPoint2(Point(x2, y2));

			c->SetSubRoom1(room->GetSubRoom(sub1_id));
			c->SetSubRoom2(room->GetSubRoom(sub2_id));
			c->SetRoom1(room);
			AddCrossing(c);

			room->GetSubRoom(sub1_id)->AddCrossing(c);
			room->GetSubRoom(sub2_id)->AddCrossing(c);
		}

		AddRoom(room);
	}

	// all rooms are read, now proceed with transitions
	XMLNode xTransNode = xMainNode.getChildNode("transitions");
	int nTrans = xTransNode.nChildNode("transition");

	for (int i = 0; i < nTrans; i++) {
		XMLNode xTrans = xTransNode.getChildNode("transition", i);

		int id = xmltoi(xTrans.getAttribute("id"), -1);
		string caption = "door " + id;
		caption = xmltoa(xTrans.getAttribute("caption"), caption.c_str());
		int room1_id = xmltoi(xTrans.getAttribute("room1_id"), -1);
		int room2_id = xmltoi(xTrans.getAttribute("room2_id"), -1);
		int subroom1_id = xmltoi(xTrans.getAttribute("subroom1_id"), -1);
		int subroom2_id = xmltoi(xTrans.getAttribute("subroom2_id"), -1);
		double x1 = xmltof(xTrans.getChildNode("vertex", 0).getAttribute("px"));
		double y1 = xmltof(xTrans.getChildNode("vertex", 0).getAttribute("py"));
		double x2 = xmltof(xTrans.getChildNode("vertex", 1).getAttribute("px"));
		double y2 = xmltof(xTrans.getChildNode("vertex", 1).getAttribute("py"));
		string type = xmltoa(xTrans.getAttribute("type"), "normal");

		Transition* t = new Transition();
		t->SetIndex(id);
		t->SetCaption(caption);
		t->SetPoint1(Point(x1, y1));
		t->SetPoint2(Point(x2, y2));
		t->SetType(type);

		if (room1_id != -1 && subroom1_id != -1) {
			Room* room = pRooms[room1_id];
			SubRoom* subroom = room->GetSubRoom(subroom1_id);

			//subroom->AddGoalID(t->GetUniqueID());
			//MPI
			room->AddTransitionID(t->GetUniqueID());
			t->SetRoom1(room);
			t->SetSubRoom1(subroom);

			//new implementation
			subroom->AddTransition(t);
		}
		if (room2_id != -1 && subroom2_id != -1) {
			Room* room = pRooms[room2_id];
			SubRoom* subroom = room->GetSubRoom(subroom2_id);
			//subroom->AddGoalID(t->GetUniqueID());
			//MPI
			room->AddTransitionID(t->GetUniqueID());
			t->SetRoom2(room);
			t->SetSubRoom2(subroom);

			//new implementation
			subroom->AddTransition(t);
		}

		AddTransition(t);
	}
	Log->Write("INFO: \tLoading building file successful!!!\n");
}


void Building::WriteToErrorLog() const {
	Log->Write("GEOMETRY: ");
	for (int i = 0; i < GetAnzRooms(); i++) {
		Room* r = GetRoom(i);
		r->WriteToErrorLog();
	}
	Log->Write("ROUTING: ");

	for (map<int, Crossing*>::const_iterator iter = _crossings.begin();
			iter != _crossings.end(); ++iter) {
		iter->second->WriteToErrorLog();
	}
	for (map<int, Transition*>::const_iterator iter = _transitions.begin();
			iter != _transitions.end(); ++iter) {
		iter->second->WriteToErrorLog();
	}
	for (map<int, Hline*>::const_iterator iter = _hLines.begin();
			iter != _hLines.end(); ++iter) {
		iter->second->WriteToErrorLog();
	}
	Log->Write("\n");
}

Room* Building::GetRoom(string caption) const {
	for (unsigned int r = 0; r < pRooms.size(); r++) {
		if (pRooms[r]->GetCaption() == caption)
			return pRooms[r];
	}
	Log->Write("Warning: Room not found with caption " + caption);
	//return NULL;
	exit(EXIT_FAILURE);
}

void Building::AddCrossing(Crossing* line) {
	if (_crossings.count(line->GetIndex()) != 0) {
		char tmp[CLENGTH];
		sprintf(tmp,
				"ERROR: Duplicate index for crossing found [%d] in Routing::AddCrossing()",
				line->GetIndex());
		Log->Write(tmp);
		exit(EXIT_FAILURE);
	}
	_crossings[line->GetIndex()] = line;
}

void Building::AddTransition(Transition* line) {
	if (_transitions.count(line->GetIndex()) != 0) {
		char tmp[CLENGTH];
		sprintf(tmp,
				"ERROR: Duplicate index for transition found [%d] in Routing::AddTransition()",
				line->GetIndex());
		Log->Write(tmp);
		exit(EXIT_FAILURE);
	}
	_transitions[line->GetIndex()] = line;
}

void Building::AddHline(Hline* line) {
	if (_hLines.count(line->GetID()) != 0) {
		char tmp[CLENGTH];
		sprintf(tmp,
				"ERROR: Duplicate index for hlines found [%d] in Routing::AddGoal()",
				line->GetID());
		Log->Write(tmp);
		exit(EXIT_FAILURE);
	}
	_hLines[line->GetID()] = line;
}

const map<int, Crossing*>& Building::GetAllCrossings() const {
	return _crossings;
}

const map<int, Transition*>& Building::GetAllTransitions() const {
	return _transitions;
}


const map<int, Hline*>& Building::GetAllHlines() const {
	return _hLines;
}

Transition* Building::GetTransition(string caption) const {
	//eventually
	map<int, Transition*>::const_iterator itr;
	for(itr = _transitions.begin(); itr != _transitions.end(); ++itr){
		if (itr->second->GetCaption() == caption)
			return itr->second;
	}

	Log->Write("WARNING: No Transition with Caption: " + caption);
	exit(EXIT_FAILURE);
}

Transition* Building::GetTransition(int ID) {
	if (_transitions.count(ID) == 1) {
		return _transitions[ID];
	} else {
		if (ID == -1)
			return NULL;
		else {
			char tmp[CLENGTH];
			sprintf(tmp,
					"ERROR: Wrong 'index' [%d] > [%d] in Routing::GetTransition()",
					ID, _transitions.size());
			Log->Write(tmp);
			exit(EXIT_FAILURE);
		}
	}
}

Crossing* Building::GetGoal(string caption) const {

	{
		//eventually
		map<int, Transition*>::const_iterator itr;
		for(itr = _transitions.begin(); itr != _transitions.end(); ++itr){
			if (itr->second->GetCaption() == caption)
				return itr->second;
		}
	}
	{
		//finally the  crossings
		map<int, Crossing*>::const_iterator itr;
		for(itr = _crossings.begin(); itr != _crossings.end(); ++itr){
			if (itr->second->GetCaption() == caption)
				return itr->second;
		}
	}

	Log->Write("WARNING: No Transition with Caption: " + caption);
	//return NULL;
	exit(EXIT_FAILURE);
}

void Building::LoadRoutingInfo(string filename) {
	Log->Write("INFO:\tLoading extra routing information");
	if (filename == "") {
		Log->Write("INFO:\t No file supplied !");
		Log->Write("INFO:\t done with loading extra routing information");
		return;
	}

	XMLNode xMainNode = XMLNode::openFileHelper(filename.c_str(), "routing");

	double version = xmltof(xMainNode.getAttribute("version"), -1);
	if (version < 0.4) {
		Log->Write("ERROR: \tOnly version > 0.4 supported");
		Log->Write("ERROR: \tparsing routing file failed!");
		exit(EXIT_FAILURE);
	}

	//actually only contains one Hlines node
	XMLNode xHlinesNode = xMainNode.getChildNode("Hlines");
	int nHlines = xHlinesNode.nChildNode("Hline");

	//processing the rooms node
	for (int i = 0; i < nHlines; i++) {
		XMLNode hline = xHlinesNode.getChildNode("hline", i);
		double id = xmltof(hline.getAttribute("id"), -1);
		int room_id = xmltoi(hline.getAttribute("room_id"), -1);
		int subroom_id = xmltoi(hline.getAttribute("subroom_id"), -1);

		double x1 = xmltof(hline.getChildNode("vertex", 0).getAttribute("px"));
		double y1 = xmltof(hline.getChildNode("vertex", 0).getAttribute("py"));
		double x2 = xmltof(hline.getChildNode("vertex", 1).getAttribute("px"));
		double y2 = xmltof(hline.getChildNode("vertex", 1).getAttribute("py"));


		Room* room = pRooms[room_id];
		SubRoom* subroom = room->GetSubRoom(subroom_id);


		//new implementation
		Hline* h = new Hline();
		h->SetID(id);
		h->SetPoint1(Point(x1, y1));
		h->SetPoint2(Point(x2, y2));
		h->SetRoom(room);
		h->SetSubRoom(subroom);

		AddHline(h);
		subroom->AddHline(h);

	}


	Log->Write("INFO:\t done with loading extra routing information");
}

void Building::LoadTrafficInfo(string filename) {

	Log->Write("INFO:\tLoading  the traffic info file");

	if (filename == "") {
		Log->Write("INFO:\t No file supplied !");
		Log->Write("INFO:\t done with loading traffic info file");
		return;
	}

	XMLNode xMainNode = XMLNode::openFileHelper(filename.c_str(), "traffic");

	double version = xmltof(xMainNode.getAttribute("version"), -1);
	if (version < 0.4) {
		Log->Write("ERROR: \tOnly version > 0.4 supported");
		Log->Write("ERROR: \tparsing traffic file failed!");
		exit(EXIT_FAILURE);
	}

	//The file has two main nodes
	//<rooms> and <transitions>

	XMLNode xRoomsNode = xMainNode.getChildNode("rooms");
	int nRooms = xRoomsNode.nChildNode("room");

	//processing the rooms node
	for (int i = 0; i < nRooms; i++) {
		XMLNode xRoom = xRoomsNode.getChildNode("room", i);
		double id = xmltof(xRoom.getAttribute("room_id"), -1);
		string state = xmltoa(xRoom.getAttribute("state"), "good");
		int status = (state == "good") ? 0 : 1;
		pRooms[id]->SetRoomState(status);
	}

	//processing the doors node
	XMLNode xDoorsNode = xMainNode.getChildNode("doors");
	int nDoors = xDoorsNode.nChildNode("door");

	for (int i = 0; i < nDoors; i++) {
		XMLNode xDoor = xDoorsNode.getChildNode("door", i);
		int id = xmltoi(xDoor.getAttribute("trans_id"), -1);
		string state = xmltoa(xDoor.getAttribute("state"), "open");

		//maybe the door caption is specified ?
		if(id==-1){
			string caption=xmltoa(xDoor.getAttribute("caption"), "-1");
			if( (caption!="-1") && (state =="close") ){
				GetTransition(caption)->Close();
			}
		}
		else {
			//store transition in a map and call getTransition/getCrossin
			if (state == "open") {
				GetTransition(id)->Open();
			} else if (state == "close") {
				GetTransition(id)->Close();
			} else {
				char tmp[CLENGTH];
				sprintf(tmp, "WARNING:\t Unknown door state: %s", state.c_str());
				Log->Write(tmp);
			}
		}
	}
	Log->Write("INFO:\t done with loading traffic info file");
}


void Building::StringExplode(string str, string separator,
		vector<string>* results) {
	size_t found;
	found = str.find_first_of(separator);
	while (found != string::npos) {
		if (found > 0) {
			results->push_back(str.substr(0, found));
		}
		str = str.substr(found + 1);
		found = str.find_first_of(separator);
	}
	if (str.length() > 0) {
		results->push_back(str);
	}
}


// FIXME: you should get rid of this method
Crossing* Building::GetGoal(int index) {
	if (_transitions.count(index) == 1) {
		return _transitions[index];
	} else if (_crossings.count(index) == 1) {
		return _crossings[index];
	}else if (_hLines.count(index) == 1) {
		exit(EXIT_FAILURE);
		//return pHlines[index];
	}else {
		if (index == -1)
			return NULL;
		else {
			char tmp[CLENGTH];
			sprintf(tmp,
					"ERROR: Wrong 'index' [%d] > [%d] in Routing::GetGoal(), counts in map= [%d]",
					index, _crossings.size(),_crossings.count(index));
			Log->Write(tmp);
			exit(EXIT_FAILURE);
		}
	}
}
