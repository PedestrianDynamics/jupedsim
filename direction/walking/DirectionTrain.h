//
// Created by Tobias Schrödter on 2019-07-25.
//
#pragma  once

#include "DirectionStrategy.h"

class DirectionTrain : public DirectionStrategy {
public:
    virtual Point GetTarget(Room* room, Pedestrian* ped) const;
};