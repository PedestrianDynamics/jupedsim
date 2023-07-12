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
