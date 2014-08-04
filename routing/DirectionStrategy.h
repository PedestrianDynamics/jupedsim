/**
 * \file        DirectionStrategy.h
 * \date        Dec 13, 2010
 * \version     v0.5
 * \copyright   <2009-2014> Forschungszentrum Jülich GmbH. All rights reserved.
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


class Room;
class Pedestrian;
class Point;

class DirectionStrategy {

public:
     DirectionStrategy();
     virtual ~DirectionStrategy();
     virtual Point GetTarget(Room* room, Pedestrian* ped) const = 0;
};

class DirectionMiddlePoint : public DirectionStrategy {
public:
     virtual Point GetTarget(Room* room, Pedestrian* ped) const;
};

class DirectionMinSeperation : public DirectionStrategy {
public:
     virtual Point GetTarget(Room* room, Pedestrian* ped) const;
};

class DirectionMinSeperationShorterLine : public DirectionStrategy {
public:
     virtual Point GetTarget(Room* room, Pedestrian* ped) const;
};

class DirectionInRangeBottleneck : public DirectionStrategy {
public:
     virtual Point GetTarget(Room* room, Pedestrian* ped) const;
};


class DirectionGeneral : public DirectionStrategy {
public:
     virtual Point GetTarget(Room* room, Pedestrian* ped) const;
};

#endif  /* _DIRECTIONSTRATEGY_H */

