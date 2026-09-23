// SPDX-License-Identifier: LGPL-3.0-or-later
#include "Geometry/WalkableSurface.hpp"

#include <boost/graph/adjacency_list.hpp>
#include <gtest/gtest.h>

#include <vector>

namespace
{
Poly make_poly(const std::vector<Point2D>& points)
{
    return Poly(std::begin(points), std::end(points));
}

/// Ground floor (counterclockwise input) and upper floor (clockwise input), connected by stairs.
struct TwoFloors {
    WalkableSurface surface{};
    size_t ground = surface.AddRegion({{{0, 0}, {5, 0}, {5, 5}, {0, 5}}, {}}, 0.0);
    size_t upper = surface.AddRegion({{{10, 0}, {10, 5}, {15, 5}, {15, 0}}, {}}, 3.0);
    size_t stairs = surface.ConnectRegions(ground, {{5, 0}, {5, 5}}, upper, {{10, 0}, {10, 5}});
    const Point2D onGround{2.5, 2.5};
    const Point2D onStairs{7.5, 2.5};
    const Point2D onUpper{12.5, 2.5};
};

/// Checks that the seam (between regions "from" and "to") connects "a" and "b" and that the point
/// "inside" is left to the seam (has to be in region "from").
void expect_seam(
    const WalkableSurface::RegionGraph2D& graph,
    size_t from,
    size_t to,
    const Point2D& a,
    const Point2D& b,
    const Point2D& inside)
{
    const auto [edge, exists] = boost::edge(from, to, graph);
    ASSERT_TRUE(exists);
    const auto& seam = graph[edge];
    const PolyWithHoles& poly = graph[from];
    ASSERT_LE(seam.ring, poly.number_of_holes());
    const Poly& ring = (seam.ring == 0) ? poly.outer_boundary() : poly.holes()[seam.ring - 1];
    ASSERT_LT(seam.index, ring.size());

    const Point2D& source = ring.vertex(seam.index);
    const Point2D& target = ring.vertex((seam.index + 1) % ring.size());
    EXPECT_TRUE(source == a && target == b);
    EXPECT_EQ(CGAL::orientation(source, target, inside), CGAL::LEFT_TURN);
}
} // namespace

TEST(WalkableSurface, UnconnectedFloors)
{
    WalkableSurface surface{};
    const auto first = surface.AddRegion({{{0, 0}, {1, 0}, {1, 1}, {0, 1}}, {}}, 0.0);
    const auto second = surface.AddRegion({{{0, 0}, {1, 0}, {1, 1}, {0, 1}}, {}}, 3.0);

    const auto graph = surface.CreateRegionGraph2D();
    EXPECT_EQ(boost::num_vertices(*graph), 2);
    EXPECT_EQ(boost::num_edges(*graph), 0);
    EXPECT_EQ(first, 0);
    EXPECT_EQ(second, 1);
}

TEST(WalkableSurface, PolygonRingsHaveCorrectOrientation)
{
    WalkableSurface surface{};
    const auto region = surface.AddRegion(
        {{{0, 0}, {0, 10}, {10, 10}, {10, 0}}, {{{2, 2}, {4, 2}, {4, 4}, {2, 4}}}}, 0.0);

    const auto graph = surface.CreateRegionGraph2D();
    const PolyWithHoles& poly = (*graph)[region];
    // boundaryshould be counterclockwise, hole clockwise.
    EXPECT_EQ(poly.outer_boundary(), make_poly({{0, 0}, {10, 0}, {10, 10}, {0, 10}}));
    ASSERT_EQ(poly.number_of_holes(), 1);
    EXPECT_EQ(poly.holes()[0], make_poly({{2, 2}, {2, 4}, {4, 4}, {4, 2}}));
}

TEST(WalkableSurface, CheckStairsPoly)
{
    TwoFloors floors{};

    const auto graph = floors.surface.CreateRegionGraph2D();
    ASSERT_EQ(boost::num_vertices(*graph), 3);
    const PolyWithHoles& stairs = (*graph)[floors.stairs];
    EXPECT_EQ(stairs.outer_boundary(), make_poly({{5, 0}, {10, 0}, {10, 5}, {5, 5}}));
    EXPECT_EQ(stairs.number_of_holes(), 0);
}

TEST(WalkableSurface, SeamsOfConnection)
{
    TwoFloors floors{};

    const auto graph = floors.surface.CreateRegionGraph2D();
    EXPECT_EQ(boost::num_edges(*graph), 4); // both directions at each seam
    EXPECT_FALSE(boost::edge(floors.ground, floors.upper, *graph).second);
    EXPECT_FALSE(boost::edge(floors.upper, floors.ground, *graph).second);

    expect_seam(*graph, floors.ground, floors.stairs, {5, 0}, {5, 5}, floors.onGround);
    expect_seam(*graph, floors.stairs, floors.ground, {5, 5}, {5, 0}, floors.onStairs);
    expect_seam(*graph, floors.stairs, floors.upper, {10, 0}, {10, 5}, floors.onStairs);
    expect_seam(*graph, floors.upper, floors.stairs, {10, 5}, {10, 0}, floors.onUpper);
}

TEST(WalkableSurface, SeamOnHoleEdge)
{
    WalkableSurface surface{};
    const auto ground = surface.AddRegion(
        {{{0, 0}, {20, 0}, {20, 10}, {0, 10}},
         {{{2, 2}, {18, 2}, {18, 8}, {2, 8}, {2, 7}, {2, 3}}}},
        0.0);
    const auto upper = surface.AddRegion({{{12, 3}, {17, 3}, {17, 7}, {12, 7}}, {}}, 1.0);
    const auto stairs = surface.ConnectRegions(ground, {{2, 3}, {2, 7}}, upper, {{12, 3}, {12, 7}});

    const auto graph = surface.CreateRegionGraph2D();
    const auto [edge, exists] = boost::edge(ground, stairs, *graph);
    ASSERT_TRUE(exists);
    EXPECT_EQ((*graph)[edge].ring, 1);
    expect_seam(*graph, ground, stairs, {2, 3}, {2, 7}, {1, 5});
    expect_seam(*graph, stairs, ground, {2, 7}, {2, 3}, {7, 5});
}
