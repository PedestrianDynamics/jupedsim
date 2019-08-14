/**
 * \file        WaitingStrategy.h
 * \date        May 13, 2019
 * \version     v0.8.1
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
 *
 * \section Description
 * Interface of a waiting strategy:
 *  - A desired walking direction at a certain time is computed
 *  - A desired waiting position is computed
 **/
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
     else  if ((ped->GetWaitingPos()-ped->GetPos()).Norm() <= 0.1 && ped->GetV0Norm() < 0.5){
          target = ped->GetPos();
          ped->SetWaitingPos(target);
     }
     // head to desired waiting position
     else {
          target = GetPath(ped);
     }

     return target;
}

Point WaitingStrategy::GetPath(Pedestrian* ped){
     return ped->GetWaitingPos();
}