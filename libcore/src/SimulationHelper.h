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
#include "pedestrian/Pedestrian.h"

#include <optional>
#include <vector>

enum class PedRelocation { SUCCESSFUL, NOT_NEEDED, FAILED };

namespace SimulationHelper
{
/**
 * Checks whether the pedestrian \p ped is still in the assigned room. If not check if \p ped is
 * in an neighbouring room/subroom and update the information. If \p ped has moved to a
 * non-neighbouring room or to the outside, the information will be set to roomID = -1, and
 * subroomID = -1.
 * @param building geometry used in the simulation
 * @param ped Pedestrian, whose position is checked and information may be updated
 * @return PedRelocation::SUCCESSFUL if \p ped moved to a neighbouring room and the information was
 * updated accordingly, PedRelocation::NOT_NEEDED if the \p ped has not changed the room or subroom,
 * PEDRELOCATION::FAILED if \p moved to the outside or any room that is no neighbour of the current
 * one.
 */
PedRelocation UpdatePedestrianRoomInformation(const Building & building, Pedestrian & ped);

/**
 * Checks for pedestrians who have reached their final goal
 * @param building geometry used in the simulation
 * @param peds list of pedestrians to check
 * @return All pedestrians who have reached their final goal in the geometry
 * @deprecated
 * TODO discuss if needed, as we do not allow inside goals at the moment
 */
std::vector<Pedestrian *>
FindPedestriansReachedFinalGoal(const Building & building, const std::vector<Pedestrian *> & peds);

/**
 * Find pedestrians who have moved to outside of the geometry, and are no longer in the simulation
 * scope.
 * @pre For each ped in peds at least once UpdateRoom(-1,-1) is called, should be done by
 * SimulationHelper::UpdatePedestriansLocations. Meaning that the pedestrian could not be located to one of
 * the neighboring rooms.
 * @param building geometry used in the simulation
 * @param peds[in,out] in: list of all pedestrians that could not be relocated, out: list of
 *  pedestrians which moved in an unusual manner
 * @return list of pedestrians who have moved to outside of the geometry
 */
std::vector<Pedestrian *>
FindPedestriansOutside(const Building & building, std::vector<Pedestrian *> & peds);

/**
 * Updates the locations (room information) of the pedestrians.
 * @param building geometry used in the simulation
 * @param peds list of pedestrians
 * @return [list of pedestrian who have changed their room,
 * list of pedestrians who moved out of their assigned room but to none of the neighboring rooms]
 */
std::tuple<std::vector<Pedestrian *>, std::vector<Pedestrian *>>
UpdatePedestriansLocations(const Building & building, const std::vector<Pedestrian *> & peds);

/**
 * Increments the door usage of the doors by the peds in \p pedsChangedRoom.
 * @param building geometry used in the simulation
 * @param pedsChangedRoom list of pedestrians who have changed their room
 */
void UpdateFlowAtDoors(Building & building, const std::vector<Pedestrian *> & pedsChangedRoom);

/**
 * Triggers the flow regulation, and closes/opens doors accordingly
 * @param building geometry used in the simulation
 * @return a change to the geometry was made
 */
bool UpdateFlowRegulation(Building & building);

/**
 * Finds the transition that was passed by a pedestrian \p ped in the last time step.
 *
 * @pre ped._lastPosition is set, hence at least one time step was triggered before
 * @param building geometry used in the simulation
 * @param ped pedestrian to find the transition
 * @return transition passed by \p in the last time step, nullopt if no transition could be found
 */
std::optional<Transition *> FindPassedDoor(const Building & building, const Pedestrian & ped);

/**
 * Removes the pedestrians \p pedsFaulty from the simulation, e.g., the building. Additionally
 * prints an error message to the log, containing the pedestrians ID and a \p message.
 * @post pedsFaulty will be cleared
 * @param building geometry used in the simulation
 * @param pedsFaulty list of faulty pedestrians, which should be deleted
 * @param message error message to be displayed to add additional information to the user
 */
void RemoveFaultyPedestrians(
    Building & building,
    std::vector<Pedestrian *> & pedsFaulty,
    std::string message);

/**
 * Removes the pedestrians \p pedsFaulty from the simulation, e.g., the building.
 * @param building geometry used in the simulation
 * @param peds list of faulty pedestrians, which should be deleted
 */
void RemovePedestrians(Building & building, std::vector<Pedestrian *> & peds);
} //namespace SimulationHelper
