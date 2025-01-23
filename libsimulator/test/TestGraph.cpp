// SPDX-License-Identifier: LGPL-3.0-or-later
#include <Graph.hpp>
#include <Point.hpp>

#include <gtest/gtest.h>

TEST(DirectedGraph, CanConstructWithVoidData)
{
    DirectedGraph<>::Builder g{};
    ASSERT_NO_FATAL_FAILURE(g.Build());
}

TEST(DirectedGraph, CanConstructSimpleGraphWithVoidData)
{
    DirectedGraph<>::Builder g{};
    const auto v1 = g.AddVertex();
    const auto v2 = g.AddVertex();
    const auto v3 = g.AddVertex();
    const auto v4 = g.AddVertex();
    g.AddEdge(v1, v2);
    g.AddEdge(v1, v3);
    g.AddEdge(v2, v4);
    g.AddEdge(v3, v4);
    g.AddEdge(v4, v1);
    const auto graph = g.Build();
    ASSERT_EQ(graph.Edges(v1).size(), 2);
    ASSERT_EQ(graph.Edges(v2).size(), 1);
    ASSERT_EQ(graph.Edges(v3).size(), 1);
    ASSERT_EQ(graph.Edges(v4).size(), 1);
}

TEST(DirectedGraph, CanConstructSimpleGraphWithData)
{
    using Graph = DirectedGraph<std::string, std::string>;
    Graph::Builder g{};
    const auto v1 = g.AddVertex("V1");
    const auto v2 = g.AddVertex("V2");
    const auto v3 = g.AddVertex("V3");
    const auto v4 = g.AddVertex("V4");
    g.AddEdge(v1, v2, "V1->V2");
    g.AddEdge(v1, v3, "V1->V3");
    g.AddEdge(v2, v4, "V2->V4");
    g.AddEdge(v3, v4, "V3->V4");
    g.AddEdge(v4, v1, "V4->V1");
    const auto graph = g.Build();
    ASSERT_EQ(graph.Edges(v1).size(), 2);
    ASSERT_EQ(graph.Edges(v2).size(), 1);
    ASSERT_EQ(graph.Edges(v3).size(), 1);
    ASSERT_EQ(graph.Edges(v4).size(), 1);
    ASSERT_EQ(graph.VertexData(v1), "V1");
    ASSERT_EQ(graph.VertexData(v2), "V2");
    ASSERT_EQ(graph.VertexData(v3), "V3");
    ASSERT_EQ(graph.VertexData(v4), "V4");

    std::set<std::string> expected_v1_out_edges{"V1->V2", "V1->V3"};
    std::set<std::string> actual{};
    for(const auto& e : graph.Edges(v1)) {
        actual.insert(std::get<1>(e));
    }
    ASSERT_EQ(expected_v1_out_edges, actual);
}
