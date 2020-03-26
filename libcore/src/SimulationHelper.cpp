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

#include "general/Logger.h"
#include "geometry/Room.h"
#include "geometry/SubRoom.h"

#include <algorithm>

std::optional<bool> SimulationHelper::UpdateRoom(const Building & building, Pedestrian & ped)
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
        ped.SetSubRoomUID(currentSubRoom->second->GetUID());
        return true;
    } else {
        ped.UpdateRoom(-1, -1);
        ped.SetSubRoomUID(-1);
        return std::nullopt;
    }
}

std::vector<Pedestrian *> SimulationHelper::FindPedsReachedFinalGoal(
    const Building & building,
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
SimulationHelper::UpdateLocations(const Building & building, const std::vector<Pedestrian *> & peds)
{
    std::vector<Pedestrian *> pedsNotRelocated;
    std::vector<Pedestrian *> pedsChangedRoom;

    // Check for peds, where relocation failed
    for(auto const ped : peds) {
        if(auto relocated = UpdateRoom(building, *ped); relocated.has_value()) {
            if(relocated.value() && ped->ChangedRoom()) {
                pedsChangedRoom.push_back(ped);
            }
        } else {
            pedsNotRelocated.push_back(ped);
        }
    }

    return {pedsChangedRoom, pedsNotRelocated};
}

std::vector<Pedestrian *> SimulationHelper::FindOutsidePedestrians(
    const Building & building,
    std::vector<Pedestrian *> & peds)
{
    std::vector<Pedestrian *> pedsOutside;
    std::vector<Pedestrian *> newPeds;

    std::partition_copy(
        std::make_move_iterator(std::begin(peds)),
        std::make_move_iterator(std::end(peds)),
        std::back_inserter(pedsOutside),
        std::back_inserter(newPeds),
        [&building](auto const ped) -> bool {
            auto transPassed = SimulationHelper::FindPassedDoor(building, *ped);
            if(!transPassed.has_value()) {
                return false;
            }

            //TODO maxDistance should depend on vmax
            double maxDistance = 0.5;
            return transPassed.value()->IsExit() && transPassed.value()->IsOpen() &&
                   transPassed.value()->DistTo(ped->GetPos()) < maxDistance;
        });
    peds = std::move(newPeds);
    return pedsOutside;
}

void SimulationHelper::UpdateFlowAtDoors(
    Building & building,
    const std::vector<Pedestrian *> & peds)
{
    for(const auto ped : peds) {
        auto closestTransition = SimulationHelper::FindPassedDoor(building, *ped);

        if(!closestTransition.has_value()) {
            LOG_WARNING("Ped {} did not cross any transition", ped->GetID());
            return;
        }

        //TODO check if exitindex is set and transition exists?
        if(closestTransition.value()->GetUniqueID() !=
           building.GetTransitionByUID(ped->GetExitIndex())->GetUniqueID()) {
            LOG_WARNING(
                "Ped {}: used an indented door {}, but wanted to go to {}.",
                ped->GetID(),
                closestTransition.value()->GetUniqueID(),
                ped->GetExitIndex());
            return;
        }

        closestTransition.value()->IncreaseDoorUsage(1, Pedestrian::GetGlobalTime());
        closestTransition.value()->IncreasePartialDoorUsage(1);
    }
}

std::optional<Transition *>
SimulationHelper::FindPassedDoor(const Building & building, const Pedestrian & ped)
{
    std::vector<Transition *> transitions;

    // TODO check if room exists?
    auto room = (ped.GetRoomID() != -1) ? (building.GetAllRooms().at(ped.GetRoomID())) :
                                          (building.GetAllRooms().at(ped.GetOldRoomID()));

    for(auto const & [_, subroom] : room->GetAllSubRooms()) {
        transitions.insert(
            std::end(transitions),
            std::begin(subroom->GetAllTransitions()),
            std::end(subroom->GetAllTransitions()));
    }

    Line step{ped.GetLastPosition(), ped.GetPos()};
    // TODO check for closed doors and distance?
    auto passedTrans = std::find_if(
        std::begin(transitions), std::end(transitions), [&step](auto const & trans) -> bool {
            return trans->IntersectionWith(step) == 1;
        });

    if(passedTrans == transitions.end()) {
        return std::nullopt;
    } else {
        return *passedTrans;
    }
}

bool SimulationHelper::UpdateFlowRegulation(Building & building)
{
    bool stateChanged = false;

    for(auto [transID, trans] : building.GetAllTransitions()) {
        DoorState state = trans->GetState();

        bool regulateFlow = trans->GetOutflowRate() < (std::numeric_limits<double>::max)() ||
                            trans->GetMaxDoorUsage() < std::numeric_limits<double>::max();

        if(regulateFlow) {
            // when more than <dn> agents pass <trans>, we start evaluating the flow
            // .. and maybe close the <trans>
            // need to be more than <dn> as multiple ped can pass a transition in one time step
            if(trans->GetPartialDoorUsage() >= trans->GetDN()) {
                trans->RegulateFlow(Pedestrian::GetGlobalTime());
                trans->ResetPartialDoorUsage();
            }
        }

        stateChanged = stateChanged || (state != trans->GetState());
    }
    return stateChanged;
}
