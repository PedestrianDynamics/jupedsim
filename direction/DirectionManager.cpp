//
// Created by Tobias Schrödter on 2019-05-13.
//

#include "DirectionManager.h"
#include "direction/waiting/WaitingStrategy.h"
#include "direction/walking/DirectionStrategy.h"
#include "geometry/Point.h"
#include "geometry/Building.h"
#include "pedestrian/Pedestrian.h"

void DirectionManager::Init(Building* building)
{
     std::cout << "DirectionManager::Init(Building*)" << std::endl;

     directionStrategy->Init(building);
     waitingStrategy->Init(building);
}

Point DirectionManager::GetTarget(Room* room, Pedestrian* ped)
{
     if (ped->IsWaiting()){
          return waitingStrategy->GetTarget(room, ped);
     }else{
          return directionStrategy->GetTarget(room, ped);
     }
}

const std::shared_ptr<WaitingStrategy>& DirectionManager::GetWaitingStrategy() const
{
     return waitingStrategy;
}

void DirectionManager::SetWaitingStrategy(const std::shared_ptr<WaitingStrategy>& waitingStrategy)
{
     DirectionManager::waitingStrategy = waitingStrategy;
}

const std::shared_ptr<DirectionStrategy>& DirectionManager::GetDirectionStrategy() const
{
     return directionStrategy;
}

void DirectionManager::SetDirectionStrategy(const std::shared_ptr<DirectionStrategy>& directionStrategy)
{
     DirectionManager::directionStrategy = directionStrategy;
}
