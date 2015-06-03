#include "cognitivemap.h"
#include "../../../geometry/Point.h"
#include "../../../geometry/Building.h"
#include "../Tools/triangle.h"
#include "../../../pedestrian/Pedestrian.h"


CognitiveMap::CognitiveMap()
{

}


CognitiveMap::CognitiveMap(ptrBuilding b, ptrPed ped)
{
    _building=b;
    _ped=ped;

    _network = std::make_shared<GraphNetwork>(b,ped);
}

CognitiveMap::~CognitiveMap()
{

}

void CognitiveMap::AddLandmarks(std::vector<ptrLandmark> landmarks)
{
    for (ptrLandmark landmark:landmarks)
    {
        if (std::find(_landmarks.begin(), _landmarks.end(), landmark)!=_landmarks.end())
        {
            continue;
        }
        else
            _landmarks.push_back(landmark);
    }
}

void CognitiveMap::AssessDoors()
{
    SubRoom * sub_room = _building->GetRoom(_ped->GetRoomID())->GetSubRoom(_ped->GetSubRoomID());
    GraphVertex * vertex = (*_network->GetNavigationGraph())[sub_room];
    const GraphVertex::EdgesContainer edges = *(vertex->GetAllEdges());
    for (GraphEdge* edge:edges)
    {
        for (ptrWaypoint waypoint:_waypContainer)
        {
            if (IsAroundWaypoint(*waypoint,edge))
            {
                edge->SetFactor(1/waypoint->GetPriority(),"SpatialKnowlegde");
            }
        }

    }

}

bool CognitiveMap::IsAroundWaypoint(const Waypoint& waypoint, GraphEdge *edge) const
{
    Triangle triangle(_ped->GetPos(),waypoint);

    return triangle.Contains(edge->GetCrossing()->GetCentre());
}

ptrGraphNetwork CognitiveMap::GetGraphNetwork() const
{
    return _network;
}




