//
// Created by Tobias Schrödter on 2019-04-14.
//

#ifndef JPSCORE_DIRECTIONMIDDLEPOINT_H
#define JPSCORE_DIRECTIONMIDDLEPOINT_H

#include "../DirectionStrategy.h"

class DirectionMiddlePoint : public DirectionStrategy {
public:
    virtual Point GetTarget(Room* room, Pedestrian* ped) const;
};


#endif //JPSCORE_DIRECTIONMIDDLEPOINT_H
