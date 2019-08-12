/**
 * \file        LocalFloorfieldViaFM.h
 * \date        Feb 1, 2016
 * \version     v0.8 +
 * \copyright   <2009-2015> Forschungszentrum Jülich GmbH. All rights reserved.
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
 * The local FloorfieldViaFM is derived from the FloorfieldViaFM class. It will
 * create Floorfields in a room (room consisting of one or more subrooms) for
 * each exit. It is to be used in conjunction with a router, yielding intermediate
 * goals which are a door in the same room. It uses all the calculations and the
 * working on grid form the motherclass, yet it has a different constructor.
 **/
#pragma once

#include "FloorfieldViaFM.h"

#include <string>

class LocalFloorfieldViaFM : public virtual FloorfieldViaFM {
public:
     LocalFloorfieldViaFM();
     LocalFloorfieldViaFM(const Room* const room, const Building* buildingArg,
                          const double hxArg, const double hyArg,
                          const double wallAvoidDistance,
                          const bool useDistancefield);

     void parseRoom(const Room* const roomArg, const double hxArg, const double hyArg);
//     void getDirectionToGoalID(const int goalID);
     void drawBlockerLines();
     void crossOutOutsideNeighbors(const long int key);
     // returns the UID of a subroom for a key of th grid, using the information from _subroomMap
     virtual SubRoom* isInside(const long int key);
protected:
     const Room* _room;
};

/*
 NOTE: both parents are derived from FloorfieldViaFM with "public virtual" (it is called "Virtual Inheritance"). This
 means that there is only one instance of FloorfieldViaFM in CentrePointLocalFFViaFM, so calls to FlorfieldViaFM's member
 variables and functions are not ambiguous. See http://www.programering.com/a/MDOxITMwATk.html for some nice diagrams.
 */
class CentrePointLocalFFViaFM : public CentrePointFFViaFM, public LocalFloorfieldViaFM {
public:
     CentrePointLocalFFViaFM(const Room* const room, const Building* buildingArg,
                             const double hxArg, const double hyArg,
                             const double wallAvoidDistance,
                             const bool useDistancefield) : LocalFloorfieldViaFM(room, buildingArg, hxArg, hyArg, wallAvoidDistance, useDistancefield) {};
};

class SubLocalFloorfieldViaFM : public FloorfieldViaFM {
public:
     SubLocalFloorfieldViaFM();
     SubLocalFloorfieldViaFM(SubRoom* const subroom, const Building* buildingArg,
           const double hxArg, const double hyArg,
           const double wallAvoidDistance,
           const bool useDistancefield);

     void parseRoom(SubRoom* const subroomArg, const double hxArg, const double hyArg);
     void getDirectionToDestination (Pedestrian* ped, Point& direction);
     void getDirectionToGoalID(const int goalID);
     virtual SubRoom* isInside(const long int key);
protected:
     SubRoom* _subroom;
};
