/**
 * \file        GraphNetwork.cpp
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
 * Cognitive Map models the pedestrian knowledge of building space in simulation.
 *
 *
 **/
#include "GraphNetwork.h"

#include "geometry/Crossing.h"
#include "geometry/Building.h"
#include "pedestrian/Pedestrian.h"

/**
 * Constructors & Destructors
 */

GraphNetwork::GraphNetwork(const Building *buildg, const Pedestrian *ped)
     : building(buildg), pedestrian(ped)
{
     navigation_graph = new NavigationGraph(buildg);

}

GraphNetwork::~GraphNetwork()
{
     delete navigation_graph;
}

void GraphNetwork::Add(const SubRoom * sub_room)
{
     navigation_graph->AddVertex(sub_room);

}

void GraphNetwork::Add(const Crossing * crossing)
{
     navigation_graph->AddEdge(crossing);
}

void GraphNetwork::AddExit(const Transition * exit)
{
     navigation_graph->AddExit(exit);
}

NavigationGraph::VerticesContainer * GraphNetwork::GetAllVertices()
{
     return navigation_graph->GetAllVertices();
}


NavigationGraph * GraphNetwork::GetNavigationGraph() const
{
     return navigation_graph;
}
const GraphEdge * GraphNetwork::GetDestination() const
{
    SubRoom * sub_room = building->GetRoom(pedestrian->GetRoomID())->GetSubRoom(pedestrian->GetSubRoomID());

    return (*navigation_graph)[sub_room]->GetCheapestDestinationByEdges(pedestrian->GetPos());
}

const GraphEdge * GraphNetwork::GetLocalDestination()
{
    SubRoom * sub_room = building->GetRoom(pedestrian->GetRoomID())->GetSubRoom(pedestrian->GetSubRoomID());

    return (*navigation_graph)[sub_room]->GetLocalCheapestDestination(pedestrian->GetPos());
}

bool GraphNetwork::HadNoDestination() const
{
    return destinations.empty();
}

void GraphNetwork::AddDestination(const GraphEdge* destination)
{
    destinations.push_back(destination);
}

std::vector<const GraphEdge *>& GraphNetwork::GetDestinations()
{
    return destinations;
}


bool GraphNetwork::ChangedSubRoom() const
{
    return current_subroom != building->GetRoom(pedestrian->GetRoomID())->GetSubRoom(pedestrian->GetSubRoomID());
}

void GraphNetwork::UpdateSubRoom()
{
    current_subroom = building->GetRoom(pedestrian->GetRoomID())->GetSubRoom(pedestrian->GetSubRoomID());
}


