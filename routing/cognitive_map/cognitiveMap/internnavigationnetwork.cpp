#include "internnavigationnetwork.h"

#include <boost/geometry.hpp>

InternNavigationNetwork::InternNavigationNetwork()
{
    _graph=Graph();
}

InternNavigationNetwork::InternNavigationNetwork(ptrSubRoom subRoom)
{
    _graph=Graph();

    _subRoom=subRoom;
}

void InternNavigationNetwork::AddVertex(ptrNavLine navLine)
{
    Vertex v = boost::add_vertex(_graph);
    _navLines.push_back(std::pair<ptrNavLine,Vertex> (navLine,v));
}

void InternNavigationNetwork::AddEdge(ptrNavLine navLine1, ptrNavLine navLine2)
{
    //find indeces of vertices(landmarks) in graph
    Vertex A;
    Vertex B;

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

ptrNavLine InternNavigationNetwork::GetNextNavLineOnShortestPathToTarget(ptrNavLine start, ptrNavLine target)
{
    int startVertex=-1;
    int targetVertex=-1;

    // get the start vertex


    for (auto it=_navLines.begin(); it!=_navLines.end(); ++it)
    {

        int counter=0;
        if (it->first==start)
        {
            startVertex=it->second;
            counter++;
            if (counter==2)
                break;
        }
        else if (it->first==target)
        {
            targetVertex=it->second;
            counter++;
            if (counter==2)
                break;
        }

    }


    if (targetVertex==-1 || startVertex==-1)
        return nullptr;

    // to be continued
}

void InternNavigationNetwork::EstablishConnections()
{

    // Check which navlines have (vice-versa) visible connectivity

    //Boost polygon
    typedef boost::geometry::model::polygon<Point> BoostPolygon;

    // current Subroom as boost polygon
    BoostPolygon currentRoom;

    for (Point point:_subRoom->GetPolygon())
    {
        boost::geometry::append(currentRoom,point);
    }
    //building lines from center points of the navlines and check if those lines intersect the polygon representing the current room
    for (auto &it:_navLines)
    {
        for (auto &it2:_navLines)
        {
            if (it.first!=it2.first)
            {
                if (!LineIntersectsPolygon(std::make_pair<const Point&,const Point&>(it.first->GetCentre(),it2.first->GetCentre()),currentRoom))
                {
                   AddEdge(it.first,it2.first);
                }
            }
        }
    }


}

bool InternNavigationNetwork::LineIntersectsPolygon(const std::pair<const Point&, const Point&> &line, const boost::geometry::model::polygon<Point> &polygon)
{
    typedef boost::geometry::model::linestring<Point> Linestring;

    Linestring lineS;
    boost::geometry::append(lineS,line.first);
    boost::geometry::append(lineS,line.second);

    return boost::geometry::intersects(lineS,polygon);

}
