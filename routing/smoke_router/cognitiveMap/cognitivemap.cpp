#include "cognitivemap.h"

#include "geometry/SubRoom.h"
#include "pedestrian/Pedestrian.h"

#include <visilibity.hpp>
#include <chrono>

//for shortest path calculations
//#include <boost/graph/graph_traits.hpp>
//#include <boost/graph/adjacency_list.hpp>
//#include <boost/graph/dijkstra_shortest_paths.hpp>
//#include <boost/geometry/geometry.hpp>
//#include <boost/geometry/algorithms/intersection.hpp>
//#include <boost/foreach.hpp>



CognitiveMap::CognitiveMap()
{

}


CognitiveMap::CognitiveMap(const Building *b, const Pedestrian *ped)
{
    _building=b;
    _ped=ped;
    _network = std::make_shared<GraphNetwork>(b,ped);
    _frame=0;
    _YAHPointer.SetPed(ped);
    _YAHPointer.SetPos(_ped->GetPos());
    _createdWayP=-1;
    //Destination and regions
    _currentRegion=nullptr;
    //Find maindestination in cogmapstorage
    _nextTarget=nullptr;
}

CognitiveMap::~CognitiveMap()
{

}

void CognitiveMap::UpdateMap()
{
    FindCurrentRegion();
    //AddAssociatedLandmarks(TriggerAssociations(LookForLandmarks()));
    SubRoom * sub_room = _building->GetRoom(_ped->GetRoomID())->GetSubRoom(_ped->GetSubRoomID());
    GraphVertex * vertex = (*_network->GetNavigationGraph())[sub_room];
    const GraphVertex::EdgesContainer edges = *(vertex->GetAllEdges());

    for (GraphEdge* edge:edges)
    {
        edge->SetFactor(1,"SpatialKnowledge");
    }

    // Has landmark been reached?
    CheckIfLandmarksReached();

}


void CognitiveMap::UpdateYAHPointer(const Point& move)
{
    double x =  std::fabs(MakeItFuzzy(move._x,0.2*move._x));
    double y =  std::fabs(MakeItFuzzy(move._y,0.2*move._y));
    _YAHPointer.UpdateYAH(Point(x,y));
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

//void CognitiveMap::AddLandmarks(std::vector<ptrLandmark> landmarks)
//{
////    for (ptrLandmark landmark:landmarks)
////    {
////        if (std::find(_landmarks.begin(), _landmarks.end(), landmark)!=_landmarks.end())
////        {
////            continue;
////        }
////        else
////        {
////            _landmarks.push_back(landmark);
////            _landmarks.back()->SetPosInMap(_landmarks.back()->GetPosInMap()-_YAHPointer.GetPosDiff());


////        }
////    }
//}

void CognitiveMap::AddLandmarkInRegion(ptrLandmark landmark, ptrRegion region)
{
    region->AddLandmark(landmark);
}

//std::vector<ptrLandmark> CognitiveMap::LookForLandmarks()
//{
////    SubRoom * sub_room = _building->GetRoom(_ped->GetRoomID())->GetSubRoom(_ped->GetSubRoomID());

////    std::vector<ptrLandmark> landmarks_found;

////    for (ptrLandmark landmark:_landmarksSubConcious)
////    {
////        if (landmark->GetRoom()==sub_room && std::find(_landmarks.begin(), _landmarks.end(), landmark)==_landmarks.end())
////        {
////           landmarks_found.push_back(landmark);
////           LandmarkReached(landmark);
////        }
////    }

////    AddLandmarks(landmarks_found);

////    return landmarks_found;
//}

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

//void CognitiveMap::AddAssociatedLandmarks(Landmarks landmarks)
//{
////    for (ptrLandmark landmark:_waypContainer)
////    {
////        landmark->SetPriority(landmark->GetPriority()+landmarks.size());
////    }
////    int n=0;
////    for (ptrLandmark landmark:landmarks)
////    {
////        if (std::find(_waypContainer.begin(), _waypContainer.end(), landmark)!=_waypContainer.end())
////        {
////            continue;
////        }
////        landmark->SetPriority(n);
////        _waypContainer.push_back(landmark);
////        //Add as new target
////        if (!landmark->Visited())
////            _waypContainerTargetsSorted.push(landmark);
////        //Add as already visited
////        else
////        {
////            LandmarkReached(landmark);
////        }

////        n++;

////    }

//}

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
    for (; it!=edges.end(); ++it)
    {
        double pathLengthDoorWayP = ShortestPathDistance((*it),landmark);
//        Log->Write(std::to_string((*it)->GetCrossing()->GetID()));
//        Log->Write(std::to_string(pathLengthDoorWayP));

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
//    Log->Write(std::to_string(sortedEdges.front()->GetCrossing()->GetID()));
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
        std::vector<Point> points =sub_room->GetPolygon();

        VisiLibity::Polygon boundary=VisiLibity::Polygon();
        VisiLibity::Polygon room=VisiLibity::Polygon();

        //points=StartFromLLCorner(points);


        for (Point point:points)
        {
           room.push_back(VisiLibity::Point(point._x,point._y));
        }

        //MakeItClockwise
        if (room.area()>=0)
            room.reverse();


        // Boundary polygon counter clockwise
        boundary.push_back(VisiLibity::Point(-999999,-999999));
        boundary.push_back(VisiLibity::Point(999999,-999999));
        boundary.push_back(VisiLibity::Point(999999,999999));
        boundary.push_back(VisiLibity::Point(-999999,999999));
        std::vector<VisiLibity::Polygon> polygons;
        polygons.push_back(boundary);
        polygons.push_back(room);

        VisiLibity::Environment environment(polygons);
        //environment.reverse_holes();
        if (!environment.is_valid())
        {
            Log->Write("Error:\tEnvironment for Visibilitypolygon not valid");
            exit(EXIT_FAILURE);
        }

        VisiLibity::Point edgeP(edge->GetCrossing()->GetCentre()._x,edge->GetCrossing()->GetCentre()._y);
        Point pointOnShortestRoute = landmark->PointOnShortestRoute(edge->GetCrossing()->GetCentre());
        //Log->Write(std::to_string(pointOnShortestRoute.GetX())+" "+std::to_string(pointOnShortestRoute.GetY()));
        VisiLibity::Point wayP(pointOnShortestRoute._x,pointOnShortestRoute._y);//,landmark->GetPos().GetY());

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






//    SubRoom* sub_room = _building->GetRoom(_ped->GetRoomID())->GetSubRoom(_ped->GetSubRoomID());
//    std::vector<Point> roomPolygon = sub_room->GetPolygon();

//    typedef boost::geometry::model::polygon<Point> Polygon;

//    Polygon boost_room;

//    for (Point point:roomPolygon)
//    {
//        boost::geometry::append(boost_room,point);
//    }

//    boost::geometry::correct(boost_room);

//    std::vector<Point> graphPoints={};
//    std::vector<Point> helpPoints={};
//    for (Point p:roomPolygon)
//    {
//        helpPoints.clear();
//        helpPoints.push_back(Point(p._x+0.2,p._y+0.2));
//        helpPoints.push_back(Point(p._x-0.2,p._y+0.2));
//        helpPoints.push_back(Point(p._x-0.2,p._y-0.2));
//        helpPoints.push_back(Point(p._x+0.2,p._y-0.2));
//        for (Point helpP:helpPoints)
//        {
//            if (!boost::geometry::intersects(helpP,boost_room))
//            {
//                graphPoints.push_back(helpP);
//            }
//        }
//    }

//    //Graph
//    Graph graph=Graph();

//    //vertices of room
//    for (size_t i=0; i<graphPoints.size(); ++i)
//    {
//        boost::add_vertex(graph);
//    }

//    // center of crossing
//    helpPoints.clear();
//    helpPoints.push_back(Point(edge->GetCrossing()->GetCentre()._x+0.2,edge->GetCrossing()->GetCentre()._y));
//    helpPoints.push_back(Point(edge->GetCrossing()->GetCentre()._x,edge->GetCrossing()->GetCentre()._y+0.2));
//    helpPoints.push_back(Point(edge->GetCrossing()->GetCentre()._x-0.2,edge->GetCrossing()->GetCentre()._y));
//    helpPoints.push_back(Point(edge->GetCrossing()->GetCentre()._x,edge->GetCrossing()->GetCentre()._y-0.2));

//    Vertex startV;

//    for (Point helpP:helpPoints)
//    {
//        if (!boost::geometry::intersects(helpP,boost_room))
//        {
//            graphPoints.push_back(helpP);
//            startV = boost::add_vertex(graph);
//        }
//    }
//    //target (landmark)
//    if (boost::geometry::intersects(landmark->GetPosInMap(),boost_room))
//    {
//        //Log->Write("INFO: Landmark muesste in meinem Raum sein!");
//        // return always 1.0 so no crossing will be preferred based on information
//        // from the cognitive map
//        return 1.0;
//    }
//    graphPoints.push_back(landmark->GetPosInMap());

//    //target (landmark)
//    Vertex targetV = boost::add_vertex(graph);

//    size_t m=0;
//    for (auto it = boost::vertices(graph); it.first != it.second; ++it.first)
//    {
//        size_t n=0;
//        for (auto it2 = boost::vertices(graph); it2.first != it2.second; ++it2.first)
//        {
//            if (*it.first!=*it2.first)
//            {
//                if (!LineIntersectsPolygon(std::pair<Point,Point>(graphPoints[m],graphPoints[n]),boost_room))
//                {
//                    Point vector = graphPoints[m]-graphPoints[n];
//                    double distance=vector.Norm();
//                    //Log->Write((it.first));
//                    boost::add_edge((*it.first),(*it2.first),distance,graph);
//                    //boost::add_edge(*it.second,*it.first,distance,graph);
//                }
//            }

//            ++n;
//        }
//        ++m;
//    }

//    std::vector<double> d(boost::num_vertices(graph));

//    boost::dijkstra_shortest_paths(graph, startV, boost::distance_map(&d[0]));

//    //Log->Write(std::to_string(d[targetV]));
//    return d[targetV];
//}

bool CognitiveMap::LineIntersectsPolygon(const std::pair<Point, Point> &line, const boost::geometry::model::polygon<Point> &polygon)
{

    typedef boost::geometry::model::linestring<Point> Linestring;


    Linestring lineS;
    boost::geometry::append(lineS,line.first);
    boost::geometry::append(lineS,line.second);

    return boost::geometry::intersects(lineS,polygon);

}

const Point &CognitiveMap::GetOwnPos()
{
    return _YAHPointer.GetPos();
}

void CognitiveMap::WriteToFile()
{
//    ++_frame;
//    string data;
//    char tmp[CLENGTH] = "";

//    sprintf(tmp, "<frame ID=\"%d\">\n", _frame);
//    data.append(tmp);


//    for (ptrLandmark landmark:_landmarks)
//    {
//        char tmp1[CLENGTH] = "";
//        sprintf(tmp1, "<landmark ID=\"%d\"\t"
//               "caption=\"%s\"\t"
//               "x=\"%.6f\"\ty=\"%.6f\"\t"
//               "z=\"%.6f\"\t"
//               "rA=\"%.2f\"\trB=\"%.2f\"/>\n",
//               landmark->GetId(), landmark->GetCaption().c_str(), landmark->GetPosInMap().GetX(),
//               landmark->GetPosInMap().GetY(),0.0 ,landmark->GetA(), landmark->GetB());

//        data.append(tmp1);
//    }
//    bool current;

//    for (ptrLandmark landmark:_waypContainer)
//    {
//        current=false;
//        if (!_waypContainerTargetsSorted.empty())
//        {
//            if (landmark==_waypContainerTargetsSorted.top())
//              f  current=true;
//        }


//        char tmp2[CLENGTH] = "";
//        sprintf(tmp2, "<Landmark ID=\"%d\"\t"
//               "caption=\"%s\"\t"
//               "x=\"%.6f\"\ty=\"%.6f\"\t"
//               "z=\"%.6f\"\t"
//               "rA=\"%.2f\"\trB=\"%.2f\"\tcurrent=\"%i\"/>\n",
//               landmark->GetId(),landmark->GetCaption().c_str(), landmark->GetPosInMap().GetX(),
//               landmark->GetPosInMap().GetY(),0.0 ,landmark->GetA(), landmark->GetB(),
//               current);
//        data.append(tmp2);
//    }

//    char tmp3[CLENGTH]="";
//    sprintf(tmp3, "<YAHPointer "
//           "x=\"%.6f\"\ty=\"%.6f\"\t"
//           "z=\"%.6f\"\t"
//           "dir=\"%.2f\"/>\n",
//           _YAHPointer.GetPos().GetX(),
//           _YAHPointer.GetPos().GetY(),0.0 ,_YAHPointer.GetDirection());

//    data.append(tmp3);


//    for (ptrConnection connection:_connections)
//    {
//        char tmp4[CLENGTH]="";
//        sprintf(tmp4, "<connection "
//               "Landmark_landmarkID1=\"%d\"\tLandmark_landmarkID2=\"%d\"/>\n",
//               connection->GetLandmarks().first->GetId(),
//               connection->GetLandmarks().second->GetId());

//        data.append(tmp4);
//    }


//    data.append("</frame>\n");
    //    _outputhandler->WriteToFile(data);
}

double CognitiveMap::MakeItFuzzy(const double &mean, const double &std)
{
    using myClock = std::chrono::high_resolution_clock;
    myClock::duration d = myClock::now().time_since_epoch();

    auto seed = d.count();

    std::default_random_engine generator(seed);
    std::normal_distribution<double> distribution(mean,std);

    double number = distribution(generator);

    return number;
}



//void CognitiveMap::SetNewLandmark()
//{
//    double a= 2.0;
//    double b=2.0;
//    ptrLandmark wayP = std::make_shared<Landmark>(_YAHPointer.GetPos(),a,b,_createdWayP);
//    _createdWayP--;
//    wayP->SetVisited(true);
//    std::vector<ptrLandmark> vec;
//    vec.push_back(wayP);
//    AddLandmarks(vec);
//}

Landmarks CognitiveMap::GetLandmarksConnectedWith(const ptrLandmark &landmark) const
{
    ptrRegion cRegion = GetRegionContaining(landmark);

    if (cRegion!=nullptr)
    {
        return cRegion->ConnectedWith(landmark);
    }
    else
    {
        return Landmarks();
    }
}

const ptrRegion CognitiveMap::GetRegionContaining(const ptrLandmark &landmark) const
{
    for (ptrRegion region:_regions)
    {
        if (region->ContainsLandmark(landmark))
            return region;
    }
    return nullptr;
}

void CognitiveMap::FindCurrentRegion()
{

    //for test purposes. has to be changed
    if (_regions.empty())
        return;

    //needs to be fixed
    _currentRegion=_regions.back();
    return;

    for (ptrRegion region:_regions)
    {
        if (region->Contains(this->_YAHPointer.GetPos()))
        {
            _currentRegion=region;
            return;
        }
    }
    //path searching to region
    _currentRegion=nullptr;
}

void CognitiveMap::CheckIfLandmarksReached()
{
    SubRoom * sub_room = _building->GetRoom(_ped->GetRoomID())->GetSubRoom(_ped->GetSubRoomID());

    if (_currentRegion!=nullptr)
    {
        for (ptrLandmark landmark:_currentRegion->GetLandmarks())
        {
            if (landmark->GetRoom()==sub_room)
            {
//                std::string str1 = landmark->GetCaption()+" has been reached.";
//                Log->Write(str1);
                _landmarksRecentlyVisited.push_back(landmark);
            }
        }
    }
}

const ptrLandmark CognitiveMap::FindConnectionPoint(const ptrRegion &currentRegion, const ptrRegion &targetRegion) const
{
    if (currentRegion!=nullptr && targetRegion!=nullptr)
    {
        for (ptrLandmark landmarka:currentRegion->GetLandmarks())
        {
            for (ptrLandmark landmarkb:targetRegion->GetLandmarks())
            {
                if (landmarka==landmarkb)
                {
                    return landmarka;
                }
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
    if (_currentRegion==nullptr || _targetRegion==nullptr)
    {
        return;
    }
    if (_targetRegion!=_currentRegion)
    {
        _nextTarget=FindConnectionPoint(_currentRegion,_targetRegion);
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

    // Function considers that nearLandmark can be the target itself if no nearer was found.
    ptrLandmark nearLandmark = FindNearLandmarkConnectedToTarget(_nextTarget);

    _nextTarget=nearLandmark;

    //Direct way to target much shorter than via near Landmark?





//    if (_nextTarget!=nullptr)
//        Log->Write(_nextTarget->GetCaption());
}

void CognitiveMap::FindShortCut()
{

}

const ptrLandmark CognitiveMap::FindNearLandmarkConnectedToTarget(const ptrLandmark &target)
{

    Landmarks landmarksConnectedToTarget = FindLandmarksConnectedToTarget(target);

    //if target has no connections return nullptr
    if (landmarksConnectedToTarget.empty())
        return target;

    //ptrLandmark nearest = nullptr;
    //look for nearest located landmark

    //look for landmarks within a circle with the radius searchlimit
    // if no landmarks were found radius will be enlarged
    // if radius = distance(Pos->target) return target

    Point vector=target->GetPosInMap()-_ped->GetPos();//_YAHPointer.GetPos();


    double distanceToTarget=vector.Norm();
    int divisor = 24;
    double searchlimit=distanceToTarget/divisor;
    Landmarks nearLandmarks;
    double shortcutFactor = 2.0;

    while (searchlimit<shortcutFactor*distanceToTarget && nearLandmarks.empty())
    {
        for (ptrLandmark landmark:landmarksConnectedToTarget)
        {

            Point vectorLandmarkTarget=landmark->GetPosInMap()-target->GetPosInMap();
            double distanceLandmarkTarget = vectorLandmarkTarget.Norm();

            vector=landmark->GetPosInMap()-_ped->GetPos();//_YAHPointer.GetPos();

            double distance = vector.Norm();



            if (distance<=searchlimit && distanceLandmarkTarget<distanceToTarget*shortcutFactor)
            {
                nearLandmarks.push_back(landmark);
            }
        }
        searchlimit+=searchlimit;

    }

    if (nearLandmarks.empty())
        return target;

    // select best route to target from one of the nearLandmarks

    return FindBestRouteFromOneOf(nearLandmarks);


}

Landmarks CognitiveMap::FindLandmarksConnectedToTarget(const ptrLandmark &target)
{

    Landmarks connectedLandmarks;

    // landmarks directly connected to target
    Landmarks firstCandidates = GetLandmarksConnectedWith(target);

    for (ptrLandmark candidate:firstCandidates)
    {
        if(std::find(_landmarksRecentlyVisited.begin(), _landmarksRecentlyVisited.end(), candidate) == _landmarksRecentlyVisited.end())
        {
            connectedLandmarks.push_back(candidate);
        }
    }



    //Landmarks connected to landmarks connected to target
    Landmarks furtherCandidates;

    for (size_t i=0; i<connectedLandmarks.size(); ++i)
    {
        furtherCandidates=GetLandmarksConnectedWith(connectedLandmarks[i]);

        for (ptrLandmark candidate : furtherCandidates)
        {
            // if candidate not already taken into account, not visited before or target itself
            if(std::find(connectedLandmarks.begin(), connectedLandmarks.end(), candidate) == connectedLandmarks.end()
                    && std::find(_landmarksRecentlyVisited.begin(), _landmarksRecentlyVisited.end(), candidate) == _landmarksRecentlyVisited.end()
                    && candidate!=target)
            {
                connectedLandmarks.push_back(candidate);

            }
        }
    }
    return connectedLandmarks;
}

const ptrLandmark CognitiveMap::FindBestRouteFromOneOf(const Landmarks &nearLandmarks)
{
    ptrLandmark bestChoice = nullptr;
    double minDistance = FLT_MAX;
    double cDistance;
    for (ptrLandmark landmark:nearLandmarks)
    {
        cDistance=_currentRegion->PathLengthFromLandmarkToTarget(landmark, _nextTarget);
        //Log->Write(landmark->GetCaption());
        //Log->Write(std::to_string(cDistance));
        if (cDistance<minDistance)
        {
            minDistance=cDistance;

            bestChoice=landmark;
        }
    }
    return bestChoice;

}

const ptrLandmark CognitiveMap::GetNearestMainTarget(const Landmarks &mainTargets)
{
    ptrLandmark nearest = mainTargets[0];
    double dNearest = (mainTargets[0]->GetRandomPoint()-GetOwnPos()).Norm();
    for (ptrLandmark landmark:mainTargets)
    {
        if (landmark==nearest)
            continue;

        Point vec = landmark->GetRandomPoint()-GetOwnPos();
        if (vec.Norm()<dNearest)
        {
            dNearest=vec.Norm();
            nearest=landmark;
        }
    }
    return nearest;
}

void CognitiveMap::InitLandmarkNetworksInRegions()
{
    for (ptrRegion region:_regions)
    {
        region->InitLandmarkNetwork();
    }
}




