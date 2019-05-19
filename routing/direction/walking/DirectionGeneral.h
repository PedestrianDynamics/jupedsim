//
// Created by Tobias Schrödter on 2019-04-14.
//

#ifndef JPSCORE_DIRECTIONGENERAL_H
#define JPSCORE_DIRECTIONGENERAL_H

#include "DirectionStrategy.h"

class DirectionGeneral : public DirectionStrategy {
public:
    virtual Point GetTarget(Room* room, Pedestrian* ped) const;
};

#endif //JPSCORE_DIRECTIONGENERAL_H
