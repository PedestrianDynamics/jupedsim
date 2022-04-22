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

#include "DoorState.hpp"
#include "geometry/Line.hpp"
#include "geometry/Point.hpp"
#include "pedestrian/Pedestrian.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>
#include <string>
#include <vector>

class AccessPoint
{
public:
    AccessPoint(int id, double center[2]);
    ~AccessPoint();

    int GetID();

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

    /**
     * Set/Get the navigation line.
     * The direction taken by the pedestrian strongly depends on this line.
     */
    void SetNavLine(Line* line);

    /**
     * Set/Get the navigation line.
     * The direction taken by the pedestrian strongly depends on this line.
     */
    Line* GetNavLine() const;

    /**
     * True if this is a goal outside the building
     */
    void SetFinalGoalOutside(bool isFinal);

    /**
     * True if this is a goal outside the building
     */
    bool GetFinalGoalOutside();

    void AddFinalDestination(int UID, double distance);
    double GetDistanceTo(int UID);
    double GetDistanceTo(AccessPoint* ap);
    void AddConnectingAP(AccessPoint* ap);

    int GetNearestTransitAPTO(int UID = FINAL_DEST_OUT);
    void AddTransitAPsTo(int UID, AccessPoint* ap);

    void SetState(DoorState state);

private:
    int _id;
    double _center[2];
    /// true if this exit leads to outside
    bool _finaExitToOutside;
    /// true if this a goal outside the building
    bool _finalGoalOutside;
    int _room1ID;
    int _room2ID;
    Point pCentre;
    Line* _navLine;
    std::string _friendlyName;

    // stores the connecting APs
    std::vector<AccessPoint*> _connectingAPs;

    // store part of a graph
    // map a final destination to the next ap to reach it
    // store the nearest AP to reach the destination
    std::map<int, int> _mapDestToAp;

    // store part of the weight matrix
    // store the total distance to the destination int
    std::map<int, double> _mapDestToDist;

    // store the navigation graph
    std::map<int, std::vector<AccessPoint*>> _navigationGraphTo;

    DoorState _state;
};
