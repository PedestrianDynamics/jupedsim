/*
 * DummyRouter.cpp
 *
 *  Created on: Aug 7, 2012
 *      Author: Ulrich Kemloh
 */

#include "DummyRouter.h"

DummyRouter::DummyRouter() {

}

DummyRouter::~DummyRouter() {

}

int DummyRouter::FindExit(Pedestrian* p) {
	p->SetExitIndex(0);
	p->SetExitLine(GetCrossing(0));
	return 1;
}

void DummyRouter::Init(Building* b) {
	Log->write("ERROR: Do not use this router !!");

	//dump all navigation lines

//	cout<<"crossing:"<<endl;
//	for (map<int, Crossing*>::const_iterator iter = pCrossings.begin();
//			iter != pCrossings.end(); ++iter) {
//		iter->second->WriteToErrorLog();
//	}
//	for (map<int, Transition*>::const_iterator iter = pTransitions.begin();
//			iter != pTransitions.end(); ++iter) {
//		iter->second->WriteToErrorLog();
//	}
//	for (map<int, Hline*>::const_iterator iter = pHlines.begin();
//			iter != pHlines.end(); ++iter) {
//		iter->second->WriteToErrorLog();
//	}
//	exit(EXIT_FAILURE);
}


