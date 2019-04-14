//
// Created by Tobias Schrödter on 2019-04-14.
//

#ifndef JPSCORE_DIRECTIONMINSEPERATION_H
#define JPSCORE_DIRECTIONMINSEPERATION_H

#include "../DirectionStrategy.h"

class DirectionMinSeperation : public DirectionStrategy {
public:
    virtual Point GetTarget(Room* room, Pedestrian* ped) const;
};

#endif //JPSCORE_DIRECTIONMINSEPERATION_H
