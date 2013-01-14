/**
 * File:   Router.h
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

class Router {
private:
	/// routing strategy as defined in the Macros.h file
	RoutingStrategy _strategy;
	/// the id as present in the persons.xml file
	int _id;

protected:

	vector<vector<int> >_trips;
	vector<int> _finalDestinations;

public:
	Router();
	virtual ~Router();

	void AddTrip(vector<int> trip);
	void AddFinalDestinationID(int id);


	//TODO: investigate Trip for compatibility with ID starting with 0 or 1.
	const vector<int> GetTrip(int id) const;
	const vector<int> GetFinalDestinations() const;

	void SetID(int id);
	int GetID() const;

	void SetStrategy(RoutingStrategy strategy);
	RoutingStrategy GetStrategy() const;

	// Ausgabe
	void WriteToErrorLog() const;

	// virtuelle Funktionen
	virtual int FindExit(Pedestrian* p) = 0;
	virtual void Init(Building* b) = 0;


};

#endif	/* _ROUTING_H */

