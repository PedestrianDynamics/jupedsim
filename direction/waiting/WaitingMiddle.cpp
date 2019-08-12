//
// Created by Tobias Schrödter on 2019-05-13.
//

#include "WaitingMiddle.h"
#include "geometry/Room.h"
#include "geometry/SubRoom.h"
#include "pedestrian/Pedestrian.h"
#include "geometry/Point.h"

Point WaitingMiddle::GetWaitingPosition(Room* room, Pedestrian* ped){
     SubRoom* subRoom = room->GetSubRoom(ped->GetSubRoomID());

     if (ped->IsInsideWaitingAreaWaiting()){
          return ped->GetBuilding()->GetFinalGoal(ped->GetLastGoalID())->GetCentroid();
     } else {
          return subRoom->GetCentroid();
     }
}
