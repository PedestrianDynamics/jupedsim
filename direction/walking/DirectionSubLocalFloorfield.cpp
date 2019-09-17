/**
 * \file        DirectionSubLocalFloorfield.cpp
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

#include "DirectionSubLocalFloorfield.h"

#include "geometry/Line.h"
#include "geometry/NavLine.h"
#include "geometry/Building.h"
#include "pedestrian/Pedestrian.h"
#include "geometry/SubRoom.h"

#include "router/ff_router/UnivFFviaFM.h"
#include "router/ff_router/FloorfieldViaFM.h"
#include "router/ff_router/ffRouter.h"

#include <chrono>

///9
Point DirectionSubLocalFloorfield::GetTarget(Room* room, Pedestrian* ped) const
{
     (void)room; // silence warning
#if DEBUG
     if (initDone && (ffviafm != nullptr)) {
#endif // DEBUG

     Point p;
     UnivFFviaFM* floorfield = _locffviafm.at(ped->GetSubRoomUID());
#if DEBUG
     if (!floorfield->getGrid()->includesPoint(ped->GetPos())) {
          Log->Write("ERROR: \tDirectionSubLocalFloorfield::GetTarget is accessing wrong floorfield. Pedestrian is not inside!");
          p = Point(0.0,0.0);
          return p;
     }
#endif
     floorfield->getDirectionToUID(ped->GetExitIndex(), ped->GetPos(),p);
//     if (floorfield->getCostToDestination(ped->GetExitIndex(), ped->GetPos()) < 1.0){
//          p = p * floorfield->getCostToDestination(ped->GetExitIndex(), ped->GetPos());
//     }
     return (p + ped->GetPos());

#if DEBUG
     }
#endif // DEBUG
}

Point DirectionSubLocalFloorfield::GetDir2Wall(Pedestrian* ped) const
{
     Point p;
     int key = ped->GetSubRoomUID();
     _locffviafm.at(key)->getDir2WallAt(ped->GetPos(), p);
     return p;
}

double DirectionSubLocalFloorfield::GetDistance2Wall(Pedestrian* ped) const
{
     return _locffviafm.at(ped->GetSubRoomUID())->getDistance2WallAt(ped->GetPos());
}

double DirectionSubLocalFloorfield::GetDistance2Target(Pedestrian* ped, int UID) {
     int subroomUID = ped->GetSubRoomUID();
     return _locffviafm.at(subroomUID)->getCostToDestination(UID, ped->GetPos());
}

void DirectionSubLocalFloorfield::Init(Building* building) {
     _building = building;

     _stepsize = building->GetConfig()->get_deltaH();
     _wallAvoidDistance = building->GetConfig()->get_wall_avoid_distance();
     _useDistancefield = building->GetConfig()->get_use_wall_avoidance();

     std::chrono::time_point<std::chrono::system_clock> start, end;
     start = std::chrono::system_clock::now();
     Log->Write("INFO: \tCalling Construtor of UnivFFviaFM(Subroom-scale)");

     for (auto& roomPair : _building->GetAllRooms()) {
          for (auto& subPair : roomPair.second->GetAllSubRooms()) {
               int subUID = subPair.second->GetUID();
               UnivFFviaFM* floorfield = new UnivFFviaFM(subPair.second.get(), _building, _stepsize, _wallAvoidDistance, _useDistancefield);
               _locffviafm[subUID] = floorfield;
               floorfield->setUser(DISTANCE_AND_DIRECTIONS_USED);
               floorfield->setMode(LINESEGMENT);
               if (_useDistancefield) {
                    floorfield->setSpeedMode(FF_WALL_AVOID);
               } else {
                    floorfield->setSpeedMode(FF_HOMO_SPEED);
               }
               floorfield->addAllTargetsParallel();
          }


     }

     if (_building->GetConfig()->get_write_VTK_files_direction()) {
          for (unsigned int i = 0; i < _locffviafm.size(); ++i) {
               auto iter = _locffviafm.begin();
               std::advance(iter, i);
               int roomNr = iter->first;
               iter->second->writeFF("direction" + std::to_string(roomNr) + ".vtk", iter->second->getKnownDoorUIDs());
          }
     }


     end = std::chrono::system_clock::now();
     std::chrono::duration<double> elapsed_seconds = end-start;
     Log->Write("INFO: \tTaken time: " + std::to_string(elapsed_seconds.count()));

     _initDone = true;

     //_locffviafm[0]->writeFF()
     //write floorfields to file, one file per subroom //ar.graf: [SWITCH writevtk ON/OFF]
//     for(unsigned int i = 0; i < subUIDs.size(); ++i) {
//          std::vector<int> targets = {};
//          targets.clear();
//          int subroomUID = subUIDs[i];
//          //if (subroomUID != 26) continue;
//
//          for (auto pair : subAndTarget) {
//               if (pair.first == subroomUID) {
//                    targets.emplace_back(pair.second);
//               }
//          }
//          std::string filename1 = "floorfield" + std::to_string(subroomUID) + ".vtk";
//          if (targets.size() > 0)
//               _locffviafm[subroomUID]->writeFF(filename1, targets);
//     }
}

DirectionSubLocalFloorfield::DirectionSubLocalFloorfield() {
     _initDone = false;
}

DirectionSubLocalFloorfield::~DirectionSubLocalFloorfield() {
     for (auto pair : _locffviafm) {
          delete pair.second;
     }
}
