/**
 * \file        DirectionManager.cpp
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
 **/

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
