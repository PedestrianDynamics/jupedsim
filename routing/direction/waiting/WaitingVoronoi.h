//
// Created by Tobias Schrödter on 2019-05-14.
//

#ifndef JPSCORE_WAITINGVORONOI_H
#define JPSCORE_WAITINGVORONOI_H

#include "WaitingStrategy.h"

class WaitingVoronoi : public WaitingStrategy{

    virtual Point GetWaitingPosition(Room* room, Pedestrian* ped) const;

};

#endif //JPSCORE_WAITINGVORONOI_H
