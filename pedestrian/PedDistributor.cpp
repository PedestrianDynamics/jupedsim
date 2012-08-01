/*
 * File:   PedDistributor.cpp
 * Author: andrea
 *
 * Created on 12. Oktober 2010, 10:52
 */

#include "PedDistributor.h"
#include "../general/xmlParser.h"

/************************************************************
 StartDistributionRoom
 ************************************************************/
StartDistributionRoom::StartDistributionRoom() {
	pRoomCaption = "no caption";
	pN = -1;
}

StartDistributionRoom::StartDistributionRoom(const StartDistributionRoom& orig) {
	pRoomCaption = orig.GetRoomCaption();
	pN = orig.GetAnz();
}

StartDistributionRoom::~StartDistributionRoom() {
}

// Getter-Funktione

string StartDistributionRoom::GetRoomCaption() const {
	return pRoomCaption;
}

int StartDistributionRoom::GetAnz() const {
	return pN;
}


// Setter-Funktionen

void StartDistributionRoom::SetRoomCaption(string caption) {
	pRoomCaption = caption;
}

void StartDistributionRoom::SetAnz(int N) {
	pN = N;
}

// Sonstige Funktionen

string StartDistributionRoom::ReadDistribution(string line) {
	char tmp[CLENGTH] = "";
	istringstream iss(line, istringstream::in);
	string room_caption = "-1";
	int N = -88;
	iss >> room_caption >> N;
	if (room_caption != "-1" && N != -88) {
		sprintf(tmp, "\t\tRaum: %s Anzahl Fußgänger: %d\n", room_caption.c_str(), N);
		SetRoomCaption(room_caption);
		SetAnz(N);
	} else {
		Log->write("ERROR: \t wrong format in StartDistributionRoom::ReadDistribution()!!!");
		exit(0);
	}
	return tmp;
}

/************************************************************
 StartDistributionSubRoom
 ************************************************************/
StartDistributionSubroom::StartDistributionSubroom() : StartDistributionRoom() {
	pSubroomID = -1;
}

StartDistributionSubroom::StartDistributionSubroom(const StartDistributionSubroom& orig) : StartDistributionRoom(orig) {
	pSubroomID = orig.GetSubroomID();
}

StartDistributionSubroom::~StartDistributionSubroom() {
}

int StartDistributionSubroom::GetSubroomID() const {
	return pSubroomID;
}

// Setter-Funktionen

void StartDistributionSubroom::SetSubroomID(int i) {
	pSubroomID = i;
}

// Sonstige Funktionen

string StartDistributionSubroom::ReadDistribution(string line) {
	char tmp[CLENGTH] = "";
	istringstream iss(line, istringstream::in);
	string room_caption = "-1";
	int subroom_ID = -88;
	int N = -88;
	iss >> room_caption >> subroom_ID >> N;
	if (room_caption != "-1" && N != -88 && subroom_ID != -88) {
		sprintf(tmp, "\t\tRaum: %s Subroom: %d Anzahl Fußgänger: %d\n", room_caption.c_str(), subroom_ID, N);
		SetRoomCaption(room_caption);
		SetSubroomID(subroom_ID);
		SetAnz(N);
	} else {
		Log->write("ERROR: \t wrong format in StartDistributionSubroom::ReadDistribution()!!!");
		exit(0);
	}
	return tmp;
}

/************************************************************
 PedDistributor
 ************************************************************/

PedDistributor::PedDistributor() {
	pv0 = new Equal(1.24, 0.26);
	pBmax = new Equal(0.25, 0.001);
	pBmin = new Equal(0.2, 0.001);
	pAtau = new Equal(0.53, 0.001);
	pAmin = new Equal(0.18, 0.001);
	pTau = new Equal(0.5, 0.001);
}

PedDistributor::PedDistributor(double v0mu, double v0sigma, double BmaxMu, double BmaxSigma,
		double BminMu, double BminSigma, double AtauMu, double AtauSigma, double AminMu,
		double AminSigma, double tauMu, double tauSigma) {
	pv0 = new Equal(v0mu, v0sigma);
	pBmax = new Equal(BmaxMu, BmaxSigma);
	pBmin = new Equal(BminMu, BminSigma);
	pAtau = new Equal(AtauMu, AtauSigma);
	pAmin = new Equal(AminMu, AminSigma);
	pTau = new Equal(tauMu, tauSigma);
	start_dis = vector<StartDistributionRoom > ();
	start_dis_sub = vector<StartDistributionSubroom> ();
}

PedDistributor::PedDistributor(const PedDistributor& orig) {
	pv0 = orig.GetV0();
	pBmax = orig.GetBmax();
	pBmin = orig.GetBmin();
	pAtau = orig.GetAtau();
	pAmin = orig.GetAmin();
	pTau = orig.GetTau();
}

PedDistributor::~PedDistributor() {
	delete pv0;
	delete pBmax;
	delete pBmin;
	delete pAtau;
	delete pAmin;
	delete pTau;
}

// Getter-Funktionen

Distribution* PedDistributor::GetV0() const {
	return pv0;
}

Distribution* PedDistributor::GetBmax() const {
	return pBmax;
}

Distribution* PedDistributor::GetBmin() const {
	return pBmin;
}

Distribution* PedDistributor::GetAtau() const {
	return pAtau;
}

Distribution* PedDistributor::GetAmin() const {
	return pAmin;
}

Distribution* PedDistributor::GetTau() const {
	return pTau;
}

void PedDistributor::InitDistributor(string filename){

	XMLNode xMainNode=XMLNode::openFileHelper(filename.c_str(),"persons");
	Log->write("INFO: \tLoading and parsing the persons file");

	//get the distribution node
	XMLNode xDist=xMainNode.getChildNode("distribution");
	int nDist=xDist.nChildNode("dist");
	for(int i=0;i<nDist;i++){
		XMLNode path=xDist.getChildNode("dist",i);

		//FIXME: id oder caption
		//int room_id=atoi(path.getAttribute("room_id"));
		string room_caption=path.getAttribute("room_caption");
		int number=atoi(path.getAttribute("number"));

		if(path.getAttribute("subroom_id")){
			int subroom_id=atoi(path.getAttribute("subroom_id"));
			StartDistributionSubroom dis = StartDistributionSubroom();
			dis.SetRoomCaption(room_caption);
			dis.SetSubroomID(subroom_id);
			dis.SetAnz(number);
			start_dis_sub.push_back(dis);
		}else{
			StartDistributionRoom dis = StartDistributionRoom();
			dis.SetRoomCaption(room_caption);
			dis.SetAnz(number);
			start_dis.push_back(dis);
		}
	}
	Log->write("INFO: \t done with loading and parsing the persons file");
}

int PedDistributor::Distribute(Building* building) const {

	int nPeds = 0; //Gesamtanzahl
		vector<Point> allpos = vector<Point > ();
		char tmp[CLENGTH];

		Log->write("INFO: \tInit Simulation");
		// Alle Starträume durchgehen
		int pid = 1; // Pedestrian ID (wird immer erhöht)
		for (int i = 0; i < (int) start_dis_sub.size(); i++) {

			string room_caption = start_dis_sub[i].GetRoomCaption();
			Room* r = building->GetRoom(room_caption);
			if(!r) continue;

			int roomID = r->GetRoomID();

			int subroomID = start_dis_sub[i].GetSubroomID();
			int N = start_dis_sub[i].GetAnz();
			if (N <= 0) {
				Log->write("ERROR: \t negative  (or null ) number of pedestrians!");
				exit(0);
			}

			SubRoom* sr = building->GetRoom(roomID)->GetSubRoom(subroomID);
			allpos = PossiblePositions(sr, building->GetRouting());
			int max_pos = allpos.size();
			if (max_pos < N) {
				sprintf(tmp, "ERROR: \tVerteilung von %d Fußgängern in Room %d nicht möglich! Maximale Anzahl: %d\n",
						N, roomID, allpos.size());
				Log->write(tmp);
				exit(0);
			} else {
				sprintf(tmp, "INFO: \tVerteilung von %d Fußgängern in [%d/%d]! Maximale Anzahl: %d", N, roomID, subroomID, max_pos);
				Log->write(tmp);
			}
			// Befüllen
			DistributeInSubRoom(sr, N, allpos, roomID, &pid, building->GetRouting());
			nPeds += N;
		}
		return nPeds;
}


vector<Point> PedDistributor::PositionsOnFixX(double min_x, double max_x, double min_y, double max_y,
		SubRoom* r, double bufx, double bufy, double dy, Routing* routing) const {
	vector<Point> positions;
	double x = (max_x + min_x)*0.5;
	double y = min_y;

	while (y < max_y) {
		Point pos = Point(x, y);
		// Abstand zu allen Wänden prüfen
		int k;
		for (k = 0; k < r->GetAnzWalls(); k++) {
			Wall w = r->GetWall(k);
			if (w.DistTo(pos) < max(bufx, bufy) || !r->IsInSubRoom(pos)) {
				break; // Punkt ist zu nah an einer Wand oder nicht im Raum => ungültig
			}
		}
		if (k == r->GetAnzWalls()) {
			vector<int> goalIDs = r->GetAllGoalIDs();
			int l;
			for (l = 0; l < (int) goalIDs.size(); l++) {
				Crossing* c = routing->GetGoal(goalIDs[l]);
				if (c->DistTo(pos) <= EPS_GOAL)
					break; // Punkt ist zu nah an einem Ausgang/ Ziel des SubRooms
			}
			if (l == (int) goalIDs.size())
				positions.push_back(pos);
		}
		y += dy;
	}
	return positions;

}

vector<Point>PedDistributor::PositionsOnFixY(double min_x, double max_x, double min_y, double max_y,
		SubRoom* r, double bufx, double bufy, double dx, Routing* routing) const {
	vector<Point> positions;
	double y = (max_y + min_y)*0.5;
	double x = min_x;

	while (x < max_x) {
		Point pos = Point(x, y);
		// Abstand zu allen Wänden prüfen
		int k;
		for (k = 0; k < r->GetAnzWalls(); k++) {
			Wall w = r->GetWall(k);
			if (w.DistTo(pos) < max(bufx, bufy) || !r->IsInSubRoom(pos)) {
				break; // Punkt ist zu nah an einer Wand oder nicht im Raum => ungültig
			}
		}
		if (k == r->GetAnzWalls()) {
			vector<int> goalIDs = r->GetAllGoalIDs();
			int l;
			for (l = 0; l < (int) goalIDs.size(); l++) {
				Crossing* c = routing->GetGoal(goalIDs[l]);
				if (c->DistTo(pos) <= EPS_GOAL)
					break; // Punkt ist zu nah an einem Ausgang/ Ziel des SubRooms
			}
			if (l == (int) goalIDs.size())
				positions.push_back(pos);
		}
		x += dx;
	}
	return positions;
}

vector<Point> PedDistributor::PossiblePositions(SubRoom* r, Routing * routing) const {
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
		positions = PositionsOnFixY(*min_x, *max_x, *min_y, *max_y, r, bufx, bufy, dx, routing);
	} else if (*max_x - *min_x < uni) {
		positions = PositionsOnFixX(*min_x, *max_x, *min_y, *max_y, r, bufx, bufy, dy, routing);
	} else {
		// Gitter erstellen
		double x = (*min_x);
		while (x < *max_x) {
			double y = (*min_y);
			while (y < *max_y) {
				Point pos = Point(x, y);
				// Abstand zu allen Wänden prüfen
				int k;
				for (k = 0; k < r->GetAnzWalls(); k++) {
					Wall w = r->GetWall(k);
					if (w.DistTo(pos) < max(bufx, bufy) || !r->IsInSubRoom(pos)) {
						break; // Punkt ist zu nah an einer Wand oder nicht im Raum => ungültig
					}
				}
				if (k == r->GetAnzWalls()) {
					vector<int> goalIDs = r->GetAllGoalIDs();
					int l;
					for (l = 0; l < (int) goalIDs.size(); l++) {
						Crossing* c = routing->GetGoal(goalIDs[l]);
						if (c->DistTo(pos) < max(bufx, bufy))
							break; // Punkt ist zu nah an einem Ausgang/ Ziel des SubRooms
					}
					if (l == (int) goalIDs.size())
						positions.push_back(pos);
				}
				y += dy;
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
void PedDistributor::DistributeInSubRoom(SubRoom* r, int N, vector<Point> positions, int roomID, int* pid, Routing * routing) const {
	char tmp[CLENGTH];
	int anz = positions.size();

	if (anz < N) {
		sprintf(tmp, "ERROR: \tVerteilung von %d Fußgängern in Subroom %d nicht möglich! Maximale Anzahl: %d\n",
				N, r->GetSubRoomID(), anz);
		Log->write(tmp);
		exit(0);
	} else {
		sprintf(tmp, "\t\tVerteilung von %d Fußgängern! Maximale Anzahl: %d",
				N, anz);
		Log->write(tmp);
	}
	// Fußgänger setzten
	for (int i = 0; i < N; ++i) {
		// erzeugen
		Pedestrian* ped = new Pedestrian();
		// PedIndex
		ped->SetPedIndex(*pid);
		// a und b setzen muss vor v0 gesetzt werden, da sonst v0 mit Null überschrieben wird
		Ellipse E = Ellipse();
		double atau = GetAtau()->GetRand();
		double amin = GetAmin()->GetRand();
		E.SetAv(atau);
		E.SetAmin(amin);
		double bmax = GetBmax()->GetRand();
		double bmin = GetBmin()->GetRand();
		E.SetBmax(bmax);
		E.SetBmin(bmin);
		ped->SetEllipse(E);
		// tau
		double tau = GetTau()->GetRand();
		ped->SetTau(tau);
		// V0
		double v0 = GetV0()->GetRand();
		ped->SetV0Norm(v0);
		//  ped->SetV0Norm(1.24);
		// Position
		int index = rand() % positions.size();
		Point pos = positions[index];
		//  Point pos=positions[*pid-2];
		ped->SetPos(pos);
		positions.erase(positions.begin() + index);
		// roomID
		ped->SetRoomID(roomID,"");
		// SubRoomID
		ped->SetSubRoomID(r->GetSubRoomID());

		routing->FindExit(ped);
		// setzen
		r->AddPedestrian(ped);
		(*pid)++;
	}

}


string PedDistributor::writeParameter() const {
	string s;
	char tmp[CLENGTH];

	s.append("\tParameter der Fußgänger:\n");
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
