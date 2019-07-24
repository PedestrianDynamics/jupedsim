//
// Created by Tobias Schrödter on 2019-04-14.
//

#include "DirectionFloorfield.h"

#include "../../../geometry/Line.h"
#include "../../../geometry/NavLine.h"
#include "../../../geometry/Building.h"
#include "../../../pedestrian/Pedestrian.h"
#include "../../../geometry/SubRoom.h"

#include "../../router/ff_router/UnivFFviaFM.h"
#include "../../router/ff_router/FloorfieldViaFM.h"
#include "../../router/ff_router/ffRouter.h"

#include <chrono>

/// 6
Point DirectionFloorfield::GetTarget(Room* room, Pedestrian* ped) const
{
     UNUSED(room);
#if DEBUG
     if (initDone && (ffviafm != nullptr)) {
#endif // DEBUG

     Point p;
     _ffviafm->getDirectionToDestination(ped, p);
     p = p.Normalized();     // @todo: argraf : scale with costvalue: " * ffviafm->getCostToTransition(ped->GetTransitionID(), ped->GetPos()) "
     return (p + ped->GetPos());

#if DEBUG
     }
#endif // DEBUG

     //this should not execute:
     std::cerr << "Failure in DirectionFloorfield::GetTarget!!" << std::endl;
     exit(EXIT_FAILURE);
}

Point DirectionFloorfield::GetDir2Wall(Pedestrian* ped) const
{
     Point p;
     _ffviafm->getDir2WallAt(ped->GetPos(), p);
     return p;
}

double DirectionFloorfield::GetDistance2Wall(Pedestrian* ped) const
{
     return _ffviafm->getDistance2WallAt(ped->GetPos());
}

void DirectionFloorfield::Init(Building* building) {

     double stepsize = building->GetConfig()->get_deltaH();
     double threshold = building->GetConfig()->get_wall_avoid_distance();
     bool useDistancMap = building->GetConfig()->get_use_wall_avoidance();

     std::chrono::time_point<std::chrono::system_clock> start, end;
     start = std::chrono::system_clock::now();
     Log->Write("INFO: \tCalling Construtor of FloorfieldViaFM");
     _ffviafm = new FloorfieldViaFM(building, stepsize, stepsize, threshold,
               useDistancMap, false);
     end = std::chrono::system_clock::now();
     std::chrono::duration<double> elapsed_seconds = end-start;
     Log->Write("INFO: \tTaken time: " + std::to_string(elapsed_seconds.count()));

     _initDone = true;
}

DirectionFloorfield::DirectionFloorfield() {
     _ffviafm = nullptr;
     _initDone = false;
};

DirectionFloorfield::~DirectionFloorfield() {
     if (_ffviafm) {
          delete _ffviafm;
     }
}
