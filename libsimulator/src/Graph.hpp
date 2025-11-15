// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "IteratorPair.hpp"
#include "Point.hpp"

#include <algorithm>
#include <iterator>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

struct Empty {
};

template <typename V = Empty, typename E = Empty>
class DirectedGraph
{
public:
    using VertexId = size_t;
    using VertexIter = typename std::vector<V>::const_iterator;
    using EdgeId = size_t;
    using EdgeIter = typename std::vector<std::tuple<VertexId, E>>::const_iterator;

private:
    struct EdgeAdjacencyInfo {
        size_t offset;
        size_t length;
    };

    std::vector<V> vertex_data{};
    std::vector<EdgeAdjacencyInfo> edge_adjacency_info{};
    std::vector<std::tuple<VertexId, E>> adjacency_list{};

public:
    class Builder
    {
        std::vector<V> vertex_data{};
        std::vector<std::tuple<VertexId, VertexId, E>> edges{};

    public:
        Builder() = default;
        ~Builder() = default;
        VertexId AddVertex(V v = {})
        {
            vertex_data.emplace_back(std::move(v));
            return vertex_data.size() - 1;
        };
        void AddEdge(VertexId from, VertexId to, E e = {})
        {
            edges.emplace_back(from, to, std::move(e));
        }
        DirectedGraph<V, E> Build()
        {
            vertex_data.shrink_to_fit();

            std::sort(std::begin(edges), std::end(edges), [](const auto& a, const auto& b) {
                return std::get<0>(a) < std::get<0>(b);
            });

            std::vector<std::tuple<VertexId, E>> adjacency_list{};
            adjacency_list.reserve(edges.size());

            std::vector<EdgeAdjacencyInfo> edge_adjacency_info{};
            edge_adjacency_info.reserve(vertex_data.size());

            VertexId current_source_id{};
            for(const auto& [from, to, data] : edges) {
                if(edge_adjacency_info.empty()) {
                    current_source_id = from;
                    edge_adjacency_info.push_back({0, 1});

                } else if(from != current_source_id) {
                    current_source_id = from;
                    const auto& last_info = edge_adjacency_info.back();
                    const auto offset = last_info.offset + last_info.length;
                    edge_adjacency_info.push_back({offset, 1});
                } else {
                    edge_adjacency_info.back().length += 1;
                }
                adjacency_list.push_back(std::make_tuple(to, data));
            }

            return DirectedGraph<V, E>{
                std::move(vertex_data), std::move(edge_adjacency_info), std::move(adjacency_list)};
        }
    };

    const auto& VertexData(VertexId id) const
        requires(!std::is_void_v<V>)
    {
        return vertex_data.at(id);
    }

    const std::vector<V>& Vertices() const { return vertex_data; }

    IteratorPair<EdgeIter> Edges(VertexId id) const
    {
        const auto& info = edge_adjacency_info.at(id);
        return {
            std::begin(adjacency_list) + info.offset,
            std::begin(adjacency_list) + info.offset + info.length};
    }

    const E& Edge(VertexId from, VertexId to) const
    {
        for(auto& [id, data] : Edges(from)) {
            if(id == to) {
                return data;
            }
        }
        throw std::out_of_range("Destination edge does not exist");
    }

    VertexId CountVertices() const { return vertex_data.size(); }

    // TODO remove
    DirectedGraph() = default;

private:
    DirectedGraph(
        std::vector<V>&& _vertex_data,
        std::vector<EdgeAdjacencyInfo>&& _edge_adjacency_info,
        std::vector<std::tuple<VertexId, E>>&& _adjacency_list)
        : vertex_data(_vertex_data)
        , edge_adjacency_info(_edge_adjacency_info)
        , adjacency_list(_adjacency_list)
    {
    }
};
