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

#include "Simulation.h"
#include "geometry/Room.h"
#include "geometry/SubRoom.h"
#include "pedestrian/Pedestrian.h"

#include <Logger.h>
#include <algorithm>
#include <iterator>
#include <memory>

std::vector<Pedestrian::UID> SimulationHelper::FindPedestriansOutside(
    const Building & building,
    const std::vector<std::unique_ptr<Pedestrian>> & peds)
{
    std::vector<Pedestrian::UID> pedsOutside;
    for(const auto & ped : peds) {
        if(!building.IsInAnySubRoom(ped->GetPos())) {
            pedsOutside.push_back(ped->GetUID());
        }
    }
    return pedsOutside;
}

void SimulationHelper::UpdateFlowAtDoors(
    Building & building,
    const std::vector<std::unique_ptr<Pedestrian>> & peds,
    double time)
{
    for(const auto & ped : peds) {
        const auto * subroom = building.IsInAnySubRoom(ped->GetPos()) ?
                                   building.GetSubRoom(ped->GetPos()) :
                                   building.GetSubRoom(ped->GetLastPosition());


        auto passedDoor = FindPassedDoor(*ped, subroom->GetAllTransitions());

        if(!passedDoor.has_value()) {
            continue;
        }

        if(ped->GetDestination() >= 0 &&
           (building.GetTransitionByUID(ped->GetDestination()) != nullptr) &&
           passedDoor.value()->GetUniqueID() !=
               building.GetTransitionByUID(ped->GetDestination())->GetUniqueID()) {
            LOG_WARNING(
                "Ped {}: used an unindented door {}, but wanted to go to {}.",
                ped->GetUID(),
                passedDoor.value()->GetUniqueID(),
                ped->GetDestination());
            continue;
        }
        passedDoor.value()->IncreaseDoorUsage(1, time, ped->GetUID());
        passedDoor.value()->IncreasePartialDoorUsage(1);
    }
}

std::optional<Transition *> SimulationHelper::FindPassedDoor(
    const Pedestrian & ped,
    const std::vector<Transition *> & transitions)
{
    Line step{ped.GetLastPosition(), ped.GetPos(), 0};
    // TODO check for closed doors and distance?
    auto passedTrans = std::find_if(
        std::begin(transitions), std::end(transitions), [&step](const Transition * trans) -> bool {
            return trans->IntersectionWith(step) == 1;
        });

    if(passedTrans == transitions.end() || (*passedTrans)->IsInLineSegment(ped.GetPos())) {
        return std::nullopt;
    }
    return *passedTrans;
}

bool SimulationHelper::UpdateFlowRegulation(Building & building, double time)
{
    bool stateChanged = false;

    for(auto [transID, trans] : building.GetAllTransitions()) {
        DoorState state = trans->GetState();
        trans->UpdateTemporaryState(building.GetConfig()->dT);

        bool regulateFlow = trans->GetOutflowRate() < std::numeric_limits<double>::max() ||
                            trans->GetMaxDoorUsage() < std::numeric_limits<double>::max();

        if(regulateFlow) {
            // when more than <dn> agents pass <trans>, we start evaluating the flow
            // .. and maybe close the <trans>
            // need to be more than <dn> as multiple ped can pass a transition in one time step
            if(trans->GetPartialDoorUsage() >= trans->GetDN()) {
                trans->RegulateFlow(time);
                trans->ResetPartialDoorUsage();
            }
        }

        stateChanged = stateChanged || (state != trans->GetState());
    }
    return stateChanged;
}

bool SimulationHelper::UpdateTrainFlowRegulation(Building & building, double time)
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
                    [&building, trainUsage, maxAgents, time](Transition trans) {
                        if(!building.GetTransition(trans.GetID())->IsClose()) {
                            building.GetTransition(trans.GetID())->Close();
                            LOG_INFO(
                                "Closing train door {} with ID {} at t={:.2f}. Door usage = {} "
                                "(Train Capacity {})",
                                trans.GetType(),
                                trans.GetID(),
                                time,
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
