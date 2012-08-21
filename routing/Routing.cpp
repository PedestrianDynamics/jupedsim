/**
 * File:   Routing.cpp
 *
 * Created on 11. November 2010, 12:55
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

#include "Routing.h"

/************************************************************
 Routing
 ************************************************************/

Routing::Routing() {

	//TODO: delete the variable goals
	goals = vector<Crossing*>();

	pCrossings = vector<Crossing*>();
	pTransitions = vector<Transition*>();
	pHlines = vector<Hline*>();
	pTrips = vector<vector<int> >();
	pFinalDestinations = vector<int>();

	pGoal=map<int, Crossing*>();
}

Routing::~Routing() {
	for (int i = 0; i < GetAnzGoals(); i++)
		delete goals[i];
}

const vector<int> Routing::GetTrip(int index) const {
	if ((index >= 0) && (index < (int) pTrips.size()))
		return pTrips[index];
	else {
		char tmp[CLENGTH];
		sprintf(tmp, "ERROR: \tWrong 'index' [%d] > [%d] in Routing::GetTrip()",index,pTrips.size());
		Log->write(tmp);
		exit(0);
	}
}



Crossing* Routing::GetGoal(int index) {

	if(pGoal.count(index)==1){
		//return goals[index];
		return pGoal[index];
	}
	else {
		if (index == -1)
			return NULL;
		else {
			char tmp[CLENGTH];
			sprintf(tmp, "ERROR: Wrong 'index' [%d] > [%d] in Routing::GetGoal()",index,goals.size());
			Log->write(tmp);
			exit(EXIT_FAILURE);
		}
	}
}

int Routing::GetAnzGoals() const {
	return goals.size();
}
// Sonstiges

void Routing::AddGoal(Crossing* line) {
	goals.push_back(line);

	//new implementation
	pGoal[line->GetIndex()]=line;
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

void Routing::AddTrip(vector<int> trip){
	pTrips.push_back(trip);
}

const vector<Crossing*>& Routing::GetAllCrossings() const{
	return pCrossings;
}

const vector<Transition*>& Routing::GetAllTransitions() const{
	return pTransitions;
}

void Routing::AddFinalDestinationID(int id){
	pFinalDestinations.push_back(id);
}

const vector<Hline*>& Routing::GetAllHlines() const{
	return pHlines;
}

const vector<int> Routing::GetFinalDestinations() const{
	return pFinalDestinations;
}

void Routing::WriteToErrorLog() const {
	for (int i = 0; i < GetAnzGoals(); i++)
		goals[i]->WriteToErrorLog();
}




