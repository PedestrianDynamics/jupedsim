//
// Created by Tobias Schrödter on 2019-07-25.
//

#ifndef JPSCORE_DIRECTIONTRAIN_H
#define JPSCORE_DIRECTIONTRAIN_H

#include "DirectionStrategy.h"

class DirectionTrain : public DirectionStrategy {
public:
    virtual Point GetTarget(Room* room, Pedestrian* ped) const;
};
#endif //JPSCORE_DIRECTIONTRAIN_H
