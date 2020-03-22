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
#include "SimulationHelper.h"

#include "geometry/Room.h"
#include "geometry/SubRoom.h"

#include <algorithm>

std::optional<bool> SimulationHelper::UpdateRoom(Building & building, Pedestrian & ped)
{
    // No relocation needed, ped is in its assigned room/subroom
    // TODO add check if room/subroom really exist
    auto oldSubRoom = building.GetRoom(ped.GetRoomID())->GetSubRoom(ped.GetSubRoomID());
    if(oldSubRoom->IsInSubRoom(&ped)) {
        return false;
    }

    // relocation needed, look if pedestrian is in one of the neighboring rooms/subrooms
    // get all connected directly connected rooms/subrooms
    // TODO check for closed doors?
    std::map<int, std::shared_ptr<SubRoom>> subroomsConnected;
    for(const auto & [roomID, room] : building.GetAllRooms()) {
        std::copy_if(
            std::begin(room->GetAllSubRooms()),
            std::end(room->GetAllSubRooms()),
            std::inserter(subroomsConnected, subroomsConnected.end()),
            [oldSubRoom](auto const & iterator) -> bool {
                return iterator.second->IsDirectlyConnectedWith(oldSubRoom);
            });
    }

    // check if pedestrian is in one of the neighboring rooms/subrooms
    auto currentSubRoom = std::find_if(
        std::begin(subroomsConnected),
        std::end(subroomsConnected),
        [&ped](auto const & iterator) -> bool {
            return iterator.second->IsInSubRoom(ped.GetPos());
        });

    // pedestrian is in one of the neighboring rooms/subrooms
    if(currentSubRoom != subroomsConnected.end()) {
        ped.UpdateRoom(currentSubRoom->second->GetRoomID(), currentSubRoom->second->GetSubRoomID());
        return true;
    } else {
        ped.UpdateRoom(-1, -1);
        return std::nullopt;
    }
}

std::vector<Pedestrian *> SimulationHelper::FindPedsReachedFinalGoal(
    Building & building,
    const std::vector<Pedestrian *> & peds)
{
    std::vector<Pedestrian *> pedsAtFinalGoal;
    const auto & goals = building.GetAllGoals();

    std::copy_if(
        std::begin(peds),
        std::end(peds),
        std::inserter(pedsAtFinalGoal, std::end(pedsAtFinalGoal)),
        [goals](auto const ped) -> bool {
            return ped->GetFinalDestination() != FINAL_DEST_OUT &&
                   goals.at(ped->GetFinalDestination())->Contains(ped->GetPos()) &&
                   goals.at(ped->GetFinalDestination())->GetIsFinalGoal();
        });
    return pedsAtFinalGoal;
}

std::tuple<std::vector<Pedestrian *>, std::vector<Pedestrian *>>
SimulationHelper::UpdateLocations(Building & building, const std::vector<Pedestrian *> & peds)
{
    std::vector<Pedestrian *> pedsNotRelocated;
    std::vector<Pedestrian *> pedsChangedRoom;

    // Check for peds, where relocation failed
    for(auto const ped : peds) {
        if(auto relocated = UpdateRoom(building, *ped); relocated.has_value()) {
            if(relocated && ped->ChangedRoom()) {
                pedsChangedRoom.push_back(ped);
            }
        } else {
            pedsNotRelocated.push_back(ped);
        }
    }

    return {pedsChangedRoom, pedsNotRelocated};
}

std::vector<Pedestrian *>
SimulationHelper::FindOutsidePedestrians(Building & building, std::vector<Pedestrian *> & peds)
{
    std::vector<Pedestrian *> pedsOutside;
    std::vector<Pedestrian *> newPeds;

    std::partition_copy(
        std::make_move_iterator(std::begin(peds)),
        std::make_move_iterator(std::end(peds)),
        std::back_inserter(pedsOutside),
        std::back_inserter(newPeds),
        [&building](auto const ped) -> bool {
            if(building.GetAllRooms().find(ped->GetOldRoomID()) != building.GetAllRooms().end()) {
                for(auto [transID, trans] : building.GetAllTransitions()) {
                    if(trans->GetRoom1()->GetID() == ped->GetOldRoomID() ||
                       trans->GetRoom2()->GetID() == ped->GetOldRoomID()) {
                        if(trans->IsOpen() && trans->IsExit()) {
                            auto distance = trans->DistTo(ped->GetPos());
                            if(distance < 0.5) {
                                Line step{ped->GetLastPosition(), ped->GetPos()};
                                auto intersect = trans->IntersectionWith(step);
                                if(intersect == 1) {
                                    return true;
                                }
                            }
                        }
                    }
                }
            }
            return false;
        });
    peds = std::move(newPeds);
    return pedsOutside;
}

