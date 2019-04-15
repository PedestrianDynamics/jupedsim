/**
 * \file        DirectionStrategy.h
 * \date        Dec 13, 2010
 * \version     v0.7
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
 *
 *
 **/


#ifndef _DIRECTIONSTRATEGY_H
#define _DIRECTIONSTRATEGY_H

#define UNUSED(x) [&x]{}()  // c++11 silence warnings

#include <map>
#include <vector>
#include "../../general/Macros.h"

class Room;
class Pedestrian;
class Point;
class Building;

class DirectionStrategy {

protected:
    RoutingPrecomputation precomputation = RoutingPrecomputation::NONE;

public:
     DirectionStrategy(){};
     virtual ~DirectionStrategy(){};
     virtual void Init(Building*){};

     virtual Point GetTarget(Room* room, Pedestrian* ped) const = 0;
     virtual double GetDistance2Wall(Pedestrian* ped) const;
     virtual double GetDistance2Target(Pedestrian* ped, int UID);
};




#endif  /* _DIRECTIONSTRATEGY_H */

