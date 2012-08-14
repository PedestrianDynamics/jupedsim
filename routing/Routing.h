/**
 * File:   Routing.h
 *
 * Created on 11. November 2010, 12:55
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

#ifndef _ROUTING_H
#define	_ROUTING_H

#include "../geometry/Crossing.h"
#include "../geometry/Transition.h"
#include "../geometry/Hline.h"
#include "../pedestrian/Pedestrian.h"
#include "../geometry/Room.h"

class Building;

/************************************************************
 Routing
 ************************************************************/

class Routing {
private:
	vector<Crossing*> pCrossings;
	vector<Crossing*> goals;
	vector<Transition*> pTransitions;
	vector<Hline*> pHlines;
	vector<vector<int> >pTrips;
	// store the possible final destination identified while parsing the person format./
	// only those destination are considered in the floyd warshal algorithm.

protected:
	vector<int> pFinalDestinations;

public:
	Routing();
	Routing(const Routing& orig);
	virtual ~Routing();

	// Getter-Funktionen
	const vector<Crossing*>& GetAllGoals() const;
	Crossing* GetGoal(int index) const;
	int GetAnzGoals() const;

	// Sonstiges
	void AddCrossing(Crossing* line);
	void AddTransition(Transition* line);
	void AddHline(Hline* line);
	void AddTrip(vector<int> trip);
	void AddFinalDestinationID(int id);

	const vector<Crossing*>& GetAllCrossings() const;
	const vector<Transition*>& GetAllTransitions() const;
	const vector<Hline*>& GetAllHlines() const;

	const vector<int> GetTrip(int id) const;
	const vector<int> GetFinalDestinations() const;

	// Ausgabe
	void WriteToErrorLog() const;

	// virtuelle Funktionen
	virtual int FindExit(Pedestrian* p) = 0;
	virtual void Init(Building* b) = 0;


	//FIXME: to be deleted
	void  AddGoal(Crossing* line);
};

#endif	/* _ROUTING_H */

