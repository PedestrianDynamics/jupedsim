#ifndef INTERNNAVIGATIONNETWORK_H
#define INTERNNAVIGATIONNETWORK_H


//based on boost

#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include "../../../geometry/SubRoom.h"
#include "../../../geometry/NavLine.h"

typedef boost::adjacency_list<boost::listS, boost::vecS, boost::directedS,
                       boost::no_property, boost::property<boost::edge_weight_t, double> > Graph;
typedef boost::graph_traits<Graph>::vertex_descriptor Vertex;
typedef boost::geometry::model::polygon<Point> BoostPolygon;


typedef std::pair<Vertex, Vertex> Edge;
typedef double Weight;
typedef std::vector<Edge> Edges;



class InternNavigationNetwork
{
public:
    InternNavigationNetwork();
    InternNavigationNetwork(std::shared_ptr<const SubRoom> subRoom);

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
    std::shared_ptr<const SubRoom> _subRoom;
    // current Subroom as boost polygon
    BoostPolygon _currentRoom;

    bool LineIntersectsPolygon(const std::pair<const Point&, const Point&> &line, const boost::geometry::model::polygon<Point> &polygon);






};

#endif // INTERNNAVIGATIONNETWORK_H
