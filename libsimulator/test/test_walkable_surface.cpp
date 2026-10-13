// SPDX-License-Identifier: LGPL-3.0-or-later
#include "geometry/walkable_surface.hpp"
#include "geometry_fixtures.hpp"
#include "simulation_error.hpp"

#include <boost/graph/adjacency_list.hpp>
#include <gtest/gtest.h>

#include <limits>
#include <vector>

namespace
{
Poly make_poly(const std::vector<Point2D>& points)
{
    return Poly(std::begin(points), std::end(points));
}

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
    const auto first = surface.add_region({{{0, 0}, {1, 0}, {1, 1}, {0, 1}}, {}}, 0.0);
    const auto second = surface.add_region({{{0, 0}, {1, 0}, {1, 1}, {0, 1}}, {}}, 3.0);

    const auto graph = surface.create_region_graph_2d();
    EXPECT_EQ(boost::num_vertices(*graph), 2);
    EXPECT_EQ(boost::num_edges(*graph), 0);
    EXPECT_EQ(first, 0);
    EXPECT_EQ(second, 1);
}

TEST(WalkableSurface, NonFiniteCoordinatesAreRejected)
{
    const WalkableSurface::Polygon square{{{0, 0}, {1, 0}, {1, 1}, {0, 1}}, {}};
    for(const double bad :
        {std::numeric_limits<double>::infinity(), std::numeric_limits<double>::quiet_NaN()}) {
        const WalkableSurface::Polygon with_bad_corner{{{0, 0}, {bad, 0}, {1, 1}, {0, 1}}, {}};
        WalkableSurface surface{};
        EXPECT_THROW(surface.add_region(with_bad_corner, 0.0), SimulationError) << bad;
        EXPECT_THROW(surface.add_region(square, bad), SimulationError) << bad;
    }
}

TEST(WalkableSurface, PolygonRingsHaveCorrectOrientation)
{
    WalkableSurface surface{};
    const auto region = surface.add_region(
        {{{0, 0}, {0, 10}, {10, 10}, {10, 0}}, {{{2, 2}, {4, 2}, {4, 4}, {2, 4}}}}, 0.0);

    const auto graph = surface.create_region_graph_2d();
    const PolyWithHoles& poly = (*graph)[region];
    // boundary should be counterclockwise, hole clockwise.
    EXPECT_EQ(poly.outer_boundary(), make_poly({{0, 0}, {10, 0}, {10, 10}, {0, 10}}));
    ASSERT_EQ(poly.number_of_holes(), 1);
    EXPECT_EQ(poly.holes()[0], make_poly({{2, 2}, {2, 4}, {4, 4}, {4, 2}}));
}

TEST(WalkableSurface, CheckStairsPoly)
{
    const auto stair = test_geometries::straight_stair_to_a_landing();
    const auto* graph = stair.geometry->region_graph_2d();
    ASSERT_NE(graph, nullptr);
    ASSERT_EQ(boost::num_vertices(*graph), 3);
    const PolyWithHoles& flight = (*graph)[stair.connector];
    EXPECT_EQ(flight.outer_boundary(), make_poly({{10, 0}, {15, 0}, {15, 8}, {10, 8}}));
    EXPECT_EQ(flight.number_of_holes(), 0);
}

TEST(WalkableSurface, SeamsOfConnection)
{
    const auto stair = test_geometries::straight_stair_to_a_landing();
    const Point2D on_ground{5, 4};
    const Point2D on_flight{12.5, 4};
    const Point2D on_landing{17.5, 4};

    const auto* graph = stair.geometry->region_graph_2d();
    ASSERT_NE(graph, nullptr);
    EXPECT_EQ(boost::num_edges(*graph), 4); // both directions at each seam
    EXPECT_FALSE(boost::edge(stair.room_1, stair.room_2, *graph).second);
    EXPECT_FALSE(boost::edge(stair.room_2, stair.room_1, *graph).second);

    expect_seam(*graph, stair.room_1, stair.connector, {10, 0}, {10, 8}, on_ground);
    expect_seam(*graph, stair.connector, stair.room_1, {10, 8}, {10, 0}, on_flight);
    expect_seam(*graph, stair.connector, stair.room_2, {15, 0}, {15, 8}, on_flight);
    expect_seam(*graph, stair.room_2, stair.connector, {15, 8}, {15, 0}, on_landing);
}

TEST(WalkableSurface, SeamOnHoleEdge)
{
    WalkableSurface surface{};
    const auto ground = surface.add_region(
        {{{0, 0}, {20, 0}, {20, 10}, {0, 10}},
         {{{2, 2}, {18, 2}, {18, 8}, {2, 8}, {2, 7}, {2, 3}}}},
        0.0);
    const auto upper = surface.add_region({{{12, 3}, {17, 3}, {17, 7}, {12, 7}}, {}}, 1.0);
    const auto stairs =
        surface.connect_regions(ground, {{2, 3}, {2, 7}}, upper, {{12, 3}, {12, 7}});

    const auto graph = surface.create_region_graph_2d();
    const auto [edge, exists] = boost::edge(ground, stairs, *graph);
    ASSERT_TRUE(exists);
    EXPECT_EQ((*graph)[edge].ring, 1);
    expect_seam(*graph, ground, stairs, {2, 3}, {2, 7}, {1, 5});
    expect_seam(*graph, stairs, ground, {2, 7}, {2, 3}, {7, 5});
}
