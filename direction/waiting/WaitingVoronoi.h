//
// Created by Tobias Schrödter on 2019-05-14.
//
#pragma  once

#include "WaitingStrategy.h"

class WaitingVoronoi : public WaitingStrategy{

    virtual Point GetWaitingPosition(Room* room, Pedestrian* ped) const;

};