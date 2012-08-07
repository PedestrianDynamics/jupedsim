/*
 * DummyRouter.cpp
 *
 *  Created on: Aug 7, 2012
 *      Author: piccolo
 */

#include "DummyRouter.h"

DummyRouter::DummyRouter() {
}

DummyRouter::~DummyRouter() {
}

int DummyRouter::FindExit(Pedestrian* p) {
	p->SetExitIndex(1);
	p->SetExitLine(GetAllGoals()[1]);
	return 1;
}

void DummyRouter::Init(Building* b) {
	Log->write("ERROR: Do not use this router !!");
}


