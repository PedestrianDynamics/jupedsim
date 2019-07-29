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

void GoalManager::SetBuilding(Building* building)
{
     _building = building;
}


void GoalManager::ProcessPedPosition(Pedestrian* ped)
{
//     std::cout << "----------" << std::endl;
//     std::cout << "Ped: " << ped->GetPos().toString() << std::endl;
//     std::cout << "Exit: " << ped->GetFinalDestination() << std::endl;
//     std::cout << "WA: " << _building->GetFinalGoal(ped->GetFinalDestination())->GetCentreCrossing()->toString() << std::endl;

     // Ped is in current waiting area
     if (CheckInsideWaitingArea(ped, ped->GetFinalDestination())){
          WaitingArea* wa = dynamic_cast<WaitingArea*>(_allGoals[ped->GetFinalDestination()]);
          wa->addPed(ped->GetID());
          ped->EnterGoal();
          if (!wa->isOpen()) {
               SetState(wa->GetId(), false);
          }

          double t = Pedestrian::GetGlobalTime();

          if ((wa->isWaiting(t, ped->GetBuilding())) && (!ped->IsWaiting())){
//               std::cout << t << ": " << ped->GetID() << " " << ped->GetRoomID() << " " << ped->GetSubRoomID() << std::endl;
               ped->StartWaiting();
          }
     }
}

void GoalManager::ProcessWaitingAreas(double time){
    for (auto goalItr : _allGoals){
         if (WaitingArea* wa = dynamic_cast<WaitingArea*>(goalItr.second)){
              if (!wa->isWaiting(time, _building)){
                   for (auto p : wa->GetPedInside()){
                        auto ped = _building->GetPedestrian(p);
                        wa->removePed(p);
                        ped->LeaveGoal();
                        if (wa->isOpen()) {
                             SetState(wa->GetId(), true);
                        }
                        ped->SetFinalDestination(wa->GetNextGoal());
                   }
              }
         }
    }
}

bool GoalManager::CheckInside(Pedestrian* ped, int goalID)
{
     Goal* goal = _allGoals[goalID];

     if (goal!=nullptr){
          return goal->IsInsideGoal(ped);
     }
     return false;

}

bool GoalManager::CheckInsideWaitingArea(Pedestrian* ped, int goalID)
{
     Goal* goal = _allGoals[goalID];

     if (goal!=nullptr){
          if (dynamic_cast<WaitingArea*>(goal)) {
               return goal->IsInsideGoal(ped);
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