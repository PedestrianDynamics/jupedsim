#include "cognitivemap.h"
#include "../../../geometry/Point.h"
#include "../../../geometry/SubRoom.h"
#include "../../../geometry/Building.h"
#include "../Tools/triangle.h"
#include "../../../pedestrian/Pedestrian.h"
#include "../../../visiLibity/source_code/visilibity.hpp"


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
        {
            _landmarks.push_back(landmark);

        }
    }
}

std::vector<ptrLandmark> CognitiveMap::LookForLandmarks()
{
    SubRoom * sub_room = _building->GetRoom(_ped->GetRoomID())->GetSubRoom(_ped->GetSubRoomID());

    std::vector<ptrLandmark> landmarks_found;

    for (ptrLandmark landmark:_landmarks)
    {
        if (landmark->GetRoom()==sub_room)
        {
           landmarks_found.push_back(landmark);
        }
    }

    return landmarks_found;
}

Waypoints CognitiveMap::TriggerAssoziations(const std::vector<ptrLandmark> &landmarks) const
{
    Waypoints waypoints;
    for (ptrLandmark landmark:landmarks)
    {
        Associations associations = landmark->GetAssociations();
        for (ptrAssociation association:associations)
        {
            waypoints.push_back(association->GetAssociation(landmark));
        }
    }
    return waypoints;
}

void CognitiveMap::AddWaypoints(Waypoints waypoints)
{
    for (ptrWaypoint waypoint:waypoints)
    {
        _waypContainer.push_back(waypoint);
    }
}

void CognitiveMap::AssessDoors()
{
    SubRoom * sub_room = _building->GetRoom(_ped->GetRoomID())->GetSubRoom(_ped->GetSubRoomID());
    GraphVertex * vertex = (*_network->GetNavigationGraph())[sub_room];
    const GraphVertex::EdgesContainer edges = *(vertex->GetAllEdges());

    //const Point rpWaypoint=_waypContainer[0]->GetPos();

    if (!_waypContainer.empty())
    {
        GraphEdge* nextDoor = DoorOnShortestPath(_waypContainer[0],edges);
        //Log->Write(std::to_string(nextDoor->GetCrossing()->GetID()));
        nextDoor->SetFactor(1/_waypContainer[0]->GetPriority(),"SpatialKnowlegde");
        //Log->Write(std::to_string(nextDoor->GetCrossing()->GetID()));
        //Log->Write("INFO: Door assessed!");
    }

}

GraphEdge *CognitiveMap::DoorOnShortestPath(ptrWaypoint waypoint, const GraphVertex::EdgesContainer edges)
{



    double min = FLT_MAX;
    GraphEdge* edgeOnShortest = (*edges.begin()); //.operator *();

    for (auto it=edges.begin(); it!=edges.end(); ++it)
    {

        double pathLengthDoorWayP = ShortestPathDistance((*it),waypoint);

        //Point vectorPathPedDoor = (*it)->GetCrossing()->GetCentre()-_ped->GetPos();

        //double pathLength = pathLengthDoorWayP+std::sqrt(std::pow(vectorPathPedDoor.GetX(),2)+std::pow(vectorPathPedDoor.GetY(),2));

        if (pathLengthDoorWayP<min)
        {

            edgeOnShortest=*it;
            min=pathLengthDoorWayP;

        }
    }

    Log->Write("subroom:\t");
    Log->Write(std::to_string(_ped->GetSubRoomID()));
    Log->Write("edgeOnShortest:");
    Log->Write(std::to_string(edgeOnShortest->GetCrossing()->GetID()));
    return edgeOnShortest;

}

//bool CognitiveMap::IsAroundWaypoint(const Waypoint& waypoint, GraphEdge *edge) const
//{
//    Triangle triangle(_ped->GetPos(),waypoint);
//    Point point(edge->GetCrossing()->GetCentre());
//    return triangle.Contains(point);
//}

ptrGraphNetwork CognitiveMap::GetGraphNetwork() const
{
    return _network;
}

double CognitiveMap::ShortestPathDistance(const GraphEdge* edge, const ptrWaypoint waypoint)
{

    SubRoom* sub_room = _building->GetRoom(_ped->GetRoomID())->GetSubRoom(_ped->GetSubRoomID());
    std::vector<Point> points = sub_room->GetPolygon();
    VisiLibity::Polygon boundary=VisiLibity::Polygon();
    VisiLibity::Polygon room=VisiLibity::Polygon();
//    for (Point point:points)
//    {
//       room.push_back(VisiLibity::Point(point.GetX(),point.GetY()));
//    }

    for (int i=0; i<room.n();++i)
    {
        Log->Write("RoomVertices:");
        std::cout << std::to_string(room[i].x()) << "\t" << std::to_string(room[i].y()) << std::endl;
    }

    room.push_back(VisiLibity::Point(7.6,0.37));
    room.push_back(VisiLibity::Point(7.6,4.37));
    room.push_back(VisiLibity::Point(9.6,4.37));
    room.push_back(VisiLibity::Point(9.6,0.37));



    boundary.push_back(VisiLibity::Point(-100,-100));
    boundary.push_back(VisiLibity::Point(100,-100));
    boundary.push_back(VisiLibity::Point(100,100));
    boundary.push_back(VisiLibity::Point(-100,100));

    std::vector<VisiLibity::Polygon> polygons;
    polygons.push_back(boundary);
    polygons.push_back(room);

    VisiLibity::Environment environment(polygons);
    //environment.reverse_holes();

    VisiLibity::Point edgeP(edge->GetCrossing()->GetCentre().GetX(),edge->GetCrossing()->GetCentre().GetY());
    VisiLibity::Point wayP(waypoint->GetPos().GetX(),waypoint->GetPos().GetY());



    VisiLibity::Polyline polyline=environment.shortest_path(edgeP,wayP,0.1);
//    for (int i=0; i<polyline.size();++i)
//    {
//        Log->Write("Polyline:");
//        std::cout << std::to_string(polyline[i].x()) << "\t" << std::to_string(polyline[i].y()) << std::endl;
//    }
//    Log->Write("ShortestPathLength");
//    Log->Write(std::to_string(polyline.length()));
    return polyline.length();
    //return 0.0;
}




