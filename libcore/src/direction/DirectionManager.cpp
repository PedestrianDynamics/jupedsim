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

#include "DirectionManager.hpp"

#include "direction/waiting/WaitingMiddle.hpp"
#include "direction/waiting/WaitingRandom.hpp"
#include "direction/waiting/WaitingStrategy.hpp"
#include "direction/waiting/WaitingStrategyType.hpp"
#include "direction/walking/DirectionStrategy.hpp"
#include "direction/walking/DirectionStrategyType.hpp"
#include "geometry/Building.hpp"
#include "geometry/Point.hpp"
#include "math/OperationalModel.hpp"
#include "pedestrian/Pedestrian.hpp"

#include <memory>

static std::unique_ptr<DirectionStrategy> make_direction_strategy(
    DirectionStrategyType type,
    const Configuration & config,
    Building * building)
{
    switch(type) {
        case DirectionStrategyType::IN_RANGE_BOTTLENECK:
            return std::make_unique<DirectionInRangeBottleneck>();
        case DirectionStrategyType::LOCAL_FLOORFIELD:
            return std::make_unique<DirectionLocalFloorfield>(config, building);
        case DirectionStrategyType::MIDDLE_POINT:
            return std::make_unique<DirectionMiddlePoint>();
        case DirectionStrategyType::MIN_SEPERATION_SHORTER_LINE:
            return std::make_unique<DirectionMinSeperationShorterLine>();
    }
}

static std::unique_ptr<WaitingStrategy>
make_waiting_strategy(std::optional<WaitingStrategyType> type)
{
    if(type) {
        switch(*type) {
            case WaitingStrategyType::RANDOM:
                return std::make_unique<WaitingRandom>();
            case WaitingStrategyType::MIDDLE:
                return std::make_unique<WaitingMiddle>();
        }
    }
    return nullptr;
}

std::unique_ptr<DirectionManager>
DirectionManager::Create(const Configuration & config, Building * building)
{
    auto directionStrategy =
        make_direction_strategy(config.directionStrategyType, config, building);
    auto waitingStrategy = make_waiting_strategy(config.waitingStrategyType);

    return std::make_unique<DirectionManager>(
        std::move(directionStrategy), std::move(waitingStrategy), building);
}

DirectionManager::DirectionManager(
    std::unique_ptr<DirectionStrategy> directionStrategy,
    std::unique_ptr<WaitingStrategy> waitingStrategy,
    const Building * building) :
    _directionStrategy(std::move(directionStrategy)),
    _waitingStrategy(std::move(waitingStrategy)),
    _building(building)
{
}

Point DirectionManager::GetTarget(const Pedestrian * ped)
{
    const auto * room = _building->GetRoom(ped->GetPos());
    if(ped->IsWaiting() && _waitingStrategy) {
        return _waitingStrategy->GetTarget(room, ped, _currentTime);
    } else {
        return _directionStrategy->GetTarget(room, ped);
    }
}

WaitingStrategy & DirectionManager::GetWaitingStrategy() const
{
    return *_waitingStrategy;
}

DirectionStrategy & DirectionManager::GetDirectionStrategy() const
{
    return *_directionStrategy;
}
