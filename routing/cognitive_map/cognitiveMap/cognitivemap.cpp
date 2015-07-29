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

    _YAHPointer.SetPos(_ped->GetPos());

}

CognitiveMap::~CognitiveMap()
{

}

void CognitiveMap::UpdateMap()
{
    AddWaypoints(TriggerAssoziations(LookForLandmarks()));
    if (_waypContainerSorted.empty())
        return;
    if (_waypContainerSorted.top()->WaypointReached(_YAHPointer.GetPos()))
    {
        ptrWaypoint cWaypoint = _waypContainerSorted.top();

        cWaypoint->SetPriority(_waypContainerSorted.size());
        _waypContainerSorted.pop();

        _waypContainerSorted.push(cWaypoint);
        //_waypContainer.pop();
        Log->Write("Prio:\t"+std::to_string(_waypContainerSorted.top()->GetPriority()));
    }
}

void CognitiveMap::UpdateYAHPointer(const Point& point)
{
    _YAHPointer.SetPos(point);
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
        if (std::find(_waypContainer.begin(), _waypContainer.end(), waypoint)!=_waypContainer.end())
        {
            continue;
        }
        _waypContainer.push_back(waypoint);
        _waypContainerSorted.push(waypoint);
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
        std::vector<GraphEdge* > sortedEdges = SortConShortestPath(_waypContainerSorted.top(),edges);
        //Log->Write(std::to_string(nextDoor->GetCrossing()->GetID()));

        for (unsigned int i=0; i<sortedEdges.size(); ++i)
        {
            sortedEdges[i]->SetFactor(0.5+0.1*i,"SpatialKnowledge");
            //Log->Write("INFO:\t "+std::to_string(sortedEdges[i]->GetCrossing()->GetID()));
            //Log->Write("INFO:\t "+std::to_string(sortedEdges[i]->GetFactor()));
        }

        //Log->Write(std::to_string(nextDoor->GetCrossing()->GetID()));
        //Log->Write("INFO: Door assessed!");
    }

}

std::vector<GraphEdge *> CognitiveMap::SortConShortestPath(ptrWaypoint waypoint, const GraphVertex::EdgesContainer edges)
{

    std::list<double> sortedPathLengths;
    sortedPathLengths.push_back(ShortestPathDistance((*edges.begin()),waypoint));
    std::list<GraphEdge* > sortedEdges;
    sortedEdges.push_back((*edges.begin()));

    auto itsortedEdges = sortedEdges.begin();
    auto it = edges.begin();
    ++it;
    ///starting at the second element
    for (it; it!=edges.end(); ++it)
    {
        double pathLengthDoorWayP = ShortestPathDistance((*it),waypoint);

        //Point vectorPathPedDoor = (*it)->GetCrossing()->GetCentre()-_ped->GetPos();
        itsortedEdges = sortedEdges.begin();
        //double pathLength = pathLengthDoorWayP+std::sqrt(std::pow(vectorPathPedDoor.GetX(),2)+std::pow(vectorPathPedDoor.GetY(),2));
        bool inserted=false;
        for (auto itLengths=sortedPathLengths.begin(); itLengths!=sortedPathLengths.end(); ++itLengths)
        {
            if (pathLengthDoorWayP >= *itLengths)
            {
                ++itsortedEdges;
                continue;
            }

            sortedPathLengths.insert(itLengths,pathLengthDoorWayP);
            sortedEdges.insert(itsortedEdges,(*it));
            inserted=true;
            break;
        }
        if (!inserted)
        {
            sortedPathLengths.push_back(pathLengthDoorWayP);
            sortedEdges.push_back((*it));
        }
    }

//    Log->Write("subroom:\t");
//    Log->Write(std::to_string(_ped->GetSubRoomID()));
//    Log->Write("edgeOnShortest:");
//    Log->Write(std::to_string(edgeOnShortest->GetCrossing()->GetID()))
    std::vector<GraphEdge* > vectorSortedEdges;
    for (GraphEdge* edge:sortedEdges)
    {
        vectorSortedEdges.push_back(edge);
        //Log->Write("INFO:\t "+std::to_string(edge->GetCrossing()->GetID()));

    }
    //Log->Write("INFO:\t Next");
    return vectorSortedEdges;

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
    for (Point point:points)
    {
       room.push_back(VisiLibity::Point(point.GetX(),point.GetY()));
    }

//    for (int i=0; i<room.n();++i)
//    {
//        Log->Write("RoomVertices:");
//        std::cout << std::to_string(room[i].x()) << "\t" << std::to_string(room[i].y()) << std::endl;
//    }

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
    Point pointOnShortestRoute = waypoint->PointOnShortestRoute(edge->GetCrossing()->GetCentre());
    //Log->Write(std::to_string(pointOnShortestRoute.GetX())+" "+std::to_string(pointOnShortestRoute.GetY()));
    VisiLibity::Point wayP(pointOnShortestRoute.GetX(),pointOnShortestRoute.GetY());//,waypoint->GetPos().GetY());

    VisiLibity::Polyline polyline=environment.shortest_path(edgeP,wayP,0.1);
//    for (int i=0; i<polyline.size();++i)
//    {
//        Log->Write("Polyline:");
//        std::cout << std::to_string(polyline[i].x()) << "\t" << std::to_string(polyline[i].y()) << std::endl;
//    }
//    Log->Write("ShortestPathLength");
//    Log->Write(std::to_string(polyline.length()));

    return polyline.length();
}




