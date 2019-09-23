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
#include "WaitingProbability.h"

#include "router/ff_router/mesh/RectGrid.h"
#include "geometry/SubRoom.h"
#include "pedestrian/Pedestrian.h"
#include "geometry/Wall.h"

void  WaitingProbability::Init(Building* building){
     Log->Write("Start init Waiting Probability");

     _building = building;

     // Init random generator
     _rdGenerator=std::mt19937(building->GetConfig()->GetSeed());
     _rdDistribution = std::uniform_real_distribution<double> (0.,1.);

     parseBuilding(building);
     computeStatic();

     Log->Write("End init Waiting Probability-----------------");
}

void WaitingProbability::parseBuilding(Building* building){
     Log->Write("Start parseBuilding");

     double stepSizeX = 1.*0.125;
     double stepSizeY = 1.*0.125;

     std::vector<Line> _wall;
     std::vector<Line> _exitsFromScope;

     for (const auto roomItr : building->GetAllRooms()){
          for (const auto subRoomItr : roomItr.second->GetAllSubRooms()){
               SubRoom* subroom = subRoomItr.second.get();

               for (auto trans : subroom->GetAllTransitions()) {
                    if (trans->IsExit() && !trans->IsClose()){
                         _exitsFromScope.emplace_back(Line ( (Line) *(trans)));
                    } else if (!trans->IsOpen()){
                         _wall.emplace_back(Line ( (Line) *(trans)));
                    }
               }

               for (auto obsItr : subroom->GetAllObstacles()){
                    for (auto obsWalls : obsItr->GetAllWalls()){
                         _wall.emplace_back(Line( (Line) obsWalls));
                    }
               }

               for (auto itWall : subroom->GetAllWalls()) {
                    _wall.emplace_back( Line( (Line) itWall));
               }

               for (auto cross : subroom->GetAllCrossings()) {
                    if (!cross->IsOpen()) {
                         _wall.emplace_back(Line((Line) *cross));
                    }
               }

               std::vector<double> bb = subroom->GetBoundingBox();
               double xMin = bb[0];
               double xMax = bb[1];
               double yMin = bb[2];
               double yMax = bb[3];

               // Create rect grid
               int uid = subroom->GetUID();
               _gridMap[uid] = new RectGrid();

               _gridMap[uid]->setBoundaries(xMin, yMin, xMax, yMax);
               _gridMap[uid]->setSpacing(stepSizeX, stepSizeY);
               _gridMap[uid]->createGrid();

               _flowMap[uid]=std::vector<double>(_gridMap[uid]->GetnPoints(), 0.);
               _boundaryMap[uid]=std::vector<double>(_gridMap[uid]->GetnPoints(), 0.);
               _distanceMap[uid]=std::vector<double>(_gridMap[uid]->GetnPoints(), 0.);
               _angleMap[uid]=std::vector<double>(_gridMap[uid]->GetnPoints(), 0.);
               _staticMap[uid]=std::vector<double>(_gridMap[uid]->GetnPoints(), 0.);
               _wallDistanceMap[uid]=std::vector<double>(_gridMap[uid]->GetnPoints(), 0.);
               _wallPreferenceMap[uid]=std::vector<double>(_gridMap[uid]->GetnPoints(), 0.);
               _attractionRepulsionMap[uid]=std::vector<double>(_gridMap[uid]->GetnPoints(), 0.);
               _forbidenMap[uid]=std::vector<double>(_gridMap[uid]->GetnPoints(), 0.);

               _distanceFieldMap[uid]=std::vector<double>(_gridMap[uid]->GetnPoints(), 0.);
               _dynamicDistanceMap[uid]=std::vector<double>(_gridMap[uid]->GetnPoints(), 0.);
               _distanceProbMap[uid]=std::vector<double>(_gridMap[uid]->GetnPoints(), 0.);
               _dynamicMap[uid]=std::vector<double>(_gridMap[uid]->GetnPoints(), 0.);
               _pathMap[uid]=std::vector<double>(_gridMap[uid]->GetnPoints(), 0.);

               _probMap[uid]=std::vector<double>(_gridMap[uid]->GetnPoints(), 0.);
               _valueMap[uid]=std::vector<double>(_gridMap[uid]->GetnPoints(), 0.);
               _filterMap[uid]=std::vector<double>(_gridMap[uid]->GetnPoints(), 0.);
          }
     }
}

Point WaitingProbability::GetWaitingPosition(Room* room, Pedestrian* ped){
     SubRoom* subRoom = room->GetSubRoom(ped->GetSubRoomID());

     int uid = subRoom->GetUID();
     double x=0., y=0.;

     computeDynamic(subRoom, ped);

     combineAll(subRoom);


     //TODO random generator as class member
     std::random_device rd;
     std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
     std::uniform_real_distribution<> dis(0.0, 1.0);

     double random = dis(gen);

     double sum = 0.;

     for (int i = 0; i<_gridMap.at(uid)->GetnPoints(); ++i) {
          sum += _probMap.at(uid).at(i);
          x = _gridMap.at(uid)->get_x_fromKey(i);
          y = _gridMap.at(uid)->get_y_fromKey(i);

          if (sum >= random){
               x = _gridMap.at(uid)->get_x_fromKey(i);
               y = _gridMap.at(uid)->get_y_fromKey(i);

               if (subRoom->IsInSubRoom(Point(x,y)) && _distanceFieldMap.at(uid).at(i) > 0.3 ){
                    break;
               }
          }

          if (i==_gridMap.at(uid)->GetnPoints()-1){
               x = subRoom->GetCentroid()._x;
               y = subRoom->GetCentroid()._y;
          }
     }

     Point ret(x,y);
     return ret;
}

Point WaitingProbability::GetPath(Pedestrian* ped){
     Point waitingPos = ped->GetWaitingPos();
     Point currentPos = ped->GetPos();


     if (_building->IsVisible(waitingPos, currentPos, std::vector<SubRoom*>(), false)){
          return waitingPos;
     } else {

          int uid = ped->GetSubRoomUID();
          uid = 1;

          double maxStep = * std::max_element(std::begin(_pathMap.at(uid)), std::end(_pathMap.at(uid))); // c++11

          int step = 0;

//          std::cout << std::endl;
          std::cout << "Path size: " << _pathMap.at(uid).size() << std::endl;
//          std::vector<double> path;
//
//          for (int i = 0; i<_pathMap.at(uid).size(); ++i){
//               path.emplace_back(_pathMap.at(uid).at(i));
//          }

          std::vector<double> path(_pathMap.at(uid));
//          std::vector<double> path(_pathMap.at(uid).size());
//          std::copy(_pathMap.at(uid).begin(), )
          double tmp = std::numeric_limits<double>::max();
          double min = std::numeric_limits<double>::max();


          // 0 = right
          // 1 = top
          // 2 = left
          // 3 = down
          int direction = -1;

          // Search grid point of waiting pos
          Point gridPoint = _gridMap.at(uid)->getNearestGridPoint(waitingPos);
          int currentIndex = _gridMap.at(uid)->getKeyAtPoint(gridPoint);

          Point current = gridPoint;

          // While no sight connection between
          do{
               // check neighbors if direction change needed
               directNeighbor neighbors = _gridMap.at(uid)->getNeighbors(currentIndex);

               double dirMin = std::numeric_limits<double>::max();
               // Set min to value in direction if reachable
               if (direction != -1 && neighbors.key[direction] != -2){
                    double x = _gridMap.at(uid)->get_x_fromKey(neighbors.key[direction]);
                    double y = _gridMap.at(uid)->get_y_fromKey(neighbors.key[direction]);

                    if (_building->GetSubRoomByUID(uid)->IsInSubRoom(Point(x,y))){
                         dirMin = path.at(neighbors.key[direction]);
                    }
               }

               double tmpMin = std::numeric_limits<double>::max();
               int tmpDirection = -1;

               for (int i=0; i<4; ++i){
                    int neighbor = neighbors.key[i];
                    double x0 = _gridMap.at(uid)->get_x_fromKey(neighbor);
                    double y0 = _gridMap.at(uid)->get_y_fromKey(neighbor);
                    if (neighbor != -2 && _building->GetSubRoomByUID(uid)->IsInSubRoom(Point(x0,y0)) ){
                         tmp = path.at(neighbor);
                         if (tmp < tmpMin && tmp < min && tmp > 0.){
                              tmpMin = tmp;
                              tmpDirection = i;
                         }
                    }
               }


               if (direction == -1) {
                    direction = tmpDirection;
                    min = tmpMin;
               } else {
                    if (tmpMin < dirMin){
                         direction = tmpDirection;
                         min = tmpMin;
                    } else {
                         direction = direction;
                         min = dirMin;
                    }
               }

               current = _gridMap.at(uid)->getPointFromKey(neighbors.key[direction]);
               currentIndex = neighbors.key[direction];

               step++;
               path.at(currentIndex) = std::numeric_limits<double>::max();

          }while(!_building->IsVisible(currentPos, current, std::vector<SubRoom*>(), false) && step < maxStep);

          return current;
     }
}

void WaitingProbability::computeDynamic(const SubRoom* subroom, Pedestrian* ped){
     Log->Write("Start compute dynamic");

     computeDistanceField(subroom, ped);
     computeDynamicDistance(subroom);
     computeDistanceProb(subroom);
     computeWallPath(subroom);

     writeVTK(subroom, "dynamic_" + std::to_string(subroom->GetUID()) +"_" + std::to_string(_numPed) + ".vtk");
     _numPed++;
}

void WaitingProbability::computeDistanceField(const SubRoom* subroom, Pedestrian* pedestrian){
     Log->Write("Start compute distancefield");
     int uid = subroom->GetUID();

     double xMin = _gridMap.at(uid)->GetxMin();
     double xMax = _gridMap.at(uid)->GetxMax();
     double yMin = _gridMap.at(uid)->GetyMin();
     double yMax = _gridMap.at(uid)->GetyMax();

     double maxDist = std::pow(xMax-xMin, 2.) + std::pow(yMax-yMin, 2.);

     double x,y;
     double minDist;

     std::vector<Pedestrian*> peds;
     _building->GetPedestrians(subroom->GetRoomID(), subroom->GetSubRoomID(), peds);
//     for (int i=0; i<10; ++i){
//          peds.push_back(new Pedestrian());
//     }
//
//     peds[0]->SetWaitingPos(Point(0.6, 3.0));
//     peds[1]->SetWaitingPos(Point(0.3, 2.7));
//     peds[2]->SetWaitingPos(Point(0.3, 2.1));
//     peds[3]->SetWaitingPos(Point(0.6, 1.2));
//     peds[4]->SetWaitingPos(Point(1.5, 0.6));
//     peds[5]->SetWaitingPos(Point(3.3, 0.3));
//     peds[6]->SetWaitingPos(Point(2.4, 1.5));
//     peds[7]->SetWaitingPos(Point(2.7, 2.1));
//     peds[8]->SetWaitingPos(Point(3.0, 3.3));
//     peds[9]->SetWaitingPos(Point(3.3, 2.7));


     for (int i = 0; i<_gridMap.at(uid)->GetnPoints(); ++i) {
//          minDist = DBL_MAX;
          minDist = maxDist;

          x = _gridMap.at(uid)->get_x_fromKey(i);
          y = _gridMap.at(uid)->get_y_fromKey(i);
          Point p(x, y);

          for (auto ped : peds){
               if (ped->GetID() == pedestrian->GetID()){
                    continue;
               }
               // TODO change to actual pos, when making it iterative
               Point pos = ped->GetWaitingPos();

               minDist = std::min(minDist, (p-pos).Norm());
          }

//          _distanceFieldMap.at(uid).at(i) = minDist>0.4?(minDist):(0.);
          _distanceFieldMap.at(uid).at(i) = minDist;
     }

//     normalize(_distanceFieldMap.at(uid));
     markOutside(_distanceFieldMap.at(uid), subroom);
}

void WaitingProbability::computeDynamicDistance(const SubRoom* subroom){
     Log->Write("Start compute DynamicDistance");

     int uid = subroom->GetUID();
     std::fill(_dynamicDistanceMap.at(uid).begin(), _dynamicDistanceMap.at(uid).end(), -1. );
     markOutside(_dynamicDistanceMap.at(uid), subroom);

     double x,y;

     std::queue<Point> points;
     std::set<Point> pointsSet;

     double minDist = DBL_MAX;

     int step = 1;
     for (auto trans : subroom->GetAllTransitions()){
          Point p1 = trans->GetPoint1();
          Point p2 = trans->GetPoint2();
          Point direction = p2 -p1;

          double stepSize = _gridMap.at(uid)->Gethx();
          Point moving = p1;
          double tmp;
          while (true){
               Point gridPoint = _gridMap.at(uid)->getNearestGridPoint(moving);
               int index = _gridMap.at(uid)->getKeyAtPoint(gridPoint);
               directNeighbor neighbors = _gridMap.at(uid)->getNeighbors(index);
               x = _gridMap.at(uid)->get_x_fromKey(index);
               y = _gridMap.at(uid)->get_y_fromKey(index);
               uniqueAdd(points, pointsSet, Point(x,y));
               _dynamicDistanceMap.at(uid).at(index) = step;
               for (int i=0; i<4; ++i){
                    int neighbor = neighbors.key[i];
                    if (neighbor != -2 ){
                         x = _gridMap.at(uid)->get_x_fromKey(neighbor);
                         y = _gridMap.at(uid)->get_y_fromKey(neighbor);
                         uniqueAdd(points, pointsSet, Point(x,y));
                    }
               }
               moving = moving + (direction * stepSize);

               tmp = (p2 - moving).Norm();

               if (tmp >= minDist || tmp < J_EPS){
                    break;
               }else{
                    minDist = tmp;
               }
          }
     }

     step++;
     points.push(Point(DBL_MAX, DBL_MAX));

     Point point;
     while(!points.empty()){
          point = points.front();

          if (point._x >= DBL_MAX-1. && point._y >= DBL_MAX-1.){
               points.pop();
               step ++;
               if (!points.empty()){
                    points.push(Point(DBL_MAX, DBL_MAX));
               }
               continue;
          }

          Point gridPoint = _gridMap.at(uid)->getNearestGridPoint(point);
          int index = _gridMap.at(uid)->getKeyAtPoint(gridPoint);

          if (_dynamicDistanceMap.at(uid).at(index) < 0. ){
               _dynamicDistanceMap.at(uid).at(index) = step;
          }

          directNeighbor neighbors = _gridMap.at(uid)->getNeighbors(index);
          for (int i=0; i<4; ++i){
               int neighbor = neighbors.key[i];
               if (neighbor != -2 && _dynamicDistanceMap.at(uid).at(neighbor) < 0. && _distanceFieldMap.at(uid).at(neighbor) > 0.3){
                    x = _gridMap.at(uid)->get_x_fromKey(neighbor);
                    y = _gridMap.at(uid)->get_y_fromKey(neighbor);
                    uniqueAdd(points, pointsSet, Point(x,y));
               }
          }

          points.pop();
          pointsSet.erase(point);
     }

     double gridsize = _gridMap.at(uid)->Gethx()*_gridMap.at(uid)->Gethy();
     double d = 0.4;


     std::copy(_dynamicDistanceMap.at(uid).begin(), _dynamicDistanceMap.at(uid).end(), _pathMap.at(uid).begin());

     for (int i = 0; i<_gridMap.at(uid)->GetnPoints(); ++i) {
          x = _gridMap.at(uid)->get_x_fromKey(i);
          y = _gridMap.at(uid)->get_y_fromKey(i);

          if (_dynamicDistanceMap.at(uid).at(i) < 0. ){
               _dynamicDistanceMap.at(uid).at(i) = 2*step;
          }

          _dynamicDistanceMap.at(uid).at(i) *= gridsize;
          _dynamicDistanceMap.at(uid).at(i) = -1. * d * _dynamicDistanceMap.at(uid).at(i);

     }

     markOutside(_dynamicDistanceMap.at(uid), subroom);
     postProcess(_dynamicDistanceMap.at(uid), subroom);
     normalize(_dynamicDistanceMap.at(uid));
}

void WaitingProbability::computeWallPath(const SubRoom* subroom){
     Log->Write("Start compute wall path");
     double tres = 1.2;

     int uid = subroom->GetUID();

     for (int i = 0; i<_gridMap.at(uid)->GetnPoints(); ++i) {
          double x = _gridMap.at(uid)->get_x_fromKey(i);
          double y = _gridMap.at(uid)->get_y_fromKey(i);
          Point p(x, y);

          if (subroom->IsInSubRoom(p)){
               if (_wallDistanceMap.at(uid).at(i) < tres && _pathMap.at(uid).at(i) > 0.){
                    _pathMap.at(uid).at(i) += 1./_wallDistanceMap.at(uid).at(i);
               }
          }
     }
}

void WaitingProbability::uniqueAdd(std::queue<Point>& points, std::set<Point>& pointsSet, Point p){
     auto it = std::find (pointsSet.begin(), pointsSet.end(), p);
     if (it == pointsSet.end()){
          points.push(p);
          pointsSet.insert(p);
     }
}

void WaitingProbability::computeDistanceProb(const SubRoom* subroom){
     Log->Write("Start compute distance prob");

     int uid = subroom->GetUID();
     double f  = 1.5;
     double f1 = 0.2;

     for (int i = 0; i<_gridMap.at(uid)->GetnPoints(); ++i) {
          _distanceProbMap.at(uid).at(i) = -1. *  f * exp(-1.*(std::pow(_distanceFieldMap.at(uid).at(i), 2.)/f1));
     }
     postProcess(_distanceProbMap.at(uid), subroom);
     normalize(_distanceProbMap.at(uid));
}

void WaitingProbability::combineAll(const SubRoom* subroom){
     Log->Write("Start compute distance prob");

     int uid = subroom->GetUID();

     for (int i = 0; i<_gridMap.at(uid)->GetnPoints(); ++i) {
          double value =
                    1.*_flowMap.at(uid).at(i)+
                    1.*_angleMap.at(uid).at(i)+
                    1.*_attractionRepulsionMap.at(uid).at(i)+
                    1.*_wallPreferenceMap.at(uid).at(i) +
                    1.*_dynamicDistanceMap.at(uid).at(i) ;

          double filter = _distanceProbMap.at(uid).at(i) *  _forbidenMap.at(uid).at(i);
          _valueMap.at(uid).at(i) = value;
          _filterMap.at(uid).at(i) = filter;

          _probMap.at(uid).at(i) = value * filter;
//          _probMap.at(uid).at(i) = (
//                         1.*_flowMap.at(uid).at(i)+
//                                   1.*_boundaryMap.at(uid).at(i)+
//                                   1.*_dynamicDistanceMap.at(uid).at(i)+
//                                   1.*_angleMap.at(uid).at(i)
//               )*_distanceProbMap.at(uid).at(i);
     }

     for (int i = 0; i<_gridMap.at(uid)->GetnPoints(); ++i) {
          if (_pathMap.at(uid).at(i) < 0){
               _probMap.at(uid).at(i) = 0.;
          }
     }


     normalize(_probMap.at(uid));
}

void WaitingProbability::computeStatic(){
     Log->Write("Start compute static");

     for (auto roomItr : _building->GetAllRooms()){
          for (auto subRoomItr : roomItr.second->GetAllSubRooms()) {
               SubRoom* subroom = subRoomItr.second.get();

               computeDistanceCost(subroom);
               computeAngleCost(subroom);
               computeFlowAvoidance(subroom);
//               computeBoundaryPreference(subroom);
               computeWallDistance(subroom);
               computeWallPreference(subroom);
               computeAttractionRepulsionZones(subroom);
               computeForbidenZones(subroom);
//               computeStatic(subroom);

//               writeVTK(subroom, "static_" + std::to_string(subroom->GetUID()) + ".vtk");
          }
     }
}

void WaitingProbability::computeStatic(const SubRoom* subroom)
{
     int uid = subroom->GetUID();

     for (int i = 0; i<_gridMap.at(uid)->GetnPoints(); ++i) {
          double value =
                    1.*_flowMap.at(uid).at(i)+
//                    1.*_boundaryMap.at(uid).at(i)+
//                    1.*_distanceMap.at(uid).at(i)+
                    1.*_angleMap.at(uid).at(i)+
                    1.*_attractionRepulsionMap.at(uid).at(i)+
                    1.*_wallPreferenceMap.at(uid).at(i);

          _staticMap.at(uid).at(i) = value;
     }

     normalize(_staticMap.at(uid));

     for (int i = 0; i<_gridMap.at(uid)->GetnPoints(); ++i) {
          _probMap.at(uid).at(i) = _staticMap.at(uid).at(i);
     }
}

void WaitingProbability::computeFlowAvoidance(const SubRoom* subroom){
     Log->Write("Start compute flow avoidance");

     double x,y, theta;
     int uid = subroom->GetUID();
     double f  = 1.5;
     double f1 = 0;
     double f2 = 0;
     Point xAxis(1., 0.);
     double flow = 0.;

     for (auto trans : subroom->GetAllTransitions()) {
          flow = 0.;

          Point centre = trans->GetCentre();
          double x0 = centre._x;
          double y0 = centre._y;

          f1 = trans->GetLength();
          f2 = 5.*f1;
          theta = Angle(xAxis, trans->GetPoint1()-trans->GetPoint2());

          float a = std::pow(cos(theta), 2.)/f1 + std::pow(sin(theta), 2.)/f2;
          float b = -1. * sin(2.*theta)/(2*f1) + sin(2.*theta)/(2.*f2);
          float c = std::pow(sin(theta), 2.)/f1 + std::pow(cos(theta), 2.)/f2;

          for (int i = 0; i<_gridMap.at(uid)->GetnPoints(); ++i) {
               flow = 0.;
               x = _gridMap.at(uid)->get_x_fromKey(i);
               y = _gridMap.at(uid)->get_y_fromKey(i);
               Point p(x, y);

               if(subroom->IsInSubRoom(p)){
                    flow = -1.*f*exp(-a * std::pow(x-x0, 2.) -2.*b*(x-x0)*(y-y0) -c*std::pow(y-y0, 2.));
               }

               _flowMap.at(uid).at(i) = std::min(_flowMap.at(uid).at(i), flow);
          }
     }
     postProcess(_flowMap.at(uid), subroom);
     normalize(_flowMap.at(uid));
}

void WaitingProbability::computeBoundaryPreference(const SubRoom* subroom){
     Log->Write("Start compute boundary preference");

     double x0, y0, x1, y1, sum;

     int uid = subroom->GetUID();

     double gridsize = _gridMap.at(uid)->Gethx()*_gridMap.at(uid)->Gethy();
     double b = 3.;

     for (int i = 0; i<_gridMap.at(uid)->GetnPoints(); ++i) {
          sum = 0;

          x0 = _gridMap.at(uid)->get_x_fromKey(i);
          y0 = _gridMap.at(uid)->get_y_fromKey(i);
          Point p0(x0, y0);

          if (subroom->IsInSubRoom(p0)) {
               for (int j = 0; j<_gridMap.at(uid)->GetnPoints(); ++j) {
                    x1 = _gridMap.at(uid)->get_x_fromKey(j);
                    y1 = _gridMap.at(uid)->get_y_fromKey(j);
                    Point p1(x1, y1);

                    if (subroom->IsInSubRoom(p1)) {
                         double distance = (p0-p1).Norm();
                         double bp = -1. * b*std::pow(1.+distance, -2.)*gridsize*gridsize;
                         sum += bp;
                    }
               }
          }

          _boundaryMap.at(uid).at(i) = sum/5.;
     }
     postProcess(_boundaryMap.at(uid), subroom);
     normalize(_boundaryMap.at(uid));
}

void WaitingProbability::computeDistanceCost(const SubRoom* subroom){
     Log->Write("Start compute distance cost");

     double x,y, minDist;
     double d = 0.4;

     int uid = subroom->GetUID();
     for (int i=0; i< _gridMap.at(uid)->GetnPoints(); ++i) {
          minDist = DBL_MAX;

          x = _gridMap.at(uid)->get_x_fromKey(i);
          y = _gridMap.at(uid)->get_y_fromKey(i);
          Point p(x, y);

          if (subroom->IsInSubRoom(p)){
               for (auto trans : subroom->GetAllTransitions()) {
                    minDist = std::min(minDist, Distance(p, trans->GetCentre()));
               }
               _distanceMap.at(uid).at(i) = -1. * d * minDist;
          }
     }
     postProcess(_distanceMap.at(uid), subroom);
     normalize(_distanceMap.at(uid));
}

void WaitingProbability::computeWallDistance(const SubRoom* subroom){
     Log->Write("Start compute wall distance");


     int uid = subroom->GetUID();
     double minDist;
     double x,y;

     double xMin = _gridMap.at(uid)->GetxMin();
     double xMax = _gridMap.at(uid)->GetxMax();
     double yMin = _gridMap.at(uid)->GetyMin();
     double yMax = _gridMap.at(uid)->GetyMax();

     double maxDist = std::sqrt(std::pow(xMax - xMin, 2.) + std::pow(yMax - yMin, 2.));

     for (int i=0; i< _gridMap.at(uid)->GetnPoints(); ++i) {
          minDist = std::numeric_limits<double>::max();

          x = _gridMap.at(uid)->get_x_fromKey(i);
          y = _gridMap.at(uid)->get_y_fromKey(i);
          Point p(x, y);

          if (subroom->IsInSubRoom(p)){

//               for (auto line : subroom->GetAllTransitions()) {
//                    minDist = std::min(minDist, line->DistTo(p));
//               }
               for (auto line : subroom->GetAllWalls()){
                    minDist = std::min(minDist, line.DistTo(p));
               }
          } else {
               minDist = maxDist;
          }
          _wallDistanceMap.at(uid).at(i) = minDist;
     }

}

void WaitingProbability::computeWallPreference(const SubRoom* subroom){
     Log->Write("Start compute wall pref");
     int uid = subroom->GetUID();

     for (int i=0; i< _gridMap.at(uid)->GetnPoints(); ++i) {
          double x = _gridMap.at(uid)->get_x_fromKey(i);
          double y = _gridMap.at(uid)->get_y_fromKey(i);
          Point p(x, y);

          if (subroom->IsInSubRoom(p)) {
               double dist = _wallDistanceMap.at(uid).at(i);
               _wallPreferenceMap.at(uid).at(i) = 0.5 * exp(-1* dist *dist);
          }
     }

     postProcess(_wallPreferenceMap.at(uid), subroom);
     normalize(_wallPreferenceMap.at(uid));
}

void WaitingProbability::computeAttractionRepulsionZones(const SubRoom* subroom){
     Log->Write("Start compute attraction cost");
     int uid = subroom->GetUID();

     //TODO read from file
     double xMinAtt = 0.5;
     double xMaxAtt = 1.5;
     double yMinAtt = 3.5;
     double yMaxAtt = 4.5;

     double xMinRep = 4.5;
     double xMaxRep = 5.5;
     double yMinRep = 3.5;
     double yMaxRep = 4.5;

     for (int i=0; i< _gridMap.at(uid)->GetnPoints(); ++i) {
          double x = _gridMap.at(uid)->get_x_fromKey(i);
          double y = _gridMap.at(uid)->get_y_fromKey(i);
          Point p(x, y);

          if (subroom->IsInSubRoom(p)) {
               if ( xMinAtt <= x && x <= xMaxAtt && yMinAtt <= y && y <= yMaxAtt){
                    _attractionRepulsionMap.at(uid).at(i) = -1.;
               }

               if ( xMinRep <= x && x <= xMaxRep && yMinRep <= y && y <= yMaxRep){
                    _attractionRepulsionMap.at(uid).at(i) = 1.;
               }
          }
     }

     postProcess(_attractionRepulsionMap.at(uid), subroom);
     normalize(_attractionRepulsionMap.at(uid));
}

void WaitingProbability::computeForbidenZones(const SubRoom* subroom){
     Log->Write("Start compute attraction cost");
     int uid = subroom->GetUID();

     //TODO read from file
     double xMin = 0.5;
     double xMax = 1.5;
     double yMin = 0.5;
     double yMax = 1.5;

     for (int i=0; i< _gridMap.at(uid)->GetnPoints(); ++i) {
          double x = _gridMap.at(uid)->get_x_fromKey(i);
          double y = _gridMap.at(uid)->get_y_fromKey(i);
          Point p(x, y);

          if (subroom->IsInSubRoom(p)) {
               if ( xMin <= x && x <= xMax && yMin <= y && y <= yMax){
                    _forbidenMap.at(uid).at(i) = 0.;
               } else {
                    _forbidenMap.at(uid).at(i) = 1.;
               }
          }
     }
}

void WaitingProbability::computeAngleCost(const SubRoom* subroom){
     Log->Write("Start compute angle cost");

     double x,y, minAngle;

     double a = 0.05;
     double alpha = 3.;

     int uid = subroom->GetUID();

     for (int i=0; i< _gridMap.at(uid)->GetnPoints(); ++i) {
          minAngle = DBL_MAX;

          x = _gridMap.at(uid)->get_x_fromKey(i);
          y = _gridMap.at(uid)->get_y_fromKey(i);
          Point p(x, y);

          if (subroom->IsInSubRoom(p)){
               for (auto trans : subroom->GetAllTransitions()) {
                    Point p0 = trans->GetPoint2() - trans->GetPoint1();

                    Point l1 = trans->GetPoint1() -p;
                    double angle1 = Angle(p0, l1);

                    Point l2 = trans->GetPoint2() -p;
                    double angle2 = Angle(p0, l2);

                    Point xAxis(1., 0.);
                    double theta = Angle(xAxis, trans->GetPoint1()-trans->GetPoint2());

                    double tmpAngle = std::abs(checkAngles(angle1, angle2) - theta);
                    minAngle = std::min(minAngle, tmpAngle);
               }
               _angleMap.at(uid).at(i) = -1. * a * std::pow(minAngle, alpha);
          }
     }

     postProcess(_angleMap.at(uid), subroom);
     normalize(_angleMap.at(uid));
}

double WaitingProbability::checkAngles(double a, double b){
     double pi = 3.141592653589793238463;
     double ad = a * (180.0/pi) ;
     double bd = b * (180.0/pi) ;

     if (ad >= 90 && bd <= 90){
          return 90 * pi / 180.0;
     } else {
          if (ad > 90 && bd > 90){
               a -= 90 * pi / 180.0;
               b -= 90 * pi / 180.0;
               return 90 * pi / 180.0 - std::min(a,b);
          } else {
               return std::max(a, b);
          }
     }
}

void WaitingProbability::postProcess(std::vector<double>& data, const SubRoom* subroom){
     double max = 0;
     for(auto it = data.begin(); it != data.end(); ++it){
          max = std::max(max, std::abs(*it));
     }

     for(auto it = data.begin(); it != data.end(); ++it){
          *it += max;
     }

     markOutside(data, subroom);
//     markDoors(data, subroom);
}

void WaitingProbability::markOutside(std::vector<double>& data, const SubRoom* subroom){

     int uid = subroom->GetUID();
     double x, y;

     for (int i=0; i< _gridMap.at(uid)->GetnPoints(); ++i) {
          x = _gridMap.at(uid)->get_x_fromKey(i);
          y = _gridMap.at(uid)->get_y_fromKey(i);
          Point p(x, y);
          long int key = _gridMap.at(uid)->getKeyAtPoint(p);
          if (!subroom->IsInSubRoom(p)) {
               data.at(key) = 0;
          }
     }
}


void WaitingProbability::normalize(std::vector<double>& data){
     double sum = 0.;

     for(auto it = data.begin(); it != data.end(); ++it){
          sum += *it;
     }

     for(auto it = data.begin(); it != data.end(); ++it){
          *it /= sum;
     }
}

void WaitingProbability::writeVTK(const SubRoom* subroom, const std::string filename){

     Log->Write("INFO: \tWrite Floorfield to file");
     Log->Write(filename);
     std::ofstream file;

     RectGrid* grid = _gridMap[subroom->GetUID()];
     int uid = subroom->GetUID();
     file.open(_building->GetConfig()->GetProjectRootDir()+filename);

     file << "# vtk DataFile Version 3.0" << std::endl;
     file << "WaitingProb Test: " << std::endl;
     file << "ASCII" << std::endl;
     file << "DATASET STRUCTURED_POINTS" << std::endl;
     file << "DIMENSIONS " <<  std::to_string(grid->GetiMax()) << " " <<
          std::to_string(grid->GetjMax()) << " 1" << std::endl;
     file << "ORIGIN " << grid->GetxMin() << " " << grid->GetyMin() << " 0" << std::endl;
     file << "SPACING " << std::to_string(grid->Gethx()) << " " << std::to_string(grid->Gethy()) << " 1" << std::endl;
     file << "POINT_DATA " << std::to_string(grid->GetnPoints()) << std::endl;

     // Print gcode//
     file << "SCALARS flow float 1" << std::endl;
     file << "LOOKUP_TABLE default" << std::endl;
     for (long int i = 0; i < grid->GetnPoints(); ++i) {
          file << _flowMap.at(uid).at(i) << std::endl;
     }

     file << "SCALARS boundary float 1" << std::endl;
     file << "LOOKUP_TABLE default" << std::endl;
     for (long int i = 0; i < grid->GetnPoints(); ++i) {
          file << _boundaryMap.at(uid).at(i) << std::endl;
     }

     file << "SCALARS distance float 1" << std::endl;
     file << "LOOKUP_TABLE default" << std::endl;
     for (long int i = 0; i < grid->GetnPoints(); ++i) {
          file << _distanceMap.at(uid).at(i) << std::endl;
     }

     file << "SCALARS angle float 1" << std::endl;
     file << "LOOKUP_TABLE default" << std::endl;
     for (long int i = 0; i < grid->GetnPoints(); ++i) {
          file << _angleMap.at(uid).at(i) << std::endl;
     }

     file << "SCALARS static float 1" << std::endl;
     file << "LOOKUP_TABLE default" << std::endl;
     for (long int i = 0; i < grid->GetnPoints(); ++i) {
          file << _staticMap.at(uid).at(i) << std::endl;
     }

     file << "SCALARS distanceField float 1" << std::endl;
     file << "LOOKUP_TABLE default" << std::endl;
     for (long int i = 0; i < grid->GetnPoints(); ++i) {
          file << _distanceFieldMap.at(uid).at(i) << std::endl;
     }

     file << "SCALARS dynamicDistance float 1" << std::endl;
     file << "LOOKUP_TABLE default" << std::endl;
     for (long int i = 0; i < grid->GetnPoints(); ++i) {
          file << _dynamicDistanceMap.at(uid).at(i) << std::endl;
     }

     file << "SCALARS path float 1" << std::endl;
     file << "LOOKUP_TABLE default" << std::endl;
     for (long int i = 0; i < grid->GetnPoints(); ++i) {
          file << _pathMap.at(uid).at(i) << std::endl;
     }

     file << "SCALARS wallDist float 1" << std::endl;
     file << "LOOKUP_TABLE default" << std::endl;
     for (long int i = 0; i < grid->GetnPoints(); ++i) {
          file << _wallDistanceMap.at(uid).at(i) << std::endl;
     }

     file << "SCALARS wallPref float 1" << std::endl;
     file << "LOOKUP_TABLE default" << std::endl;
     for (long int i = 0; i < grid->GetnPoints(); ++i) {
          file << _wallPreferenceMap.at(uid).at(i) << std::endl;
     }

     file << "SCALARS attRep float 1" << std::endl;
     file << "LOOKUP_TABLE default" << std::endl;
     for (long int i = 0; i < grid->GetnPoints(); ++i) {
          file << _attractionRepulsionMap.at(uid).at(i) << std::endl;
     }

     file << "SCALARS distanceProb float 1" << std::endl;
     file << "LOOKUP_TABLE default" << std::endl;
     for (long int i = 0; i < grid->GetnPoints(); ++i) {
          file << _distanceProbMap.at(uid).at(i) << std::endl;
     }

     file << "SCALARS prob float 1" << std::endl;
     file << "LOOKUP_TABLE default" << std::endl;
     for (long int i = 0; i < grid->GetnPoints(); ++i) {
          file << _probMap.at(uid).at(i) << std::endl;
     }

     file << "SCALARS forbiden float 1" << std::endl;
     file << "LOOKUP_TABLE default" << std::endl;
     for (long int i = 0; i < grid->GetnPoints(); ++i) {
          file << _forbidenMap.at(uid).at(i) << std::endl;
     }
     file << "SCALARS value float 1" << std::endl;
     file << "LOOKUP_TABLE default" << std::endl;
     for (long int i = 0; i < grid->GetnPoints(); ++i) {
          file << _valueMap.at(uid).at(i) << std::endl;
     }
     file << "SCALARS filter float 1" << std::endl;
     file << "LOOKUP_TABLE default" << std::endl;
     for (long int i = 0; i < grid->GetnPoints(); ++i) {
          file << _filterMap.at(uid).at(i) << std::endl;
     }


     file.close();
}
