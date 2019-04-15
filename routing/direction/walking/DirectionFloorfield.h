//
// Created by Tobias Schrödter on 2019-04-14.
//

#ifndef JPSCORE_DIRECTIONFLOORFIELD_H
#define JPSCORE_DIRECTIONFLOORFIELD_H

#include "../DirectionStrategy.h"

class Building;
class FloorfieldViaFM;

class DirectionFloorfield : public DirectionStrategy {
public:
    DirectionFloorfield();
    ~DirectionFloorfield();

    virtual void Init(Building* building);
    virtual Point GetTarget(Room* room, Pedestrian* ped) const;
    virtual Point GetDir2Wall(Pedestrian* ped) const;
    virtual double GetDistance2Wall(Pedestrian* ped) const;


private:
    FloorfieldViaFM* _ffviafm;
    bool _initDone;
};

#endif //JPSCORE_DIRECTIONFLOORFIELD_H
