//
// Created by Tobias Schrödter on 2019-05-14.
//

#ifndef JPSCORE_WAITINGRANDOM_H
#define JPSCORE_WAITINGRANDOM_H

#include "WaitingStrategy.h"
class WaitingRandom : public WaitingStrategy{
    virtual void Init(){
         srand(time(0));
    };

    virtual Point GetWaitingPosition(Room* room, Pedestrian* ped);

};

#endif //JPSCORE_WAITINGRANDOM_H
