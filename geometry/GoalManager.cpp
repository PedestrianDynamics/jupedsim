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
//     Goal* goal = _allGoals[ped->GetFinalDestination()];
//
//     if (goal!=nullptr){
//          if (WaitingArea* wa = dynamic_cast<WaitingArea*>(goal)) {
//               bool pedInside = wa->IsInsideGoal(ped->GetPos());
//
//               if (pedInside) {
//                    wa->addPed(ped->GetID());
//
//                    if (!wa->isOpen()) {
//                         for (auto& goalItr : _allGoals) {
//                              if (WaitingArea* goal = dynamic_cast<WaitingArea*>(goalItr.second)) {
//                                   goal->updateProbabilities(false, wa->GetId());
//                              }
//                         }
//                    }
//
//               }
//               else {
//
//
//                    // if just has left goal
//                    //   decrease numPed in previous goal
//                    //   if numPed <= maxPed
//                    //        open Goal
//                    //        update for goal where next goal has goal id
//               }
//          }
//     }

     // Ped is in current waiting area
     if (CheckInsideWaitingArea(ped, ped->GetFinalDestination())){
          WaitingArea* wa = dynamic_cast<WaitingArea*>(_allGoals[ped->GetFinalDestination()]);
          wa->addPed(ped->GetID());
          ped->EnterGoal();
          if (!wa->isOpen()) {
               SetState(wa->GetId(), false);
          }
     }

     if ((ped->GetLastGoalID() != -1 ) && (ped->GetFinalDestination() != ped->GetLastGoalID())){
          if (!CheckInsideWaitingArea(ped, ped->GetLastGoalID())){
               WaitingArea* wa = dynamic_cast<WaitingArea*>(_allGoals[ped->GetLastGoalID()]);

               if (wa != nullptr){
                    wa->removePed(ped->GetID());
                    ped->LeaveGoal();
                    if (wa->isOpen()) {
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
          if (WaitingArea* wa = dynamic_cast<WaitingArea*>(goal)) {
               return goal->IsInsideGoal(ped->GetPos());
          }
     }
     return false;

}

void GoalManager::SetState(int goalID, bool state)
{
     for (auto& goalItr : _allGoals) {
          if (WaitingArea* goal = dynamic_cast<WaitingArea*>(goalItr.second)) {
               goal->updateProbabilities(state, goalID);
          }
     }

}