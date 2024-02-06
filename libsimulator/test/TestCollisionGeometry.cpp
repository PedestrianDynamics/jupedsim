// Copyright © 2012-2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "CollisionGeometry.hpp"
#include "LineSegment.hpp"

#include "gtest/gtest.h"
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <gtest/gtest.h>

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
    EXPECT_EQ(IsN8Adjacent(c, neighbor), expected);
    EXPECT_EQ(IsN8Adjacent(neighbor, c), expected);
}

// clang-format off
INSTANTIATE_TEST_SUITE_P(
    CellNeighborhood,
    CellAdjacencyTest,
    testing::Values(
        // A cell is not a neighbor of itself
        CellAdjacencyTestData{{0, 0}, {0, 0}, false},
        // N4 neigbors
        CellAdjacencyTestData{{0, 0}, {CELL_EXTEND, 0}, true},
        CellAdjacencyTestData{{0, 0}, {0, CELL_EXTEND}, true},
        CellAdjacencyTestData{{0, 0}, {CELL_EXTEND, CELL_EXTEND}, true},
        CellAdjacencyTestData{{0, 0}, {-CELL_EXTEND, 0}, true},
        CellAdjacencyTestData{{0, 0}, {0, -CELL_EXTEND}, true},
        // N8 neighbors
        CellAdjacencyTestData{{0, 0}, {-CELL_EXTEND, -CELL_EXTEND}, true},
        CellAdjacencyTestData{{0, 0}, {CELL_EXTEND, -CELL_EXTEND}, true},
        CellAdjacencyTestData{{0, 0}, {-CELL_EXTEND, CELL_EXTEND}, true},
        // Layer beyond N8 (all false, no direct neighbors)
        CellAdjacencyTestData{{0, 0}, {CELL_EXTEND*2, 0}, false},
        CellAdjacencyTestData{{0, 0}, {CELL_EXTEND*2, CELL_EXTEND}, false},
        CellAdjacencyTestData{{0, 0}, {CELL_EXTEND*2, CELL_EXTEND*2}, false},
        CellAdjacencyTestData{{0, 0}, {CELL_EXTEND, CELL_EXTEND*2}, false},
        CellAdjacencyTestData{{0, 0}, {0, CELL_EXTEND*2}, false},
        CellAdjacencyTestData{{0, 0}, {-CELL_EXTEND, CELL_EXTEND*2}, false},
        CellAdjacencyTestData{{0, 0}, {-CELL_EXTEND*2, CELL_EXTEND*2}, false},
        CellAdjacencyTestData{{0, 0}, {-CELL_EXTEND*2, CELL_EXTEND}, false},
        CellAdjacencyTestData{{0, 0}, {-CELL_EXTEND*2, 0}, false},
        CellAdjacencyTestData{{0, 0}, {-CELL_EXTEND*2, -CELL_EXTEND}, false},
        CellAdjacencyTestData{{0, 0}, {-CELL_EXTEND*2, -CELL_EXTEND*2}, false},
        CellAdjacencyTestData{{0, 0}, {-CELL_EXTEND, -CELL_EXTEND*2}, false},
        CellAdjacencyTestData{{0, 0}, {0, -CELL_EXTEND*2}, false},
        CellAdjacencyTestData{{0, 0}, {CELL_EXTEND, -CELL_EXTEND*2}, false},
        CellAdjacencyTestData{{0, 0}, {CELL_EXTEND*2, -CELL_EXTEND*2}, false},
        CellAdjacencyTestData{{0, 0}, {CELL_EXTEND*2, -CELL_EXTEND}, false}
    )
);
// clang-format on

class CellsFromLSTest : public ::testing::TestWithParam<std::tuple<LineSegment, std::set<Cell>>>
{
};

TEST_P(CellsFromLSTest, All)
{
    const auto [input, expected] = GetParam();
    const LineSegment reverseInput{input.p2, input.p1};
    EXPECT_EQ(expected, cellsFromLineSegment(input));
    EXPECT_EQ(expected, cellsFromLineSegment(reverseInput));
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

PolyWithHoles constructPolyFromPoints(const std::vector<Point>& points)
{
    using CGALPoint = PolyWithHoles::Polygon_2::Point_2;
    std::vector<CGALPoint> cgalPoints{};
    cgalPoints.reserve(points.size());
    std::transform(
        std::begin(points), std::end(points), std::back_inserter(cgalPoints), [](const auto& p) {
            return CGALPoint{p.x, p.y};
        });
    return PolyWithHoles(Poly{cgalPoints.begin(), cgalPoints.end()});
}

class ApproximateDistanceSimpleRectangle : public ::testing::Test
{
protected:
    CollisionGeometry collisionGeometry;

    ApproximateDistanceSimpleRectangle()
        : collisionGeometry(constructPolyFromPoints({{1., 1.}, {3., 1.}, {3., 3.}, {1., 3.}}))
    {
    }
};

TEST_F(ApproximateDistanceSimpleRectangle, InsideSingleCell)
{
    const std::set<LineSegment> expected = {
        {{1., 1.}, {3., 1.}},
        {{3., 1.}, {3., 3.}},
        {{3., 3.}, {1., 3.}},
        {{1., 3.}, {1., 1.}},
    };

    const auto result = collisionGeometry.LineSegmentsInApproxDistanceTo({2., 2.});
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
        {-CELL_EXTEND, -CELL_EXTEND},
        {-CELL_EXTEND, 0},
        {-CELL_EXTEND, CELL_EXTEND},
        {0, -CELL_EXTEND},
        {0, 0},
        {0, CELL_EXTEND},
        {CELL_EXTEND, -CELL_EXTEND},
        {CELL_EXTEND, 0},
        {CELL_EXTEND, CELL_EXTEND}};

    for(const auto& point : candidates) {
        const auto result = collisionGeometry.LineSegmentsInApproxDistanceTo(point);
        const std::set<LineSegment> actual(std::begin(result), std::end(result));

        ASSERT_EQ(actual, expected);
    }
}

class LongDiagonalRectangle : public ::testing::Test
{
protected:
    CollisionGeometry collisionGeometry;

    LongDiagonalRectangle()
        : collisionGeometry(
              constructPolyFromPoints({{-11., -13.}, {5., 11.}, {6., 10.}, {-10., -14.}}))
    {
    }
};

TEST_F(LongDiagonalRectangle, FarCellsOutside)
{
    const std::vector<Cell> candidates = {
        {-16., -4}, {-16, 0}, {-16, 4}, {-16, 8},  {-16, 12}, {-12, 4}, {-12, 8},
        {-12, 12},  {-8, 8},  {-8, 12}, {-4, -20}, {0, -16},  {4, -20}, {4, -16},
        {4, -12},   {4, -8},  {8, -20}, {8, -16},  {8, -12},  {8, -8},  {8, -4}};

    for(const auto& point : candidates) {
        const auto result = collisionGeometry.LineSegmentsInApproxDistanceTo(point);
        ASSERT_TRUE(result.empty());
    }
}

TEST_F(LongDiagonalRectangle, CellsAtBottomLeft)
{
    const std::set<LineSegment> expected = {
        {{-11., -13.}, {5., 11.}}, {{6., 10.}, {-10., -14.}}, {{-10., -14.}, {-11., -13.}}};

    const auto middleCell = Cell{-12, -16};
    const std::vector<Cell> candidates = {
        {middleCell.x - CELL_EXTEND, middleCell.y - CELL_EXTEND},
        {middleCell.x - CELL_EXTEND, middleCell.y},
        {middleCell.x - CELL_EXTEND, middleCell.y + CELL_EXTEND},
        {middleCell.x, middleCell.y - CELL_EXTEND},
        {middleCell.x, middleCell.y},
        {middleCell.x, middleCell.y + CELL_EXTEND},
        {middleCell.x + CELL_EXTEND, middleCell.y - CELL_EXTEND},
        {middleCell.x + CELL_EXTEND, middleCell.y},
        {middleCell.x + CELL_EXTEND, middleCell.y + CELL_EXTEND}};

    for(const auto& point : candidates) {
        const auto result = collisionGeometry.LineSegmentsInApproxDistanceTo(point);
        const std::set<LineSegment> actual(std::begin(result), std::end(result));

        ASSERT_EQ(actual, expected);
    }
}

TEST_F(LongDiagonalRectangle, CellsAtTopRight)
{
    const std::set<LineSegment> expected = {
        {{-11., -13.}, {5., 11.}}, {{5., 11.}, {6., 10.}}, {{6., 10.}, {-10., -14.}}};

    const auto middleCell = Cell{4, 8};
    const std::vector<Cell> candidates = {
        {middleCell.x - CELL_EXTEND, middleCell.y - CELL_EXTEND},
        {middleCell.x - CELL_EXTEND, middleCell.y},
        {middleCell.x - CELL_EXTEND, middleCell.y + CELL_EXTEND},
        {middleCell.x, middleCell.y - CELL_EXTEND},
        {middleCell.x, middleCell.y},
        {middleCell.x, middleCell.y + CELL_EXTEND},
        {middleCell.x + CELL_EXTEND, middleCell.y - CELL_EXTEND},
        {middleCell.x + CELL_EXTEND, middleCell.y},
        {middleCell.x + CELL_EXTEND, middleCell.y + CELL_EXTEND}};

    for(const auto& point : candidates) {
        const auto result = collisionGeometry.LineSegmentsInApproxDistanceTo(point);
        const std::set<LineSegment> actual(std::begin(result), std::end(result));

        ASSERT_EQ(actual, expected);
    }
}

TEST_F(LongDiagonalRectangle, CellsWithOneLSLeft)
{
    const std::set<LineSegment> expected = {{{-11., -13.}, {5., 11.}}};

    const std::vector<Cell> candidates = {{-12, 0}, {-4, 12}};

    for(const auto& point : candidates) {
        const auto result = collisionGeometry.LineSegmentsInApproxDistanceTo(point);
        const std::set<LineSegment> actual(std::begin(result), std::end(result));

        ASSERT_EQ(actual, expected);
    }
}

TEST_F(LongDiagonalRectangle, CellsWithOneLSRight)
{
    const std::set<LineSegment> expected = {{{6., 10.}, {-10., -14.}}};

    const std::vector<Cell> candidates = {{0, -12}, {8, 0}};

    for(const auto& point : candidates) {
        const auto result = collisionGeometry.LineSegmentsInApproxDistanceTo(point);
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
        const auto result = collisionGeometry.LineSegmentsInApproxDistanceTo(point);
        const std::set<LineSegment> actual(std::begin(result), std::end(result));

        ASSERT_EQ(actual, expected);
    }
}
