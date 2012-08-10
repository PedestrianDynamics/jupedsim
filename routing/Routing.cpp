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
	goals = vector<Crossing* > ();
	pCrossings = vector<Crossing* > ();
	pTransitions = vector<Transition* > ();
	pHlines = vector<Hline* > ();
	pTrips=vector<vector<int> > ();
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

const vector<int> Routing::GetTrip(int id) const {
	return pTrips[id];
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

void Routing::AddTrip(vector<int> trip){
	pTrips.push_back(trip);
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



