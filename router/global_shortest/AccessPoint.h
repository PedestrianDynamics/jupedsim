/**
 * \file        AccessPoint.h
 * \date        Aug 24, 2010
 * \version     v0.7
 * \copyright   <2009-2015> Forschungszentrum J�lich GmbH. All rights reserved.
 *
 * \section License
 * This file is part of JuPedSim.
 *
 * JuPedSim is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * JuPedSim is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with JuPedSim. If not, see <http://www.gnu.org/licenses/>.
 *
 * \section Description
 *
 *
 **/
#pragma once

#include "pedestrian/Pedestrian.h"
#include "geometry/Point.h"
#include "geometry/Line.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>
#include <string>
#include <vector>

class AccessPoint {
public:

     /**
      * create a new access point with the provided data
      *
      * @param id
      * @param center
      * @param radius
      */
     AccessPoint(int id, double center[2], double radius=0.30);

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
     bool IsInRange(double xPed, double yPed, int roomID);

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

     int GetConnectingRoom1()
     {
          return _room1ID;
     }

     int GetConnectingRoom2()
     {
          return _room2ID;
     }

     /**
      * return the distance to the point x ,y
      * @param x
      * @param y
      * @return
      */
     double DistanceTo(double x, double y);

     /**
      *  set/get a human readable name for this points.
      *  t23 means Transition with id 23
      *  c23 means Crossing with id 23
      *  h23 means Hlines with id 23
      */
     void SetFriendlyName(const std::string& name);

     /**
      *  set/get a human readable name for this points.
      *  t23 means Transition with id 23
      *  c23 means Crossing with id 23
      *  h23 means Hlines with id 23
      */
     const std::string GetFriendlyName();

     /**
      * dump the class
      */
     void Dump();

     /**
      * Set/Get the centre of the navigation line defining this access point
      */
     const Point& GetCentre() const;

     /**
      * True if this is the last exit that leads to the outside
      */
     void SetFinalExitToOutside(bool isFinal);

     /**
      * True if this is the last exit that leads to the outside
      */
     bool GetFinalExitToOutside();

     /**
      * @return true if the door is closed
      */
     bool IsClosed();
//
//     /**
//      * Close the door
//      * @param isClosed
//      */
//     void  SetClosed(int isClosed);

     /**
      * Set/Get the navigation line.
      * The direction taken by the pedestrian strongly depends on this line.
      */
     void SetNavLine(NavLine* line);

     /**
      * Set/Get the navigation line.
      * The direction taken by the pedestrian strongly depends on this line.
      */
     NavLine* GetNavLine() const;

     /**
      * True if this is a goal outside the building
      */
     void SetFinalGoalOutside(bool isFinal);

     /**
      * True if this is a goal outside the building
      */
     bool GetFinalGoalOutside();

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

     void SetState(DoorState state);
     DoorState GetState() const;

     bool IsGoalInside() const;

     void SetGoalInside(bool goalInside);

private:
    int _id;
    double _center[2];
     double _radius;
     /// true if this exit leads to outside
     bool _finaExitToOutside;
     /// true if this a goal outside the building
     bool _finalGoalOutside;
     bool _goalInside;

     int _room1ID;
     int _room2ID;
     Point pCentre;
     NavLine* _navLine;
     std::vector<Pedestrian*> _transitPedestrians;
//     int _isClosed;
     std::string _friendlyName;

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

     DoorState _state;
};
