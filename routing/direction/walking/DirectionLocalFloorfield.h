//
// Created by Tobias Schrödter on 2019-04-14.
//

#ifndef JPSCORE_DIRECTIONLOCALFLOORFIELD_H
#define JPSCORE_DIRECTIONLOCALFLOORFIELD_H

#include "../DirectionStrategy.h"

class Building;
class UnivFFviaFM;

class DirectionLocalFloorfield : public DirectionStrategy {
public:
    DirectionLocalFloorfield();
    ~DirectionLocalFloorfield();

    virtual void Init(Building* building);
    virtual Point GetTarget(Room* room, Pedestrian* ped) const;
    virtual Point GetDir2Wall(Pedestrian* ped) const;
    virtual double GetDistance2Wall(Pedestrian* ped) const;
    virtual double GetDistance2Target(Pedestrian* ped, int UID);

protected:
    std::map<int, UnivFFviaFM*> _locffviafm;
    bool _initDone;
    Building* _building;
    double _stepsize;
    double _wallAvoidDistance;
    bool _useDistancefield;
    std::string _filename;
};


#endif //JPSCORE_DIRECTIONLOCALFLOORFIELD_H
