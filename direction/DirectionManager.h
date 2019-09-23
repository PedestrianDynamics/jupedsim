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

#include "general/Macros.h"
#include <memory>

class WaitingStrategy;
class DirectionStrategy;
class Point;
class Pedestrian;
class Building;
class Room;

class DirectionManager {

private:
     std::shared_ptr<WaitingStrategy> _waitingStrategy;
     std::shared_ptr<DirectionStrategy> _directionStrategy;

public:
    /**
     * Inits the used strategies.
     */
    void Init(Building*);

    /**
     * Get the desired direction of the pedestrians at the current time step.
     * @param room the room ped is in
     * @param ped pedestrian whose desired direction is computed
     * @return desired direction of ped
     */
    Point GetTarget(Room* room, Pedestrian* ped);

    /**
     * Getter for the waiting strategy.
     * @return the waiting strategy used in the simulation
     */
    const std::shared_ptr<WaitingStrategy>& GetWaitingStrategy() const;

    /**
     * Setter for the waiting strategy.
     * @param waitingStrategy the waiting strategy which is used in the simulation
     */
    void SetWaitingStrategy(const std::shared_ptr<WaitingStrategy>& waitingStrategy);

    /**
     * Getter for the direction/walking strategy.
     * @return the direction/walking strategy used in the simulation
     */
    const std::shared_ptr<DirectionStrategy>& GetDirectionStrategy() const;

    /**
     * Setter for the direction/walking strategy.
     * @param directionStrategy the direction/walking strategy which is used in the simulation
     */
    void SetDirectionStrategy(const std::shared_ptr<DirectionStrategy>& directionStrategy);
};