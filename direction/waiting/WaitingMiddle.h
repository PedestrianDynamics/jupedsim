//
// Created by Tobias Schrödter on 2019-05-13.
//
#pragma  once

#include "WaitingStrategy.h"

class WaitingMiddle : public WaitingStrategy{

    virtual Point GetWaitingPosition(Room* room, Pedestrian* ped);

};