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
	pCrossings = map<int, Crossing*>();
	pTransitions = map<int, Transition*>();
	pHlines = map<int, Hline*>();

	pTrips = vector<vector<int> >();
	pFinalDestinations = vector<int>();
}

Routing::~Routing() {
	for (map<int, Crossing*>::const_iterator iter = pCrossings.begin();
			iter != pCrossings.end(); ++iter) {
		delete iter->second;
	}
	for (map<int, Transition*>::const_iterator iter = pTransitions.begin();
			iter != pTransitions.end(); ++iter) {
		delete iter->second;
	}
	for (map<int, Hline*>::const_iterator iter = pHlines.begin();
			iter != pHlines.end(); ++iter) {
		delete iter->second;
	}
}

const vector<int> Routing::GetTrip(int index) const {
	if ((index >= 0) && (index < (int) pTrips.size()))
		return pTrips[index];
	else {
		char tmp[CLENGTH];
		sprintf(tmp, "ERROR: \tWrong 'index' [%d] > [%d] in Routing::GetTrip()",
				index, pTrips.size());
		Log->write(tmp);
		exit(0);
	}
}

Crossing* Routing::GetGoal(int index) {

	if (pTransitions.count(index) == 1) {
		return pTransitions[index];
	} else if (pCrossings.count(index) == 1) {
		return pCrossings[index];
	}else if (pHlines.count(index) == 1) {
		exit(0);
//		return pHlines[index];
	}else {
		if (index == -1)
			return NULL;
		else {
			char tmp[CLENGTH];
			sprintf(tmp,
					"ERROR: Wrong 'index' [%d] > [%d] in Routing::GetGoal(), counts in map= [%d]",
					index, pGoals.size(),pGoals.count(index));
			Log->write(tmp);
			exit(EXIT_FAILURE);
		}
	}
}

int Routing::GetAnzGoals() const {
	return pCrossings.size()+ pTransitions.size()+pHlines.size();
}
// Sonstiges


// Sonstiges
void Routing::AddCrossing(Crossing* line) {
	if (pCrossings.count(line->GetIndex()) != 0) {
		char tmp[CLENGTH];
		sprintf(tmp,
				"ERROR: Duplicate index for crossing found [%d] in Routing::AddCrossing()",
				line->GetIndex());
		Log->write(tmp);
		exit(EXIT_FAILURE);
	}
	pCrossings[line->GetIndex()] = line;
}

void Routing::AddTransition(Transition* line) {
	if (pTransitions.count(line->GetIndex()) != 0) {
		char tmp[CLENGTH];
		sprintf(tmp,
				"ERROR: Duplicate index for transition found [%d] in Routing::AddTransition()",
				line->GetIndex());
		Log->write(tmp);
		exit(EXIT_FAILURE);
	}
	pTransitions[line->GetIndex()] = line;
}

void Routing::AddHline(Hline* line) {
	if (pHlines.count(line->GetID()) != 0) {
		char tmp[CLENGTH];
		sprintf(tmp,
				"ERROR: Duplicate index for hlines found [%d] in Routing::AddGoal()",
				line->GetID());
		Log->write(tmp);
		exit(EXIT_FAILURE);
	}
	pHlines[line->GetID()] = line;
}

void Routing::AddTrip(vector<int> trip) {
	pTrips.push_back(trip);
}

Crossing* Routing::GetCrossing(int ID /* not the unique id*/) {
	if (pCrossings.count(ID) == 1) {
		return pCrossings[ID];
	} else {
		if (ID == -1)
			return NULL;
		else {
			char tmp[CLENGTH];
			sprintf(tmp,
					"ERROR: Wrong 'index' [%d] > [%d] in Routing::GetGoal()",
					ID, pCrossings.size());
			Log->write(tmp);
			exit(EXIT_FAILURE);
		}
	}
}
Transition* Routing::GetTransition(int ID /* not the unique id*/) {
	if (pTransitions.count(ID) == 1) {
		return pTransitions[ID];
	} else {
		if (ID == -1)
			return NULL;
		else {
			char tmp[CLENGTH];
			sprintf(tmp,
					"ERROR: Wrong 'index' [%d] > [%d] in Routing::GetTransition()",
					ID, pTransitions.size());
			Log->write(tmp);
			exit(EXIT_FAILURE);
		}
	}
}

Hline* Routing::GetHLine(int ID /* not the unique id*/) {
	if (pHlines.count(ID) == 1) {
		return pHlines[ID];
	} else {
		if (ID == -1)
			return NULL;
		else {
			char tmp[CLENGTH];
			sprintf(tmp,
					"ERROR: Wrong 'index' [%d] > [%d] in Routing::GetGoal()",
					ID, pHlines.size());
			Log->write(tmp);
			exit(EXIT_FAILURE);
		}
	}
}

const map<int, Crossing*>& Routing::GetAllCrossings() const {
	return pCrossings;
}

const map<int, Transition*>& Routing::GetAllTransitions() const {
	return pTransitions;
}


const map<int, Hline*>& Routing::GetAllHlines() const {
	return pHlines;
}

void Routing::AddFinalDestinationID(int id) {
	pFinalDestinations.push_back(id);
}

const vector<int> Routing::GetFinalDestinations() const {
	return pFinalDestinations;
}

void Routing::WriteToErrorLog() const {
	for (map<int, Crossing*>::const_iterator iter = pCrossings.begin();
			iter != pCrossings.end(); ++iter) {
		iter->second->WriteToErrorLog();
	}
	for (map<int, Transition*>::const_iterator iter = pTransitions.begin();
			iter != pTransitions.end(); ++iter) {
		iter->second->WriteToErrorLog();
	}
	for (map<int, Hline*>::const_iterator iter = pHlines.begin();
			iter != pHlines.end(); ++iter) {
		iter->second->WriteToErrorLog();
	}
}

