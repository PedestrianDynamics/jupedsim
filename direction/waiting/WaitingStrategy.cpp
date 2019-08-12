//
// Created by Tobias Schrödter on 2019-05-13.
//

#include "WaitingStrategy.h"
#include "geometry/Room.h"
#include "geometry/SubRoom.h"
#include "pedestrian/Pedestrian.h"
#include "geometry/Point.h"

Point WaitingStrategy::GetTarget(Room* room, Pedestrian* ped)
{
     Point waitingPos = ped->GetWaitingPos();
     Point target;

     // check if waiting pos is set
     if (waitingPos._x == std::numeric_limits<double>::max() && waitingPos._y == std::numeric_limits<double>::max()){
          do {
               target = GetWaitingPosition(room, ped);
          }while(!ped->GetBuilding()->GetSubRoomByUID(ped->GetSubRoomUID())->IsInSubRoom(target));

          ped->SetWaitingPos(target);
     }
     // check if in close range to desired position, hard coded!
//     else  if ((ped->GetWaitingPos()-ped->GetPos()).Norm() <= 0.5){
     else  if ((ped->GetWaitingPos()-ped->GetPos()).Norm() <= 0.1 && ped->GetV0Norm() < 0.5){
          target = ped->GetPos();
          ped->SetWaitingPos(target);
     }
     // head to desired waiting position
     else {
          target = GetPath(ped);
//          target = waitingPos;
     }

     return target;
}

Point WaitingStrategy::GetPath(Pedestrian* ped){
     return ped->GetWaitingPos();
}