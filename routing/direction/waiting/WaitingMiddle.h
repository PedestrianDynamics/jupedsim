//
// Created by Tobias Schrödter on 2019-05-13.
//

#ifndef JPSCORE_WAITINGMIDDLE_H
#define JPSCORE_WAITINGMIDDLE_H

#include "WaitingStrategy.h"

class WaitingMiddle : public WaitingStrategy{

    virtual Point GetWaitingPosition(Room* room, Pedestrian* ped) const;

};

#endif //JPSCORE_WAITINGMIDDLE_H
