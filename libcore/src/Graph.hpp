#pragma once

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>


/// Represents a undirected graph with edge weights and vertices with
/// coordinates (tuple) as data. Shortes path query is avaibable as method.
/// To create a Graph use the nested Builder class, see Graph::Builder.
///
/// Design notes:
/// * This class is a thin layer over boost::graph mostly to hide the
///   complexity of boost::graph.
/// * Shortes path querries cache results internally so that repeated
///   querries for the same target node are constant time lookups
class Graph
{
public:
    using VertexId    = uint32_t;
    using EdgeWeight  = double;
    using VertexValue = std::tuple<double, double>;
    using Type        = boost::adjacency_list<
        boost::vecS,
        boost::vecS,
        boost::undirectedS,
        std::tuple<double, double>,
        boost::property<boost::edge_weight_t, int>>;

private:
    Type _graph;
    std::map<VertexId, std::vector<Type::vertex_descriptor>> _path_cache;

public:
    class Builder;
    class Exception;
    /// Do not construct a graph directly use the Graph::Builder instead.
    explicit Graph(Type && graph);
    Graph()                    = default;
    ~Graph()                   = default;
    Graph(const Graph & other) = default;
    Graph & operator=(const Graph & other) = default;
    Graph(Graph && other)                  = default;
    Graph & operator=(Graph && other) = default;
    /// Returns the next node on the path to the desired node.
    /// WARNING: Be aware that NO BOUNDS checks are done. Using an unknown id will
    /// result in a out of bounds read.
    /// @param from, vertex id where to start the path.
    /// @param to, vertex id of the destination.
    /// @return vertex id of the next vertex along the path.
    VertexId NextVertexTo(VertexId from, VertexId to);
    /// Read the associated value of vertex
    /// WARNING: Be aware that NO BOUNDS checks are done. Using an unknown id will
    /// result in a out of bounds read.
    /// @param id of vertex to read.
    /// @return VertexValue of vertex with this id.
    VertexValue Value(VertexId id) const;
};

class Graph::Builder
{
    using Edge = std::pair<VertexId, VertexId>;
    Type _g{};

public:
    Builder()                      = default;
    ~Builder()                     = default;
    Builder(const Builder & other) = default;
    Builder & operator=(const Builder & other) = default;
    Builder(Builder && other)                  = default;
    Builder & operator=(Builder && other) = default;
    /// Builds a Graph instance.
    /// The Builder can be reused and is equivallent to a newly constructed Builder
    /// @return the new Graph
    Graph Build();
    /// Adds a new vertex to the graph.
    /// @param value of the new vertex
    /// @return Id of new vertex
    Graph::VertexId AddVertex(VertexValue value);
    /// Adds a new edge to the graph.
    /// @param from first vertex of this edge
    /// @param to second vertex of this edge
    /// @param weight of this edge
    void AddEdge(VertexId from, VertexId to, double weight);
};
