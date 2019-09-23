#pragma once

#include "connection.h"
#include "landmark.h"

#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>

#include <unordered_map>

typedef boost::adjacency_list<boost::listS, boost::vecS, boost::directedS,
                       boost::no_property, boost::property<boost::edge_weight_t, double> > Graph;
typedef boost::graph_traits<Graph>::vertex_descriptor Vertex;
typedef std::pair<Vertex, Vertex> Edge;
typedef double Weight;
typedef std::vector<Edge> Edges;


class AILandmarkNetwork
{
public:
    AILandmarkNetwork();
    AILandmarkNetwork(const AIRegion *region, const AILandmarks& landmarks, const std::vector<AIConnection> &connections);
    ~AILandmarkNetwork();
    void AddLandmark(const AILandmark* landmark);
    void RemoveLandmark(const AILandmark* landmark);
    void AddConnection(const AIConnection* connection);

    //Calculations
    std::pair<std::vector<const AILandmark *>, double> LengthofShortestPathToTarget(const AILandmark *landmark, const AILandmark *target) const;


private:
    Graph _graph;
    std::unordered_map<const AILandmark*,Vertex> _landmarks; //vertices
    std::list<std::pair<Edge,Weight>> _connections; //edges; Weight equals length
                                                    //between two random point in the connected landmarks
    const AIRegion* _region;

    void RemoveAdjacentEdges(const Vertex& vertex);



};
