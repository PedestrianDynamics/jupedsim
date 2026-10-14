// SPDX-License-Identifier: LGPL-3.0-or-later
#include "geometric_functions.hpp"
#include "point.hpp"
#include "polygon.hpp"
#include "simulation_error.hpp"
#include "test_common.hpp"

#include <gtest/gtest.h>

#include <limits>

TEST(Polygon, PointIsInside)
{
    const std::vector<Point> points{{0, 0}, {1, 1}, {0, 2}, {-1, 1}};
    const Point pt{0, 0.5};
    Polygon poly(points);
    ASSERT_TRUE(poly.is_inside(pt));
}

TEST(Polygon, PointIsInsideRightHalf)
{
    const std::vector<Point> points{{0, 0}, {1, 1}, {0, 2}, {-1, 1}};
    const Point pt{-0.25, 0.5};
    Polygon poly(points);
    ASSERT_TRUE(poly.is_inside(pt));
}

TEST(Polygon, PointIsInsideLeftHalf)
{
    const std::vector<Point> points{{0, 0}, {1, 1}, {0, 2}, {-1, 1}};
    const Point pt{0.25, 0.5};
    Polygon poly(points);
    ASSERT_TRUE(poly.is_inside(pt));
}

TEST(Polygon, PointIsOutsideRightOfFirstLineSegmentOfPolygon)
{
    const std::vector<Point> points{{0, 0}, {1, 1}, {0, 2}, {-1, 1}};
    const Point pt{0.6, 0.5};
    Polygon poly(points);
    ASSERT_FALSE(poly.is_inside(pt));
}

TEST(Polygon, PointIsOutsideLeftOfLastLineSegmentOfPolygon)
{
    const std::vector<Point> points{{0, 0}, {1, 1}, {0, 2}, {-1, 1}};
    const Point pt{-0.6, 0.5};
    Polygon poly(points);
    ASSERT_FALSE(poly.is_inside(pt));
}

TEST(Polygon, PointIsOutsideRightOfMiddleLineSegmentOfPolygon)
{
    const std::vector<Point> points{{0, 0}, {1, 1}, {0, 2}, {-1, 1}};
    const Point pt{0.5, 1.6};
    Polygon poly(points);
    ASSERT_FALSE(poly.is_inside(pt));
}

TEST(Polygon, RepeatedClosingPointIsDropped)
{
    const std::vector<Point> open{{0, 0}, {1, 0}, {0, 1}};
    const std::vector<Point> closed{{0, 0}, {1, 0}, {0, 1}, {0, 0}};
    const Poly from_open = Polygon(open);
    const Poly from_closed = Polygon(closed);
    EXPECT_EQ(from_closed, from_open);
}

TEST(Polygon, NonFiniteCoordinatesAreRejected)
{
    for(const double bad :
        {std::numeric_limits<double>::infinity(), std::numeric_limits<double>::quiet_NaN()}) {
        const std::vector<Point> points{{0, 0}, {bad, 0}, {1, 1}, {0, 1}};
        EXPECT_THROW(Polygon{points}, SimulationError) << bad;
    }
}
