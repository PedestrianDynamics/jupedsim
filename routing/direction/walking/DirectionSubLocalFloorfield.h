//
// Created by Tobias Schrödter on 2019-04-14.
//

#ifndef JPSCORE_DIRECTIONSUBLOCALFLOORFIELD_H
#define JPSCORE_DIRECTIONSUBLOCALFLOORFIELD_H

#include "../DirectionStrategy.h"

class DirectionSubLocalFloorfield : public DirectionStrategy {
public:
    DirectionSubLocalFloorfield();
    void Init(Building* building, double stepsize, double threshold,
              bool useDistanceMap);
    ~DirectionSubLocalFloorfield();
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
#endif //JPSCORE_DIRECTIONSUBLOCALFLOORFIELD_H
