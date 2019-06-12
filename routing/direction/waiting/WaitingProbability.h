//
// Created by Tobias Schrödter on 2019-06-11.
//

#ifndef JPSCORE_WAITINGPROBABILITY_H
#define JPSCORE_WAITINGPROBABILITY_H

#include "WaitingStrategy.h"

//#include <boost/numeric/ublas/matrix.hpp>
//#include <boost/numeric/ublas/io.hpp>

class RectGrid;
class SubRoom;

class WaitingProbability : public WaitingStrategy{

private:
    Building* _building;
    std::map<int, RectGrid> _gridMap;
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

protected:
    RoutingPrecomputation precomputation = RoutingPrecomputation::NONE;

public:
    virtual void Init(Building* building);

    virtual Point GetWaitingPosition(Room* room, Pedestrian* ped) const;

private:
     void computeStatic(RectGrid grid, std::shared_ptr<SubRoom>& subroom);
     void computeFlowAvoidance(RectGrid grid, std::shared_ptr<SubRoom>& subroom);
     void computeBoundaryPreference(RectGrid grid, std::shared_ptr<SubRoom>& subroom);
     void computeDistanceCost(RectGrid grid, std::shared_ptr<SubRoom>& subroom);
     void computeAngleCost(RectGrid grid, std::shared_ptr<SubRoom>& subroom);

     void writeVTK(RectGrid grid, std::vector<double> values, std::string filename);
};

#endif //JPSCORE_WAITINGPROBABILITY_H
