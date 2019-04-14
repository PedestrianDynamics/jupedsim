//
// Created by Tobias Schrödter on 2019-04-14.
//

#ifndef JPSCORE_DIRECTIONINRANGEBOTTLENECK_H
#define JPSCORE_DIRECTIONINRANGEBOTTLENECK_H

#include "../DirectionStrategy.h"

class DirectionInRangeBottleneck : public DirectionStrategy {
public:
    virtual Point GetTarget(Room* room, Pedestrian* ped) const;
};


#endif //JPSCORE_DIRECTIONINRANGEBOTTLENECK_H
