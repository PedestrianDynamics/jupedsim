//
// Created by Tobias Schrödter on 2019-05-13.
//

#include "WaitingStrategy.h"
#include "../../../geometry/Room.h"
#include "../../../geometry/SubRoom.h"
#include "../../../pedestrian/Pedestrian.h"
#include "../../../geometry/Point.h"

Point WaitingStrategy::GetTarget(Room* room, Pedestrian* ped)
{
     Point waitingPos = ped->GetWaitingPos();
     Point target;

     // check if waiting pos is set
     if (waitingPos._x == std::numeric_limits<double>::max() && waitingPos._y == std::numeric_limits<double>::max()){
          target = GetWaitingPosition(room, ped);
          ped->SetWaitingPos(target);
     }
     // check if in close range to desired position, hard coded!
     else  if ((ped->GetWaitingPos()-ped->GetPos()).Norm() <= 0.25){
          target = ped->GetPos();
     }
     // head to desired waiting position
     else {
          target = waitingPos;
     }

     return target;

}
