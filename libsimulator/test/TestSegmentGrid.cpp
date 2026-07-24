// SPDX-License-Identifier: LGPL-3.0-or-later
#include "Geometry/SegmentGrid.hpp"
#include "LineSegment.hpp"
#include "Point.hpp"

#include <gtest/gtest.h>

#include <iterator>
#include <vector>

namespace
{
/// A single vertical wall from (0,0) to (0,10).
SegmentGrid singleVerticalWall()
{
    return SegmentGrid{std::vector<LineSegment>{LineSegment{{0., 0.}, {0., 10.}}}};
}
} // namespace

TEST(SegmentGrid, IntersectsAnyDetectsACrossingSegment)
{
    const auto grid = singleVerticalWall();
    EXPECT_TRUE(grid.IntersectsAny(LineSegment{{-1., 5.}, {1., 5.}}));
}

TEST(SegmentGrid, IntersectsAnyRejectsAFarAwaySegment)
{
    const auto grid = singleVerticalWall();
    EXPECT_FALSE(grid.IntersectsAny(LineSegment{{5., 5.}, {7., 5.}}));
}

TEST(SegmentGrid, IntersectsAnyRejectsANearButParallelSegment)
{
    // Shares grid cells with the wall but never crosses it.
    const auto grid = singleVerticalWall();
    EXPECT_FALSE(grid.IntersectsAny(LineSegment{{1., 0.}, {1., 10.}}));
}

TEST(SegmentGrid, DistanceQueryReturnsSegmentsWithinRange)
{
    const auto grid = singleVerticalWall();
    // Perpendicular distance from (1,5) to the wall is 1.
    EXPECT_EQ(std::ranges::distance(grid.LineSegmentsInDistanceTo(2., {1., 5.})), 1);
}

TEST(SegmentGrid, DistanceQueryExcludesSegmentsOutOfRange)
{
    const auto grid = singleVerticalWall();
    // Perpendicular distance from (3,3) to the wall is 3.
    EXPECT_TRUE(grid.LineSegmentsInDistanceTo(2., {3., 3.}).empty());
}

TEST(SegmentGrid, DistanceQueryGrowingRadiusPicksUpParallelSegmentsOneByOne)
{
    // Two parallel diagonals (direction (1,1)); p sits between them, closer to A.
    // dist(p, A) = sqrt(0.5) ~= 0.707, dist(p, B) = sqrt(4.5) ~= 2.121.
    const SegmentGrid grid{std::vector<LineSegment>{
        LineSegment{{0., 0.}, {10., 10.}}, // A
        LineSegment{{0., 4.}, {10., 14.}}}}; // B
    const Point p{5., 6.};

    EXPECT_EQ(std::ranges::distance(grid.LineSegmentsInDistanceTo(0.5, p)), 0); // below both
    EXPECT_EQ(std::ranges::distance(grid.LineSegmentsInDistanceTo(1.0, p)), 1); // reaches A only
    EXPECT_EQ(std::ranges::distance(grid.LineSegmentsInDistanceTo(3.0, p)), 2); // reaches both
}

TEST(SegmentGrid, DistanceQueryMeasuresToTheSegmentNotItsInfiniteLine)
{
    // Checks with a point along "continuation" of the line segment.
    const SegmentGrid grid{std::vector<LineSegment>{LineSegment{{0., 0.}, {10., 0.}}}};
    const Point beyondEnd{13., 0.}; // collinear, 3 past the (10,0) endpoint

    EXPECT_TRUE(grid.LineSegmentsInDistanceTo(2., beyondEnd).empty()); // 3 > 2
    EXPECT_EQ(std::ranges::distance(grid.LineSegmentsInDistanceTo(4., beyondEnd)), 1); // 3 <= 4
}

TEST(SegmentGrid, ApproxDistanceQueryReturnsCandidatesNearby)
{
    const auto grid = singleVerticalWall();
    EXPECT_FALSE(grid.LineSegmentsInApproxDistanceTo({1., 5.}).empty());
}

TEST(SegmentGrid, ApproxDistanceQueryIsEmptyFarAway)
{
    const auto grid = singleVerticalWall();
    EXPECT_TRUE(grid.LineSegmentsInApproxDistanceTo({100., 100.}).empty());
}

TEST(SegmentGrid, EmptyGridAnswersAllQueriesEmpty)
{
    const SegmentGrid grid{std::vector<LineSegment>{}};
    EXPECT_FALSE(grid.IntersectsAny(LineSegment{{-1., 5.}, {1., 5.}}));
    EXPECT_TRUE(grid.LineSegmentsInDistanceTo(10., {0., 0.}).empty());
    EXPECT_TRUE(grid.LineSegmentsInApproxDistanceTo({0., 0.}).empty());
}
