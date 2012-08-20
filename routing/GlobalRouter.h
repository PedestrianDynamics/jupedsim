/**
 * @file    GlobalRouter.h
 * @author  Ulrich Kemloh <kemlohulrich@gmail.com>
 * @version 0.1
 * Created on: Dec 15, 2010
 * Copyright (C) <2009-2011>
 *
 * @section LICENSE
 * This file is part of OpenPedSim.
 *
 * OpenPedSim is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * OpenPedSim is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenPedSim. If not, see <http://www.gnu.org/licenses/>.
 *
 * @section DESCRIPTION
 *
 *
 *
 */

#ifndef GLOBALROUTER_H_
#define GLOBALROUTER_H_


// forwarded classes
class Pedestrian;
class AccessPoint;
class Building;
class OutputHandler;

//log output
extern OutputHandler* Log;

#include "Routing.h"


class GlobalRouter: public Routing {

public:
	//GlobalRouter(Building* building);
	GlobalRouter();
	virtual ~GlobalRouter();

	virtual void Init(Building* building);

	//performs a check of the geometry and fixes if possible. transitions/crossings and hlines.
	void CheckInconsistencies();

	virtual int FindExit(Pedestrian* p);


	/**
	 * write the graph as GV format to be used with graphviz
	 * @param filename
	 */
	void WriteGraphGV(string filename, int finalDestination  ,const vector<string> rooms= vector<string>());

protected:
	void GetPath(int transID1, int transID2);
	void FloydWarshall();
	void DumpAccessPoints(int p=-1);

	// check if 2 points can see each other in the geometry
	bool CanSeeEachother(const Point&pt1, const Point&pt2);
	//http://alienryderflex.com/intersect/

	bool CanSeeEachOther(Crossing* c1, Crossing* c2);
	/**
	 * find if this two exits are connectable
	 * what does connectable means?
	 * this is very special to the tribune of the arena
	 */
	bool Connectable(SubRoom* sub, int from, int to);

	/**
	 * @obsolete
	 * return a random exit
	 */
	int GetBestDefaultRandomExit(Pedestrian* p);

	SubRoom* GetCommonSubRoom(Crossing* c1, Crossing* c2);

	template<typename A>
	bool IsElementInVector(const std::vector<A> &vec, A& el) {
		typename std::vector<A>::const_iterator it;
		it = find (vec.begin(), vec.end(), el);
		if(it==vec.end()){
			return false;
		}else{
			return true;
		}
	}



protected:
	int **pPathsMatrix;
	double **pDistMatrix;
	std::vector< int > pTmpPedPath;

	//map the internal crossings/transition id to the global ID (description) for that final destination
	map<int, int> pMapIdToFinalDestination;
	Building *pBuilding;
	std::vector<AccessPoint*> pAccessPoints;

};

#endif /* GLOBALROUTER_H_ */
