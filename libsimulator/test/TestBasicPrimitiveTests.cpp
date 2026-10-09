// SPDX-License-Identifier: LGPL-3.0-or-later
#include "GeometricFunctions.hpp"
#include "Point.hpp"
#include "Polygon.hpp"
#include "TestCommon.hpp"

#include <gtest/gtest.h>

#include <cmath>

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

TEST(Polygon, FromCircleKeepsHasShortEdges)
{
    constexpr double target_length = 0.4;
    for(const double radius : {0.05, 0.2, 1.0, 10.0}) {
        const Poly circle = Polygon::from_circle({3, -2}, radius);
        EXPECT_GE(circle.size(), 4u) << "radius " << radius;
        for(auto edge = circle.edges_begin(); edge != circle.edges_end(); ++edge) {
            EXPECT_LE(std::sqrt(edge->squared_length()), target_length) << "radius " << radius;
        }
    }
    // Not finer than needed: the edges of a large circle come close to the limit.
    const Poly large = Polygon::from_circle({3, -2}, 10.0);
    EXPECT_GT(std::sqrt(large.edges_begin()->squared_length()), target_length - 0.05);
}
