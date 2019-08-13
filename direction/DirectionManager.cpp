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

     _directionStrategy->Init(building);
     if (_waitingStrategy){
          _waitingStrategy->Init(building);
     }
}

Point DirectionManager::GetTarget(Room* room, Pedestrian* ped)
{
     if (ped->IsWaiting() && _waitingStrategy){
          return _waitingStrategy->GetTarget(room, ped);
     }else{
          return _directionStrategy->GetTarget(room, ped);
     }
}

const std::shared_ptr<WaitingStrategy>& DirectionManager::GetWaitingStrategy() const
{
     return _waitingStrategy;
}

void DirectionManager::SetWaitingStrategy(const std::shared_ptr<WaitingStrategy>& waitingStrategy)
{
     DirectionManager::_waitingStrategy = waitingStrategy;
}

const std::shared_ptr<DirectionStrategy>& DirectionManager::GetDirectionStrategy() const
{
     return _directionStrategy;
}

void DirectionManager::SetDirectionStrategy(const std::shared_ptr<DirectionStrategy>& directionStrategy)
{
     DirectionManager::_directionStrategy = directionStrategy;
}
