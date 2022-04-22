/**
 * \file        DirectionManager.h
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

#pragma once

#include "direction/waiting/WaitingStrategy.hpp"
#include "direction/walking/DirectionStrategy.hpp"
#include "general/Configuration.hpp"
#include "general/Macros.hpp"
#include "pedestrian/Pedestrian.hpp"

#include <memory>

class Building;

class DirectionManager
{
private:
    std::unique_ptr<DirectionStrategy> _directionStrategy;
    std::unique_ptr<WaitingStrategy> _waitingStrategy;
    const Building* _building;
    double _currentTime;

public:
    static std::unique_ptr<DirectionManager>
    Create(const Configuration& config, Building* building);

    DirectionManager(
        std::unique_ptr<DirectionStrategy> directionStrategy,
        std::unique_ptr<WaitingStrategy> waitingStrategy,
        const Building* building);
    ~DirectionManager() = default;

    void Update(double time) { _currentTime = time; };

    /**
     * Get the desired direction of the pedestrians at the current time step.
     * @param room the room ped is in
     * @param ped pedestrian whose desired direction is computed
     * @return desired direction of ped
     */
    Point GetTarget(const Pedestrian* ped);

    /**
     * Getter for the waiting strategy.
     * @return the waiting strategy used in the simulation
     */
    WaitingStrategy& GetWaitingStrategy() const;

    /**
     * Getter for the direction/walking strategy.
     * @return the direction/walking strategy used in the simulation
     */
    DirectionStrategy& GetDirectionStrategy() const;
};
