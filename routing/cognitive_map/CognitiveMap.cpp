/**
 * \file        CognitiveMap.cpp
 * \date        Jan 1, 2014
 * \version     v0.6
 * \copyright   <2009-2014> Forschungszentrum Jülich GmbH. All rights reserved.
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


#include "CognitiveMap.h"
#include "NavigationGraph.h"

#include "navigation_graph/GraphVertex.h"
#include "navigation_graph/GraphEdge.h"
#include "../../geometry/Crossing.h"
#include "../../geometry/Building.h"
#include "../../geometry/NavLine.h"
#include "../../pedestrian/Pedestrian.h"

using namespace std;

/**
 * Constructors & Destructors
 */

CognitiveMap::CognitiveMap(const Building * building, const Pedestrian * pedestrian)
     : building(building), pedestrian(pedestrian)
{
     navigation_graph = new NavigationGraph(building);
}

CognitiveMap::~CognitiveMap()
{
     delete navigation_graph;
}

void CognitiveMap::Add(const SubRoom * sub_room)
{
     navigation_graph->AddVertex(sub_room);
}

void CognitiveMap::Add(const Crossing * crossing)
{
     navigation_graph->AddEdge(crossing);
}

void CognitiveMap::AddExit(const Transition * exit)
{
     navigation_graph->AddExit(exit);
}

NavigationGraph::VerticesContainer * CognitiveMap::GetAllVertices()
{
     return navigation_graph->GetAllVertices();
}


NavigationGraph * CognitiveMap::GetNavigationGraph() const
{
     return navigation_graph;
}
const GraphEdge * CognitiveMap::GetDestination()
{
    SubRoom * sub_room = building->GetRoom(pedestrian->GetRoomID())->GetSubRoom(pedestrian->GetSubRoomID());
    return (*navigation_graph)[sub_room]->GetCheapestDestinationByEdges(pedestrian->GetPos());
}

const GraphEdge * CognitiveMap::GetLocalDestination()
{
    SubRoom * sub_room = building->GetRoom(pedestrian->GetRoomID())->GetSubRoom(pedestrian->GetSubRoomID());
//    if(pedestrian->GetID() == 4) navigation_graph->WriteToDotFile("/home/david/graph.dot");

    return (*navigation_graph)[sub_room]->GetLocalCheapestDestination(pedestrian->GetPos());
}

bool CognitiveMap::HadNoDestination() const
{
    return destinations.empty();
}

void CognitiveMap::AddDestination(const GraphEdge* destination)
{
    destinations.push_back(destination);
}

std::vector<const GraphEdge *>& CognitiveMap::GetDestinations()
{
    return destinations;
}

bool CognitiveMap::ChangedSubRoom() const
{
    return current_subroom != building->GetRoom(pedestrian->GetRoomID())->GetSubRoom(pedestrian->GetSubRoomID());
}

void CognitiveMap::UpdateSubRoom()
{
    current_subroom = building->GetRoom(pedestrian->GetRoomID())->GetSubRoom(pedestrian->GetSubRoomID());
}
