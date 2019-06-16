//
// Created by Tobias Schrödter on 2019-06-11.
//

#ifndef JPSCORE_WAITINGPROBABILITY_H
#define JPSCORE_WAITINGPROBABILITY_H

#include <memory>
#include "WaitingStrategy.h"

#include <random>

class RectGrid;
class SubRoom;
class Line;

class WaitingProbability : public WaitingStrategy{

private:
    Building* _building;
    std::map<int, RectGrid*> _gridMap;

    // Static influences
    std::map<int, std::vector<double>> _flowMap;
    std::map<int, std::vector<double>> _boundaryMap;
    std::map<int, std::vector<double>> _distanceMap;
    std::map<int, std::vector<double>> _angleMap;
    std::map<int, std::vector<double>> _staticMap;

    // random generator
    std::mt19937 _rdGenerator;
    std::uniform_real_distribution<double> _rdDistribution;


protected:
    RoutingPrecomputation precomputation = RoutingPrecomputation::NONE;

public:
    virtual void Init(Building* building);

    virtual Point GetWaitingPosition(Room* room, Pedestrian* ped) const;

private:
     void parseBuilding();
     void computeStatic();
     void computeDynamic();

     void computeStatic(std::shared_ptr<SubRoom> subroom);
     void computeFlowAvoidance(std::shared_ptr<SubRoom>);
     void computeBoundaryPreference(std::shared_ptr<SubRoom>);
     void computeDistanceCost(std::shared_ptr<SubRoom>);
     void computeAngleCost(std::shared_ptr<SubRoom>);

     void normalize(std::vector<double>& data);
     void postProcess(std::vector<double>& data, std::shared_ptr<SubRoom> subroom);
     double checkAngles(double a, double b);

     void writeVTK(std::shared_ptr<SubRoom>, std::string filename);

};

#endif //JPSCORE_WAITINGPROBABILITY_H
