//
// Created by Tobias Schrödter on 2019-05-13.
//

#ifndef JPSCORE_WAITINGSTRATEGY_H
#define JPSCORE_WAITINGSTRATEGY_H

#include "../../../general/Macros.h"

class Room;
class Pedestrian;
class Point;
class Building;

class WaitingStrategy {

protected:
    RoutingPrecomputation precomputation = RoutingPrecomputation::NONE;

public:
    virtual void Init(){};

    virtual Point GetTarget(Room* room, Pedestrian* ped);

    virtual Point GetWaitingPosition(Room* room, Pedestrian* ped) const = 0;

};

#endif //JPSCORE_WAITINGSTRATEGY_H
