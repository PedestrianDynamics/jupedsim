//
// Created by Tobias Schrödter on 2019-05-14.
//

#include "WaitingRandom.h"
#include "geometry/Room.h"
#include "geometry/SubRoom.h"
#include "pedestrian/Pedestrian.h"
#include "geometry/Point.h"

double fRand(double fMin, double fMax)
{
     double f = static_cast<double>(std::rand()) / RAND_MAX;
     return fMin + f * (fMax - fMin);
}


Point WaitingRandom::GetWaitingPosition(Room* room, Pedestrian* ped){

     std::vector<Point> polygon;

     if (ped->IsInsideWaitingAreaWaiting()){
          polygon = ped->GetBuilding()->GetFinalGoal(ped->GetLastGoalID())->GetPolygon();
     } else {
          SubRoom* subRoom = room->GetSubRoom(ped->GetSubRoomID());

          polygon = subRoom->GetPolygon();
     }

     double xMin = std::numeric_limits<double>::max(),
            xMax =std::numeric_limits<double>::min(),
            yMin = std::numeric_limits<double>::max(),
            yMax = std::numeric_limits<double>::min();

     for (const auto & poly : polygon){
          xMin = (xMin <= poly._x)?(xMin):(poly._x);
          xMax = (xMax >= poly._x)?(xMax):(poly._x);

          yMin = (yMin <= poly._y)?(yMin):(poly._y);
          yMax = (yMax >= poly._y)?(yMax):(poly._y);
     }

     Point target;

     if (ped->IsInsideWaitingAreaWaiting()){
          auto goal = ped->GetBuilding()->GetFinalGoal(ped->GetLastGoalID());

          do{
               target._x = fRand(xMin, xMax);
               target._y = fRand(yMin, yMax);
          }while (!goal->IsInsideGoal(target));

     } else {
          SubRoom* subRoom = room->GetSubRoom(ped->GetSubRoomID());

          do{
               target._x = fRand(xMin, xMax);
               target._y = fRand(yMin, yMax);
          }while (!subRoom->IsInSubRoom(target));
     }


     std::cout << "Ped " << ped->GetID() << " Target: " << target.toString() << std::endl;

     return target;
}

