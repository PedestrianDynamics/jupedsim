#include <Graph.hpp>
#include <gtest/gtest.h>

TEST(Graph, CanConstructFromEmptyBuilder)
{
    ASSERT_NO_THROW(auto g = Graph::Builder().Build());
}

TEST(Graph, CanConstructFromBuilder)
{
    Graph::Builder b{};
    const auto vt1 = b.AddVertex({0, 0});
    const auto vt2 = b.AddVertex({1, 1});
    const auto vt3 = b.AddVertex({-1, -1});
    b.AddEdge(vt1, vt2, 1);
    b.AddEdge(vt2, vt3, 1);
    const auto g = b.Build();
    ASSERT_EQ(g.Value(vt1), std::make_tuple(0, 0));
    ASSERT_EQ(g.Value(vt2), std::make_tuple(1, 1));
    ASSERT_EQ(g.Value(vt3), std::make_tuple(-1, -1));
}

TEST(Graph, CanComuteShortestPath)
{
    Graph::Builder b{};
    const auto vt1 = b.AddVertex({0, 0});
    const auto vt2 = b.AddVertex({1, 1});
    const auto vt3 = b.AddVertex({1, 1});
    const auto vt4 = b.AddVertex({1, 1});
    b.AddEdge(vt1, vt2, 1);
    b.AddEdge(vt1, vt3, 2);
    b.AddEdge(vt2, vt4, 1);
    b.AddEdge(vt4, vt4, 1);

    auto g = b.Build();
    ASSERT_EQ(g.NextVertexTo(vt1, vt4), vt2);
    ASSERT_EQ(g.NextVertexTo(vt2, vt4), vt4);
    ASSERT_EQ(g.NextVertexTo(vt4, vt4), vt4);
}
