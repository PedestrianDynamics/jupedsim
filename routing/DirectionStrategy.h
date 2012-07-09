/* 
 * File:   DirectionStrategy.h
 * Author: andrea
 *
 * Created on 13. Dezember 2010, 17:02
 */

#ifndef _DIRECTIONSTRATEGY_H
#define	_DIRECTIONSTRATEGY_H

#include "../geometry/Room.h"

/************************************************************
 DirectionStrategy (abstrakte Klasse)
 ************************************************************/

class DirectionStrategy {
private:

public:
    DirectionStrategy();
    DirectionStrategy(const DirectionStrategy& orig);
    virtual ~DirectionStrategy();
    // virtuelle Funktionen werden in den abgeleiteten Klassen implementiert
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

#endif	/* _DIRECTIONSTRATEGY_H */

