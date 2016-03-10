#include "cognitivemap.h"
#include "../../../geometry/Point.h"
#include "../../../geometry/SubRoom.h"
#include "../../../geometry/Building.h"
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

    std::string str(b->GetProjectRootDir()+"cogmap.xml");
    _outputhandler = std::make_shared<CogMapOutputHandler>(str.c_str());

    const double fps = UPDATE_RATE;
    _outputhandler->WriteToFileHeader(ped->GetID(),fps);
    _frame=0;

    _YAHPointer.SetPed(ped);
    _YAHPointer.SetPos(_ped->GetPos());

    _createdWayP=-1;

    //Destination and regions
    _currentRegion=nullptr;
    FindMainDestination();
    _nextTarget=nullptr;

}

CognitiveMap::~CognitiveMap()
{

}

void CognitiveMap::UpdateMap()
{
    AddAssociatedLandmarks(TriggerAssociations(LookForLandmarks()));
    if (_waypContainerTargetsSorted.empty())
        return;
    if (_waypContainerTargetsSorted.top()->LandmarkReached(_YAHPointer.GetPos()))
    {
        //ptrLandmark cLandmark = _waypContainerTargetsSorted.top();
        LandmarkReached(_waypContainerTargetsSorted.top());
        //cLandmark->SetPriority(_waypContainerTargetsSorted.size());
        _waypContainerTargetsSorted.pop();

        SubRoom * sub_room = _building->GetRoom(_ped->GetRoomID())->GetSubRoom(_ped->GetSubRoomID());
        GraphVertex * vertex = (*_network->GetNavigationGraph())[sub_room];
        const GraphVertex::EdgesContainer edges = *(vertex->GetAllEdges());

        for (GraphEdge* edge:edges)
        {
            edge->SetFactor(1,"SpatialKnowledge");
        }

        //_waypContainerTargetsSorted.push(cLandmark);
        //_waypContainer.pop();
        //Log->Write("Prio:\t"+std::to_string(_waypContainerTargetsSorted.top()->GetPriority()));
    }
}

void CognitiveMap::UpdateDirection()
{
    if (_ped->GetV().GetX()!=0 || _ped->GetV().GetY()!=0)
    {
        double angle = std::acos(_ped->GetV().GetX()/(std::sqrt(std::pow(_ped->GetV().GetX(),2)+std::pow(_ped->GetV().GetY(),2))));
        if (_ped->GetV().GetY()<0)
            angle=-angle;
        _YAHPointer.SetDirection(angle);
    }
}

void CognitiveMap::UpdateYAHPointer(const Point& move)
{
    _YAHPointer.UpdateYAH(move);
}

void CognitiveMap::AddRegions(Regions regions)
{
    _regions.swap(regions);
}

void CognitiveMap::AddRegion(ptrRegion region)
{
    _regions.push_back(region);
}

ptrRegion CognitiveMap::GetRegionByID(const int &regionID) const
{
    for (ptrRegion region:_regions)
    {
        if (region->GetId()==regionID)
        {
            return region;
        }
    }
    return nullptr;
}

void CognitiveMap::AddLandmarksSC(std::vector<ptrLandmark> landmarks)
{
    for (ptrLandmark landmark:landmarks)
    {
        if (std::find(_landmarksSubConcious.begin(), _landmarksSubConcious.end(), landmark)!=_landmarksSubConcious.end())
        {
            continue;
        }
        else
        {
            _landmarksSubConcious.push_back(landmark);

        }
    }

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
            _landmarks.back()->SetPosInMap(_landmarks.back()->GetPosInMap()-_YAHPointer.GetPosDiff());


        }
    }
}

void CognitiveMap::AddLandmarkInRegion(ptrLandmark landmark, ptrRegion region)
{
    region->AddLandmark(landmark);
}

std::vector<ptrLandmark> CognitiveMap::LookForLandmarks()
{
    SubRoom * sub_room = _building->GetRoom(_ped->GetRoomID())->GetSubRoom(_ped->GetSubRoomID());

    std::vector<ptrLandmark> landmarks_found;

    for (ptrLandmark landmark:_landmarksSubConcious)
    {
        if (landmark->GetRoom()==sub_room && std::find(_landmarks.begin(), _landmarks.end(), landmark)==_landmarks.end())
        {
           landmarks_found.push_back(landmark);
           LandmarkReached(landmark);
        }
    }

    AddLandmarks(landmarks_found);

    return landmarks_found;
}

Landmarks CognitiveMap::TriggerAssociations(const std::vector<ptrLandmark> &landmarks)
{
    Landmarks associatedlandmarks;
    for (ptrLandmark landmark:landmarks)
    {
        Associations associations = landmark->GetAssociations();
        for (ptrAssociation association:associations)
        {
            if (association->GetLandmarkAssociation(landmark)!=nullptr)
            {
                associatedlandmarks.push_back(association->GetLandmarkAssociation(landmark));
                //pos-yahpointer diff
                associatedlandmarks.back()->SetPosInMap(landmarks.back()->GetPosInMap()-_YAHPointer.GetPosDiff());
            }
            if (association->GetConnectionAssoziation()!=nullptr)
            {
                //AddConnection(association->GetConnectionAssoziation());
            }
        }
    }
    return associatedlandmarks;
}

void CognitiveMap::AddAssociatedLandmarks(Landmarks landmarks)
{
    for (ptrLandmark landmark:_waypContainer)
    {
        landmark->SetPriority(landmark->GetPriority()+landmarks.size());
    }
    int n=0;
    for (ptrLandmark landmark:landmarks)
    {
        if (std::find(_waypContainer.begin(), _waypContainer.end(), landmark)!=_waypContainer.end())
        {
            continue;
        }
        landmark->SetPriority(n);
        _waypContainer.push_back(landmark);
        //Add as new target
        if (!landmark->Visited())
            _waypContainerTargetsSorted.push(landmark);
        //Add as already visited
        else
        {
            LandmarkReached(landmark);
        }

        n++;


    }

}

void CognitiveMap::AssessDoors()
{
    SubRoom * sub_room = _building->GetRoom(_ped->GetRoomID())->GetSubRoom(_ped->GetSubRoomID());
    GraphVertex * vertex = (*_network->GetNavigationGraph())[sub_room];
    const GraphVertex::EdgesContainer edges = *(vertex->GetAllEdges());

    //const Point rpLandmark=_waypContainer[0]->GetPos();

    if (_nextTarget!=nullptr)
    {
        std::vector<GraphEdge* > sortedEdges = SortConShortestPath(_nextTarget,edges);
        //Log->Write(std::to_string(nextDoor->GetCrossing()->GetID()));

        for (unsigned int i=0; i<sortedEdges.size(); ++i)
        {
            sortedEdges[i]->SetFactor(0.6+0.1*i,"SpatialKnowledge");
//            Log->Write("INFO:\t "+std::to_string(sortedEdges[i]->GetCrossing()->GetID()));
//            Log->Write("INFO:\t "+std::to_string(sortedEdges[i]->GetFactor()));
        }

        //Log->Write(std::to_string(nextDoor->GetCrossing()->GetID()));
        //Log->Write("INFO: Door assessed!");
    }

}

std::vector<GraphEdge *> CognitiveMap::SortConShortestPath(ptrLandmark landmark, const GraphVertex::EdgesContainer edges)
{

    std::list<double> sortedPathLengths;
    sortedPathLengths.push_back(ShortestPathDistance((*edges.begin()),landmark));
    std::list<GraphEdge* > sortedEdges;
    sortedEdges.push_back((*edges.begin()));

    auto itsortedEdges = sortedEdges.begin();
    auto it = edges.begin();
    ++it;
    //starting at the second element
    for (it; it!=edges.end(); ++it)
    {
        double pathLengthDoorWayP = ShortestPathDistance((*it),landmark);

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

//bool CognitiveMap::IsAroundLandmark(const Landmark& landmark, GraphEdge *edge) const
//{
//    Triangle triangle(_ped->GetPos(),landmark);
//    Point point(edge->GetCrossing()->GetCentre());
//    return triangle.Contains(point);
//}

ptrGraphNetwork CognitiveMap::GetGraphNetwork() const
{
    return _network;
}

double CognitiveMap::ShortestPathDistance(const GraphEdge* edge, const ptrLandmark landmark)
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
    Point pointOnShortestRoute = landmark->PointOnShortestRoute(edge->GetCrossing()->GetCentre());
    //Log->Write(std::to_string(pointOnShortestRoute.GetX())+" "+std::to_string(pointOnShortestRoute.GetY()));
    VisiLibity::Point wayP(pointOnShortestRoute.GetX(),pointOnShortestRoute.GetY());//,landmark->GetPos().GetY());

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

const Point &CognitiveMap::GetOwnPos()
{
    return _YAHPointer.GetPos();
}

void CognitiveMap::WriteToFile()
{
    ++_frame;
    string data;
    char tmp[CLENGTH] = "";

    sprintf(tmp, "<frame ID=\"%d\">\n", _frame);
    data.append(tmp);


    for (ptrLandmark landmark:_landmarks)
    {
        char tmp1[CLENGTH] = "";
        sprintf(tmp1, "<landmark ID=\"%d\"\t"
               "caption=\"%s\"\t"
               "x=\"%.6f\"\ty=\"%.6f\"\t"
               "z=\"%.6f\"\t"
               "rA=\"%.2f\"\trB=\"%.2f\"/>\n",
               landmark->GetId(), landmark->GetCaption().c_str(), landmark->GetPosInMap().GetX(),
               landmark->GetPosInMap().GetY(),0.0 ,landmark->GetA(), landmark->GetB());

        data.append(tmp1);
    }
    bool current;

    for (ptrLandmark landmark:_waypContainer)
    {
        current=false;
        if (!_waypContainerTargetsSorted.empty())
        {
            if (landmark==_waypContainerTargetsSorted.top())
                current=true;
        }


        char tmp2[CLENGTH] = "";
        sprintf(tmp2, "<Landmark ID=\"%d\"\t"
               "caption=\"%s\"\t"
               "x=\"%.6f\"\ty=\"%.6f\"\t"
               "z=\"%.6f\"\t"
               "rA=\"%.2f\"\trB=\"%.2f\"\tcurrent=\"%i\"/>\n",
               landmark->GetId(),landmark->GetCaption().c_str(), landmark->GetPosInMap().GetX(),
               landmark->GetPosInMap().GetY(),0.0 ,landmark->GetA(), landmark->GetB(),
               current);
        data.append(tmp2);
    }

    char tmp3[CLENGTH]="";
    sprintf(tmp3, "<YAHPointer "
           "x=\"%.6f\"\ty=\"%.6f\"\t"
           "z=\"%.6f\"\t"
           "dir=\"%.2f\"/>\n",
           _YAHPointer.GetPos().GetX(),
           _YAHPointer.GetPos().GetY(),0.0 ,_YAHPointer.GetDirection());

    data.append(tmp3);


//    for (ptrConnection connection:_connections)
//    {
//        char tmp4[CLENGTH]="";
//        sprintf(tmp4, "<connection "
//               "Landmark_landmarkID1=\"%d\"\tLandmark_landmarkID2=\"%d\"/>\n",
//               connection->GetLandmarks().first->GetId(),
//               connection->GetLandmarks().second->GetId());

//        data.append(tmp4);
//    }


    data.append("</frame>\n");
    _outputhandler->WriteToFile(data);
}

void CognitiveMap::SetNewLandmark()
{
    double a= 2.0;
    double b=2.0;
    ptrLandmark wayP = std::make_shared<Landmark>(_YAHPointer.GetPos(),a,b,_createdWayP);
    _createdWayP--;
    wayP->SetVisited(true);
    std::vector<ptrLandmark> vec;
    vec.push_back(wayP);
    AddLandmarks(vec);
}

void CognitiveMap::LandmarkReached(ptrLandmark landmark)
{
    //build connection
//    if (!_landmarksRecentlyVisited.empty())
//        AddConnection(_landmarksRecentlyVisited.back(),landmark);

//    _landmarksRecentlyVisited.push_back(landmark);

//    if (_landmarksRecentlyVisited.size()>2)
//        _landmarksRecentlyVisited.pop();

}

std::vector<ptrConnection> CognitiveMap::GetAllConnections() const
{
    std::vector<ptrConnection> con{ std::begin(_connections), std::end(_connections) };
    return con;
}

void CognitiveMap::AddConnection(const ptrConnection& connection)
{
    _connections.push_back(connection);
}

void CognitiveMap::AddConnection(const ptrLandmark &landmark1, const ptrLandmark &landmark2)
{
    _connections.push_back(std::make_shared<Connection>(landmark1,landmark2));
}

void CognitiveMap::RemoveConnections(const ptrLandmark &landmark)
{
    for (ptrConnection connection:_connections)
    {
        if (connection->GetLandmarks().first==landmark || connection->GetLandmarks().second==landmark)
        {
            _connections.remove(connection);
        }
    }
}

Landmarks CognitiveMap::ConnectedWith(const ptrLandmark &landmark) const
{
    Landmarks cLandmarks;
    for (ptrConnection connection:_connections)
    {
        if (connection->GetLandmarks().first==landmark )
        {
            cLandmarks.push_back(connection->GetLandmarks().second);
        }
        else if (connection->GetLandmarks().second==landmark )
        {
            cLandmarks.push_back(connection->GetLandmarks().first);
        }
    }


    return cLandmarks;

}

void CognitiveMap::FindCurrentRegion()
{

}

const ptrLandmark CognitiveMap::FindConnectionPoint(const ptrRegion &regionA, const ptrRegion &regionB) const
{
    for (ptrLandmark landmarka:regionA->GetLandmarks())
    {
        for (ptrLandmark landmarkb:regionB->GetLandmarks())
        {
            if (landmarka==landmarkb)
            {
                return landmarka;
            }
        }
    }
    return nullptr;

}

void CognitiveMap::FindMainDestination()
{
    for (ptrRegion region:_regions)
    {
        for (ptrLandmark landmark:region->GetLandmarks())
        {
            if (landmark->GetType()=="main")
            {
                _mainDestination=landmark;
                _targetRegion=region;
                return;
            }
        }
    }
    // if no destination was found
    _mainDestination=nullptr;
    _targetRegion=nullptr;
    return;
}

void CognitiveMap::FindNextTarget()
{
    _nextTarget=nullptr;
    // if not already in the region of the maindestination
    if (_targetRegion!=_currentRegion)
    {
        _nextTarget=FindConnectionPoint(_targetRegion,_currentRegion);

        // if connection point does not exist: Path searching to region
        if (_nextTarget==nullptr)
        {
            //Region is target
            _nextTarget=_targetRegion->GetRegionAsLandmark();
            return;
        }

    }
    else //destination is in current region
    {
       _nextTarget=_mainDestination;
    }

    ptrLandmark nearLandmark = FindNearLandmarkConnectedToTarget(_nextTarget);
    if (nearLandmark!=nullptr)
        _nextTarget=nearLandmark;

    if (_nextTarget!=nullptr)
        Log->Write("TargetFound");
}

const ptrLandmark CognitiveMap::FindNearLandmarkConnectedToTarget(const ptrLandmark &target)
{
    Landmarks landmarksConnectedToTarget = FindLandmarksConnectedToTarget(target);

    //if target has no connections return nullptr
    if (landmarksConnectedToTarget.empty())
        return nullptr;

    ptrLandmark nearest = nullptr;
    //look for nearest located landmark
    for (ptrLandmark landmark:landmarksConnectedToTarget)
    {
        double mindistance=FLT_MAX;

        Point vector=landmark->GetPosInMap()-_YAHPointer.GetPos();
        double distance = vector.absoluteValue();
        if (distance<=mindistance)
        {
            nearest=landmark;
            mindistance=distance;
        }


    }

    return nearest;
}

Landmarks CognitiveMap::FindLandmarksConnectedToTarget(const ptrLandmark &target)
{
    Landmarks connectedLandmarks = ConnectedWith(target);

    // landmarks directly connected to target

    //for (ptrLandmark )
    //connectedLandmarks.push_back(ConnectedWith(target));

    //Landmarks connected to landmarks connected to target
    Landmarks furtherCandidates;

    for (int i=0; i<connectedLandmarks.size(); ++i)
    {
        furtherCandidates=ConnectedWith(connectedLandmarks[i]);

        for (ptrLandmark candidate : furtherCandidates)
        {
            // if candidate not already taken into account, not visited before or target itself
            if(std::find(connectedLandmarks.begin(), connectedLandmarks.end(), candidate) != connectedLandmarks.end()
                    && std::find(_landmarksRecentlyVisited.begin(), _landmarksRecentlyVisited.end(), candidate) != _landmarksRecentlyVisited.end()
                    && candidate!=target)
            {
                connectedLandmarks.push_back(candidate);
            }
        }
    }
}




