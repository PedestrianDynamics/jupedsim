/**
 * \file        DiscoverDoorsSensor.cpp
 * \date        Jan 1, 2014
 * \version     v0.7
 * \copyright   <2009-2015> Forschungszentrum Jülich GmbH. All rights reserved.
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
 *
 * \section Description
 *
 *
 *
 **/
#include "DiscoverDoorsSensor.h"

#include "geometry/Building.h"
#include "geometry/SubRoom.h"
#include "pedestrian/Pedestrian.h"
#include "router/smoke_router/cognitiveMap/cognitivemap.h"
#include "router/smoke_router/NavigationGraph.h"

DiscoverDoorsSensor::~DiscoverDoorsSensor()
{
}

std::string DiscoverDoorsSensor::GetName() const
{
    return "DiscoverDoorsSensor";
}

void DiscoverDoorsSensor::execute(const Pedestrian * pedestrian, CognitiveMap& cognitive_map) const
{
    SubRoom * sub_room = building->GetRoom(pedestrian->GetRoomID())->GetSubRoom(pedestrian->GetSubRoomID());

    const std::vector<Crossing *>& crossings = sub_room->GetAllCrossings();
    for(std::vector<Crossing*>::const_iterator it = crossings.begin(); it != crossings.end(); ++it)
    {
        if ((*it)->IsOpen())
            cognitive_map.GetGraphNetwork()->Add((*it));
    }
    //Add transitions as edges
    const std::vector<Transition *>& transitions = sub_room->GetAllTransitions();
    for(std::vector<Transition *>::const_iterator it = transitions.begin(); it != transitions.end(); ++it)
    {
        if ((*it)->IsOpen())
        {
            if((*it)->IsExit()) {
                cognitive_map.GetGraphNetwork()->AddExit((*it));
            } else {
                cognitive_map.GetGraphNetwork()->Add((*it));
            }
        }
    }
}
