/**
 * \file        RoomToFloorSensor.cpp
 * \date        Mar 1, 2014
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
 **/
#include "RoomToFloorSensor.h"

#include "geometry/Building.h"
#include "geometry/SubRoom.h"
#include "pedestrian/Pedestrian.h"
#include "router/smoke_router/cognitiveMap/cognitivemap.h"
#include "router/smoke_router/NavigationGraph.h"

RoomToFloorSensor::~RoomToFloorSensor()
{
}

std::string RoomToFloorSensor::GetName() const
{
     return "RoomToFloorSensor";
}

void RoomToFloorSensor::execute(const Pedestrian * pedestrian, CognitiveMap &cognitive_map) const
{
    // NavigationGraph::VerticesContainer * vertices = cognitive_map->GetAllVertices();

    // //iterating over all Vertices
    // for(NavigationGraph::VerticesContainer::iterator it_vertices = vertices->begin(); it_vertices != vertices->end(); ++it_vertices) {
    //     GraphVertex::EdgesContainer * edges = (it_vertices->second)->GetAllEdges();
    //     //iterating over all edges
    //     for(GraphVertex::EdgesContainer::iterator it_edges = edges->begin(); it_edges != edges->end(); ++it_edges) {
    //         if((*it_edges)->GetDest() == NULL || (*it_edges)->GetDest()->GetSubRoom()->GetType() == (*it_edges)->GetSrc()->GetSubRoom()->GetType()) {
    //             (*it_edges)->SetFactor(1.0, GetName());
    //         } else {
    //             if((*it_edges)->GetDest()->GetSubRoom()->GetType() == "floor")
    //                 (*it_edges)->SetFactor(.5 , GetName());
    //             else
    //                 (*it_edges)->SetFactor(3.0 , GetName());
    //         }
    //     }
    // }

    SubRoom * sub_room = building->GetRoom(pedestrian->GetRoomID())->GetSubRoom(pedestrian->GetSubRoomID());
    GraphVertex * vertex = cognitive_map.GetGraphNetwork()->GetNavigationGraph()->operator [](sub_room);
    const GraphVertex::EdgesContainer * edges = vertex->GetAllOutEdges();
   for(GraphVertex::EdgesContainer::iterator it_edges = edges->begin(); it_edges != edges->end(); ++it_edges) {
        if((*it_edges)->GetDest() == nullptr || (*it_edges)->GetDest()->GetSubRoom()->GetType() == (*it_edges)->GetSrc()->GetSubRoom()->GetType()) {
            (*it_edges)->SetFactor(1.0, GetName());
        } else {
            if((*it_edges)->GetDest()->GetSubRoom()->GetType() == "Corridor")
                (*it_edges)->SetFactor(.3 , GetName());
            else if ((*it_edges)->GetDest()->GetSubRoom()->GetType() == "stair")
                (*it_edges)->SetFactor(.3 , GetName());
            else if ((*it_edges)->GetDest()->GetSubRoom()->GetType() == "floor")
                (*it_edges)->SetFactor(.3 , GetName());
            else if ((*it_edges)->GetDest()->GetSubRoom()->GetType() == "Entrance")
                (*it_edges)->SetFactor(.1 , GetName());
            else if ((*it_edges)->GetDest()->GetSubRoom()->GetType() == "Lobby")
                (*it_edges)->SetFactor(.2 , GetName());
            else
                (*it_edges)->SetFactor(5.0 , GetName());
        }
    }

    // for(auto it_edges : edges) {
    //     if(it_edges.GetDest() == NULL || it_edges.GetDest()->GetSubRoom()->GetType() == it_edges.GetSrc()->GetSubRoom()->GetType()) {
    //         it_edges.SetFactor(1.0, GetName());
    //     } else {
    //         if(it_edges.GetDest()->GetSubRoom()->GetType() == "floor")
    //             it_edges.SetFactor(.3 , GetName());
    //         else
    //             it_edges.SetFactor(5.0 , GetName());
    //     }
    // }
}
