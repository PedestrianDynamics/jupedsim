//
// Created by Tobias Schrödter on 2019-04-14.
//

#ifndef JPSCORE_DIRECTIONMINSEPERATIONSHORTERLINE_H
#define JPSCORE_DIRECTIONMINSEPERATIONSHORTERLINE_H

#include "DirectionStrategy.h"

class DirectionMinSeperationShorterLine : public DirectionStrategy {
public:
    virtual Point GetTarget(Room* room, Pedestrian* ped) const;
};

#endif //JPSCORE_DIRECTIONMINSEPERATIONSHORTERLINE_H
