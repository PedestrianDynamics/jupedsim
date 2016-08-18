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
typedef std::pair<Vertex, Vertex> Edge;
typedef double Weight;
typedef std::vector<Edge> Edges;

using ptrNavLine = std::shared_ptr<const NavLine>;
using ptrSubRoom = std::shared_ptr<const SubRoom>;

class InternNavigationNetwork
{
public:
    InternNavigationNetwork();
    InternNavigationNetwork(ptrSubRoom subRoom);

    void AddVertex(ptrNavLine navLine);  // add navline (crossing / transition or HLine)
    //void RemoveVertex(ptrNavLine navLine);
    void AddEdge(ptrNavLine navLine1, ptrNavLine navLine2); // add connection (edge) between two navlines. Function should only be
    // used if each of the navline is visible from the position of the other.
    //void RemoveEdge(ptrNavLine navLine1, ptrNavLine navLine2);

    ptrNavLine GetNextNavLineOnShortestPathToTarget(ptrNavLine start, ptrNavLine target);

    //Create edges (connection (edge) will be established if two vertices are visible from each other
    void EstablishConnections();



private:
    Graph _graph;
    std::list<std::pair<ptrNavLine,Vertex>> _navLines; //vertices
    std::list<std::pair<Edge,Weight>> _connections; //edges; Weight equals length
    ptrSubRoom _subRoom;

    bool LineIntersectsPolygon(const std::pair<const Point&, const Point&> &line, const boost::geometry::model::polygon<Point> &polygon);






};

#endif // INTERNNAVIGATIONNETWORK_H
