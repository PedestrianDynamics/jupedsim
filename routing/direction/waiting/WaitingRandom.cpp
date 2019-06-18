//
// Created by Tobias Schrödter on 2019-05-14.
//

#include "WaitingRandom.h"
#include "../../../geometry/Room.h"
#include "../../../geometry/SubRoom.h"
#include "../../../pedestrian/Pedestrian.h"
#include "../../../geometry/Point.h"

double fRand(double fMin, double fMax)
{
     double f = (double)rand() / RAND_MAX;
     return fMin + f * (fMax - fMin);
}


Point WaitingRandom::GetWaitingPosition(Room* room, Pedestrian* ped){
//     srand(time(0));

     SubRoom* subRoom = room->GetSubRoom(ped->GetSubRoomID());

     double xMin = std::numeric_limits<double>::max(),
          xMax =std::numeric_limits<double>::min(),
          yMin = std::numeric_limits<double>::max(),
          yMax = std::numeric_limits<double>::min();

     for (auto poly : subRoom->GetPolygon()){
          xMin = (xMin <= poly._x)?(xMin):(poly._x);
          xMax = (xMax >= poly._x)?(xMax):(poly._x);

          yMin = (yMin <= poly._y)?(yMin):(poly._y);
          yMax = (yMax >= poly._y)?(yMax):(poly._y);
     }

     Point target;
     do{
          target._x = fRand(xMin, xMax);
          target._y = fRand(yMin, yMax);
     }while (!subRoom->IsInSubRoom(target));

     std::cout << "Ped " << ped->GetID() << " Target: " << target.toString() << std::endl;

     return target;
}

