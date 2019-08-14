/**
 * \file        WaitingProbability.cpp
 * \date        Jun 11, 2019
 * \version     v0.8.1
 * \copyright   <2009-2025> Forschungszentrum Jülich GmbH. All rights reserved.
 *
 * \section License
 * This file is part of JuPedSim.
 *
 * JuPedSim is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * JuPedSim is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with JuPedSim. If not, see <http://www.gnu.org/licenses/>.
 *
 * \section Description
 * Implementation of a waiting strategy:
 * Goal of this strategy is a random point of the subroom or waiting area based on a individual distribution.
 * The distribution is determined by static and dynamic influences.
 **/
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
    int _numPed = 1;
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
    std::mt19937 _rdGenerator;
    std::uniform_real_distribution<double> _rdDistribution;

    std::map<int, std::vector<Point>> _pedPaths;

protected:
    RoutingPrecomputation _precomputation = RoutingPrecomputation::NONE;

public:
    void Init(Building* building) override;

    Point GetWaitingPosition(Room* room, Pedestrian* ped) override;

    Point GetPath(Pedestrian* ped) override;

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