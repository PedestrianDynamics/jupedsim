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
 * Class mananging pedestrians who enter/leave waiting areas
 */

#include "GoalManager.h"

#include "../pedestrian/Pedestrian.h"
#include "WaitingArea.h"

void GoalManager::SetGoals(std::map<int, Goal*> goals)
{
     _allGoals = goals;
}

void GoalManager::ProcessPedPosition(Pedestrian* ped)
{
     // Ped is in current waiting area
     if (CheckInsideWaitingArea(ped, ped->GetFinalDestination())){
          WaitingArea* wa = dynamic_cast<WaitingArea*>(_allGoals[ped->GetFinalDestination()]);
          // Add ped to waiting area
          wa->AddPed(ped->GetID());
          // Ped enters waiting area
          ped->EnterGoal();

          // Check state of waiting area
          if (!wa->IsOpen()) {
               SetState(wa->GetId(), false);
          }
     }

     // Ped already had a goal AND has not reached final goal
     if ((ped->GetLastGoalID() != -1 ) && (ped->GetFinalDestination() != ped->GetLastGoalID())){
          // Ped is not in waiting area which was the last goal
          if (!CheckInsideWaitingArea(ped, ped->GetLastGoalID())){
               WaitingArea* wa = dynamic_cast<WaitingArea*>(_allGoals[ped->GetLastGoalID()]);

               if (wa != nullptr){
                    // Remove ped from waiting area
                    wa->RemovePed(ped->GetID());
                    // ped leaves waiting area
                    ped->LeaveGoal();

                    // Check state of waiting area
                    if (wa->IsOpen()) {
                         SetState(wa->GetId(), true);
                    }
               }
          }
     }
}

bool GoalManager::CheckInside(Pedestrian* ped, int goalID)
{
     Goal* goal = _allGoals[goalID];

     if (goal!=nullptr){
          return goal->IsInsideGoal(ped->GetPos());
     }
     return false;
}

bool GoalManager::CheckInsideWaitingArea(Pedestrian* ped, int goalID)
{
     Goal* goal = _allGoals[goalID];

     if (goal!=nullptr){
          if (auto wa = dynamic_cast<WaitingArea*>(goal)) {
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