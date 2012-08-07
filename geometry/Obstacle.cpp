/*
 * Obstacle.cpp
 *
 *  Created on: Jul 31, 2012
 *      Author:  Ulrich Kemloh
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
		Wall w = pWalls[j];
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



