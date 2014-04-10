/**
 * @file   CognitiveMap.cpp
 * @author David Haensel (d.haensel@fz-juelich.de)
 * @date   January, 2014
 * @brief  Cognitive Map models the pedestrian knowledge of building space in simulation.
 *
 */

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
