/**
 * @file    AccessPoint.h
 * @author  Ulrich Kemloh <kemlohulrich@gmail.com>
 * @version 0.1
 * Created on: 24 Aug 2010
 * Copyright (C) <2009-2010>
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

#ifndef ACCESSPOINT_H_
#define ACCESSPOINT_H_

#include <map>
#include <vector>
#include <cmath>
#include <iostream>
#include <algorithm>
#include "../pedestrian/Pedestrian.h"
#include "../geometry/Point.h"
#include "../geometry/Line.h"

class AccessPoint {
public:

	/**
	 * create a new access point with the provided data
	 *
	 * @param id
	 * @param center
	 * @param radius
	 */

	AccessPoint(int id, double center[2],double radius=0.30);

	/**
	 *
	 * destroy the access point
	 */
	virtual ~AccessPoint();


	/**
	 * @return the Id of the access point
	 */
	int GetID();


	/**
	 * determines if the given coordinates (x ,y) are in the range of that Aps.
	 *
	 * @param x
	 * @param y
	 * @return
	 */
	bool isInRange(double xPed, double yPed, int roomID);


	/**
	 * given the actual room of the pedestrian
	 * determine if that AP is 'visible'
	 */

	bool isInRange(int roomID);

	/**
	 * each AP is connected to at most 2 rooms
	 * @param r1
	 * @param r1
	 */
	void setConnectingRooms(int r1, int r2);

	/**
	 * return the distance to the point x ,y
	 * @param x
	 * @param y
	 * @return
	 */
	double distanceTo(double x, double y);

	/**
	 * dump the class
	 */
	void Dump();

	const Point& GetCentre() const;

	void setFinalDestination(bool isFinal);
	bool isFinalDestination();
	int IsClosed();
	void  SetClosed(int isClosed);
	void SetNavLine(NavLine* line);
	NavLine* GetNavLine() const;


	void AddIntermediateDest(int final, int inter);
	void AddFinalDestination(int UID, double distance);
	double GetDistanceTo(int UID);
	double GetDistanceTo(AccessPoint* ap);
	void RemoveConnectingAP(AccessPoint* ap);

	// reset all setting relative to the destination
	void Reset(int UID=FINAL_DEST_OUT);

	//FIXME: remove those functions
	void AddConnectingAP(AccessPoint* ap);
	int  GetNextApTo(int UID=FINAL_DEST_OUT); //default is the shortest path to the outside ( -1 )
	const std::vector <AccessPoint*>& GetConnectingAPs();


	const std::vector <AccessPoint*>& GetTransitAPsTo(int UID=FINAL_DEST_OUT);
	int GetNearestTransitAPTO(int UID=FINAL_DEST_OUT);
	void AddTransitAPsTo(int UID,AccessPoint* ap);


	// re routing functions
	//FIXME: not compatible with pedestrians reset target after 10 sec
	void AddTransitPed(Pedestrian* ped);
	void DeleteTransitPed(Pedestrian* ped);
	const std::vector<Pedestrian*>& GetAllTransitPed() const;



private:
	int _id;
	double _center[2];
	double _radius;
	bool _finalDestination;
	int _room1ID;
	int _room2ID;
	Point pCentre;
	NavLine* _navLine;
	std::vector<Pedestrian*> _transitPedestrians;
	int _isClosed;


	// stores the connecting APs
	std::vector<AccessPoint*>_connectingAPs;

	// store part of a graph
	// map a final destination to the next ap to reach it
	// store the nearest AP to reach the destination
	std::map<int, int> _mapDestToAp;

	// store part of the weight matrix
	// store the total distance to the destination int
	std::map <int,double> _mapDestToDist;

	//store the navigation graph
	std::map<int,std::vector<AccessPoint*> > _navigationGraphTo;

};

#endif /* ACCESSPOINT_H_ */
