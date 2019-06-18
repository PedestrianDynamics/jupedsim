//
// Created by Tobias Schrödter on 2019-06-11.
//

#ifndef JPSCORE_WAITINGPROBABILITY_H
#define JPSCORE_WAITINGPROBABILITY_H

#include <memory>
#include "WaitingStrategy.h"

#include <random>
#include <queue>
#include <set>
#include <algorithm>

class RectGrid;
class SubRoom;
class Line;

class WaitingProbability : public WaitingStrategy{

private:
    int numPed = 1;
    Building* _building;
    std::map<int, RectGrid*> _gridMap;

    // Static influences
    std::map<int, std::vector<double>> _flowMap;
    std::map<int, std::vector<double>> _boundaryMap;
    std::map<int, std::vector<double>> _distanceMap;
    std::map<int, std::vector<double>> _angleMap;
    std::map<int, std::vector<double>> _staticMap;

    // dynamic influences
    std::map<int, std::vector<double>> _distanceFieldMap;
    std::map<int, std::vector<double>> _distanceProbMap;

    std::map<int, std::vector<double>> _dynamicDistanceMap;
    std::map<int, std::vector<double>> _dynamicMap;

    // combined influences
    std::map<int, std::vector<double>> _probMap;

    // random generator
    std::mt19937 _rdGenerator;
    std::uniform_real_distribution<double> _rdDistribution;


protected:
    RoutingPrecomputation precomputation = RoutingPrecomputation::NONE;

public:
    virtual void Init(Building* building);

    virtual Point GetWaitingPosition(Room* room, Pedestrian* ped);

private:
     void parseBuilding();
     void computeStatic();
     void computeDynamic(const std::shared_ptr<SubRoom>&);

     void computeStatic(const std::shared_ptr<SubRoom>& subroom);
     void computeFlowAvoidance(const std::shared_ptr<SubRoom>&);
     void computeBoundaryPreference(const std::shared_ptr<SubRoom>&);
     void computeDistanceCost(const std::shared_ptr<SubRoom>&);
     void computeAngleCost(const std::shared_ptr<SubRoom>&);

     void computeDistanceField(const std::shared_ptr<SubRoom>&);
     void computeDynamicDistance(const std::shared_ptr<SubRoom>& subroom);
     void computeDistanceProb(const std::shared_ptr<SubRoom>& subroom);
     void combineAll(const std::shared_ptr<SubRoom>& subroom);

     void uniqueAdd(std::queue<Point>& points, std::set<Point>& pointsSet, Point p);
     void normalize(std::vector<double>& data);
     void postProcess(std::vector<double>& data, const std::shared_ptr<SubRoom>& subroom);
     void markOutside(std::vector<double>& data, const std::shared_ptr<SubRoom>& subroom);
     double checkAngles(double a, double b);

     void writeVTK(const std::shared_ptr<SubRoom>&, std::string filename);

};

#endif //JPSCORE_WAITINGPROBABILITY_H
