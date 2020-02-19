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

std::optional<bool> SimulationHelper::RelocatePedestrian(Building & building, Pedestrian & ped)
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
        [ped](auto const & iterator) -> bool {
            return iterator.second->IsInSubRoom(ped.GetPos());
        });

    // pedestrian is in one of the neighboring rooms/subrooms
    if(currentSubRoom != subroomsConnected.end()) {
        ped.SetRoomID(currentSubRoom->second->GetRoomID(), "");
        ped.SetSubRoomID(currentSubRoom->second->GetSubRoomID());
        return true;
    } else {
        return std::nullopt;
    }
}
