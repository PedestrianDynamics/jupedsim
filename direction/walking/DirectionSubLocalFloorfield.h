//
// Created by Tobias Schrödter on 2019-04-14.
//
#pragma  once

#include "DirectionStrategy.h"

class Building;
class UnivFFviaFM;

class DirectionSubLocalFloorfield : public DirectionStrategy {
public:
    DirectionSubLocalFloorfield();
    ~DirectionSubLocalFloorfield();

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