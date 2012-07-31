/*
 * Obstacle.cpp
 *
 *  Created on: Jul 31, 2012
 *      Author:  Ulrich Kemloh
 */

#include "Obstacle.h"
#include "Wall.h"
#include "Point.h"

Obstacle::Obstacle() {}

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


