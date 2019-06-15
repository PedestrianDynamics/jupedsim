//
// Created by Tobias Schrödter on 2019-06-11.
//

#ifndef JPSCORE_WAITINGPROBABILITY_H
#define JPSCORE_WAITINGPROBABILITY_H

#include <memory>
#include "WaitingStrategy.h"

//#include <boost/numeric/ublas/matrix.hpp>
//#include <boost/numeric/ublas/io.hpp>

class RectGrid;
class SubRoom;
class Line;

class WaitingProbability : public WaitingStrategy{

private:
    Building* _building;
    std::map<int, RectGrid*> _gridMap;
//    std::map<int, boost::numeric::ublas::matrix<double>> _flowMap;
//    std::map<int, boost::numeric::ublas::matrix<double>> _boundaryMap;
//    std::map<int, boost::numeric::ublas::matrix<double>> _distanceMap;
//    std::map<int, boost::numeric::ublas::matrix<double>> _angleMap;
//    std::map<int, boost::numeric::ublas::matrix<double>> _staticMap;
    std::map<int, std::vector<double>> _flowMap;
    std::map<int, std::vector<double>> _boundaryMap;
    std::map<int, std::vector<double>> _distanceMap;
    std::map<int, std::vector<double>> _angleMap;
    std::map<int, std::vector<double>> _staticMap;

//     int* _gcode;
//     double* _distance;

//    unsigned int _numOfExits;
//    RectGrid* _grid = nullptr;


protected:
    RoutingPrecomputation precomputation = RoutingPrecomputation::NONE;

public:
    virtual void Init(Building* building);

    virtual Point GetWaitingPosition(Room* room, Pedestrian* ped) const;

private:
     void parseBuilding();
     void computeStatic();
     void computeFlowAvoidance();
     void computeBoundaryPreference();
     void computeDistanceCost(std::shared_ptr<SubRoom>);
     void computeAngleCost(std::shared_ptr<SubRoom>);

     double checkAngles(double a, double b);

     void writeVTK(std::shared_ptr<SubRoom>, std::string filename);

    template <typename T>
    void drawLinesOnGrid(std::vector<Line>& wallArg, T* const target, const T value);

};

#endif //JPSCORE_WAITINGPROBABILITY_H
