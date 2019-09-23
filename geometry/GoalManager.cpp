/**
 * \file        GoalManager.cpp
 * \date        Feb 17, 2019
 * \version     v0.8
 * \copyright   <2016-2022> Forschungszentrum Jülich GmbH. All rights reserved.
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
 * Class managing pedestrians who enter/leave waiting areas
 */
#include "GoalManager.h"

#include "WaitingArea.h"

#include "pedestrian/Pedestrian.h"

void GoalManager::SetGoals(const std::map<int, Goal*>& goals)
{
     _allGoals = goals;
}

void GoalManager::SetBuilding(Building* building)
{
     _building = building;
}


void GoalManager::ProcessPedPosition(Pedestrian* ped)
{
     // Ped is in current waiting area
     if (CheckInsideWaitingArea(ped, ped->GetFinalDestination())){
          WaitingArea* wa = dynamic_cast<WaitingArea*>(_allGoals[ped->GetFinalDestination()]);
          wa->AddPed(ped->GetID());
          ped->EnterGoal();
          if (!wa->IsOpen()) {
               SetState(wa->GetId(), false);
          }

          double t = Pedestrian::GetGlobalTime();

          if ((wa->IsWaiting(t, ped->GetBuilding())) && (!ped->IsWaiting())){
               ped->StartWaiting();
          }
     }

}

void GoalManager::ProcessWaitingAreas(double time){
     if (_building){
          for (auto goalItr : _allGoals) {
               if (auto wa = dynamic_cast<WaitingArea*>(goalItr.second)) {
                    if (!wa->IsWaiting(time, _building)) {
                         auto pedsInside = wa->GetPedInside();
                         for (auto p : pedsInside) {
                              auto ped = _building->GetPedestrian(p);
                              wa->RemovePed(p);
                              ped->LeaveGoal();
                              if (wa->IsOpen()) {
                                   SetState(wa->GetId(), true);
                              }
                              ped->SetFinalDestination(wa->GetNextGoal());
                         }
                    }
               }
          }
     }
}

bool GoalManager::CheckInside(Pedestrian* ped, int goalID)
{
     if (goalID < 0){
          return false;
     }

     Goal* goal = _allGoals[goalID];

     if (goal!=nullptr){
          return goal->IsInsideGoal(ped->GetPos());
     }
     return false;
}

bool GoalManager::CheckInsideWaitingArea(Pedestrian* ped, int goalID)
{
     if (goalID < 0){
          return false;
     }

     Goal* goal = _allGoals[goalID];

     if (goal!=nullptr){
          if (dynamic_cast<WaitingArea*>(goal)) {
               return goal->IsInsideGoal(ped->GetPos());
          }
     }
     return false;
}

void GoalManager::SetState(int goalID, bool state)
{
     for (auto& goalItr : _allGoals) {
          if (auto goal = dynamic_cast<WaitingArea*>(goalItr.second)) {
               goal->UpdateProbabilities(state, goalID);
          }
     }
}