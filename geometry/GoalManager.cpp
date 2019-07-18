//
// Created by Tobias Schrödter on 2019-02-17.
//

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
          wa->AddPed(ped->GetID());
          ped->EnterGoal();
          if (!wa->IsOpen()) {
               SetState(wa->GetId(), false);
          }
     }

     if ((ped->GetLastGoalID() != -1 ) && (ped->GetFinalDestination() != ped->GetLastGoalID())){
          if (!CheckInsideWaitingArea(ped, ped->GetLastGoalID())){
               WaitingArea* wa = dynamic_cast<WaitingArea*>(_allGoals[ped->GetLastGoalID()]);

               if (wa != nullptr){
                    wa->RemovePed(ped->GetID());
                    ped->LeaveGoal();
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