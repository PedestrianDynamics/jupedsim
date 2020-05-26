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

#include <libshared>
#include <algorithm>

PedRelocation
SimulationHelper::UpdatePedestrianRoomInformation(const Building & building, Pedestrian & ped)
{
    // No relocation needed, ped is in its assigned room/subroom
    // TODO add check if room/subroom really exist
    auto oldSubRoom = building.GetRoom(ped.GetRoomID())->GetSubRoom(ped.GetSubRoomID());
    if(oldSubRoom->IsInSubRoom(&ped)) {
        return PedRelocation::NOT_NEEDED;
    }

    // relocation needed, look if pedestrian is in one of the neighboring rooms/subrooms
    // get all connected directly connected rooms/subrooms
    // TODO check for closed doors?
    auto subroomsConnected = oldSubRoom->GetNeighbors();
    auto currentSubRoom    = std::find_if(
        std::begin(subroomsConnected),
        std::end(subroomsConnected),
        [&ped](auto const & subroom) -> bool { return subroom->IsInSubRoom(ped.GetPos()); });

    // pedestrian is in one of the neighboring rooms/subrooms
    if(currentSubRoom != subroomsConnected.end()) {
        ped.UpdateRoom((*currentSubRoom)->GetRoomID(), (*currentSubRoom)->GetSubRoomID());
        ped.SetSubRoomUID((*currentSubRoom)->GetUID());
        return PedRelocation::SUCCESSFUL;
    } else {
        ped.UpdateRoom(-1, -1);
        ped.SetSubRoomUID(-1);
        return PedRelocation::FAILED;
    }
}

std::vector<Pedestrian *> SimulationHelper::FindPedestriansReachedFinalGoal(
    const Building & building,
    const std::vector<Pedestrian *> & peds)
{
    std::vector<Pedestrian *> pedsAtFinalGoal;
    const auto & goals = building.GetAllGoals();

    std::copy_if(
        std::begin(peds),
        std::end(peds),
        std::inserter(pedsAtFinalGoal, std::end(pedsAtFinalGoal)),
        [goals](const Pedestrian * ped) -> bool {
            return ped->GetFinalDestination() != FINAL_DEST_OUT &&
                   goals.at(ped->GetFinalDestination())->Contains(ped->GetPos()) &&
                   goals.at(ped->GetFinalDestination())->GetIsFinalGoal();
        });
    return pedsAtFinalGoal;
}

std::tuple<std::vector<Pedestrian *>, std::vector<Pedestrian *>>
SimulationHelper::UpdatePedestriansLocations(
    const Building & building,
    const std::vector<Pedestrian *> & peds)
{
    std::vector<Pedestrian *> pedsNotRelocated;
    std::vector<Pedestrian *> pedsChangedRoom;

    // Check for peds, where relocation failed
    for(auto const ped : peds) {
        auto relocated = UpdatePedestrianRoomInformation(building, *ped);
        if(relocated == PedRelocation::SUCCESSFUL && ped->ChangedRoom()) {
            pedsChangedRoom.push_back(ped);
        } else if(relocated == PedRelocation::FAILED) {
            pedsNotRelocated.push_back(ped);
        }
    }

    return {pedsChangedRoom, pedsNotRelocated};
}

std::vector<Pedestrian *> SimulationHelper::FindPedestriansOutside(
    const Building & building,
    std::vector<Pedestrian *> & peds)
{
    std::vector<Pedestrian *> pedsOutside;
    std::vector<Pedestrian *> newPeds;
    double maxDistance = 0.5;

    std::partition_copy(
        std::begin(peds),
        std::end(peds),
        std::back_inserter(pedsOutside),
        std::back_inserter(newPeds),
        [&building, maxDistance](const Pedestrian * ped) -> bool {
            auto transPassed = SimulationHelper::FindPassedDoor(building, *ped);
            if(!transPassed.has_value()) {
                return false;
            }

            //TODO maxDistance should depend on vmax
            bool passedExit     = transPassed.value()->IsExit();
            bool passedOpenDoor = transPassed.value()->IsOpen();
            bool doorIsClose    = transPassed.value()->DistTo(ped->GetPos()) < maxDistance;
            return passedExit && passedOpenDoor && doorIsClose;
        });
    peds = std::move(newPeds);
    return pedsOutside;
}

void SimulationHelper::UpdateFlowAtDoors(
    Building & building,
    const std::vector<Pedestrian *> & pedsChangedRoom)
{
    for(const auto ped : pedsChangedRoom) {
        auto closestTransition = SimulationHelper::FindPassedDoor(building, *ped);

        if(!closestTransition.has_value()) {
            LOG_WARNING("Ped {} did not cross any transition", ped->GetID());
            return;
        }

        if(ped->GetExitIndex() >= 0 &&
           (building.GetTransitionByUID(ped->GetExitIndex()) != nullptr) &&
           closestTransition.value()->GetUniqueID() !=
               building.GetTransitionByUID(ped->GetExitIndex())->GetUniqueID()) {
            LOG_WARNING(
                "Ped {}: used an unindented door {}, but wanted to go to {}.",
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

    for(auto const & [subroomID, subroom] : room->GetAllSubRooms()) {
        transitions.insert(
            std::end(transitions),
            std::begin(subroom->GetAllTransitions()),
            std::end(subroom->GetAllTransitions()));
    }

    Line step{ped.GetLastPosition(), ped.GetPos()};
    // TODO check for closed doors and distance?
    auto passedTrans = std::find_if(
        std::begin(transitions), std::end(transitions), [&step](const Transition * trans) -> bool {
            return trans->IntersectionWith(step) == 1;
        });

    if(passedTrans == transitions.end() || (*passedTrans)->IsInLineSegment(ped.GetPos())) {
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
        trans->UpdateTemporaryState(building.GetConfig()->Getdt());

        bool regulateFlow = trans->GetOutflowRate() < std::numeric_limits<double>::max() ||
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

bool SimulationHelper::UpdateTrainFlowRegulation(Building & building)
{
    bool geometryChanged = false;
    for(auto const & [trainID, trainType] : building.GetTrains()) {
        auto trainAddedDoors = building.GetTrainDoorsAdded(trainID);
        if(trainAddedDoors.has_value()) {
            auto trainDoors = trainAddedDoors.value();

            int trainUsage = std::accumulate(
                std::begin(trainDoors),
                std::end(trainDoors),
                0,
                [&building](int i, const Transition & trans) {
                    return building.GetTransition(trans.GetID())->GetDoorUsage() + i;
                });

            int maxAgents = trainType._maxAgents;
            if(trainUsage > maxAgents) {
                std::for_each(
                    std::begin(trainDoors),
                    std::end(trainDoors),
                    [&building, trainUsage, maxAgents](Transition trans) {
                        if(!building.GetTransition(trans.GetID())->IsClose()) {
                            building.GetTransition(trans.GetID())->Close();
                            LOG_INFO(
                                "Closing train door {} with ID {} at t={:.2f}. Door usage = {} "
                                "(Train Capacity {})",
                                trans.GetType(),
                                trans.GetID(),
                                Pedestrian::GetGlobalTime(),
                                trainUsage,
                                maxAgents);
                        }
                    });
                geometryChanged = true;
            }
        }
    }
    return geometryChanged;
}

void SimulationHelper::RemoveFaultyPedestrians(
    Building & building,
    std::vector<Pedestrian *> & pedsFaulty,
    std::string message)
{
    std::for_each(std::begin(pedsFaulty), std::end(pedsFaulty), [&message](Pedestrian * ped) {
        LOG_ERROR(
            "Pedestrian {} at ({:.3f} | {:.3f}): {}",
            ped->GetID(),
            ped->GetPos()._x,
            ped->GetPos()._y,
            message);
    });

    SimulationHelper::RemovePedestrians(building, pedsFaulty);
}

void SimulationHelper::RemovePedestrians(Building & building, std::vector<Pedestrian *> & peds)
{
    std::for_each(std::begin(peds), std::end(peds), [&building](Pedestrian * ped) {
        building.DeletePedestrian(ped);
    });

    peds.clear();
}
