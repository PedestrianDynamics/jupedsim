#include "Graph.hpp"

#include <boost/graph/detail/adjacency_list.hpp>
#include <fmt/format.h>

Graph::Graph(Graph::Type&& graph) : _graph(std::move(graph))
{
}

Graph::VertexId Graph::NextVertexTo(Graph::VertexId from, Graph::VertexId to)
{
    auto iter = _path_cache.find(to);
    if(iter == _path_cache.end()) {
        auto [new_iter, success] =
            _path_cache.insert({to, std::vector<Type::vertex_descriptor>(num_vertices(_graph))});
        assert(success);
        iter = new_iter;
    }

    boost::dijkstra_shortest_paths(
        _graph,
        to,
        boost::predecessor_map(boost::make_iterator_property_map(
            iter->second.begin(), get(boost::vertex_index, _graph))));

    return iter->second[from];
}

Graph::VertexValue Graph::Value(Graph::VertexId id) const
{
    return _graph[id];
};

Graph Graph::Builder::Build()
{
    auto g = Graph{std::move(_g)};
    _g = Type{};
    return g;
}

Graph::VertexId Graph::Builder::AddVertex(Graph::VertexValue value)
{
    return boost::add_vertex(value, _g);
}

void Graph::Builder::AddEdge(Graph::VertexId from, Graph::VertexId to, Graph::EdgeWeight weight)
{
    boost::add_edge(from, to, weight, _g);
}
