//
// Created by Tobias Schrödter on 2019-05-14.
//
#pragma  once

#include "WaitingStrategy.h"
class WaitingRandom : public WaitingStrategy{
    virtual void Init(){
         std::srand(time(0));
    };

    virtual Point GetWaitingPosition(Room* room, Pedestrian* ped);

};
