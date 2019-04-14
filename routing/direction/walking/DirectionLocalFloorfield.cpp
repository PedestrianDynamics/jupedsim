//
// Created by Tobias Schrödter on 2019-04-14.
//

#include "DirectionLocalFloorfield.h"

#include "../../../geometry/Line.h"
#include "../../../geometry/NavLine.h"
#include "../../../geometry/Building.h"
#include "../../../pedestrian/Pedestrian.h"
#include "../../../geometry/SubRoom.h"

#include "../../router/ff_router/UnivFFviaFM.h"
#include "../../router/ff_router/FloorfieldViaFM.h"
#include "../../router/ff_router/ffRouter.h"

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

void DirectionLocalFloorfield::Init(Building* buildingArg, double stepsize,
          double threshold, bool useDistanceMap) {
     _stepsize = stepsize;
     _building = buildingArg;
     _wallAvoidDistance = threshold;
     _useDistancefield = useDistanceMap;


     std::chrono::time_point<std::chrono::system_clock> start, end;
     start = std::chrono::system_clock::now();
     Log->Write("INFO: \tCalling Constructor of UnivFFviaFM(Room-scale) in DirectionLocalFloorfield::Init(...)");

     for (auto& roomPair : _building->GetAllRooms()) {
          UnivFFviaFM* newfield =  new UnivFFviaFM(roomPair.second.get(), _building, stepsize, _wallAvoidDistance, _useDistancefield);
          _locffviafm[roomPair.first] = newfield;
          newfield->setUser(DISTANCE_AND_DIRECTIONS_USED);
          newfield->setMode(LINESEGMENT);
          if (useDistanceMap) {
               newfield->setSpeedMode(FF_WALL_AVOID);
          } else {
               newfield->setSpeedMode(FF_HOMO_SPEED);
          }
          newfield->addAllTargetsParallel();

          //newfield->writeFF("directionsOfRoom" + std::to_string(roomPair.first) + ".vtk", newfield->getKnownDoorUIDs());
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

