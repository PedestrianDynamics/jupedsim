/**
 * \file        DirectionLocalFloorfield.cpp
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
 **/

#include "DirectionLocalFloorfield.h"

#include "geometry/Line.h"
#include "geometry/NavLine.h"
#include "geometry/Building.h"
#include "pedestrian/Pedestrian.h"
#include "geometry/SubRoom.h"

#include "router/ff_router/UnivFFviaFM.h"
#include "router/ff_router/FloorfieldViaFM.h"
#include "router/ff_router/ffRouter.h"

#include <chrono>


/// 8
Point DirectionLocalFloorfield::GetTarget(Room* room, Pedestrian* ped) const
{
#if DEBUG
     if (initDone && (ffviafm != nullptr)) {
#endif // DEBUG

     Point p;
     UnivFFviaFM* floorfield = _locffviafm.at(room->GetID());
#if DEBUG
     if (!floorfield->getGrid()->includesPoint(ped->GetPos())) {
          Log->Write("ERROR: \tDirectionLocalFloorfield::GetTarget is accessing wrong floorfield. Pedestrian is not inside!");
          p = Point(0.0,0.0);
          return p;
     }
#endif
     floorfield->getDirectionToUID(ped->GetExitIndex(), ped->GetPos(), p);
//     if (floorfield->getCostToDestination(ped->GetExitIndex(), ped->GetPos()) < 1.0) {
//          p = p * floorfield->getCostToDestination(ped->GetExitIndex(), ped->GetPos());
//     }
     return (p + ped->GetPos());

#if DEBUG
     }
#endif // DEBUG

     //this should not execute:
     //std::cerr << "Failure in DirectionFloorfield::GetTarget!!" << std::endl;
     // exit(EXIT_FAILURE);
}

Point DirectionLocalFloorfield::GetDir2Wall(Pedestrian* ped) const
{
     Point p;
     int roomID = ped->GetRoomID();
     _locffviafm.at(roomID)->getDir2WallAt(ped->GetPos(), p);
     return p;
}

double DirectionLocalFloorfield::GetDistance2Wall(Pedestrian* ped) const
{
     return _locffviafm.at(ped->GetRoomID())->getDistance2WallAt(ped->GetPos());
}

double DirectionLocalFloorfield::GetDistance2Target(Pedestrian* ped, int UID) {
     int roomID = ped->GetRoomID();
     return _locffviafm.at(roomID)->getCostToDestination(UID, ped->GetPos());
}

void DirectionLocalFloorfield::Init(Building* building) {
     _building = building;

     _stepsize = building->GetConfig()->get_deltaH();
     _wallAvoidDistance = building->GetConfig()->get_wall_avoid_distance();
     _useDistancefield = building->GetConfig()->get_use_wall_avoidance();

     std::chrono::time_point<std::chrono::system_clock> start, end;
     start = std::chrono::system_clock::now();
     Log->Write("INFO: \tCalling Constructor of UnivFFviaFM(Room-scale) in DirectionLocalFloorfield::Init(...)");

     for (auto& roomPair : _building->GetAllRooms()) {
          UnivFFviaFM* newfield =  new UnivFFviaFM(roomPair.second.get(), _building, _stepsize, _wallAvoidDistance, _useDistancefield);
          _locffviafm[roomPair.first] = newfield;
          newfield->setUser(DISTANCE_AND_DIRECTIONS_USED);
          newfield->setMode(LINESEGMENT);
          if (_useDistancefield) {
               newfield->setSpeedMode(FF_WALL_AVOID);
          } else {
               newfield->setSpeedMode(FF_HOMO_SPEED);
          }
          newfield->addAllTargetsParallel();

//          newfield->writeFF("directionsOfRoom" + std::to_string(roomPair.first) + ".vtk", newfield->getKnownDoorUIDs());
     }

//     if (_building->GetConfig()->get_write_VTK_files_direction()) {
          for (unsigned int i = 0; i < _locffviafm.size(); ++i) {
               auto iter = _locffviafm.begin();
               std::advance(iter, i);
               int roomNr = iter->first;
               iter->second->writeFF("direction" + std::to_string(roomNr) + ".vtk", iter->second->getKnownDoorUIDs());
          }
//     }

     end = std::chrono::system_clock::now();
     std::chrono::duration<double> elapsed_seconds = end-start;
     Log->Write("INFO: \tTime to construct FF in DirectionLocalFloorfield: " + std::to_string(elapsed_seconds.count()));
     _initDone = true;
}

DirectionLocalFloorfield::DirectionLocalFloorfield() {
     _initDone = false;
}

DirectionLocalFloorfield::~DirectionLocalFloorfield() {
     for (auto pair : _locffviafm) {
          if (pair.second) {
               delete pair.second;
          }
     }
}

