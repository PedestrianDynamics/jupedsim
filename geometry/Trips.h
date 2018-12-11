//
// Created by Tobias Schrödter on 23.11.18.
//

#ifndef JPSCORE_TRIPS_H
#define JPSCORE_TRIPS_H

#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <ostream>
#include <vector>

class VertexProperty {
public:
    VertexProperty(): id(-1){}
    VertexProperty(int pId ) : id(pId) { }

    std::string toString() const {
         std::ostringstream oss;
         oss << *this;
         return oss.str();
    }

    int getID() { return id; }

private:
    friend std::ostream& operator<<(std::ostream& os, VertexProperty const& v) {
         return os << "id " << v.id;
    }

    int id;
};

class EdgeProperty {
public:
    EdgeProperty():  distance(-1.), probability(-1.){}

    EdgeProperty(double pDistance, double pProbability) : distance(pDistance), probability(pProbability) {
         distance = pDistance;
         probability = pProbability;
    }
    double getProbabilty() { return probability; }
    double getDistance() { return distance; }

    std::string toString() const {
         std::ostringstream oss;
         oss << *this;
         return oss.str();
    }

private:
    friend std::ostream& operator<<(std::ostream& os, EdgeProperty const& e) {
         return os << "distance " << e.distance << " probability=" << std::fixed << e.probability;
    }

    double distance;
    double probability;
};


using DirectedGraph = boost::adjacency_list<
          boost::vecS,
          boost::vecS,
          boost::directedS,
          VertexProperty,
          EdgeProperty> ;

using VertexItr = boost::graph_traits<DirectedGraph>::vertex_iterator; // Define Vertex iterator
using EdgeItr = boost::graph_traits<DirectedGraph>::edge_iterator; // Define Edge iterator

class Trips {

private:
    DirectedGraph trips;

public:
    void addGoal(int id);

    friend std::ostream& operator<<(std::ostream& os, const Trips& trips);

    void addConnection(int sourceId, int destinationId, EdgeProperty& weight);
    VertexItr getGoal(int id);
    std::vector<int>& getConnections(int id);
    int getNextGoal(int id);
};

#endif //JPSCORE_TRIPS_H