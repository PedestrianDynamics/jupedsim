#pragma once

#include "geometry/SubRoom.h"
#include "geometry/NavLine.h"

#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>

typedef boost::adjacency_list<boost::listS, boost::vecS, boost::directedS,
                       boost::no_property, boost::property<boost::edge_weight_t, double> > Graph;
typedef boost::graph_traits<Graph>::vertex_descriptor Vertex;
typedef boost::geometry::model::polygon<Point> BoostPolygon;
typedef boost::geometry::model::linestring<Point> Linestring;


typedef std::pair<Vertex, Vertex> Edge;
typedef double Weight;
typedef std::vector<Edge> Edges;



class InternNavigationNetwork
{
public:
    InternNavigationNetwork();
    InternNavigationNetwork(const SubRoom* subRoom);

    void AddVertex(const NavLine* navLine);  // add navline (crossing / transition or HLine)
    //void RemoveVertex(ptrNavLine navLine);
    void AddEdge(const NavLine* navLine1, const NavLine* navLine2); // add connection (edge) between two navlines. Function should only be
    // used if each of the navline is visible from the position of the other.
    //void RemoveEdge(ptrNavLine navLine1, ptrNavLine navLine2);

    const NavLine* GetNextNavLineOnShortestPathToTarget(const Point& pos, const NavLine* target);

    //Create edges (connection (edge) will be established if two vertices are visible from each other
    void EstablishConnections();



private:
    Graph _graph;
    std::list<std::pair<const NavLine*,Vertex>> _navLines; //vertices
    std::list<std::pair<Edge,Weight>> _connections; //edges; Weight equals length
    const SubRoom* _subRoom;
    // current Subroom as boost polygon
    std::vector<Linestring> _currentRoom;

    bool LineIntersectsWalls(const std::pair<const Point&, const Point&> &line, const std::vector<Linestring> &walls);
};
