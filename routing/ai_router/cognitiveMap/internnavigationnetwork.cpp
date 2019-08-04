#include "internnavigationnetwork.h"

#include "geometry/Obstacle.h"
#include "geometry/Wall.h"

#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <boost/geometry.hpp>


InternNavigationNetwork::InternNavigationNetwork()
{
    _graph=Graph();
}

InternNavigationNetwork::InternNavigationNetwork(const SubRoom *subRoom)
{
    _graph=Graph();

    _subRoom=subRoom;

    for (Wall wall:_subRoom->GetAllWalls())
    {
        Linestring lineS;
        boost::geometry::append(lineS,wall.GetPoint1());
        boost::geometry::append(lineS,wall.GetPoint2());
        _currentRoom.push_back(lineS);

    }

    const std::vector<Obstacle* > obstacles = _subRoom->GetAllObstacles();

    for (Obstacle* obstacle:obstacles)
    {
        for (Wall wall: obstacle->GetAllWalls())
        {
            Linestring lineS;
            boost::geometry::append(lineS,wall.GetPoint1());
            boost::geometry::append(lineS,wall.GetPoint2());
            _currentRoom.push_back(lineS);
        }
    }
}

void InternNavigationNetwork::AddVertex(const NavLine *navLine)
{
    Vertex v = boost::add_vertex(_graph);
    _navLines.push_back(std::pair<const NavLine*,Vertex> (navLine,v));
}


void InternNavigationNetwork::AddEdge(const NavLine *navLine1, const NavLine *navLine2)
{
    //find indeces of vertices(landmarks) in graph
    Vertex A{};
    Vertex B{};

    for (auto it=_navLines.begin(); it!=_navLines.end(); ++it)
    {
        int counter=0;
        if (it->first==navLine1)
        {
            A = it->second;
            counter++;
            if (counter==2)
                break;
        }
        else if (it->first==navLine2)
        {
            B = it->second;
            counter++;
            if (counter==2)
                break;
        }
    }

    //vector between navLines
    Point vector = navLine1->GetCentre()-navLine2->GetCentre();
    //distance between navLines
    double distance = vector.Norm();
    _connections.push_back(std::pair<Edge,Weight>(Edge(A,B),distance));

    boost::add_edge(A,B,distance,_graph);
    boost::add_edge(B,A,distance,_graph);
}

const NavLine *InternNavigationNetwork::GetNextNavLineOnShortestPathToTarget(const Point &pos, const NavLine *target)
{
    // new vertex for the actual position of the pedestrian
    Vertex v = boost::add_vertex(_graph);

    // check whether actual position has intervisual contact with doors or hlines

    //building lines from center points of the navlines and pos and check if those lines intersect the polygon representing the current room
    for (auto &it:_navLines)
    {
        if (!LineIntersectsWalls(std::make_pair<const Point&,const Point&>(it.first->GetCentre(),pos),_currentRoom))
        {
            //vector between navLines
            Point vector = pos-target->GetCentre();
            //distance between navLines
            double distance = vector.Norm();

            boost::add_edge(v,it.second,distance,_graph);
            boost::add_edge(it.second,v,distance,_graph);
        }
    }

    //determine Vertex belonging to ptrNavline target
    int targetVertex=-1;

    for (auto it=_navLines.begin(); it!=_navLines.end(); ++it)
    {
        if (it->first==target)
        {
            targetVertex=it->second;
            break;
        }
    }

    if (targetVertex==-1)
        return nullptr;

    // determine shortest path to target
    std::vector<Vertex> pMap(boost::num_vertices(_graph));
    boost::dijkstra_shortest_paths(_graph, targetVertex, boost::predecessor_map(&pMap[0]));
    // next vertex on shortest path:
    Vertex predecessor = pMap[v];


    //remove pos from graph network
    boost::clear_vertex(v,_graph);
    boost::remove_vertex(v,_graph);

    //return navline belonging to predecessor vertex on shortest path
    for (auto it=_navLines.begin(); it!=_navLines.end(); ++it)
    {
        if (it->second==predecessor)
        {
             return it->first;
        }
    }

    return nullptr;

}

void InternNavigationNetwork::EstablishConnections()
{

    // Check which navlines have (vice-versa) visible connectivity

    //building lines from center points of the navlines and check if those lines intersect the polygon representing the current room
    for (auto it=_navLines.begin(); it!=_navLines.end();++it)
    {
        auto it2=it;
        it2++;
        for (; it2!=_navLines.end(); ++it2)
        {
            if (it->first!=it2->first)
            {
                if (!LineIntersectsWalls(std::make_pair<const Point&,const Point&>(it->first->GetCentre(),it2->first->GetCentre()),_currentRoom))
                {
                   AddEdge(it->first,it2->first);
                }
            }
        }
    }
}

bool InternNavigationNetwork::LineIntersectsWalls(const std::pair<const Point&, const Point&> &line, const std::vector<Linestring> &walls)
{

    Linestring lineS;
    boost::geometry::append(lineS,line.first);
    boost::geometry::append(lineS,line.second);

    for (Linestring wall:walls)
    {
       if (boost::geometry::intersects(wall,lineS))
           return true;
    }

    return false;

}
