/**
 * File:   SubRoom.cpp
 *
 * Created on 8. October 2010, 10:56
 *
 * @section LICENSE
 *
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



#include "SubRoom.h"
#include "Transition.h"

/************************************************************
 SubRoom
 ************************************************************/


SubRoom::SubRoom() {
	pID = -1;
	pRoomID=-1;
	pWalls = vector<Wall > ();
	pPoly = vector<Point > ();
	pPeds = vector<Pedestrian* > ();
	pObstacles=vector<Obstacle*> ();

	pCrossings = vector<Crossing*>();
	pTransitions = vector<Transition*>();
	pHlines = vector<Hline*>();

	pGoalIDs = vector<int> ();
	pArea = 0.0;
	pClosed=false;
}

SubRoom::SubRoom(const SubRoom& orig) {
	pID = orig.GetSubRoomID();
	pWalls = orig.GetAllWalls();
	pPoly = orig.GetPolygon();
	pPeds = orig.GetAllPedestrians();
	pGoalIDs = orig.GetAllGoalIDs();
	pArea = orig.GetArea();
	pClosed=orig.GetClosed();
	pRoomID=orig.GetRoomID();
}

SubRoom::~SubRoom() {
	if (pWalls.size() > 0) pWalls.clear();
	if (pPoly.size() > 0) pPoly.clear();
	for (unsigned int i = 0; i < pPeds.size(); i++) {
		delete pPeds[i];
	}
	if (pGoalIDs.size() > 0) pGoalIDs.clear();

	for (unsigned int i = 0; i < pObstacles.size(); i++) {
		delete pObstacles[i];
	}
	pObstacles.clear();
}

// Setter -Funktionen

void SubRoom::SetSubRoomID(int ID) {
	pID = ID;
}
void SubRoom::SetClosed(double closed) {
	pClosed = closed;
}

void SubRoom::SetRoomID(int ID) {
	pRoomID = ID;
}

void SubRoom::SetAllWalls(const vector<Wall>& walls) {
	pWalls = walls;
}

void SubRoom::SetWall(const Wall& wall, int index) {
	if ((index >= 0) && (index < GetAnzWalls())) {
		pWalls[index] = wall;
	} else {
		Log->write("ERROR: Wrong Index in SubRoom::SetWall()");
		exit(0);
	}
}

void SubRoom::SetPolygon(const vector<Point>& poly) {
	pPoly = poly;
}

void SubRoom::SetAllPedestrians(const vector<Pedestrian*>& peds) {
	pPeds = peds;
}

void SubRoom::SetPedestrian(Pedestrian* ped, int index) {
	if ((index >= 0) && (index < GetAnzPedestrians())) {
		pPeds[index] = ped;
	} else {
		Log->write("ERROR: Wrong Index in SubRoom::SetPedestrian()");
		exit(0);
	}
}

void SubRoom::SetArea(double a) {
	pArea = a;
}
// Getter - Funktionen

int SubRoom::GetSubRoomID() const {
	return pID;
}

double SubRoom::GetClosed() const {
	return pClosed;
}

// unique identifier for this subroom
int SubRoom::GetUID() const {
	return pRoomID * 1000 + pID;
}

double SubRoom::GetArea() const {
	return pArea;
}

int SubRoom::GetRoomID() const {
	return pRoomID;
}

int SubRoom::GetAnzWalls() const {
	return pWalls.size();
}

const vector<Wall>& SubRoom::GetAllWalls() const {
	return pWalls;
}

const Wall SubRoom::GetWall(int index) const {
	if ((index >= 0) && (index < GetAnzWalls()))
		return pWalls[index];
	else {
		Log->write("ERROR: Wrong 'index' in SubRoom::GetWall()");
		exit(0);
	}

}

const vector<Point>& SubRoom::GetPolygon() const {
	return pPoly;
}

int SubRoom::GetAnzPedestrians() const {
	return pPeds.size();
}

const vector<Pedestrian*>& SubRoom::GetAllPedestrians() const {
	return pPeds;
}

const vector<Obstacle*>& SubRoom::GetAllObstacles() const {
	return pObstacles;
}


Pedestrian* SubRoom::GetPedestrian(int index) const {
	if ((index >= 0) && (index < (int) GetAnzPedestrians()))
		return pPeds[index];
	else {
		Log->write("ERROR: Wrong 'index' in SubRoom::GetPedestrian()");
		exit(0);
	}
}

int SubRoom::GetAnzGoalIDs() const {
	return pGoalIDs.size();
}

const vector<int>& SubRoom::GetAllGoalIDs() const {
	return pGoalIDs;
}

int SubRoom::GetGoalID(int index) const {
	if ((index >= 0) && (index < (int) GetAnzGoalIDs()))
		return pGoalIDs[index];
	else {
		Log->write("ERROR: Wrong 'index' in SubRoom::GetGoalID()");
		exit(0);
	}
}

// Sonstiges

void SubRoom::AddWall(const Wall& w) {
	pWalls.push_back(w);
}

void SubRoom::DeleteWall(int index) {
	if ((index >= 0) && (index < (int) pWalls.size()))
		pWalls.erase(pWalls.begin() + index);
	else {
		Log->write("ERROR: Wrong Index in SubRoom::DeleteWall()");
		exit(0);
	}
}

void SubRoom::AddPedestrian(Pedestrian* ped) {
	pPeds.push_back(ped);
}

void SubRoom::AddObstacle(Obstacle* obs){
	pObstacles.push_back(obs);
}

void SubRoom::DeletePedestrian(int index) {
	if ((index >= 0) && (index < (int) GetAnzPedestrians())) {
		pPeds.erase(pPeds.begin() + index);

	} else {
		Log->write("ERROR: Wrong Index in SubRoom::DeletePedestrian()");
		exit(0);
	}
}

void SubRoom::AddGoalID(int ID) {
	pGoalIDs.push_back(ID);
}

void SubRoom::AddCrossing(Crossing* line){
	pCrossings.push_back(line);
}

void SubRoom::AddTransition(Transition* line){
	pTransitions.push_back(line);
}

void SubRoom::AddHline(Hline* line){
	pHlines.push_back(line);
}

const vector<Crossing*>& SubRoom::GetAllCrossings() const{
	return pCrossings;
}

const vector<Transition*>& SubRoom::GetAllTransitions() const{
	return pTransitions;
}

const vector<Hline*>& SubRoom::GetAllHlines() const{
	return pHlines;
}

const Crossing* SubRoom::GetCrossing(int i) const {
	return pCrossings[i];
}

const Transition* SubRoom::GetTransition(int i) const {
	return pTransitions[i];
}

const Hline* SubRoom::GetHline(int i) const {
	return pHlines[i];
}

void SubRoom::RemoveGoalID(int ID){
	for (unsigned int i=0;i<pGoalIDs.size();i++){
		if(pGoalIDs[i]==ID){
			Log->write("Removing goal");
			pGoalIDs.erase(pGoalIDs.begin()+i);
			return;
		}
	}
	Log->write("There is no goal with that id to remove");
}

// berechnet die Fläche eines beliebigen Polygons und setzt die Variable pArea entsprechend

void SubRoom::CalculateArea() {
	double sum = 0;
	int n = (int) pPoly.size();
	for (int i = 0; i < n; i++) {
		sum += (pPoly[i].GetY() + pPoly[(i + 1) % n].GetY())*(pPoly[i].GetX() - pPoly[(i + 1) % n].GetX());
	}
	SetArea(0.5 * fabs(sum));
}

void SubRoom::LoadWall(string line) {
	string tmp; //Schlüsselwort "wall"
	istringstream iss(line, istringstream::in);
	double x1, y1, x2, y2;
	iss >> tmp >> x1 >> y1 >> x2 >> y2;
	Wall wall = Wall(Point(x1, y1), Point(x2, y2));
	AddWall(wall);
}

bool SubRoom::IsInSubRoom(Pedestrian* ped) const {
	Point pos = ped->GetPos();
	if (ped->GetExitLine()->DistTo(pos) <= EPS_GOAL)
		return true;
	else
		return IsInSubRoom(pos);
}


// this is the case if they share a transition or crossing
bool SubRoom::IsDirectlyConnectedWith(const SubRoom* sub) const {

	//check the crossings
	const vector<Crossing*>& crossings = sub->GetAllCrossings();
	for (unsigned int i = 0; i < crossings.size(); i++) {
		for (unsigned int j = 0; j < pCrossings.size(); j++) {
			int uid1 = crossings[i]->GetUniqueID();
			int uid2 = pCrossings[j]->GetUniqueID();
			// ignore my transition
			if (uid1 == uid2)
				return true;
		}
	}

	// and finally the transitions
	const vector<Transition*>& transitions = sub->GetAllTransitions();
	for (unsigned int i = 0; i < transitions.size(); i++) {
		for (unsigned int j = 0; j < pTransitions.size(); j++) {
			int uid1 = transitions[i]->GetUniqueID();
			int uid2 = pTransitions[j]->GetUniqueID();
			// ignore my transition
			if (uid1 == uid2)
				return true;
		}
	}

	return false;
}


void SubRoom::ClearAllPedestrians(){
	for(unsigned int p=0;p<pPeds.size();p++){
		delete pPeds[p];
	}
	pPeds.clear();
}


/************************************************************
 NormalSubRoom
 ************************************************************/
NormalSubRoom::NormalSubRoom() : SubRoom() {

}

NormalSubRoom::NormalSubRoom(const NormalSubRoom& orig) : SubRoom(orig) {

}

NormalSubRoom::~NormalSubRoom() {
}

string NormalSubRoom::WriteSubRoom() const {
	string s;
	Point pos;
	for (int j = 0; j < GetAnzWalls(); j++) {
		Wall w = GetWall(j);
		s.append(w.Write());
		pos = pos + w.GetPoint1() + w.GetPoint2();
	}
	pos = pos * (0.5 / GetAnzWalls());
	//add the subroom caption
	char tmp[CLENGTH];
	sprintf(tmp, "\t\t<label centerX=\"%.2f\" centerY=\"%.2f\" centerZ=\"0\" text=\"%d\" color=\"100\" />\n"
			, pos.GetX() * FAKTOR, pos.GetY() * FAKTOR, GetSubRoomID());
	s.append(tmp);

	//write the obstacles
	for( unsigned int j=0;j<GetAllObstacles().size(); j++) {
		s.append(GetAllObstacles()[j]->Write());
	}

	return s;
}

void NormalSubRoom::WriteToErrorLog() const {
	Log->write("\t\tNormal SubRoom:\n");
	for (int i = 0; i < GetAnzWalls(); i++) {
		Wall w = GetWall(i);
		w.WriteToErrorLog();
	}
}

void NormalSubRoom::ConvertLineToPoly(vector<Line*> goals) {
	vector<Line*> copy;
	vector<Point> tmpPoly;
	Point point;
	Line* line;
	// Alle Linienelemente in copy speichern
	for (int i = 0; i < GetAnzWalls(); i++) {
		copy.push_back(&pWalls[i]);
	}
	// Transitions und Crossings sind in goal abgespeichert
	copy.insert(copy.end(), goals.begin(), goals.end());

	line = copy[0];
	tmpPoly.push_back(line->GetPoint1());
	point = line->GetPoint2();
	copy.erase(copy.begin());
	// Polygon aus allen Linen erzeugen
	for (int i = 0; i < (int) copy.size(); i++) {
		line = copy[i];
		if ((point - line->GetPoint1()).Norm() < TOLERANZ) {
			tmpPoly.push_back(line->GetPoint1());
			point = line->GetPoint2();
			copy.erase(copy.begin() + i);
			// von vorne suchen
			i = -1;
		} else if ((point - line->GetPoint2()).Norm() < TOLERANZ) {
			tmpPoly.push_back(line->GetPoint2());
			point = line->GetPoint1();
			copy.erase(copy.begin() + i);
			// von vorne suchen
			i = -1;
		}
	}
	if ((tmpPoly[0] - point).Norm() > TOLERANZ) {
		char tmp[CLENGTH];
		sprintf(tmp, "ERROR: \tNormalSubRoom::ConvertLineToPoly(): SubRoom %d Room %d Anfangspunkt ungleich Endpunkt!!!\n"
				"\t(%f, %f) != (%f, %f)\n", GetSubRoomID(), GetRoomID(), tmpPoly[0].GetX(), tmpPoly[0].GetY(), point.GetX(),
				point.GetY());
		Log->write(tmp);
		exit(0);
	}
	pPoly = tmpPoly;
}


// private Funktionen

// gibt zuruck in welchen Quadranten vertex liegt, wobei hitPos der Koordinatenursprung ist

int NormalSubRoom::WhichQuad(const Point& vertex, const Point& hitPos) const {
	return (vertex.GetX() > hitPos.GetX()) ? ((vertex.GetY() > hitPos.GetY()) ? 1 : 4) :
			((vertex.GetY() > hitPos.GetY()) ? 2 : 3);

}

// x-Koordinate der Linie von einer Eccke zur nächsten

double NormalSubRoom::Xintercept(const Point& point1, const Point& point2, double hitY) const {
	return (point2.GetX() - (((point2.GetY() - hitY) * (point1.GetX() - point2.GetX())) /
			(point1.GetY() - point2.GetY())));
}


// neue Version auch für konkave Polygone

bool NormalSubRoom::IsInSubRoom(const Point& ped) const {
	short edge, first, next;
	short quad, next_quad, delta, total;

	/////////////////////////////////////////////////////////////
	edge = first = 0;
	quad = WhichQuad(pPoly[edge], ped);
	total = 0; // COUNT OF ABSOLUTE SECTORS CROSSED
	/* LOOP THROUGH THE VERTICES IN A SECTOR */
	do {
		next = (edge + 1) % pPoly.size();
		next_quad = WhichQuad(pPoly[next], ped);
		delta = next_quad - quad; // HOW MANY QUADS HAVE I MOVED

		// SPECIAL CASES TO HANDLE CROSSINGS OF MORE THEN ONE
		//QUAD

		switch (delta) {
			case 2: // IF WE CROSSED THE MIDDLE, FIGURE OUT IF IT
				//WAS CLOCKWISE OR COUNTER
			case -2: // US THE X POSITION AT THE HIT POINT TO
				// DETERMINE WHICH WAY AROUND
				if (Xintercept(pPoly[edge], pPoly[next], ped.GetY()) > ped.GetX())
					delta = -(delta);
				break;
			case 3: // MOVING 3 QUADS IS LIKE MOVING BACK 1
				delta = -1;
				break;
			case -3: // MOVING BACK 3 IS LIKE MOVING FORWARD 1
				delta = 1;
				break;
		}
		/* ADD IN THE DELTA */
		total += delta;
		quad = next_quad; // RESET FOR NEXT STEP
		edge = next;
	} while (edge != first);

	/* AFTER ALL IS DONE IF THE TOTAL IS 4 THEN WE ARE INSIDE */
	if (abs(total) == 4)
		return true;
	else
		return false;
}

/************************************************************
 Stair
 ************************************************************/

Stair::Stair() : SubRoom() {
	pUp = Point();
	pDown = Point();
}

Stair::Stair(const Stair & orig) : SubRoom(orig) {
	pUp = orig.GetUp();
	pDown = orig.GetDown();
}

Stair::~Stair() {
}

// Setter-Funktionen

void Stair::SetUp(const Point & p) {
	pUp = p;
}

void Stair::SetDown(const Point & p) {
	pDown = p;
}

// Getter-Funktionen

const Point & Stair::GetUp() const {
	return pUp;
}

const Point & Stair::GetDown() const {
	return pDown;
}

string Stair::WriteSubRoom() const {
	string s;
	Point pos;
	char tmp_c[300];
	for (int j = 0; j < GetAnzWalls(); j++) {
		Wall w = GetWall(j);
		pos = pos + w.GetPoint1() + w.GetPoint2();
		s.append(w.Write());
	}
	//Line tmp = Line(GetUp(), GetDown());
	// s.append(tmp.Write());
	sprintf(tmp_c, "\t\t<sphere centerX=\"%.2f\" centerY=\"%.2f\" centerZ=\"0\" radius=\"20\" color=\"100\" />\n"
			, GetUp().GetX() * FAKTOR, GetUp().GetY() * FAKTOR);
	//s.append(tmp_c);

	//add the subroom caption
	pos = pos * (0.5 / GetAnzWalls());
	sprintf(tmp_c, "\t\t<label centerX=\"%.2f\" centerY=\"%.2f\" centerZ=\"0\" text=\"%d\" color=\"100\" />\n"
			, pos.GetX() * FAKTOR, pos.GetY() * FAKTOR, GetSubRoomID());
	s.append(tmp_c);

	return s;
}

void Stair::WriteToErrorLog() const {
	Log->write("\t\tStair:\n");
	for (int i = 0; i < GetAnzWalls(); i++) {
		Wall w = GetWall(i);
		w.WriteToErrorLog();
	}
}

/* prüft ob die Punkte p1, p2 und p3 auf einer Linie liegen, oder eine Ecke bilden.
 * Dabei liegt p2 IMMER in der Mitte und entspricht aktPoint
 * */
const Point* Stair::CheckCorner(const Point** otherPoint, const Point** aktPoint, const Point* nextPoint) {

	Point l1 = **otherPoint - **aktPoint;
	Point l2 = *nextPoint - **aktPoint;
	const Point* rueck = NULL;
	// Punkte bilden eine Linie
	if (fabs(fabs(l1.ScalarP(l2) / (l1.Norm() * l2.Norm())) - 1) < 0.1) {
		*aktPoint = nextPoint;
	} else // aktPoint/p2 ist eine Ecke
	{
		rueck = *aktPoint;
		*otherPoint = *aktPoint;
		*aktPoint = nextPoint;
	}
	return rueck;
}

void Stair::ConvertLineToPoly(vector<Line*> goals) {
	vector<Line*> copy;
	vector<Point> orgPoly = vector<Point > ();
	const Point* aktPoint;
	const Point* otherPoint;
	const Point* nextPoint;
	const Point* firstAktPoint;
	const Point* firstOtherPoint;
	Line *nextLine;

	// Alle Linienelemente in copy speichern
	for (int i = 0; i < GetAnzWalls(); i++) {
		copy.push_back(&pWalls[i]);
	}
	// Transitions und Crossings sind in goal abgespeichert
	copy.insert(copy.end(), goals.begin(), goals.end());

	aktPoint = &copy[0]->GetPoint1();
	firstAktPoint = aktPoint;
	otherPoint = &copy[0]->GetPoint2();
	firstOtherPoint = otherPoint;
	copy.erase(copy.begin());

	// Polygon aus allen Linen erzeugen
	for (int i = 0; i < (int) copy.size(); i++) {
		nextLine = copy[i];
		nextPoint = NULL;
		if ((*aktPoint - nextLine->GetPoint1()).Norm() < TOLERANZ) {
			nextPoint = &nextLine->GetPoint2();
		} else if ((*aktPoint - nextLine->GetPoint2()).Norm() < TOLERANZ) {
			nextPoint = &nextLine->GetPoint1();
		}
		if (nextPoint != NULL) {
			const Point* rueck = CheckCorner(&otherPoint, &aktPoint, nextPoint);
			if (rueck != NULL)
				orgPoly.push_back(*rueck);
			copy.erase(copy.begin() + i);
			i = -1; // von vorne suchen
		}
	}
	if ((*aktPoint - *firstOtherPoint).Norm() < TOLERANZ) {
		const Point* rueck = CheckCorner(&otherPoint, &aktPoint, firstAktPoint);
		if (rueck != NULL)
			orgPoly.push_back(*rueck);
	} else {
		char tmp[CLENGTH];
		double x1, y1, x2, y2;
		x1 = firstOtherPoint->GetX();
		y1 = firstOtherPoint->GetY();
		x2 = aktPoint->GetX();
		y2 = aktPoint->GetY();
		sprintf(tmp, "ERROR: \tStair::ConvertLineToPoly(): SubRoom %d Room %d Anfangspunkt ungleich Endpunkt!!!\n"
				"\t(%f, %f) != (%f, %f)\n", GetSubRoomID(), GetRoomID(), x1, y1, x2, y2);
		Log->write(tmp);
	}

	if (orgPoly.size() != 4) {
		char tmp[CLENGTH];
		sprintf(tmp, "ERROR: \tStair::ConvertLineToPoly(): Stair %d Room %d ist kein Viereck!!!\n"
				"Anzahl Ecken: %d\n", GetSubRoomID(), GetRoomID(), orgPoly.size());
		Log->write(tmp);
		exit(0);
	}
	vector<Point> neuPoly = (orgPoly);
	// ganz kleine Treppen (nur eine Stufe) nicht
	if ((neuPoly[0] - neuPoly[1]).Norm() > 0.9 && (neuPoly[1] - neuPoly[2]).Norm() > 0.9) {
		for (int i1 = 0; i1 < (int) orgPoly.size(); i1++) {
			int i2 = (i1 + 1) % orgPoly.size();
			int i3 = (i2 + 1) % orgPoly.size();
			int i4 = (i3 + 1) % orgPoly.size();
			Point p1 = neuPoly[i1];
			Point p2 = neuPoly[i2];
			Point p3 = neuPoly[i3];
			Point p4 = neuPoly[i4];

			Point l1 = p2 - p1;
			Point l2 = p3 - p2;

			if (l1.Norm() < l2.Norm()) {
				neuPoly[i2] = neuPoly[i2] + l1.Normalized() * 2 * EPS_GOAL;
				l2 = p3 - p4;
				neuPoly[i3] = neuPoly[i3] + l2.Normalized() * 2 * EPS_GOAL;
			}
		}
	}
	pPoly = neuPoly;
}

bool Stair::IsInSubRoom(const Point& ped) const {
	bool rueck = false;
	int N = (int) pPoly.size();
	int sum = 0;

	for (int i = 0; i < N; i++) {
		Line l = Line(pPoly[i], pPoly[(i + 1) % N]);
		Point s = l.LotPoint(ped);
		if (l.IsInLine(s))
			sum++;
	}
	if (sum == 4)
		rueck = true;

	return rueck;
}

