/**
 * \file        DirectionStrategy.cpp
 * \date        Dec 13, 2010
 * \version     v0.7
 * \copyright   <2009-2015> Forschungszentrum Jülich GmbH. All rights reserved.
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
 *
 *
 **/

//#include "DirectionStrategy.h"
#include "../../geometry/Line.h"
#include "../../geometry/NavLine.h"
#include "../../geometry/Building.h"
//#include "../geometry/Room.h"
#include "../../pedestrian/Pedestrian.h"
#include "../../geometry/SubRoom.h"
//#include "../geometry/Wall.h"
//#include "../routing/router/ff_router/UnivFFviaFM.h"
//#include "../routing/router/ff_router/FloorfieldViaFM.h"
//#include "../routing/router/ff_router/ffRouter.h"
//#include <fstream>
//#include <ctime>
#include <chrono>


DirectionStrategy::DirectionStrategy()
{
}


DirectionStrategy::~DirectionStrategy()
{
}

double DirectionStrategy::GetDistance2Wall(Pedestrian* ped) const
{
     return -1.;
}
double DirectionStrategy::GetDistance2Target(Pedestrian* ped, int UID)
{
     return -1.;
}

/////10
//Point DirectionSubLocalFloorfieldTrips::GetTarget(Room* room, Pedestrian* ped) const
//{
//     Goal* goal = ped->GetBuilding()->GetFinalGoal(ped->GetFinalDestination());
//     // Pedestrian is inside a waiting area
//     if ((goal!=nullptr) && (goal->IsInsideGoal(ped->GetPos()))){
//
//          std::vector<Point> polygon(goal->GetPolygon());
//          std::set<Point> triangle;
//
//          int min=0, max;
//
//          // Get randomly 3 points of polygon (
//          while (triangle.size() < 3){
//               max = polygon.size()-1;
//               int index = min + (std::rand() % static_cast<int>(max - min + 1));
//
//               triangle.insert(polygon.at(index));
//          }
//
//          double r1, r2;
//          r1 = ((double) std::rand() / (RAND_MAX));
//          r2 = ((double) std::rand() / (RAND_MAX));
//          Point p1 = polygon[0] * (1. - sqrt(r1));
//          Point p2 = polygon[1] * (sqrt(r1) * (1 - r2));
//          Point p3 = polygon[2] * (sqrt(r1) * r2);
//
//          Point p = p1 + p2 + p3;
//
////          UnivFFviaFM* floorfield = _locffviafm.at(ped->GetSubRoomUID());
////          floorfield->getDirectionToUID(ped->GetExitIndex(), ped->GetPos(),p);
////          return (p + ped->GetPos());
//          return p;
//     } else {
//          Point p;
//          UnivFFviaFM* floorfield = _locffviafm.at(ped->GetSubRoomUID());
//          floorfield->getDirectionToUID(ped->GetExitIndex(), ped->GetPos(),p);
//          return (p + ped->GetPos());
//
//     }
//}
//
//Point DirectionSubLocalFloorfieldTrips::GetDir2Wall(Pedestrian* ped) const
//{
//     Point p;
//     int key = ped->GetSubRoomUID();
//     _locffviafm.at(key)->getDir2WallAt(ped->GetPos(), p);
//     return p;
//}
//
//double DirectionSubLocalFloorfieldTrips::GetDistance2Wall(Pedestrian* ped) const
//{
//     return _locffviafm.at(ped->GetSubRoomUID())->getDistance2WallAt(ped->GetPos());
//}
//
//double DirectionSubLocalFloorfieldTrips::GetDistance2Target(Pedestrian* ped, int UID) {
//     int subroomUID = ped->GetSubRoomUID();
//     return _locffviafm.at(subroomUID)->getCostToDestination(UID, ped->GetPos());
//}
//
//void DirectionSubLocalFloorfieldTrips::Init(Building* buildingArg, double stepsize,
//          double threshold, bool useDistanceMap) {
//     _stepsize = stepsize;
//     _building = buildingArg;
//     _wallAvoidDistance = threshold;
//     _useDistancefield = useDistanceMap;
//
//     std::chrono::time_point<std::chrono::system_clock> start, end;
//     start = std::chrono::system_clock::now();
//     Log->Write("INFO: \tCalling Construtor of UnivFFviaFMTrips(Subroom-scale)");
//
//     for (auto& roomPair : _building->GetAllRooms()) {
//          for (auto& subPair : roomPair.second->GetAllSubRooms()) {
//               int subUID = subPair.second->GetUID();
//               UnivFFviaFM* floorfield = new UnivFFviaFM(subPair.second.get(), _building, stepsize, _wallAvoidDistance, _useDistancefield);
//               _locffviafm[subUID] = floorfield;
//               floorfield->setUser(DISTANCE_AND_DIRECTIONS_USED);
//               floorfield->setMode(LINESEGMENT);
//               if (useDistanceMap) {
//                    floorfield->setSpeedMode(FF_WALL_AVOID);
//               } else {
//                    floorfield->setSpeedMode(FF_HOMO_SPEED);
//               }
//               floorfield->addAllTargetsParallel();
//          }
//
//
//     }
//
//     if (_building->GetConfig()->get_write_VTK_files_direction()) {
//          for (unsigned int i = 0; i < _locffviafm.size(); ++i) {
//               auto iter = _locffviafm.begin();
//               std::advance(iter, i);
//               int roomNr = iter->first;
//               iter->second->writeFF("direction" + std::to_string(roomNr) + ".vtk", iter->second->getKnownDoorUIDs());
//          }
//     }
//
//
//     end = std::chrono::system_clock::now();
//     std::chrono::duration<double> elapsed_seconds = end-start;
//     Log->Write("INFO: \tTaken time: " + std::to_string(elapsed_seconds.count()));
//
//     _initDone = true;
//
//     //_locffviafm[0]->writeFF()
//     //write floorfields to file, one file per subroom //ar.graf: [SWITCH writevtk ON/OFF]
////     for(unsigned int i = 0; i < subUIDs.size(); ++i) {
////          std::vector<int> targets = {};
////          targets.clear();
////          int subroomUID = subUIDs[i];
////          //if (subroomUID != 26) continue;
////
////          for (auto pair : subAndTarget) {
////               if (pair.first == subroomUID) {
////                    targets.emplace_back(pair.second);
////               }
////          }
////          std::string filename1 = "floorfield" + std::to_string(subroomUID) + ".vtk";
////          if (targets.size() > 0)
////               _locffviafm[subroomUID]->writeFF(filename1, targets);
////     }
//}
//
//DirectionSubLocalFloorfieldTrips::DirectionSubLocalFloorfieldTrips() {
//     _initDone = false;
//}
//
//DirectionSubLocalFloorfieldTrips::~DirectionSubLocalFloorfieldTrips() {
//     for (auto pair : _locffviafm) {
//          delete pair.second;
//     }
//}
//
/////11
//Point DirectionSubLocalFloorfieldTripsVoronoi::GetTarget(Room* room, Pedestrian* ped) const
//{
//     Goal* goal = ped->GetBuilding()->GetFinalGoal(ped->GetFinalDestination());
//     // Pedestrian is inside a waiting area
//     if ((goal!=nullptr) && (goal->IsInsideGoal(ped->GetPos()))){
//
//          std::vector<Point> polygon(goal->GetPolygon());
//          std::set<Point> triangle;
//
//          int min=0, max;
//
//          // Get randomly 3 points of polygon (
//          while (triangle.size() < 3){
//               max = polygon.size()-1;
//               int index = min + (std::rand() % static_cast<int>(max - min + 1));
//
//               triangle.insert(polygon.at(index));
//          }
//
//          double r1, r2;
//          r1 = ((double) std::rand() / (RAND_MAX));
//          r2 = ((double) std::rand() / (RAND_MAX));
//          Point p1 = polygon[0] * (1. - sqrt(r1));
//          Point p2 = polygon[1] * (sqrt(r1) * (1 - r2));
//          Point p3 = polygon[2] * (sqrt(r1) * r2);
//
//          Point p = p1 + p2 + p3;
//
//          return p;
//     } else {
//          Point p;
//          UnivFFviaFM* floorfield = _locffviafm.at(ped->GetSubRoomUID());
//          floorfield->getDirectionToUID(ped->GetExitIndex(), ped->GetPos(),p);
//
//          return (p + ped->GetPos());
//
//     }
//}
//
//Point DirectionSubLocalFloorfieldTripsVoronoi::GetDir2Wall(Pedestrian* ped) const
//{
//     Point p;
//     int key = ped->GetSubRoomUID();
//     _locffviafm.at(key)->getDir2WallAt(ped->GetPos(), p);
//     return p;
//}
//
//double DirectionSubLocalFloorfieldTripsVoronoi::GetDistance2Wall(Pedestrian* ped) const
//{
//     return _locffviafm.at(ped->GetSubRoomUID())->getDistance2WallAt(ped->GetPos());
//}
//
//double DirectionSubLocalFloorfieldTripsVoronoi::GetDistance2Target(Pedestrian* ped, int UID) {
//     int subroomUID = ped->GetSubRoomUID();
//     return _locffviafm.at(subroomUID)->getCostToDestination(UID, ped->GetPos());
//}
//
//void DirectionSubLocalFloorfieldTripsVoronoi::Init(Building* buildingArg, double stepsize,
//          double threshold, bool useDistanceMap) {
//     _stepsize = stepsize;
//     _building = buildingArg;
//     _wallAvoidDistance = threshold;
//     _useDistancefield = useDistanceMap;
//
//     std::chrono::time_point<std::chrono::system_clock> start, end;
//     start = std::chrono::system_clock::now();
//     Log->Write("INFO: \tCalling Construtor of UnivFFviaFMTripsVoronoi(Subroom-scale)");
//
//     for (auto& roomPair : _building->GetAllRooms()) {
//          for (auto& subPair : roomPair.second->GetAllSubRooms()) {
//               int subUID = subPair.second->GetUID();
//               UnivFFviaFM* floorfield = new UnivFFviaFM(subPair.second.get(), _building, stepsize, _wallAvoidDistance, _useDistancefield);
//               _locffviafm[subUID] = floorfield;
//               floorfield->setUser(DISTANCE_AND_DIRECTIONS_USED);
//               floorfield->setMode(LINESEGMENT);
//               if (useDistanceMap) {
//                    floorfield->setSpeedMode(FF_WALL_AVOID);
//               } else {
//                    floorfield->setSpeedMode(FF_HOMO_SPEED);
//               }
//               floorfield->addAllTargetsParallel();
//          }
//
//
//     }
//
//     if (_building->GetConfig()->get_write_VTK_files_direction()) {
//          for (unsigned int i = 0; i < _locffviafm.size(); ++i) {
//               auto iter = _locffviafm.begin();
//               std::advance(iter, i);
//               int roomNr = iter->first;
//               iter->second->writeFF("direction" + std::to_string(roomNr) + ".vtk", iter->second->getKnownDoorUIDs());
//          }
//     }
//
//
//     end = std::chrono::system_clock::now();
//     std::chrono::duration<double> elapsed_seconds = end-start;
//     Log->Write("INFO: \tTaken time: " + std::to_string(elapsed_seconds.count()));
//
//     _initDone = true;
//
//     //_locffviafm[0]->writeFF()
//     //write floorfields to file, one file per subroom //ar.graf: [SWITCH writevtk ON/OFF]
////     for(unsigned int i = 0; i < subUIDs.size(); ++i) {
////          std::vector<int> targets = {};
////          targets.clear();
////          int subroomUID = subUIDs[i];
////          //if (subroomUID != 26) continue;
////
////          for (auto pair : subAndTarget) {
////               if (pair.first == subroomUID) {
////                    targets.emplace_back(pair.second);
////               }
////          }
////          std::string filename1 = "floorfield" + std::to_string(subroomUID) + ".vtk";
////          if (targets.size() > 0)
////               _locffviafm[subroomUID]->writeFF(filename1, targets);
////     }
//}
//
//DirectionSubLocalFloorfieldTripsVoronoi::DirectionSubLocalFloorfieldTripsVoronoi() {
//     _initDone = false;
//}
//
//DirectionSubLocalFloorfieldTripsVoronoi::~DirectionSubLocalFloorfieldTripsVoronoi() {
//     for (auto pair : _locffviafm) {
//          delete pair.second;
//     }
//}

