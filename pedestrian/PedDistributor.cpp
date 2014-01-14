/**
 * File:   PedDistributor.cpp
 *
 * Created on 12. October 2010, 10:52
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

#include "PedDistributor.h"
#include "../tinyxml/tinyxml.h"
#include "../geometry/Obstacle.h"
#include "../routing/RoutingEngine.h"
#include "../pedestrian/Pedestrian.h"

using namespace std;


/************************************************************
 StartDistributionRoom
 ************************************************************/
StartDistributionRoom::StartDistributionRoom() {
	_roomID = -1;
	_nPeds = -1;
	_groupID = -1;
	_goalID = -1;
	_routerID = -1;
	_routeID = -1;
	_age = -1;
	_height = -1;
	_startX = NAN;
	_startY = NAN;
	_startZ = NAN;
	_gender = "male";
}

StartDistributionRoom::~StartDistributionRoom() {
}


int StartDistributionRoom::GetAgentsNumber() const {
	return _nPeds;
}


// Setter-Funktionen

void StartDistributionRoom::SetRoomID(int id) {
	_roomID = id;
}

int StartDistributionRoom::GetAge() const {
	return _age;
}

void StartDistributionRoom::SetAge(int age) {
	_age = age;
}

const std::string& StartDistributionRoom::GetGender() const {
	return _gender;
}

void StartDistributionRoom::SetGender(const std::string& gender) {
	_gender = gender;
}

int StartDistributionRoom::GetGoalId() const {
	return _goalID;
}

void StartDistributionRoom::SetGoalId(int goalId) {
	_goalID = goalId;
}

int StartDistributionRoom::GetGroupId() const {
	return _groupID;
}

void StartDistributionRoom::SetGroupId(int groupId) {
	_groupID = groupId;
}

int StartDistributionRoom::GetHeight() const {
	return _height;
}

void StartDistributionRoom::SetHeight(int height) {
	_height = height;
}

int StartDistributionRoom::GetRoomId() const {
	return _roomID;
}

void StartDistributionRoom::SetRoomId(int roomId) {
	_roomID = roomId;
}

int StartDistributionRoom::GetRouteId() const {
	return _routeID;
}

void StartDistributionRoom::SetRouteId(int routeId) {
	_routeID = routeId;
}

int StartDistributionRoom::GetRouterId() const {
	return _routerID;
}

void StartDistributionRoom::SetRouterId(int routerId) {
	_routerID = routerId;
}

void StartDistributionRoom::SetAgentsNumber(int N) {
	_nPeds = N;
}


/************************************************************
 StartDistributionSubRoom
 ************************************************************/
StartDistributionSubroom::StartDistributionSubroom() : StartDistributionRoom() {
	_subroomID = -1;
}


StartDistributionSubroom::~StartDistributionSubroom() {
}

int StartDistributionSubroom::GetSubroomID() const {
	return _subroomID;
}

// Setter-Funktionen

void StartDistributionSubroom::SetSubroomID(int i) {
	_subroomID = i;
}


/************************************************************
 PedDistributor
 ************************************************************/

PedDistributor::PedDistributor() {
	_v0 = new Equal(1.24, 0.26);
	_Bmax = new Equal(0.25, 0.001);
	_Bmin = new Equal(0.2, 0.001);
	_Atau = new Equal(0.53, 0.001);
	_Amin = new Equal(0.18, 0.001);
	_Tau = new Equal(0.5, 0.001);
}

PedDistributor::PedDistributor(double v0mu, double v0sigma, double BmaxMu, double BmaxSigma,
		double BminMu, double BminSigma, double AtauMu, double AtauSigma, double AminMu,
		double AminSigma, double tauMu, double tauSigma) {
	_v0 = new Equal(v0mu, v0sigma);
	_Bmax = new Equal(BmaxMu, BmaxSigma);
	_Bmin = new Equal(BminMu, BminSigma);
	_Atau = new Equal(AtauMu, AtauSigma);
	_Amin = new Equal(AminMu, AminSigma);
	_Tau = new Equal(tauMu, tauSigma);
	_start_dis = vector<StartDistributionRoom* > ();
	_start_dis_sub = vector<StartDistributionSubroom* > ();
}


PedDistributor::~PedDistributor() {
	delete _v0;
	delete _Bmax;
	delete _Bmin;
	delete _Atau;
	delete _Amin;
	delete _Tau;

	for (unsigned int i = 0; i < _start_dis.size(); i++) {
		delete _start_dis[i];
	}
	for (unsigned int i = 0; i < _start_dis_sub.size(); i++) {
		delete _start_dis_sub[i];
	}
	_start_dis_sub.clear();
	_start_dis.clear();
}


Distribution* PedDistributor::GetV0() const {
	return _v0;
}

Distribution* PedDistributor::GetBmax() const {
	return _Bmax;
}

Distribution* PedDistributor::GetBmin() const {
	return _Bmin;
}

Distribution* PedDistributor::GetAtau() const {
	return _Atau;
}

Distribution* PedDistributor::GetAmin() const {
	return _Amin;
}

Distribution* PedDistributor::GetTau() const {
	return _Tau;
}

void PedDistributor::InitDistributor(const string& filename){
	_projectFilename=filename;
	Log->Write("INFO: \tLoading and parsing the persons attributes");

	TiXmlDocument doc(_projectFilename);

	if (!doc.LoadFile()){
		Log->Write("ERROR: \t%s", doc.ErrorDesc());
		Log->Write("ERROR: \t could not parse the project file");
		exit(EXIT_FAILURE);
	}


	TiXmlNode* xRootNode = doc.RootElement()->FirstChild("agents");
	if( ! xRootNode ) {
		Log->Write("ERROR:\tcould not load persons attributes");
		exit(EXIT_FAILURE);
	}


	TiXmlNode* xDist=xRootNode->FirstChild("agents_distribution");
	for(TiXmlElement* e = xDist->FirstChildElement("group"); e;
			e = e->NextSiblingElement("group")) {

		int room_id = xmltoi(e->Attribute("room_id"));
		int subroom_id = xmltoi(e->Attribute("subroom_id"),-1);
		int number = xmltoi(e->Attribute("number"),0);

		int goal_id = xmltoi(e->Attribute("goal_id"), FINAL_DEST_OUT);
		int router_id = xmltoi(e->Attribute("router_id"), -1);
		int route_id = xmltoi(e->Attribute("route_id"), -1);
		int age = xmltoi(e->Attribute("age"), -1);
		string gender = xmltoa(e->Attribute("gender"), "male");
		double height = xmltof(e->Attribute("height"), -1);

		StartDistributionRoom* dis=NULL;

		if(subroom_id==-1){
			dis = new StartDistributionRoom();
			_start_dis.push_back(dis);
		}else{
			dis = new StartDistributionSubroom();
			dynamic_cast<StartDistributionSubroom*>(dis)->SetSubroomID(subroom_id);
			_start_dis_sub.push_back(dynamic_cast<StartDistributionSubroom*>(dis));
		}

		dis->SetRoomID(room_id);
		dis->SetAgentsNumber(number);
		dis->SetAge(age);
		dis->SetGender(gender);
		dis->SetGoalId(goal_id);
		dis->SetRouteId(route_id);
		dis->SetRouterId(router_id);
		dis->SetHeight(height);

		if(e->Attribute("startX") && e->Attribute("startY")){
			double startX = xmltof(e->Attribute("startX"),NAN);
			double startY = xmltof(e->Attribute("startY"),NAN);
			//todo: verify that the position is valid (not nan)
			dis->SetStartPosition(startX,startY,0.0);
		}
	}

	//TODO: Parse the sources
	TiXmlNode* xSources=xRootNode->FirstChild("agents_sources");
	if(xSources)
	for(TiXmlElement* e = xSources->FirstChildElement("source"); e;
			e = e->NextSiblingElement("source")) {
		Log->Write("INFO:\tSource with id %s will not be parsed !",e->Attribute("id"));

	}

    Log->Write("INFO: \t...Done");
}

int PedDistributor::Distribute(Building* building) const {

	Log->Write("INFO: \tInit Distribute");

	int nPeds = 0;

	//first compute all possible positions in the geometry
	vector<vector< vector<Point > > > allFreePos = vector<vector< vector<Point > > >();
	for (int r = 0; r < building->GetNumberOfRooms(); r++) {
		vector< vector<Point > >  allFreePosRoom = vector< vector<Point > > ();
		Room* room = building->GetRoom(r);
		if(room->GetCaption()=="outside") continue;
		for (int s = 0; s < room->GetNumberOfSubRooms(); s++) {
			SubRoom* subr = room->GetSubRoom(s);
			allFreePosRoom.push_back(PossiblePositions(subr));
		}
		allFreePos.push_back(allFreePosRoom);
	}

	// first perform the distribution according to the  subrooms (if any)

	int pid = 1; // the pedID is being increased throughout...
	for (int i = 0; i < (int) _start_dis_sub.size(); i++) {

		int room_id = _start_dis_sub[i]->GetRoomId();
		Room* r = building->GetRoom(room_id);
		if(!r) continue;

		int roomID = r->GetID();

		int subroomID = _start_dis_sub[i]->GetSubroomID();
		int N = _start_dis_sub[i]->GetAgentsNumber();
		if (N < 0) {
			Log->Write("ERROR: \t negative  (or null ) number of pedestrians!");
			exit(EXIT_FAILURE);
		}

		vector<Point> &allpos = allFreePos[roomID][subroomID];
		int max_pos = allpos.size();
		if (max_pos < N) {
			Log->Write("ERROR: \tCannot distribute %d agents in Room %d . Maximum allowed: %d\n",
					N, roomID, allpos.size());
			exit(EXIT_FAILURE);
		}

		// Distributing
		Log->Write("INFO: \tDistributing %d Agents in Room/Subrom [%d/%d]! Maximum allowed: %d", N, roomID, subroomID, max_pos);
		SubRoom* sr = building->GetRoom(roomID)->GetSubRoom(subroomID);
		DistributeInSubRoom(sr, N, allpos, &pid,_start_dis_sub[i],building);
		Log->Write("\t...Done");

		nPeds += N;
	}

	// then continue the distribution according to the rooms
	for (int i = 0; i < (int) _start_dis.size(); i++) {
		int room_id = _start_dis[i]->GetRoomId();
		Room* r = building->GetRoom(room_id);
		if(!r) continue;
		int N = _start_dis[i]->GetAgentsNumber();
		if (N < 0) {
			Log->Write("ERROR: \t negative number of pedestrians! Ignoring");
			continue;
		}

		double sum_area = 0;
		int max_pos = 0;
		double ppm; // pedestrians per square meter
		int ges_anz = 0;
		vector<int> max_anz = vector<int>();
		vector<int> akt_anz = vector<int>();

		vector< vector<Point > >&  allFreePosInRoom=allFreePos[room_id];
		for (int is = 0; is < r->GetNumberOfSubRooms(); is++) {
			SubRoom* sr = r->GetSubRoom(is);
			double area = sr->GetArea();
			sum_area += area;
			int anz = allFreePosInRoom[is].size();
			max_anz.push_back(anz);
			max_pos += anz;
		}
		if (max_pos < N) {
			Log->Write("ERROR: \t Distribution of %d pedestrians in Room %d not possible! Maximum allowed: %d\n",
					N, r->GetID(), max_pos);
			exit(0);
		}
		ppm = N / sum_area;
		// Anzahl der Personen pro SubRoom bestimmen
		for (int is = 0; is < r->GetNumberOfSubRooms(); is++) {
			SubRoom* sr = r->GetSubRoom(is);
			int anz = sr->GetArea() * ppm + 0.5; // wird absichtlich gerundet
			while (anz > max_anz[is]) {
				anz--;
			}
			akt_anz.push_back(anz);
			ges_anz += anz;
		}
		// Falls N noch nicht ganz erreicht, von vorne jeweils eine Person dazu
		int j = 0;
		while (ges_anz < N) {
			if (akt_anz[j] < max_anz[j]) {
				akt_anz[j] = akt_anz[j] + 1;
				ges_anz++;
			}
			j = (j + 1) % max_anz.size();
		}
		j = 0;
		while (ges_anz > N) {
			if (akt_anz[j] > 0) {
				akt_anz[j] = akt_anz[j] - 1;
				ges_anz--;
			}
			j = (j + 1) % max_anz.size();
		}
		// distributing
		for (unsigned int is = 0; is < akt_anz.size(); is++) {
			SubRoom* sr = r->GetSubRoom(is);
			if (akt_anz[is] > 0)
				DistributeInSubRoom(sr, akt_anz[is], allFreePosInRoom[is], &pid, (StartDistributionSubroom*)_start_dis[i],building);
		}
		nPeds += N;
	}

	return nPeds;
}

bool PedDistributor::FindPedAndDeleteFromRoom(Building* building,Pedestrian*ped) const {

	for (int i = 0; i < building->GetNumberOfRooms(); i++) {
		Room* room = building->GetRoom(i);
		for (int j = 0; j < room->GetNumberOfSubRooms(); j++) {
			SubRoom* sub = room->GetSubRoom(j);
			for (int k = 0; k < sub->GetNumberOfPedestrians(); k++) {
				Pedestrian* p=sub->GetPedestrian(k);
				if(p->GetID()==ped->GetID()){
					sub->DeletePedestrian(k);
					return true;
				}
			}
		}
	}
	return false;
}


vector<Point> PedDistributor::PositionsOnFixX(double min_x, double max_x, double min_y, double max_y,
		SubRoom* r, double bufx, double bufy, double dy) const {
	vector<Point> positions;
	double x = (max_x + min_x)*0.5;
	double y = min_y;

	while (y < max_y) {
		Point pos = Point(x, y);
		// Abstand zu allen Wänden prüfen
		int k;
		for (k = 0; k < r->GetNumberOfWalls(); k++) {
			if (r->GetWall(k).DistTo(pos) < max(bufx, bufy) || !r->IsInSubRoom(pos)) {
				break; // Punkt ist zu nah an einer Wand oder nicht im Raum => ungültig
			}
		}
		if (k == r->GetNumberOfWalls()) {
			//check all transitions
			bool tooNear=false;
			for(unsigned int t=0;t<r->GetAllTransitions().size();t++){
				if(r->GetTransition(t)->DistTo(pos)<J_EPS_GOAL){
					//too close
					tooNear=true;
					break;
				}
			}


			for(unsigned int c=0;c<r->GetAllCrossings().size();c++){
				if(r->GetCrossing(c)->DistTo(pos)<J_EPS_GOAL){
					//too close
					tooNear=true;
					break;
				}
			}
			if(tooNear==false) positions.push_back(pos);
		}
		y += dy;
	}
	return positions;

}

vector<Point>PedDistributor::PositionsOnFixY(double min_x, double max_x, double min_y, double max_y,
		SubRoom* r, double bufx, double bufy, double dx) const {
	vector<Point> positions;
	double y = (max_y + min_y)*0.5;
	double x = min_x;

	while (x < max_x) {
		Point pos = Point(x, y);
		// check distance to wall
		int k;
		for (k = 0; k < r->GetNumberOfWalls(); k++) {
			if (r->GetWall(k).DistTo(pos) < max(bufx, bufy) || !r->IsInSubRoom(pos)) {
				break; // Punkt ist zu nah an einer Wand oder nicht im Raum => ungültig
			}
		}
		if (k == r->GetNumberOfWalls()) {
			//check all transitions
			bool tooNear=false;
			for(unsigned int t=0;t<r->GetAllTransitions().size();t++){
				if(r->GetTransition(t)->DistTo(pos)<J_EPS_GOAL){
					//too close
					tooNear=true;
					break;
				}
			}

			for(unsigned int c=0;c<r->GetAllCrossings().size();c++){
				if(r->GetCrossing(c)->DistTo(pos)<J_EPS_GOAL){
					//too close
					tooNear=true;
					break;
				}
			}
			if(tooNear==false) positions.push_back(pos);
		}
		x += dx;
	}
	return positions;
}

//TODO: this can be speeded up by passing position as reference
vector<Point> PedDistributor::PossiblePositions(SubRoom* r) const {
	double uni = 0.7; // wenn ein Raum in x oder y -Richtung schmaler ist als 0.7 wird in der Mitte verteilt
	double bufx = 0.12;
	double bufy = 0.12;

	double dx = GetAmin()->GetMean() + bufx;
	double dy = GetBmax()->GetMean() + bufy;

	vector<double>::iterator min_x, max_x, min_y, max_y;
	vector<Point> poly = r->GetPolygon();
	vector<Point> positions;
	vector<double> xs;
	vector<double> ys;

	for (int p = 0; p < (int) poly.size(); ++p) {
		xs.push_back(poly[p].GetX());
		ys.push_back(poly[p].GetY());
	}

	min_x = min_element(xs.begin(), xs.end());
	max_x = max_element(xs.begin(), xs.end());
	min_y = min_element(ys.begin(), ys.end());
	max_y = max_element(ys.begin(), ys.end());

	if (*max_y - *min_y < uni) {
		positions = PositionsOnFixY(*min_x, *max_x, *min_y, *max_y, r, bufx, bufy, dx);
	} else if (*max_x - *min_x < uni) {
		positions = PositionsOnFixX(*min_x, *max_x, *min_y, *max_y, r, bufx, bufy, dy);
	} else {
		// create the grid
		double x = (*min_x);
		while (x < *max_x) {
			double y = (*min_y);
			while (y < *max_y) {
				y += dy;
				Point pos = Point(x, y);
				bool tooNear=false;

				// check the distance to all Wall
				for (int k = 0; k < r->GetNumberOfWalls(); k++) {
					const Wall& w = r->GetWall(k);
					if (w.DistTo(pos) < max(bufx, bufy) || !r->IsInSubRoom(pos)) {
						tooNear=true;
						break; // too close
					}
				}

				//check all transitions
				if(tooNear==true) continue;
				for(unsigned int t=0;t<r->GetAllTransitions().size();t++){
					if(r->GetTransition(t)->DistTo(pos)<max(bufx, bufy)){
						//too close
						tooNear=true;
						break;
					}
				}

				//  and check all crossings
				if(tooNear==true) continue;
				for(unsigned int c=0;c<r->GetAllCrossings().size();c++){
					if(r->GetCrossing(c)->DistTo(pos)<max(bufx, bufy)){
						//too close
						tooNear=true;
						break;
					}
				}

				// and finally all opened obstacles
				if(tooNear==true) continue;

				const vector<Obstacle*>& obstacles = r->GetAllObstacles();
				for (unsigned int obs = 0; obs < obstacles.size(); ++obs) {
					Obstacle *obst =obstacles[obs];
					const vector<Wall>& walls = obst->GetAllWalls();
					for (unsigned int i = 0; i < walls.size(); i++) {
						if (walls[i].DistTo(pos) < max(bufx, bufy) || !r->IsInSubRoom(pos)) {
							tooNear=true;
							break; // too close
						}
					}

					//only continue if...
					if(tooNear==true) continue;

					if((obst->GetClosed()==1) && (obst->Contains(pos)==true)){
						tooNear=true;
						break; // too close
					}
				}

				if(tooNear==false) positions.push_back(pos);
			}
			x += dx;
		}
	}

	return positions;
}
/* Verteilt N Fußgänger in SubRoom r
 * Algorithms:
 *   - Lege Gitter von min_x bis max_x mit Abstand dx und entsprechend min_y bis max_y mit
 *     Abstand dy
 *   - Prüfe alle so enstandenen Gitterpunkte, ob ihr Abstand zu Wänden und Übergängen aus
 *     reicht
 *   - Wenn nicht verwerfe Punkt, wenn ja merke Punkt in Vector positions
 *   - Wähle zufällig einen Punkt aus dem Vektor aus Position der einzelnen Fußgänger
 * Parameter:
 *   - r:       SubRoom in den verteilt wird
 *   - N:       Anzahl der Fußgänger für SubRoom r
 *   - pid:     fortlaufender Index der Fußgänger (wird auch wieder zurückgegeben, für den
 *              nächsten Aufruf)
 *   - routing: wird benötigt um die Zielline der Fußgänger zu initialisieren
 * */
void PedDistributor::DistributeInSubRoom(SubRoom* r,int nAgents , vector<Point>& positions, int* pid,
		StartDistributionSubroom* para, Building* building) const {

	// set the pedestrians
	for (int i = 0; i < nAgents; ++i) {

		Pedestrian* ped = new Pedestrian();
		// PedIndex
		ped->SetID(*pid);
		ped->SetAge(para->GetAge());
		ped->SetGender(para->GetGender());
		ped->SetHeight(para->GetHeight());
		ped->SetFinalDestination(para->GetGoalId());
		ped->SetGroup(para->GetGroupId());
		ped->SetRouter(building->GetRoutingEngine()->GetRouter(para->GetRouterId()));
		//ped->SetTrip(); //todo: not implemented

		// a und b setzen muss vor v0 gesetzt werden,
		// da sonst v0 mit Null überschrieben wird
		JEllipse E = JEllipse();
		E.SetAv(GetAtau()->GetRand());
		E.SetAmin(GetAmin()->GetRand());
		E.SetBmax(GetBmax()->GetRand());
		E.SetBmin(GetBmin()->GetRand());
		ped->SetEllipse(E);
		ped->SetTau(GetTau()->GetRand());
		ped->SetV0Norm(GetV0()->GetRand());
		// Position
		int index = rand() % positions.size();
		Point pos = positions[index];
		ped->SetPos(pos);
		positions.erase(positions.begin() + index);
		ped->SetRoomID(para->GetRoomId(),"");
		ped->SetSubRoomID(r->GetSubRoomID());

		Point start_pos=para->GetStartPosition();
		if((isnan(start_pos._x)==0 ) && (isnan(start_pos._y)==0 ) ){
			ped->SetPos(start_pos);
			Log->Write("INFO: \t fixed position for ped %d in Room %d %s",
					pid, para->GetRoomId(), start_pos.toString().c_str());
		}

		// setzen
		r->AddPedestrian(ped);
		(*pid)++;

	}
}


string PedDistributor::writeParameter() const {
	string s;
	char tmp[CLENGTH];

	s.append("\tPedestrians Parameter:\n");
	sprintf(tmp, "\t\tv0 ~ N(%f, %f)\n", GetV0()->GetMean(), GetV0()->GetSigma());
	s.append(tmp);
	sprintf(tmp, "\t\tb_max ~ N(%f, %f)\n", GetBmax()->GetMean(), GetBmax()->GetSigma());
	s.append(tmp);
	sprintf(tmp, "\t\tb_min ~ N(%f, %f)\n", GetBmin()->GetMean(), GetBmin()->GetSigma());
	s.append(tmp);
	sprintf(tmp, "\t\ta_min ~ N(%f, %f)\n", GetAmin()->GetMean(), GetAmin()->GetSigma());
	s.append(tmp);
	sprintf(tmp, "\t\ta_tau ~ N(%f, %f)\n", GetAtau()->GetMean(), GetAtau()->GetSigma());
	s.append(tmp);
	sprintf(tmp, "\t\ttau ~ N(%f, %f)\n", GetTau()->GetMean(), GetTau()->GetSigma());
	s.append(tmp);

	return s;
}

void StartDistributionRoom::SetStartPosition(double x, double y, double z) {
	if(_nPeds!=1){
		Log->Write("INFO:\t you cannot specify the same start position for many agents");
		Log->Write("INFO:\t Ignoring the start position");
		return;
	}
	_startX=x;
	_startY=y;
	_startZ=z;
}

Point StartDistributionRoom::GetStartPosition() const {
	return Point(_startX, _startY);
}
