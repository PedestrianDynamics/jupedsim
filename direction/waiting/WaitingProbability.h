//
// Created by Tobias Schrödter on 2019-06-11.
//
#pragma  once

#include <memory>
#include "WaitingStrategy.h"

#include <random>
#include <queue>
#include <set>
#include <algorithm>

class RectGrid;
class SubRoom;
class Line;
class Wall;

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
    std::map<int, std::vector<double>> _wallDistanceMap;
    std::map<int, std::vector<double>> _wallPreferenceMap;
    std::map<int, std::vector<double>> _attractionRepulsionMap;
    std::map<int, std::vector<double>> _forbidenMap;

    // dynamic influences
    std::map<int, std::vector<double>> _distanceFieldMap;
    std::map<int, std::vector<double>> _distanceProbMap;

    std::map<int, std::vector<double>> _dynamicDistanceMap;
    std::map<int, std::vector<double>> _pathMap;
    std::map<int, std::vector<double>> _dynamicMap;

    // combined influences
    std::map<int, std::vector<double>> _probMap;
    std::map<int, std::vector<double>> _valueMap;
    std::map<int, std::vector<double>> _filterMap;

    // random generator
    std::mt19937 _rdGenerator;::
    std::uniform_real_distribution<double> _rdDistribution;

    std::map<int, std::vector<Point>> _pedPaths;

protected:
    RoutingPrecomputation precomputation = RoutingPrecomputation::NONE;

public:
    virtual void Init(Building* building);

    virtual Point GetWaitingPosition(Room* room, Pedestrian* ped);

    virtual Point GetPath(Pedestrian* ped);

private:
     void parseBuilding(Building* building);
     void computeStatic();
     void computeDynamic(const SubRoom*, Pedestrian* ped);

     void computeStatic(const SubRoom* subroom);
     void computeFlowAvoidance(const SubRoom*);
     void computeBoundaryPreference(const SubRoom*);
     void computeDistanceCost(const SubRoom*);
     void computeAngleCost(const SubRoom*);
     void computeWallDistance(const SubRoom*);
     void computeWallPreference(const SubRoom*);
     void computeAttractionRepulsionZones(const SubRoom*);
     void computeForbidenZones(const SubRoom*);

     void computeDistanceField(const SubRoom*, Pedestrian* ped);
     void computeDynamicDistance(const SubRoom* subroom);
     void computeDistanceProb(const SubRoom* subroom);
     void computeWallPath(const SubRoom* subroom);
     void combineAll(const SubRoom* subroom);

     void uniqueAdd(std::queue<Point>& points, std::set<Point>& pointsSet, Point p);
     void normalize(std::vector<double>& data);
     void postProcess(std::vector<double>& data, const SubRoom* subroom);
     void markOutside(std::vector<double>& data, const SubRoom* subroom);
     double checkAngles(double a, double b);

     void writeVTK(const SubRoom*, std::string filename);
};