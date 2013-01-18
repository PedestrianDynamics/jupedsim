/**
 * File:   DirectionStrategy.h
 *
 * Created on 13. December 2010, 17:02
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
 */

#ifndef _DIRECTIONSTRATEGY_H
#define	_DIRECTIONSTRATEGY_H

#include "../geometry/Room.h"



/**
 *
 */
class DirectionStrategy {

public:
    DirectionStrategy();
    DirectionStrategy(const DirectionStrategy& orig);
    virtual ~DirectionStrategy();
    virtual Point GetTarget(Room* room, Pedestrian* ped) const = 0;
};

class DirectionMiddlePoint : public DirectionStrategy
{
    public:
        virtual Point GetTarget(Room* room, Pedestrian* ped) const;
};

class DirectionMinSeperation : public DirectionStrategy
{
    public:
        virtual Point GetTarget(Room* room, Pedestrian* ped) const;
};

class DirectionMinSeperationShorterLine : public DirectionStrategy
{
    public:
        virtual Point GetTarget(Room* room, Pedestrian* ped) const;
};

class DirectionInRangeBottleneck : public DirectionStrategy
{
    public:
        virtual Point GetTarget(Room* room, Pedestrian* ped) const;
};

#endif	/* _DIRECTIONSTRATEGY_H */

