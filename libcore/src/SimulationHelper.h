/**
 * Copyright (c) 2020 Forschungszentrum Jülich GmbH. All rights reserved.
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

#include "geometry/Building.h"
#include "geometry/Transition.h"
#include "pedestrian/Pedestrian.h"

#include <memory>
#include <optional>
#include <vector>


namespace SimulationHelper
{
/**
 * Find pedestrians who have moved to outside of the geometry, and are no longer in the simulation
 * scope.
 * @pre For each ped in peds at least once UpdateRoom(-1,-1) is called, should be done by
 * SimulationHelper::UpdatePedestriansLocations. Meaning that the pedestrian could not be located to one of
 * the neighboring rooms.
 * @param building geometry used in the simulation
 * @param peds list of pedestrians to check
 * @return list of pedestrians who have moved to outside of the geometry
 */
std::vector<Pedestrian *> FindPedestriansOutside(
    const Building & building,
    const std::vector<std::unique_ptr<Pedestrian>> & peds);


/**
 * Increments the door usage of the doors by the peds in \p pedsChangedRoom.
 * @param building geometry used in the simulation
 * @param pedsChangedRoom list of pedestrians who have changed their room
 * @param time elapsed of the simulation
 */
void UpdateFlowAtDoors(
    Building & building,
    const std::vector<std::unique_ptr<Pedestrian>> & peds,
    double time);

/**
 * Triggers the flow regulation, and closes/opens doors accordingly
 * @param building geometry used in the simulation
 * @return a change to the geometry was made
 */
bool UpdateFlowRegulation(Building & building, double time);

/**
 * Triggers the flow regulation for trains, and closes/opens doors accordingly
 * @param building geometry used in the simulation
 * @return a change to the geometry was made
 */
bool UpdateTrainFlowRegulation(Building & building, double time);

/**
 * Finds the transition that was passed by a pedestrian \p ped in the last time step.
 *
 * @pre ped._lastPosition is set, hence at least one time step was triggered before
 * @param ped pedestrian to find the transition
 * @return transition passed by \p in the last time step, nullopt if no transition could be found
 */
std::optional<Transition *>
FindPassedDoor(const Pedestrian & ped, const std::vector<Transition *> & transitions);


/**
 * Removes the pedestrians \p pedsFaulty from the simulation, e.g., the building. Additionally
 * prints an error message to the log, containing the pedestrians ID and a \p message.
 * @post pedsFaulty will be cleared
 * @param building geometry used in the simulation
 * @param pedsFaulty list of faulty pedestrians, which should be deleted
 * @param message error message to be displayed to add additional information to the user
 */
void RemoveFaultyPedestrians(
    Simulation & simulation,
    std::vector<Pedestrian *> & pedsFaulty,
    std::string message);

/**
 * Removes the pedestrians \p pedsFaulty from the simulation, e.g., the building.
 * @param building geometry used in the simulation
 * @param peds list of faulty pedestrians, which should be deleted
 */
void RemovePedestrians(Simulation & simulation, std::vector<Pedestrian *> & peds);
} //namespace SimulationHelper
