// Copyright © 2012-2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "IteratorPair.hpp"
#include "Point.hpp"

#include <algorithm>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/detail/adjacency_list.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <fmt/format.h>

#include <iterator>
#include <map>
#include <poly2tri/common/shapes.h>
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
template <typename VertexValue, typename EdgeValue>
class Graph
{
public:
    using VertexId = uint32_t;
    using Type =
        boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS, VertexValue, EdgeValue>;
    using OutEdgeIterator = typename Type::out_edge_iterator;

private:
    Type _graph;
    std::map<VertexId, std::vector<typename Type::vertex_descriptor>> _path_cache;

public:
    class Builder;
    class Exception;
    /// Do not construct a graph directly use the Graph::Builder instead.
    explicit Graph(Type&& graph);
    Graph() = default;
    ~Graph() = default;
    Graph(const Graph& other) = default;
    Graph& operator=(const Graph& other) = default;
    Graph(Graph&& other) = default;
    Graph& operator=(Graph&& other) = default;
    /// Returns the next node on the path to the desired node.
    /// WARNING: Be aware that NO BOUNDS checks are done. Using an unknown id will
    /// result in a out of bounds read.
    /// @param from, vertex id where to start the path.
    /// @param to, vertex id of the destination.
    /// @return vertex id of the next vertex along the path.
    VertexId NextVertexTo(VertexId from, VertexId to);
    std::vector<VertexId> Path(VertexId from, VertexId to);
    /// Read the associated value of vertex
    /// WARNING: Be aware that NO BOUNDS checks are done. Using an unknown id will
    /// result in a out of bounds read.
    /// @param id of vertex to read.
    /// @return VertexValue of vertex with this id.
    const VertexValue& Vertex(VertexId id) const;
    auto Vertices() const;
    const EdgeValue& Edge(VertexId from, VertexId to) const;
    std::vector<EdgeValue> EdgesFor(VertexId id) const;
};

template <typename VertexValue, typename EdgeValue>
class Graph<VertexValue, EdgeValue>::Builder
{
    using Edge = std::pair<VertexId, VertexId>;
    Type _g{};

public:
    Builder() = default;
    ~Builder() = default;
    Builder(const Builder& other) = default;
    Builder& operator=(const Builder& other) = default;
    Builder(Builder&& other) = default;
    Builder& operator=(Builder&& other) = default;
    /// Builds a Graph instance.
    /// The Builder can be reused and is equivallent to a newly constructed Builder
    /// @return the new Graph
    Graph Build();
    /// Adds a new vertex to the graph.
    /// @param value of the new vertex
    /// @return Id of new vertex
    Graph::VertexId AddVertex(const VertexValue& value);
    /// Adds a new edge to the graph.
    /// @param from first vertex of this edge
    /// @param to second vertex of this edge
    /// @param weight of this edge
    void AddEdge(VertexId from, VertexId to, const EdgeValue& value);
};

////////////////////////////////////////////////////////////////////////////////
// Graph
////////////////////////////////////////////////////////////////////////////////
template <typename VertexValue, typename EdgeValue>
Graph<VertexValue, EdgeValue>::Graph(Graph::Type&& graph) : _graph(std::move(graph))
{
}

template <typename VertexValue, typename EdgeValue>
typename Graph<VertexValue, EdgeValue>::VertexId
Graph<VertexValue, EdgeValue>::NextVertexTo(Graph::VertexId from, Graph::VertexId to)
{
    auto iter = _path_cache.find(to);
    if(iter == _path_cache.end()) {
        auto [new_iter, success] = _path_cache.insert(
            {to, std::vector<typename Type::vertex_descriptor>(num_vertices(_graph))});
        assert(success);
        iter = new_iter;

        boost::dijkstra_shortest_paths(
            _graph,
            to,
            boost::predecessor_map(boost::make_iterator_property_map(
                                       iter->second.begin(), get(boost::vertex_index, _graph)))
                .weight_map(boost::get(&EdgeValue::weight, _graph)));
    }

    return iter->second[from];
}

template <typename VertexValue, typename EdgeValue>
std::vector<typename Graph<VertexValue, EdgeValue>::VertexId>
Graph<VertexValue, EdgeValue>::Path(Graph::VertexId from, Graph::VertexId to)
{
    std::vector<VertexId> path{from};
    auto current = from;
    do {
        current = NextVertexTo(current, to);
        path.emplace_back(current);
    } while(current != to);
    return path;
}

template <typename VertexValue, typename EdgeValue>
const VertexValue& Graph<VertexValue, EdgeValue>::Vertex(Graph::VertexId id) const
{
    return _graph[id];
};

template <typename VertexValue, typename EdgeValue>
auto Graph<VertexValue, EdgeValue>::Vertices() const
{
    auto [begin, end] = boost::vertices(_graph);
    return IteratorPair{begin, end};
}

template <typename VertexValue, typename EdgeValue>
const EdgeValue& Graph<VertexValue, EdgeValue>::Edge(Graph::VertexId from, Graph::VertexId to) const
{
    return _graph[boost::edge(from, to, _graph).first];
}

template <typename VertexValue, typename EdgeValue>
std::vector<EdgeValue> Graph<VertexValue, EdgeValue>::EdgesFor(Graph::VertexId id) const
{
    const auto count = boost::out_degree(id, _graph);
    auto [begin, end] = boost::out_edges(id, _graph);
    std::vector<EdgeValue> edges{};
    edges.resize(count);
    std::transform(
        begin, end, std::back_inserter(edges), [this](const auto& iter) { return _graph[iter]; });
    return edges;
}

////////////////////////////////////////////////////////////////////////////////
// Graph::Builder
////////////////////////////////////////////////////////////////////////////////
template <typename VertexValue, typename EdgeValue>
Graph<VertexValue, EdgeValue> Graph<VertexValue, EdgeValue>::Builder::Build()
{
    auto g = Graph{std::move(_g)};
    _g = Type{};
    return g;
}

template <typename VertexValue, typename EdgeValue>
typename Graph<VertexValue, EdgeValue>::VertexId
Graph<VertexValue, EdgeValue>::Builder::AddVertex(const VertexValue& value)
{
    return boost::add_vertex(value, _g);
}

template <typename VertexValue, typename EdgeValue>
void Graph<VertexValue, EdgeValue>::Builder::AddEdge(
    Graph::VertexId from,
    Graph::VertexId to,
    const EdgeValue& value)
{
    boost::add_edge(from, to, value, _g);
}
