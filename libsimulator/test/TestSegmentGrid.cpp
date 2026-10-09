// SPDX-License-Identifier: LGPL-3.0-or-later
#include "Geometry/SegmentGrid.hpp"
#include "LineSegment.hpp"
#include "Point.hpp"
#include "TestCommon.hpp"

#include <fmt/format.h>
#include <fmt/ranges.h>
#include <gtest/gtest.h>

#include <cstddef>
#include <iterator>
#include <set>
#include <vector>

namespace
{
/// A single vertical wall from (0,0) to (0,10).
SegmentGrid single_vertical_wall()
{
    return SegmentGrid{std::vector<LineSegment>{LineSegment{{0., 0.}, {0., 10.}}}};
}
} // namespace

TEST(SegmentGrid, IntersectsAnyDetectsACrossingSegment)
{
    const auto grid = single_vertical_wall();
    EXPECT_TRUE(grid.intersects_any(LineSegment{{-1., 5.}, {1., 5.}}));
}

TEST(SegmentGrid, IntersectsAnyRejectsAFarAwaySegment)
{
    const auto grid = single_vertical_wall();
    EXPECT_FALSE(grid.intersects_any(LineSegment{{5., 5.}, {7., 5.}}));
}

TEST(SegmentGrid, IntersectsAnyRejectsANearButParallelSegment)
{
    // Shares grid cells with the wall but never crosses it.
    const auto grid = single_vertical_wall();
    EXPECT_FALSE(grid.intersects_any(LineSegment{{1., 0.}, {1., 10.}}));
}

TEST(SegmentGrid, DistanceQueryReturnsSegmentsWithinRange)
{
    const auto grid = single_vertical_wall();
    // Perpendicular distance from (1,5) to the wall is 1.
    EXPECT_EQ(std::ranges::distance(grid.line_segments_in_distance_to(2., {1., 5.})), 1);
}

TEST(SegmentGrid, DistanceQueryExcludesSegmentsOutOfRange)
{
    const auto grid = single_vertical_wall();
    // Perpendicular distance from (3,3) to the wall is 3.
    EXPECT_TRUE(grid.line_segments_in_distance_to(2., {3., 3.}).empty());
}

TEST(SegmentGrid, DistanceQueryGrowingRadiusPicksUpParallelSegmentsOneByOne)
{
    // Two parallel diagonals (direction (1,1)); p sits between them, closer to A.
    // dist(p, A) = sqrt(0.5) ~= 0.707, dist(p, B) = sqrt(4.5) ~= 2.121.
    const SegmentGrid grid{std::vector<LineSegment>{
        LineSegment{{0., 0.}, {10., 10.}}, // A
        LineSegment{{0., 4.}, {10., 14.}}}}; // B
    const Point p{5., 6.};

    EXPECT_EQ(std::ranges::distance(grid.line_segments_in_distance_to(0.5, p)), 0); // below both
    EXPECT_EQ(
        std::ranges::distance(grid.line_segments_in_distance_to(1.0, p)), 1); // reaches A only
    EXPECT_EQ(std::ranges::distance(grid.line_segments_in_distance_to(3.0, p)), 2); // reaches both
}

TEST(SegmentGrid, DistanceQueryMeasuresToTheSegmentNotItsInfiniteLine)
{
    // Checks with a point along "continuation" of the line segment.
    const SegmentGrid grid{std::vector<LineSegment>{LineSegment{{0., 0.}, {10., 0.}}}};
    const Point beyond_end{13., 0.}; // collinear, 3 past the (10,0) endpoint

    EXPECT_TRUE(grid.line_segments_in_distance_to(2., beyond_end).empty()); // 3 > 2
    EXPECT_EQ(
        std::ranges::distance(grid.line_segments_in_distance_to(4., beyond_end)), 1); // 3 <= 4
}

TEST(SegmentGrid, ApproxDistanceQueryReturnsCandidatesNearby)
{
    const auto grid = single_vertical_wall();
    EXPECT_FALSE(grid.line_segments_in_approx_distance_to({1., 5.}).empty());
}

TEST(SegmentGrid, ApproxDistanceQueryIsEmptyFarAway)
{
    const auto grid = single_vertical_wall();
    EXPECT_TRUE(grid.line_segments_in_approx_distance_to({100., 100.}).empty());
}

TEST(SegmentGrid, EmptyGridAnswersAllQueriesEmpty)
{
    const SegmentGrid grid{std::vector<LineSegment>{}};
    EXPECT_FALSE(grid.intersects_any(LineSegment{{-1., 5.}, {1., 5.}}));
    EXPECT_TRUE(grid.line_segments_in_distance_to(10., {0., 0.}).empty());
    EXPECT_TRUE(grid.line_segments_in_approx_distance_to({0., 0.}).empty());
}

struct CellAdjacencyTestData {
    Cell c;
    Cell neighbor;
    bool expected;
};

class CellAdjacencyTest : public ::testing::TestWithParam<CellAdjacencyTestData>
{
};

TEST_P(CellAdjacencyTest, All)
{
    const auto [c, neighbor, expected] = GetParam();
    EXPECT_EQ(is_n4_adjacent(c, neighbor), expected);
    EXPECT_EQ(is_n4_adjacent(neighbor, c), expected);
}

// clang-format off
INSTANTIATE_TEST_SUITE_P(
    CellNeighborhood,
    CellAdjacencyTest,
    testing::Values(
        // A cell is not a neighbor of itself
        CellAdjacencyTestData{{0, 0}, {0, 0}, false},
        // N4 neighbors
        CellAdjacencyTestData{{0, 0}, {cell_extend, 0}, true},
        CellAdjacencyTestData{{0, 0}, {0, cell_extend}, true},
        CellAdjacencyTestData{{0, 0}, {-cell_extend, 0}, true},
        CellAdjacencyTestData{{0, 0}, {0, -cell_extend}, true},
        // N8 neighbors
        CellAdjacencyTestData{{0, 0}, {cell_extend, cell_extend}, false},
        CellAdjacencyTestData{{0, 0}, {-cell_extend, -cell_extend}, false},
        CellAdjacencyTestData{{0, 0}, {cell_extend, -cell_extend}, false},
        CellAdjacencyTestData{{0, 0}, {-cell_extend, cell_extend}, false},
        // Layer beyond N8 (all false, no direct neighbors)
        CellAdjacencyTestData{{0, 0}, {cell_extend*2, 0}, false},
        CellAdjacencyTestData{{0, 0}, {cell_extend*2, cell_extend}, false},
        CellAdjacencyTestData{{0, 0}, {cell_extend*2, cell_extend*2}, false},
        CellAdjacencyTestData{{0, 0}, {cell_extend, cell_extend*2}, false},
        CellAdjacencyTestData{{0, 0}, {0, cell_extend*2}, false},
        CellAdjacencyTestData{{0, 0}, {-cell_extend, cell_extend*2}, false},
        CellAdjacencyTestData{{0, 0}, {-cell_extend*2, cell_extend*2}, false},
        CellAdjacencyTestData{{0, 0}, {-cell_extend*2, cell_extend}, false},
        CellAdjacencyTestData{{0, 0}, {-cell_extend*2, 0}, false},
        CellAdjacencyTestData{{0, 0}, {-cell_extend*2, -cell_extend}, false},
        CellAdjacencyTestData{{0, 0}, {-cell_extend*2, -cell_extend*2}, false},
        CellAdjacencyTestData{{0, 0}, {-cell_extend, -cell_extend*2}, false},
        CellAdjacencyTestData{{0, 0}, {0, -cell_extend*2}, false},
        CellAdjacencyTestData{{0, 0}, {cell_extend, -cell_extend*2}, false},
        CellAdjacencyTestData{{0, 0}, {cell_extend*2, -cell_extend*2}, false},
        CellAdjacencyTestData{{0, 0}, {cell_extend*2, -cell_extend}, false}
    )
);
// clang-format on

class CellsFromLSTest : public ::testing::TestWithParam<std::tuple<LineSegment, std::set<Cell>>>
{
};

TEST_P(CellsFromLSTest, All)
{
    const auto [input, expected] = GetParam();
    const LineSegment reverse_input{input.p2, input.p1};
    EXPECT_EQ(expected, cells_from_line_segment(input));
    EXPECT_EQ(expected, cells_from_line_segment(reverse_input));
}

// clang-format off
INSTANTIATE_TEST_SUITE_P(
    CellComputation,
    CellsFromLSTest,
    testing::Values(
        std::make_tuple(
            LineSegment{{0, 0}, {0, 0}},
            std::set<Cell>{{0, 0}}
        ),
        std::make_tuple(
            LineSegment{{1, 0}, {0, 0}},
            std::set<Cell>{{0, 0}}
        ),
        std::make_tuple(
            LineSegment{{0, 1}, {0, 0}},
            std::set<Cell>{{0, 0}}
        ),
        std::make_tuple(
            LineSegment{{0, 0}, {1, 1}},
            std::set<Cell>{{0, 0}}
        ),
        std::make_tuple(
            LineSegment{{1, 3}, {11, 5}},
            std::set<Cell>{{0,0}, {4, 0}, {4, 4}, {8, 4}}
        ),
        std::make_tuple(
            LineSegment{{-3, 0}, {6, 0}},
            std::set<Cell>{{-4, 0}, {0, 0}, {4, 0}}
        ),
        std::make_tuple(
            LineSegment{{0, 0}, {-7, -7}},
            std::set<Cell>{{-8,-8},{0, 0}, {-4, -4}}
        ),
        std::make_tuple(
            LineSegment{{0, 0}, {7, 7}},
            std::set<Cell>{{0, 0}, {4, 4}}
        ),
        std::make_tuple(
            LineSegment{{1, 1}, {3, 4}},
            std::set<Cell>{{0, 0}, {0, 4}}
        ),
        std::make_tuple(
            LineSegment{{1, 1}, {7, 5}},
            std::set<Cell>{{0, 0}, {4, 0}, {4, 4}}
        ),
        std::make_tuple(
            LineSegment{{1, 1}, {5, 7}},
            std::set<Cell>{{0, 0}, {0, 4}, {4, 4}}
        ),
        std::make_tuple(
            LineSegment{{4, 12}, {8, 0}},
            std::set<Cell>{{4, 12}, {4, 8}, {4,4}, {4,0}, {8,0}}
        ),
        std::make_tuple(
            LineSegment{{0, 0}, {0, 8}},
            std::set<Cell>{{0, 0}, {0, 4}, {0, 8}}
        ),
        std::make_tuple(
            LineSegment{{0, 0}, {8, 0}},
            std::set<Cell>{{0, 0}, {4, 0}, {8, 0}}
        ),
        std::make_tuple(
            LineSegment{{0, 0}, {0, 0}},
            std::set<Cell>{{0, 0}}
        ),
        std::make_tuple(
            LineSegment{{0, 0}, {0, 0}},
            std::set<Cell>{{0, 0}}
        )
    )
);
// clang-format on

/// The closed ring of segments through @p corners, the way a polygon boundary arrives.
std::vector<LineSegment> ring(const std::vector<Point>& corners)
{
    std::vector<LineSegment> segments{};
    for(std::size_t i = 1; i < corners.size(); ++i) {
        segments.emplace_back(corners[i - 1], corners[i]);
    }
    segments.emplace_back(corners.back(), corners.front());
    return segments;
}

class ApproximateDistanceSimpleRectangle : public ::testing::Test
{
protected:
    SegmentGrid _grid;

    ApproximateDistanceSimpleRectangle() : _grid(ring({{1., 1.}, {3., 1.}, {3., 3.}, {1., 3.}})) {}
};

TEST_F(ApproximateDistanceSimpleRectangle, InsideSingleCell)
{
    const std::set<LineSegment> expected = {
        {{1., 1.}, {3., 1.}},
        {{3., 1.}, {3., 3.}},
        {{3., 3.}, {1., 3.}},
        {{1., 3.}, {1., 1.}},
    };

    const auto result = _grid.line_segments_in_approx_distance_to({2., 2.});
    const std::set<LineSegment> actual(std::begin(result), std::end(result));

    ASSERT_EQ(actual, expected);
}

TEST_F(ApproximateDistanceSimpleRectangle, outsideInRange)
{
    const std::set<LineSegment> expected = {
        {{1., 1.}, {3., 1.}},
        {{3., 1.}, {3., 3.}},
        {{3., 3.}, {1., 3.}},
        {{1., 3.}, {1., 1.}},
    };

    const std::vector<Cell> candidates = {
        {-cell_extend, -cell_extend},
        {-cell_extend, 0},
        {-cell_extend, cell_extend},
        {0, -cell_extend},
        {0, 0},
        {0, cell_extend},
        {cell_extend, -cell_extend},
        {cell_extend, 0},
        {cell_extend, cell_extend}};

    for(const auto& point : candidates) {
        const auto result = _grid.line_segments_in_approx_distance_to(point);
        const std::set<LineSegment> actual(std::begin(result), std::end(result));

        ASSERT_EQ(actual, expected);
    }
}

class LongDiagonalRectangle : public ::testing::Test
{
protected:
    SegmentGrid _grid;

    LongDiagonalRectangle() : _grid(ring({{-11., -13.}, {5., 11.}, {6., 10.}, {-10., -14.}})) {}
};

TEST_F(LongDiagonalRectangle, FarCellsOutside)
{
    const std::vector<Cell> candidates = {
        {-16., -4}, {-16, 0}, {-16, 4}, {-16, 8},  {-16, 12}, {-12, 4}, {-12, 8},
        {-12, 12},  {-8, 8},  {-8, 12}, {-4, -20}, {0, -16},  {4, -20}, {4, -16},
        {4, -12},   {4, -8},  {8, -20}, {8, -16},  {8, -12},  {8, -8},  {8, -4}};

    for(const auto& point : candidates) {
        const auto result = _grid.line_segments_in_approx_distance_to(point);
        ASSERT_TRUE(result.empty());
    }
}

TEST_F(LongDiagonalRectangle, CellsAtBottomLeft)
{
    const std::set<LineSegment> expected = {
        {{-11., -13.}, {5., 11.}}, {{6., 10.}, {-10., -14.}}, {{-10., -14.}, {-11., -13.}}};

    const auto middle_cell = Cell{-12, -16};
    const std::vector<Cell> candidates = {
        {middle_cell.x - cell_extend, middle_cell.y - cell_extend},
        {middle_cell.x - cell_extend, middle_cell.y},
        {middle_cell.x - cell_extend, middle_cell.y + cell_extend},
        {middle_cell.x, middle_cell.y - cell_extend},
        {middle_cell.x, middle_cell.y},
        {middle_cell.x, middle_cell.y + cell_extend},
        {middle_cell.x + cell_extend, middle_cell.y - cell_extend},
        {middle_cell.x + cell_extend, middle_cell.y},
        {middle_cell.x + cell_extend, middle_cell.y + cell_extend}};

    for(const auto& point : candidates) {
        const auto result = _grid.line_segments_in_approx_distance_to(point);
        const std::set<LineSegment> actual(std::begin(result), std::end(result));

        ASSERT_EQ(actual, expected);
    }
}

TEST_F(LongDiagonalRectangle, CellsAtTopRight)
{
    const std::set<LineSegment> expected = {
        {{-11., -13.}, {5., 11.}}, {{5., 11.}, {6., 10.}}, {{6., 10.}, {-10., -14.}}};

    const auto middle_cell = Cell{4, 8};
    const std::vector<Cell> candidates = {
        {middle_cell.x - cell_extend, middle_cell.y - cell_extend},
        {middle_cell.x - cell_extend, middle_cell.y},
        {middle_cell.x - cell_extend, middle_cell.y + cell_extend},
        {middle_cell.x, middle_cell.y - cell_extend},
        {middle_cell.x, middle_cell.y},
        {middle_cell.x, middle_cell.y + cell_extend},
        {middle_cell.x + cell_extend, middle_cell.y - cell_extend},
        {middle_cell.x + cell_extend, middle_cell.y},
        {middle_cell.x + cell_extend, middle_cell.y + cell_extend}};

    for(const auto& point : candidates) {
        const auto result = _grid.line_segments_in_approx_distance_to(point);
        const std::set<LineSegment> actual(std::begin(result), std::end(result));

        ASSERT_EQ(actual, expected);
    }
}

TEST_F(LongDiagonalRectangle, CellsWithOneLSLeft)
{
    const std::set<LineSegment> expected = {{{-11., -13.}, {5., 11.}}};

    const std::vector<Cell> candidates = {{-12, 0}, {-4, 12}};

    for(const auto& point : candidates) {
        const auto result = _grid.line_segments_in_approx_distance_to(point);
        const std::set<LineSegment> actual(std::begin(result), std::end(result));

        ASSERT_EQ(actual, expected);
    }
}

TEST_F(LongDiagonalRectangle, CellsWithOneLSRight)
{
    const std::set<LineSegment> expected = {{{6., 10.}, {-10., -14.}}};

    const std::vector<Cell> candidates = {{0, -12}, {8, 0}};

    for(const auto& point : candidates) {
        const auto result = _grid.line_segments_in_approx_distance_to(point);
        const std::set<LineSegment> actual(std::begin(result), std::end(result));

        ASSERT_EQ(actual, expected);
    }
}

TEST_F(LongDiagonalRectangle, CellsWithTwoLSMiddle)
{
    const std::set<LineSegment> expected = {{{-11., -13.}, {5., 11.}}, {{6., 10.}, {-10., -14.}}};

    const std::vector<Cell> candidates = {
        {-16, -8}, {-12, -8}, {-12, -4}, {-8, -8}, {-8, -4}, {-8, 0}, {-8, 4},
        {-4, -16}, {-4, -12}, {-4, -8},  {-4, -4}, {-4, 0},  {-4, 4}, {-4, 8},
        {0, -8},   {0, -4},   {0, 0},    {4, -4},  {4, 0},
    };

    for(const auto& point : candidates) {
        const auto result = _grid.line_segments_in_approx_distance_to(point);
        const std::set<LineSegment> actual(std::begin(result), std::end(result));

        ASSERT_EQ(actual, expected);
    }
}
