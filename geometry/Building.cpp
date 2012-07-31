/*
 * File:   Building.cpp
 * Author: andrea
 *
 * Created on 1. October 2010, 09:25
 */

#include "Building.h"
#include "../general/xmlParser.h"
#include "Obstacle.h"


using namespace std;

/*************************************************************
 private Funktionen
 ************************************************************/

void Building::LoadHeader(ifstream* buildingfile, int* i) {
	string line;

	getline(*buildingfile, line);
	(*i)++;
	while (line.find("</header>") == string::npos) {
		istringstream iss(line, istringstream::in);
		string tmp; // Schlüsselwort, z.B.: "caption"
		if (line.find("caption") != string::npos) {
			iss >> tmp >> pCaption;
		} else if (line.find("version") != string::npos) {
			double version;
			iss >> tmp >> version;
			if (version != VERSION) {
				char tmp[CLENGTH];
				sprintf(tmp, "ERROR: \tBuilding::LoadHeader() "
						"neue Version im Geometrieformat!!! %f != %f", version,
						VERSION);
				Log->write(tmp);
				exit(0);
			}
		} else {
			char tmp[CLENGTH];
			sprintf(tmp, "ERROR: \tBuilding::LoadHeader() "
					"Wrong object in building file <header>: [%s] line %d ",
					line.c_str(), *i);
			Log->write(tmp);
			exit(0);
		}
		getline(*buildingfile, line);
		(*i)++;
	}
}

void Building::LoadRooms(ifstream* buildingfile, int* i) {
	string line;
	getline(*buildingfile, line);
	(*i)++;
	int elements = 0;
	while (line.find("</rooms>") == string::npos) {
		istringstream iss(line, istringstream::in);
		string tmp; // Schlüsselwort
		if (line.find("elements") != string::npos) {
			iss >> tmp >> elements;
		} else if (line.find("<room>") != string::npos) {
			LoadRoom(buildingfile, i);
		} else {
			char tmp[CLENGTH];
			sprintf(tmp, "ERROR: \tBuilding::LoadRooms() "
					"Wrong object in building file <rooms>: [%s] line %d ",
					line.c_str(), *i);
			Log->write(tmp);
			exit(0);
		}
		getline(*buildingfile, line);
		(*i)++;
	}
	if (elements != GetAnzRooms()) {
		char tmp[CLENGTH];
		sprintf(tmp, "ERROR: \tBuilding::LoadRooms() "
				"Wrong number of rooms: %d != %d", elements, pRooms.size());
		Log->write(tmp);
		exit(0);
	}
}

void Building::LoadRoom(ifstream* buildingfile, int* i) {
	string line;
	int NSubRooms = 0;
	int NStairs = 0;
	int anzNormal = 0;
	int anzStairs = 0;
	getline(*buildingfile, line);
	(*i)++;
	Room* room = new Room();
	while (line.find("</room>") == string::npos) {
		istringstream iss(line, istringstream::in);
		string tmp; // Schlüsselwort, z.B.: "caption"
		if (line.find("caption") != string::npos) {
			string caption;
			iss >> tmp >> caption;
			room->SetCaption(caption);
		} else if (line.find("index") != string::npos) {
			int ID;
			iss >> tmp >> ID;
			room->SetRoomID(ID);
		} else if (line.find("zpos") != string::npos) {
			float zpos;
			iss >> tmp >> zpos;
			room->SetZPos(zpos);
		} else if (line.find("nsubrooms") != string::npos) {
			iss >> tmp >> NSubRooms;
		} else if (line.find("nstairs") != string::npos) {
			iss >> tmp >> NStairs;
		} else if (line.find("<subroom>") != string::npos) {
			room->LoadNormalSubRoom(buildingfile, i);
			anzNormal++;
		} else if (line.find("<stair>") != string::npos) {
			room->LoadStair(buildingfile, i);
			anzStairs++;
		} else if (line.find("<crossings>") != string::npos) {
			pRouting->LoadCrossings(buildingfile, i, room);
		} else if (line.find("<Hlines>") != string::npos) {
			pRouting->LoadOrientationLines(buildingfile, i, room);
		} else {
			char tmp[CLENGTH];
			sprintf(tmp, "ERROR: \tBuilding::LoadRoom() "
					"Wrong object in building file <room> :"
					"[%s] line %d ", line.c_str(), *i);
			Log->write(tmp);
			exit(0);
		}
		getline(*buildingfile, line);
		(*i)++;
	}
	if (NSubRooms != anzNormal) {
		char tmp[CLENGTH];
		sprintf(tmp, "ERROR: \tBuilding::LoadRoom() "
				"Wrong number of subrooms: %d != %d", NSubRooms, anzNormal);
		Log->write(tmp);
		exit(0);
	}

	if (NStairs != anzStairs) {
		char tmp[CLENGTH];
		sprintf(tmp, "ERROR: \tBuilding::LoadRoom() "
				"Wrong number of subrooms: %d != %d", NStairs, anzStairs);
		Log->write(tmp);
		exit(0);
	}
	AddRoom(room);
}

/************************************************************
 Konstruktoren
 ************************************************************/

Building::Building() {
	pCaption = "no_caption";
	pRooms = vector<Room*> ();
	pRouting = NULL;
	pLinkedCellGrid = NULL;
	pSavePathway=false;
	//MPI:
	pPedtransfering = vector<Pedestrian*> ();
}

Building::Building(const Building& orig) {
	pCaption = orig.GetCaption();
	pRooms = orig.GetAllRooms();
	pRouting = orig.GetRouting();
}

Building::~Building() {
	for (int i = 0; i < GetAnzRooms(); i++)
		delete pRooms[i];
	delete pRouting;

	delete pLinkedCellGrid;

	if(PpathWayStream.is_open())
		PpathWayStream.close();
}

/*************************************************************
 Setter-Funktionen
 ************************************************************/
void Building::SetCaption(string s) {
	pCaption = s;
}

void Building::SetRouting(Routing* r) {
	pRouting = r;
}

void Building::SetAllRooms(const vector<Room*>& rooms) {
	pRooms = rooms;
}

void Building::SetRoom(Room* room, int index) {
	if ((index >= 0) && (index < (int) pRooms.size())) {
		pRooms[index] = room;
	} else {
		Log->write("ERROR: \tWrong Index in CBuilding::SetRoom()");
		exit(0);
	}
}

/*************************************************************
 Getter-Funktionen
 ************************************************************/

string Building::GetCaption() const {
	return pCaption;
}

Routing* Building::GetRouting() const {
	return pRouting;
}

int Building::GetAnzRooms() const {
	return pRooms.size();
}

const vector<Room*>& Building::GetAllRooms() const {
	return pRooms;
}

Room* Building::GetRoom(int index) const {
	if ((index >= 0) && (index < (int) pRooms.size())) {
		return pRooms[index];
	} else {
		char tmp[CLENGTH];
		sprintf(
				tmp,
				"ERROR: Wrong 'index' in CBuiling::GetRoom() index: %d size: %d",
				index, pRooms.size());
		Log->write(tmp);
		exit(0);
	}
}

int Building::GetAnzPedestrians() const {
	int sum = 0;
	const vector<int>& workingArea = pMPIDispatcher->GetMyWorkingArea();
	for (unsigned int wa = 0; wa < workingArea.size(); wa++) {
		sum += pRooms[workingArea[wa]]->GetAnzPedestrians();
	}
	return sum;
}

LCGrid* Building::GetGrid() const {
	return pLinkedCellGrid;
}
/*************************************************************
 Sonstiges
 ************************************************************/

void Building::AddRoom(Room* room) {
	pRooms.push_back(room);
}


void Building::AddSurroundingRoom(){
	Log->write("INFO: \tAdding the room 'outside' or 'world' ");
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

	SubRoom* bigSubroom= new NormalSubRoom();
	bigSubroom->AddWall(Wall (Point(x_min,y_min), Point(x_min,y_max)));
	bigSubroom->AddWall(Wall (Point(x_min,y_max), Point(x_max,y_max)));
	bigSubroom->AddWall(Wall (Point(x_max,y_max), Point(x_max,y_min)));
	bigSubroom->AddWall(Wall (Point(x_max,y_min), Point(x_min,y_min)));

	Room * bigRoom = new Room();
	bigRoom->AddSubRoom(bigSubroom);
	bigRoom->SetCaption("outside");
	bigRoom->SetRoomID(pRooms.size());
	AddRoom(bigRoom);
}

void Building::InitGeometry() {
	Log->write("INFO: \tInit Geometry");
	for (int i = 0; i < GetAnzRooms(); i++) {
		Room* room = GetRoom(i);
		// Polygone berechnen
		for (int j = 0; j < room->GetAnzSubRooms(); j++) {
			SubRoom* s = room->GetSubRoom(j);
			// Alle Übergänge in diesem Raum bestimmen
			// Übergänge müssen zu Wänden ergänzt werden
			// dabei werden die Hilfslinien ignoriert
			vector<Line*> goals = vector<Line*> ();
			for (int k = 0; k < pRouting->GetAnzGoals(); k++) {
				Crossing* goal = pRouting->GetGoal(k);
				if (goal->GetSubRoom1() != goal->GetSubRoom2()) // Kennzeichen fuer Hlines
					if (goal->IsInRoom(i) && goal->IsInSubRoom(j)) {
						goals.push_back(goal);
					}
			}
			// anschliessend ist pPoly initialisiert
			s->ConvertLineToPoly(goals);
			s->CalculateArea();
			goals.clear();
		}
	}
	Log->write("INFO: \tInit Geometry sucessfull!!!\n");
}

void Building::Update() {
	// some peds may change the room via another crossing than the primary intended one
	// in that case they are set in the wrong room.
	vector<Pedestrian*> nonConformPeds;

	const vector<int>& workingArea = pMPIDispatcher->GetMyWorkingArea();

	for (unsigned int wa = 0; wa < workingArea.size(); wa++) {
		int roomID = workingArea[wa];
		Room* room = pRooms[roomID];

		for (int j = 0; j < room->GetAnzSubRooms(); j++) {
			SubRoom* sub = room->GetSubRoom(j);
			for (int k = 0; k < sub->GetAnzPedestrians(); k++) {
				Pedestrian* ped = sub->GetPedestrian(k);
				//set the new room if needed
				if (!sub->IsInSubRoom(ped)) {
					Crossing* cross = pRouting->GetGoal(ped->GetExitIndex());
					// the peds has changed the room and is farther than 50 cm from
					// the exit, thats a real problem.
					if (ped->GetExitLine()->DistTo(ped->GetPos()) > 0.50) {
						char tmp[CLENGTH];
						sprintf(
								tmp,
								"WARNING: Building::update() pedestrian [%d] left the room/subroom [%s][%d/%d] "
								"via unknown exit[??%d] Position: (%f, %f)",
								ped->GetPedIndex(),
								pRooms[ped->GetRoomID()]->GetCaption().c_str(),
								ped->GetRoomID(), ped->GetSubRoomID(),
								ped->GetExitIndex(), ped->GetPos().GetX(),
								ped->GetPos().GetY());
						//ped->Dump(ped->GetPedIndex());
						Log->write(tmp);
						//exit(0);
						//DeletePedestrian(ped);
						nonConformPeds.push_back(ped);
						sub->DeletePedestrian(k); //k--;
						continue; // next pedestrian
					}

					SubRoom* other_sub = cross->GetOtherSubRoom(roomID, j);
					if (other_sub) {
						int nextSubRoom = other_sub->GetSubRoomID();
						int nextRoom = other_sub->GetRoomID();
						ped->SetSubRoomID(nextSubRoom);
						ped->SetRoomID(nextRoom,GetRoom(nextRoom)->GetCaption());
						// MPI:
						// add only if this is my working area
						if (pMPIDispatcher->IsMyWorkingArea(nextRoom)) {
							other_sub->AddPedestrian(ped);
						} else {
							pPedtransfering.push_back(ped);
							DeletePedestrian(ped);
							//continue;
						}
					} else {
						DeletePedestrian(ped);
						//continue;
					}
					// Lösche Fußgänger aus aktuellem SubRoom
					sub->DeletePedestrian(k); // k--;
				}
				// neues Ziel setzten
				//pRouting->FindExit(ped);
			}
		}
	}

	// reset that pedestrians who left their room not via the intended exit
	for (int p = 0; p < (int) nonConformPeds.size(); p++) {
		Pedestrian* ped = nonConformPeds[p];
		bool assigned = false;
		for (int i = 0; i < GetAnzRooms(); i++) {
			Room* room = GetRoom(i);
			for (int j = 0; j < room->GetAnzSubRooms(); j++) {
				SubRoom* sub = room->GetSubRoom(j);
				//only relocate in the same room
				// or only in neighbouring rooms
				if (i != ped->GetRoomID())
					continue;
				if (sub->IsInSubRoom(ped->GetPos())) {
					//set in the new room
					char tmp[CLENGTH];
					sprintf(
							tmp,
							"pedestrian %d relocated from room/subroom [%s] %d/%d to [%s] %d/%d ",
							ped->GetPedIndex(),
							GetRoom(ped->GetRoomID())->GetCaption().c_str(),
							ped->GetRoomID(), ped->GetSubRoomID(),
							room->GetCaption().c_str(), i, j);
					Log->write(tmp);
					ped->SetRoomID(i,room->GetCaption());
					ped->SetSubRoomID(j);
					ped->ClearMentalMap(); // reset the destination
					pRouting->FindExit(ped);
					sub->AddPedestrian(ped);
					assigned = true;
					break;
				}
			}
			if (assigned == true)
				break; // stop the loop
		}
		if (assigned == false) {
			DeletePedestrian(ped);
		}
	}

	// find the new goals, the parallel way

	const unsigned int nSize = pAllPedestians.size();
	int nThreads = omp_get_max_threads(); //nThreads=1;
	// check if worth sharing the work
	if (nSize < 12)
		nThreads = 1;
	int partSize = nSize / nThreads;

#pragma omp parallel  default(shared) num_threads(nThreads)
	{
		const int threadID = omp_get_thread_num();
		int start = threadID * partSize;
		int end = (threadID + 1) * partSize - 1;
		if ((threadID == nThreads - 1))
			end = nSize - 1;

		for (int p = start; p <= end; ++p) {

			if(pRouting->FindExit(pAllPedestians[p])==-1){
				//a destination could not be found for that pedestrian
				DeletePedFromSim(pAllPedestians[p]);
			}
		}
	}

	//cleaning up. removing some long standing pedestrians
	//CleanUpTheScene();
}

void Building::InitPhiAllPeds() {
	const vector<int>& workingArea = pMPIDispatcher->GetMyWorkingArea();
	for (unsigned int wa = 0; wa < workingArea.size(); wa++) {
		Room* room = GetRoom(workingArea[wa]);
		for (int j = 0; j < room->GetAnzSubRooms(); j++) {
			SubRoom* sub = room->GetSubRoom(j);
			for (int k = 0; k < sub->GetAnzPedestrians(); k++) {
				double cosPhi, sinPhi;
				Pedestrian* ped = sub->GetPedestrian(k);
				Line* e = ped->GetExitLine();
				const Point& e1 = e->GetPoint1();
				const Point& e2 = e->GetPoint2();
				Point target = (e1 + e2) * 0.5;
				Point d = target - ped->GetPos();
				double dist = d.Norm();
				if (dist != 0.0) {
					cosPhi = d.GetX() / dist;
					sinPhi = d.GetY() / dist;
				} else {
					Log->write(
							"ERROR: \tBuilding::InitPhiAllPeds() cannot initialise phi! "
							"dist to target ist 0\n");
					exit(0);
				}

				Ellipse E = ped->GetEllipse();
				E.SetCosPhi(cosPhi);
				E.SetSinPhi(sinPhi);
				ped->SetEllipse(E);
				ped->SetRoomID(room->GetRoomID(),room->GetCaption());
			}
		}
	}
}

void Building::UpdateGrid() {
	pLinkedCellGrid->Update(pAllPedestians);
}

void Building::InitGrid(double cellSize) {

	char tmp[CLENGTH];
	sprintf(tmp,"INFO: \tInitializing the grid with cell size: %f ",cellSize);
	Log->write(tmp);
	// first look for the geometry boundaries
	double x_min = FLT_MAX;
	double x_max = FLT_MIN;
	double y_min = FLT_MAX;
	double y_max = FLT_MIN;

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

	const vector<int>& workingArea = pMPIDispatcher->GetMyWorkingArea();
	for (unsigned int wa = 0; wa < workingArea.size(); wa++) {
		Room* room = GetRoom(workingArea[wa]);
		for (int j = 0; j < room->GetAnzSubRooms(); j++) {
			SubRoom* sub = room->GetSubRoom(j);
			for (int k = 0; k < sub->GetAnzPedestrians(); k++) {
				Pedestrian* ped = sub->GetPedestrian(k);
				pAllPedestians.push_back(ped);
			}
		}
	}

	//make the grid slightly larger.
	x_min = x_min - 1.0;
	x_max = x_max + 1.0;
	y_min = y_min - 1.0;
	y_max = y_max + 1.0;

	double boundaries[] = { x_min, x_max, y_min, y_max };
	//int pedsCount=pAllPedestians.size();
	int pedsCount = pMPIDispatcher->GetGlobalPedestriansCount();
	//cout<<"received: "<<pedsCount<<" pedestrians"<<endl;
	pLinkedCellGrid = new LCGrid(boundaries, cellSize, pedsCount);
	pLinkedCellGrid->ShallowCopy(pAllPedestians);

	Log->write("INFO: \tDone with Initializing the grid ");
}

/*************************************************************
 Ein-Ausgabe
 ************************************************************/

void Building::LoadBuilding(string filename) {

	Log->write("INFO: \tParsing the geometry file");

	XMLNode xMainNode=XMLNode::openFileHelper(filename.c_str(),"geometry");

	double version=xmltof(xMainNode.getAttribute("version"),-1);
	if(version<0.4){
		Log->write("ERROR: \tOnly version > 0.4 supported");
		Log->write("ERROR: \tparsing geometry file failed!");
		exit(EXIT_FAILURE);
	}
	pCaption=xmltoa(xMainNode.getAttribute("caption"),"virtual building");

	//The file has two main nodes
	//<rooms> and <transitions>

	XMLNode xRoomsNode = xMainNode.getChildNode("rooms");
	int nRooms=xRoomsNode.nChildNode("room");

	//processing the rooms node
	for(int i=0;i<nRooms;i++){
		XMLNode xRoom = xRoomsNode.getChildNode("room",i);
		Room* room = new Room();

		string room_id=xmltoa(xRoom.getAttribute("id"),"-1");
		room->SetRoomID(xmltoi(room_id.c_str(),-1));

		string caption="room " + room_id;
		room->SetCaption(xmltoa(xRoom.getAttribute("caption"),caption.c_str() ));

		room->SetZPos(xmltoi(xRoom.getAttribute("zpos"),0.0));

		//parsing the subrooms
		int nSubRooms=xRoom.nChildNode("subroom");

		for(int s=0;s<nSubRooms;s++){
			XMLNode xSubroomsNode = xRoom.getChildNode("subroom",s);

			string subroom_id= xmltoa(xSubroomsNode.getAttribute("id"),"-1");
			string closed= xmltoa(xSubroomsNode.getAttribute("closed"),"0");
			string type= xmltoa(xSubroomsNode.getAttribute("class"),"subroom");

			SubRoom* subroom=NULL;

			if(type=="stair"){
				subroom= new Stair();
			}else{
				//normal subroom or corridor
				subroom= new NormalSubRoom();
			}

			subroom->SetRoomID(room->GetRoomID());
			subroom->SetSubRoomID(xmltoi(subroom_id.c_str(),-1));

			//looking for polygones (walls)
			int nPoly=xSubroomsNode.nChildNode("polygone");
			for(int p=0;p<nPoly;p++){
				XMLNode xPolyVertices=xSubroomsNode.getChildNode("polygone",p);
				int nVertices=xSubroomsNode.getChildNode("polygone",p).nChildNode("vertex");


				for(int v=0;v<nVertices-1;v++){
					double x1=xmltof(xPolyVertices.getChildNode("vertex",v).getAttribute("px"));
					double y1=xmltof(xPolyVertices.getChildNode("vertex",v).getAttribute("py"));

					double x2=xmltof(xPolyVertices.getChildNode("vertex",v+1).getAttribute("px"));
					double y2=xmltof(xPolyVertices.getChildNode("vertex",v+1).getAttribute("py"));
					subroom->AddWall(Wall(Point(x1, y1), Point(x2, y2)));
				}

			}

			//looking for obstacles
			int nObst=xSubroomsNode.nChildNode("obstacle");
			for(int obst=0;obst<nObst;obst++){
				XMLNode xObstacle=xSubroomsNode.getChildNode("obstacle",obst);
				int nPoly=xObstacle.nChildNode("polygone");
				int id= xmltof(xObstacle.getAttribute("id"),-1);
				int height= xmltof(xObstacle.getAttribute("height"),0);
				double closed= xmltof(xObstacle.getAttribute("closed"),0);
				string caption= xmltoa(xObstacle.getAttribute("caption"),"-1");

				Obstacle* obstacle= new Obstacle();
				obstacle->SetId(id);
				obstacle->SetCaption(caption);
				obstacle->SetClosed(closed);
				obstacle->SetHeight(height);

				for(int p=0;p<nPoly;p++){
					XMLNode xPolyVertices=xObstacle.getChildNode("polygone",p);
					int nVertices=xObstacle.getChildNode("polygone",p).nChildNode("vertex");

					for(int v=0;v<nVertices-1;v++){
						double x1=xmltof(xPolyVertices.getChildNode("vertex",v).getAttribute("px"));
						double y1=xmltof(xPolyVertices.getChildNode("vertex",v).getAttribute("py"));

						double x2=xmltof(xPolyVertices.getChildNode("vertex",v+1).getAttribute("px"));
						double y2=xmltof(xPolyVertices.getChildNode("vertex",v+1).getAttribute("py"));
						obstacle->AddWall(Wall(Point(x1, y1), Point(x2, y2)));
					}
				}
				subroom->AddObstacle(obstacle);
			}
			room->AddSubRoom(subroom);
		}

		//parsing the crossings
		XMLNode xCrossingsNode = xRoomsNode.getChildNode("crossings");
		int nCrossing =xCrossingsNode.nChildNode("crossing");


		//processing the rooms node
		for(int i=0;i<nCrossing;i++){
			XMLNode xCrossing = xCrossingsNode.getChildNode("crossing",i);
			Crossing* crossing = new crossing();

			string room_id=xmltoa(xRoom.getAttribute("id"),"-1");
			room->SetRoomID(xmltoi(room_id.c_str(),-1));



		AddRoom(room);
	}

		exit(0);

	if(!xMainNode.getChildNode("seed").isEmpty()){
		const char* seed=xMainNode.getChildNode("seed").getText();
		Log->write("INFO: \tseed <"+string(seed)+">");
	}




	ifstream buildingfile;
	string line;

	// Datei oeffnen
	buildingfile.open(filename.c_str(), fstream::in);
	if (!buildingfile) {
		Log->write("ERROR: \tCannot load building file: " + filename);
		exit(0);
	} else {
		Log->write("INFO: \tLoading building file");
		int i = 0;
		while (getline(buildingfile, line)) {
			i++; // Zeilenindex zum debuggen, nach jedem getline() erhöhen
			if (line.find("<header>") != string::npos) {
				LoadHeader(&buildingfile, &i);
			} else if (line.find("<rooms>") != string::npos) {
				LoadRooms(&buildingfile, &i);
			} else if (line.find("<transitions>") != string::npos) {
				pRouting->LoadTransitions(&buildingfile, &i, pRooms);
			} else {
				char tmp[CLENGTH];
				sprintf(tmp, "ERROR: \tBuilding::LoadBuilding() "
						"Wrong object in building file: [%s] line %d ",
						line.c_str(), i);
				Log->write(tmp);
				exit(0);
			}
		}

		// load the manual transitions
		//pRouting->LoadAdditionalCrossings();
		buildingfile.close();
		buildingfile.clear();
		Log->write("INFO: \tLoading building file sucessfull!!!\n");
	}
}

void Building::DumpSubRoomInRoom(int roomID, int subID) {
	SubRoom* sub = GetRoom(roomID)->GetSubRoom(subID);
	if (sub->GetAnzPedestrians() == 0)
		return;
	cout << "dumping room/subroom " << roomID << " / " << subID << endl;
	for (int p = 0; p < sub->GetAnzPedestrians(); p++) {
		Pedestrian* ped = sub->GetPedestrian(p);
		cout << " ID: " << ped->GetPedIndex();
		cout << " Index: " << p << endl;
	}

}

void Building::WriteToErrorLog() const {
	Log->write("GEOMETRY: ");
	for (int i = 0; i < GetAnzRooms(); i++) {
		Room* r = GetRoom(i);
		r->WriteToErrorLog();
	}
	Log->write("ROUTING: ");
	pRouting->WriteToErrorLog();
	Log->write("\n");
}

Room* Building::GetRoom(string caption) const {
	for (unsigned int r = 0; r < pRooms.size(); r++) {
		if (pRooms[r]->GetCaption() == caption)
			return pRooms[r];
	}
	Log->write("Warning: Room not found with caption " + caption);
	return NULL;
	//exit(EXIT_FAILURE);
}

Transition* Building::GetTransition(string caption) const {
	for (unsigned int r = 0; r < pRooms.size(); r++) {
		const vector<int>& trans_ids = pRooms[r]->GetAllTransitionsIDs();
		for (unsigned int t = 0; t < trans_ids.size(); t++) {
			int id = trans_ids[t];
			Transition* tr = static_cast<Transition*> (pRouting->GetGoal(id));
			if (tr->GetCaption() == caption)
				return tr;
		}

	}
	Log->write("WARNING: No Transition with Caption: " + caption);
	//return NULL;
	exit(EXIT_FAILURE);
}

Crossing* Building::GetGoal(string caption) const {
	const vector<Crossing*>& allGoals =  GetRouting()->GetAllGoals();

	for (unsigned int g=0;g<allGoals.size();g++){
		Crossing* cr= pRouting->GetGoal(g);
		if (cr->GetCaption() == caption)
			return cr;
	}

	Log->write("WARNING: No Transition with Caption: " + caption);
	//return NULL;
	exit(EXIT_FAILURE);
}

void Building::LoadStatesOfRooms(string filename) {
	ifstream infile;
	string line;
	string output;

	if (filename == "") {
		Log->write("WARNING:\tempty rooms states file");
		return;
	}
	Log->write("INFO:\tLoading rooms states file");
	infile.open(filename.c_str(), fstream::in);
	if (infile.is_open() == false) {
		Log->write(
				"ERROR: \tCannot load rooms initialisation files: " + filename);
		exit(EXIT_FAILURE);
	} else {

		while (getline(infile, line)) {
			if (line.find("#") != 0) { // Kommentarzeile wird überlesen
				istringstream iss(line, istringstream::in);
				string room_caption = "";
				int state = -1;
				iss >> room_caption >> state;
				if ((room_caption != "") && (state != -1)) {
					Room* r = GetRoom(room_caption);
					if (!r)
						continue;
					r->SetRoomState(state);
				} else {
					Log->write("ERROR:\t Wrong entry in the room states file:");
					Log->write(line);
					exit(EXIT_FAILURE);
				}
			}
		}
	}
	Log->write("INFO:\t done with loading rooms states file");
}

void Building::LoadStatesOfDoors(string filename) {
	ifstream infile;
	string line;
	string output;

	if (filename == "") {
		Log->write("WARNING:\tempty doors states file");
		return;
	}
	Log->write("INFO:\tLoading doors states file");
	infile.open(filename.c_str(), fstream::in);
	if (infile.is_open() == false) {
		Log->write(
				"ERROR: \tCannot load doors initialisation files: " + filename);
		exit(EXIT_FAILURE);
	} else {

		while (getline(infile, line)) {
			if (line.find("#") != 0) { // Kommentarzeile wird überlesen
				istringstream iss(line, istringstream::in);
				string door_caption = "";
				int state = -1;
				iss >> door_caption >> state;
				if ((door_caption != "") && (state != -1)) {

					if (GetTransition(door_caption) == NULL)
						continue;
					//look for that door
					if (state == 0) {
						GetTransition(door_caption)->Open();
						Log->write("INFO: Opening door " + door_caption);
					} else if (state == 1) {
						GetTransition(door_caption)->Close();
						Log->write("INFO: Closing door " + door_caption);

						//special case of doors AR0800315 and AR0800317
						if ((door_caption == "AR0800315") || (door_caption
								== "AR0800317")) {
							door_caption = door_caption + "a";
							GetTransition(door_caption)->Close();
							Log->write("INFO: Closing door " + door_caption);
						}
					}

				} else {
					Log->write("ERROR:\t Wrong entry in the doors states file:");
					Log->write(line);
					exit(EXIT_FAILURE);
				}
			}
		}
	}
	Log->write("INFO:\t done with loading doors states file");
}

void Building::DeletePedestrian(Pedestrian* ped) {
	vector<Pedestrian*>::iterator it;
	it = find(pAllPedestians.begin(), pAllPedestians.end(), ped);
	if (it == pAllPedestians.end()) {
		cout << " Ped not found" << endl;
	} else {
		//save the path history for this pedestrian before removing from the simulation
		if(pSavePathway){
			string results;
			string path=(*it)->GetPath();
			vector<string> brokenpaths;
			StringExplode(path, ">",&brokenpaths);
			for(unsigned int i=0;i<brokenpaths.size();i++){
				vector<string> tags;
				StringExplode(brokenpaths[i], ":",&tags);
				string room=pRooms[atoi(tags[0].c_str())]->GetCaption();
				string trans=pRouting->GetAllGoals()[atoi(tags[1].c_str())]->GetCaption();
				//ignore crossings/hlines
				if(trans!="")
				PpathWayStream<<room <<" "<<trans<<endl;
			}

		}
		cout << "deleting " << (*it)->GetPedIndex() << endl;
		pAllPedestians.erase(it);
	}
}

void Building::DeletePedFromSim(Pedestrian* ped){
	SubRoom* sub = pRooms[ped->GetRoomID()]->GetSubRoom(
			ped->GetSubRoomID());
	for (int p = 0; p < sub->GetAnzPedestrians(); p++) {
		if (sub->GetPedestrian(p)->GetPedIndex()
				== ped->GetPedIndex()) {
			sub->DeletePedestrian(p);
			DeletePedestrian(ped);
			return;
		}
	}
}

const vector<Pedestrian*>& Building::GetAllPedestrians() const {
	return pAllPedestians;
}

void Building::AddPedestrian(Pedestrian* ped) {

	//	for(unsigned int p = 0;p<pAllPedestians.size();p++){
	//		Pedestrian* ped1=pAllPedestians[p];
	//		if(ped->GetPedIndex()==ped1->GetPedIndex()){
	//			cout<<"Pedestrian already in the room ??? "<<ped->GetPedIndex()<<endl;
	//			return;
	//		}
	//	}
	pAllPedestians.push_back(ped);

}

void Building::SetMPIDispatcher(MPIDispatcher *mpi) {
	pMPIDispatcher = mpi;
}

MPIDispatcher* Building::GetMPIDispatcher() const {
	return pMPIDispatcher;
}

void Building::AddPedestrianWaitingForTransfer(Pedestrian* ped) {
	pPedtransfering.push_back(ped);
}

void Building::GetPedestriansTransferringToRoom(int roomID,
		vector<Pedestrian*>& transfer) {

	for (unsigned int p = 0; p < pPedtransfering.size(); p++) {
		Pedestrian* ped = pPedtransfering[p];
		if (ped->GetRoomID() == roomID) {
			transfer.push_back(ped);
			pPedtransfering.erase(pPedtransfering.begin() + p);
			printf("transferring [%d] to Room [%s][%d] \n", ped->GetPedIndex(),
					pRooms[roomID]->GetCaption().c_str(), ped->GetRoomID());
		}
	}
}

void Building::InitRoomsAndSubroomsMap(){
	Log->write("INFO: \tcreating the rooms maps!!!\n");

	for (int i=0;i<16;i++)
		for (int j=0;j<130;j++)
			for (int k=0;k<16;k++)
				for (int l=0;l<130;l++)
					pSubroomConnectionMap[i][j][k][l]=0;

	//create the subroom connections map
	for (unsigned int r1=0;r1< pRooms.size();r1++){
		Room* room1= GetRoom(r1);
		const vector <SubRoom*> sb1s=room1->GetAllSubRooms();

		for (unsigned int r2=0;r2<pRooms.size();r2++){
			Room* room2= GetRoom(r2);
			const vector <SubRoom*> sb2s=room2->GetAllSubRooms();

			// now looping over all subrooms
			for(unsigned int s1=0;s1<sb1s.size();s1++){
				for(unsigned int s2=0;s2<sb2s.size();s2++){
					if(sb1s[s1]->IsDirectlyConnectedWith(sb2s[s2])){
						pSubroomConnectionMap[r1][s1][r2][s2]=1;
						//cout<<"connected"<<endl;
					}else{
						pSubroomConnectionMap[r1][s1][r2][s2]=0;
					}
				}
			}
		}
	}
}

void Building::InitSavePedPathway(string filename){
	PpathWayStream.open(filename.c_str());
	pSavePathway=true;

	if (PpathWayStream.is_open()){
		Log->write("#INFO:\tsaving pedestrian paths to [ "+filename+" ]");
		PpathWayStream<<"##pedestrian ways"<<endl;
		PpathWayStream<<"#nomenclature roomid  caption"<<endl;
		//		for (unsigned int r=0;r< pRooms.size();r++){
		//			Room* room= GetRoom(r);
		//			const vector<int>& goals=room->GetAllTransitionsIDs();
		//
		//			for(unsigned int g=0;g<goals.size();g++){
		//				int exitid=goals[g];
		//				string exit_caption=pRouting->GetGoal(exitid)->GetCaption();
		//				PpathWayStream<<exitid<<" "<<exit_caption<<endl;
		//			}
		//		}
		//
		PpathWayStream<<"#data room exit_id"<<endl;
	}
	else{
		Log->write("#INFO:\t Unable to open [ "+filename+" ]");
		Log->write("#INFO:\t saving to stdout");

	}
}

// just for control should always be empty
void Building::ClearTranfer() {
	if (pPedtransfering.size() > 0){
		printf(" there should not be any pedestrian waiting to be transfered");
		exit(0);
	}
}

void Building::CleanUpTheScene() {
	//return;
	static int counter = 0;
	counter++;
	static int totalSliced=0;

	int updateRate = 80.0 / 0.01; // 20 seconds/pDt

	if (counter % updateRate == 0) {
		for (unsigned int i = 0; i < pAllPedestians.size(); i++) {
			Pedestrian* ped = pAllPedestians[i];

			if (ped->GetDistanceSinceLastRecord() < 0.1) {
				//delete from the simulation
				DeletePedFromSim(ped);

				totalSliced++;
				char msg[CLENGTH];
				sprintf(msg, "INFO:\t slicing Ped %d from room %s, total [%d]",
						ped->GetPedIndex(),
						pRooms[ped->GetRoomID()]->GetCaption().c_str(),totalSliced);
				Log->write(msg);
			} else {
				ped->RecordActualPosition();
			}

		}
	}

}

bool  Building::IsDirectlyConnected(int room1, int subroom1, int room2,
		int subroom2) {
	return pSubroomConnectionMap[room1][subroom1][room2][subroom2];
}

void Building::StringExplode(string str, string separator, vector<string>* results){
	size_t found;
	found = str.find_first_of(separator);
	while(found != string::npos){
		if(found > 0){
			results->push_back(str.substr(0,found));
		}
		str = str.substr(found+1);
		found = str.find_first_of(separator);
	}
	if(str.length() > 0){
		results->push_back(str);
	}
}
