/**
 * \file        JamSensor.cpp
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
 * detects jam in the next rooms
 *
 *
 **/
#include "JamSensor.h"

#include "geometry/Building.h"
#include "geometry/SubRoom.h"
#include "pedestrian/Pedestrian.h"
#include "router/smoke_router/cognitiveMap/cognitivemap.h"
#include "router/smoke_router/NavigationGraph.h"

JamSensor::~JamSensor()
{
}

std::string JamSensor::GetName() const
{
    return "JamSensor";
}

void JamSensor::execute(const Pedestrian * pedestrian, CognitiveMap &cognitive_map) const
{
    SubRoom * sub_room = building->GetRoom(pedestrian->GetRoomID())->GetSubRoom(pedestrian->GetSubRoomID());
    GraphVertex * vertex = cognitive_map.GetGraphNetwork()->GetNavigationGraph()->operator [](sub_room);
    const GraphVertex::EdgesContainer * edges = vertex->GetAllOutEdges();

    for(GraphVertex::EdgesContainer::const_iterator it = edges->begin(); it != edges->end(); ++it) {
        //count pedestrians heading to this door
        double own_distance = (*it)->GetCrossing()->DistTo(pedestrian->GetPos());

        double pedestrians_count = 0;
        if((*it)->GetSrc()) {
//            const std::vector<Pedestrian*>& pedestrians = (*it)->GetSrc()->GetSubRoom()->GetAllPedestrians();
            std::vector<Pedestrian*> pedestrians;
            const SubRoom * sub_room_src = (*it)->GetSrc()->GetSubRoom();
            building->GetPedestrians(sub_room_src->GetRoomID(), sub_room_src->GetSubRoomID(), pedestrians);
            for(std::vector<Pedestrian*>::const_iterator ped = pedestrians.begin(); ped != pedestrians.end(); ++ped) {
                /**
                 * If other pedestrians are heading the same destination (corresponding to the edge)
                 * they are counted.
                 * the count depends on the pedestrians velocity to avoid congestions.
                 * If the velocity is zero we take 100.
                 * THIS IS JUST A PROOF OF CONCEPT.
                 * PARAMETERS SHOULD BE CALIBRATED!
                 */
                if((*ped)->GetExitIndex() == (*it)->GetCrossing()->GetUniqueID() && (*it)->GetCrossing()->DistTo((*ped)->GetPos()) < own_distance) {
                    if((*ped)->GetV().Norm() > 1.0 || (*ped)->GetV().Norm() == 0.0) {
                        pedestrians_count = pedestrians_count + 1;
                    } else {
                        pedestrians_count = pedestrians_count +2;
                    }
                }

            }


        }
        if((*it)->GetDest()) {
//            const std::vector<Pedestrian*>& pedestrians = (*it)->GetDest()->GetSubRoom()->GetAllPedestrians();
            std::vector<Pedestrian*> pedestrians;
            const SubRoom * sub_room_dest = (*it)->GetDest()->GetSubRoom();
            building->GetPedestrians(sub_room_dest->GetRoomID(), sub_room_dest->GetSubRoomID(), pedestrians);

            for(std::vector<Pedestrian*>::const_iterator ped = pedestrians.begin(); ped != pedestrians.end(); ++ped) {
                if((*ped)->GetExitIndex() == (*it)->GetCrossing()->GetUniqueID()) {

                    if((*ped)->GetV().Norm() > 0)
                        pedestrians_count = pedestrians_count + 2;
                }
            }
        }

        //normalize the pedestrian count with the door width
        double pedestrians_normalized = pedestrians_count / (*it)->GetCrossing()->Length();

        if(pedestrians_normalized > 0.0 && pedestrians_count > 7) (*it)->SetFactor(pedestrians_normalized, GetName());


    }
}
