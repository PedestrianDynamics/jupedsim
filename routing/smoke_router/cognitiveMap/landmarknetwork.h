#pragma once

#include "landmark.h"
#include "connection.h"

#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>

typedef boost::adjacency_list<boost::listS, boost::vecS, boost::directedS,
                       boost::no_property, boost::property<boost::edge_weight_t, double> > Graph;
typedef boost::graph_traits<Graph>::vertex_descriptor Vertex;
typedef std::pair<Vertex, Vertex> Edge;
typedef double Weight;
typedef std::vector<Edge> Edges;

using ptrConnection = std::shared_ptr<Connection>;

class LandmarkNetwork
{
public:
    LandmarkNetwork();
    LandmarkNetwork(const Landmarks& landmarks, const std::vector<ptrConnection>& connections);
    ~LandmarkNetwork();
    void AddLandmark(const ptrLandmark& landmark);
    void RemoveLandmark(const ptrLandmark& landmark);
    void AddConnection(const ptrConnection& connection);

    //Calculations
    double LengthofShortestPathToTarget(const ptrLandmark& landmark, const ptrLandmark &target) const;


private:
    Graph _graph;
    std::list<std::pair<ptrLandmark,Vertex>> _landmarks; //vertices
    std::list<std::pair<Edge,Weight>> _connections; //edges; Weight equals length
                                                    //between two random point in the connected landmarks

    void RemoveAdjacentEdges(const Vertex& vertex);
};
