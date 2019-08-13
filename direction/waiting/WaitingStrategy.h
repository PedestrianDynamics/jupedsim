//
// Created by Tobias Schrödter on 2019-05-13.
//
#pragma  once

#include "general/Macros.h"

class Room;
class Pedestrian;
class Point;
class Building;

class WaitingStrategy {

protected:
    RoutingPrecomputation precomputation = RoutingPrecomputation::NONE;

public:
    virtual void Init(Building* ){};

    virtual Point GetTarget(Room* room, Pedestrian* ped);

    virtual Point GetWaitingPosition(Room* room, Pedestrian* ped) = 0;

    virtual Point GetPath(Pedestrian* ped);
};