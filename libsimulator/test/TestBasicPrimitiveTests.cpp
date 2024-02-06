// Copyright © 2012-2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "GeometricFunctions.hpp"

#include "Point.hpp"
#include "Polygon.hpp"

#include <gtest/gtest.h>

TEST(Polygon, PointIsInside)
{
    const std::vector<Point> points{{0, 0}, {1, 1}, {0, 2}, {-1, 1}};
    const Point pt{0, 0.5};
    Polygon poly(points);
    ASSERT_TRUE(poly.IsInside(pt));
}

TEST(Polygon, PointIsInsideRightHalf)
{
    const std::vector<Point> points{{0, 0}, {1, 1}, {0, 2}, {-1, 1}};
    const Point pt{-0.25, 0.5};
    Polygon poly(points);
    ASSERT_TRUE(poly.IsInside(pt));
}

TEST(Polygon, PointIsInsideLeftHalf)
{
    const std::vector<Point> points{{0, 0}, {1, 1}, {0, 2}, {-1, 1}};
    const Point pt{0.25, 0.5};
    Polygon poly(points);
    ASSERT_TRUE(poly.IsInside(pt));
}

TEST(Polygon, PointIsOutsideRightOfFirstLineSegmentOfPolygon)
{
    const std::vector<Point> points{{0, 0}, {1, 1}, {0, 2}, {-1, 1}};
    const Point pt{0.6, 0.5};
    Polygon poly(points);
    ASSERT_FALSE(poly.IsInside(pt));
}

TEST(Polygon, PointIsOutsideLeftOfLastLineSegmentOfPolygon)
{
    const std::vector<Point> points{{0, 0}, {1, 1}, {0, 2}, {-1, 1}};
    const Point pt{-0.6, 0.5};
    Polygon poly(points);
    ASSERT_FALSE(poly.IsInside(pt));
}

TEST(Polygon, PointIsOutsideRightOfMiddleLineSegmentOfPolygon)
{
    const std::vector<Point> points{{0, 0}, {1, 1}, {0, 2}, {-1, 1}};
    const Point pt{0.5, 1.6};
    Polygon poly(points);
    ASSERT_FALSE(poly.IsInside(pt));
}
