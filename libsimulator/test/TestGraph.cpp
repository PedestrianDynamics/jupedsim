// Copyright © 2012-2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#include <Graph.hpp>
#include <Point.hpp>

#include <gtest/gtest.h>

struct Edge {
    double weight;
};
using GraphType = Graph<int, Edge>;

TEST(Graph, CanConstructFromEmptyBuilder)
{
    ASSERT_NO_THROW(auto g = GraphType::Builder().Build());
}

TEST(Graph, CanConstructFromBuilder)
{
    GraphType::Builder b{};
    const auto vt1 = b.AddVertex(1);
    const auto vt2 = b.AddVertex(2);
    const auto vt3 = b.AddVertex(3);
    b.AddEdge(vt1, vt2, {1});
    b.AddEdge(vt2, vt3, {1});
    const auto g = b.Build();
    ASSERT_EQ(g.Vertex(vt1), 1);
    ASSERT_EQ(g.Vertex(vt2), 2);
    ASSERT_EQ(g.Vertex(vt3), 3);
}

TEST(Graph, CanComuteShortestPath)
{
    GraphType::Builder b{};
    const auto vt1 = b.AddVertex(1);
    const auto vt2 = b.AddVertex(2);
    const auto vt3 = b.AddVertex(3);
    const auto vt4 = b.AddVertex(4);
    b.AddEdge(vt1, vt2, {99});
    b.AddEdge(vt2, vt1, {99});

    b.AddEdge(vt2, vt3, {1});
    b.AddEdge(vt3, vt1, {1});

    b.AddEdge(vt3, vt4, {1});
    b.AddEdge(vt4, vt3, {1});

    b.AddEdge(vt1, vt4, {1});
    b.AddEdge(vt4, vt1, {1});

    auto g = b.Build();
    ASSERT_EQ(g.NextVertexTo(vt1, vt4), vt4);
    ASSERT_EQ(g.NextVertexTo(vt4, vt4), vt4);
}
