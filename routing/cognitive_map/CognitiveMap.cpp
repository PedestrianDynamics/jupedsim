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


const NavigationGraph * CognitiveMap::GetNavigationGraph() const
{
    return navigation_graph;
}
const NavLine * CognitiveMap::GetDestination()
{
    SubRoom * sub_room = building->GetRoom(pedestrian->GetRoomID())->GetSubRoom(pedestrian->GetSubRoomID());

    std::pair<const GraphEdge*, double> cheapest_destination = (*navigation_graph)[sub_room]->GetCheapestDestinationByEdges(pedestrian->GetPos());

//    if(pedestrian->GetID() == 81) Log->Write("Pedestrian 81 wants from " +cheapest_destination.first->GetSrc()->GetCaption()+" to "+cheapest_destination.first->GetDest()->GetCaption()+" the distance is: "+std::to_string(cheapest_destination.second));

    return cheapest_destination.first->GetCrossing();
}
