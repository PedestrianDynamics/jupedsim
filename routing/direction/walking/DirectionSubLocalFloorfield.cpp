//
// Created by Tobias Schrödter on 2019-04-14.
//

#include "DirectionSubLocalFloorfield.h"

#include "../../../geometry/Line.h"
#include "../../../geometry/NavLine.h"
#include "../../../geometry/Building.h"
#include "../../../pedestrian/Pedestrian.h"
#include "../../../geometry/SubRoom.h"

#include "../../router/ff_router/UnivFFviaFM.h"
#include "../../router/ff_router/FloorfieldViaFM.h"
#include "../../router/ff_router/ffRouter.h"

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

void DirectionSubLocalFloorfield::Init(Building* buildingArg, double stepsize,
          double threshold, bool useDistanceMap) {
     _stepsize = stepsize;
     _building = buildingArg;
     _wallAvoidDistance = threshold;
     _useDistancefield = useDistanceMap;

     std::chrono::time_point<std::chrono::system_clock> start, end;
     start = std::chrono::system_clock::now();
     Log->Write("INFO: \tCalling Construtor of UnivFFviaFM(Subroom-scale)");

     for (auto& roomPair : _building->GetAllRooms()) {
          for (auto& subPair : roomPair.second->GetAllSubRooms()) {
               int subUID = subPair.second->GetUID();
               UnivFFviaFM* floorfield = new UnivFFviaFM(subPair.second.get(), _building, stepsize, _wallAvoidDistance, _useDistancefield);
               _locffviafm[subUID] = floorfield;
               floorfield->setUser(DISTANCE_AND_DIRECTIONS_USED);
               floorfield->setMode(LINESEGMENT);
               if (useDistanceMap) {
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
