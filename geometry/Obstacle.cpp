/**
 * Obstacle.cpp
 *
 *  Created on: Jul 31, 2012
 *
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

#include "Obstacle.h"
#include "Wall.h"
#include "Point.h"

Obstacle::Obstacle() {
	pClosed=0.0;
	pHeight=0.0;
	pID=-1;
	pCaption="obstacle";
	pWalls = vector<Wall > ();
	pPoly = vector<Point > ();
}

Obstacle::~Obstacle() {}


void Obstacle::AddWall(const Wall& w) {
	pWalls.push_back(w);
}

string Obstacle::GetCaption() const {
	return pCaption;
}

void Obstacle::SetCaption(string caption) {
	pCaption = caption;
}

double Obstacle::GetClosed() const {
	return pClosed;
}

void Obstacle::SetClosed(double closed) {
	pClosed = closed;
}

double Obstacle::GetHeight() const {
	return pHeight;
}

void Obstacle::SetHeight(double height) {
	pHeight = height;
}

int Obstacle::GetId() const {
	return pID;
}

void Obstacle::SetId(int id) {
	pID = id;
}

string Obstacle::Write() {
	string s;
	Point pos;

	for (unsigned int j = 0; j < pWalls.size(); j++) {
		const Wall& w = pWalls[j];
		s.append(w.Write());
		pos = pos + w.GetPoint1() + w.GetPoint2();
	}
	pos = pos * (0.5 / pWalls.size());

	//add the obstacle caption
	char tmp[CLENGTH];
	sprintf(tmp, "\t\t<label centerX=\"%.2f\" centerY=\"%.2f\" centerZ=\"0\" text=\"%s\" color=\"100\" />\n"
			, pos.GetX() * FAKTOR, pos.GetY() * FAKTOR, pCaption.c_str());
	s.append(tmp);

	return s;
}

const vector<Wall>& Obstacle::GetAllWalls() const {
	return pWalls;
}

int Obstacle::WhichQuad(const Point& vertex, const Point& hitPos) const {
	return (vertex.GetX() > hitPos.GetX()) ? ((vertex.GetY() > hitPos.GetY()) ? 1 : 4) :
			((vertex.GetY() > hitPos.GetY()) ? 2 : 3);

}

// x-Koordinate der Linie von einer Eccke zur nächsten
double Obstacle::Xintercept(const Point& point1, const Point& point2, double hitY) const {
	return (point2.GetX() - (((point2.GetY() - hitY) * (point1.GetX() - point2.GetX())) /
			(point1.GetY() - point2.GetY())));
}


bool Obstacle::Contains(const Point& ped) const {

	// in the case the obstacle is not a close surface, allow
	// pedestrians distribution 'inside'
	if(pClosed==0.0) {
		char tmp[CLENGTH];
		sprintf(tmp, "ERROR: \tObstacle::Contains(): the obstacle [%d] is open!!!\n", pID);
		Log->write(tmp);
		exit(EXIT_FAILURE);
	}

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

void Obstacle::ConvertLineToPoly() {

	if(pClosed==0.0){
		char tmp[CLENGTH];
		sprintf(tmp, "INFO: \tObstacle [%d] is not closed. Not converting to polyline.\n", pID);
		Log->write(tmp);
		return;
	}
	vector<Line*> copy;
	vector<Point> tmpPoly;
	Point point;
	Line* line;
	// Alle Linienelemente in copy speichern
	for (unsigned int i = 0; i < pWalls.size(); i++) {
		copy.push_back(&pWalls[i]);
	}

	line = copy[0];
	tmpPoly.push_back(line->GetPoint1());
	point = line->GetPoint2();
	copy.erase(copy.begin());
	// Polygon aus allen Linen erzeugen
	for (int i = 0; i < (int) copy.size(); i++) {
		line = copy[i];
		if ((point - line->GetPoint1()).Norm() < J_TOLERANZ) {
			tmpPoly.push_back(line->GetPoint1());
			point = line->GetPoint2();
			copy.erase(copy.begin() + i);
			// von vorne suchen
			i = -1;
		} else if ((point - line->GetPoint2()).Norm() < J_TOLERANZ) {
			tmpPoly.push_back(line->GetPoint2());
			point = line->GetPoint1();
			copy.erase(copy.begin() + i);
			// von vorne suchen
			i = -1;
		}
	}
	if ((tmpPoly[0] - point).Norm() > J_TOLERANZ) {
		char tmp[CLENGTH];
		sprintf(tmp, "ERROR: \tObstacle::ConvertLineToPoly(): ID %d !!!\n", pID);
		Log->write(tmp);
		exit(0);
	}
	pPoly = tmpPoly;
}
