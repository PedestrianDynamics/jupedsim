/*
 * File:   Routing.cpp
 * Author: andrea
 *
 * Created on 11. November 2010, 12:55
 */

#include "Routing.h"

/************************************************************
 Routing
 ************************************************************/

Routing::Routing() {
	goals = vector<Crossing* > ();
	pCrossings = vector<Crossing* > ();
	pTransitions = vector<Transition* > ();
	pHlines = vector<Hline* > ();
}

Routing::Routing(const Routing& orig) {
	goals = orig.GetAllGoals();
}

Routing::~Routing() {
	for (int i = 0; i < GetAnzGoals(); i++)
		delete goals[i];
}


const vector<Crossing*>& Routing::GetAllGoals() const {
	return goals;
}

Crossing* Routing::GetGoal(int index) const {
	if ((index >= 0) && (index < (int) goals.size()))
		return goals[index];
	else {
		if (index == -1)
			return NULL;
		else {
			char tmp[CLENGTH];
			sprintf(tmp, "ERROR: Wrong 'index' [%d] > [%d] in Routing::GetGoal()",index,goals.size());
			Log->write(tmp);
			exit(0);
		}
	}
}

int Routing::GetAnzGoals() const {
	return goals.size();
}
// Sonstiges

void Routing::AddGoal(Crossing* line) {
	goals.push_back(line);
}


// Sonstiges
void Routing::AddCrossing(Crossing* line){
	pCrossings.push_back(line);
}

void Routing::AddTransition(Transition* line){
	pTransitions.push_back(line);
}

void Routing::AddHline(Hline* line){
	pHlines.push_back(line);
}

const vector<Crossing*>& Routing::GetAllCrossings() const{
	return pCrossings;
}

const vector<Transition*>& Routing::GetAllTransitions() const{
	return pTransitions;
}

const vector<Hline*>& Routing::GetAllHlines() const{
	return pHlines;
}


// Ausgabe

void Routing::WriteToErrorLog() const {
	for (int i = 0; i < GetAnzGoals(); i++)
		goals[i]->WriteToErrorLog();

}



