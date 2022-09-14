/// Copyright © 2012-2022 Forschungszentrum Jülich GmbH
/// SPDX-License-Identifier: LGPL-3.0-or-later
#include "GeometricFunctions.hpp"

#include "ConvexPolygon.hpp"
#include "Point.hpp"

#include <gtest/gtest.h>

TEST(Polygon, PointIsInside)
{
    const Point points[]{{0, 0}, {1, 1}, {0, 2}, {-1, 1}};
    const Point pt{0, 0.5};
    ConvexPolygon poly(points);
    ASSERT_TRUE(poly.Inside(pt));
}

TEST(Polygon, PointIsInsideRightHalf)
{
    const Point points[]{{0, 0}, {1, 1}, {0, 2}, {-1, 1}};
    const Point pt{-0.25, 0.5};
    ConvexPolygon poly(points);
    ASSERT_TRUE(poly.Inside(pt));
}

TEST(Polygon, PointIsInsideLeftHalf)
{
    const Point points[]{{0, 0}, {1, 1}, {0, 2}, {-1, 1}};
    const Point pt{0.25, 0.5};
    ConvexPolygon poly(points);
    ASSERT_TRUE(poly.Inside(pt));
}

TEST(Polygon, PointIsOutsideRightOfFirstLineSegmentOfPolygon)
{
    const Point points[]{{0, 0}, {1, 1}, {0, 2}, {-1, 1}};
    const Point pt{0.6, 0.5};
    ConvexPolygon poly(points);
    ASSERT_FALSE(poly.Inside(pt));
}

TEST(Polygon, PointIsOutsideLeftOfLastLineSegmentOfPolygon)
{
    const Point points[]{{0, 0}, {1, 1}, {0, 2}, {-1, 1}};
    const Point pt{-0.6, 0.5};
    ConvexPolygon poly(points);
    ASSERT_FALSE(poly.Inside(pt));
}

TEST(Polygon, PointIsOutsideRightOfMiddleLineSegmentOfPolygon)
{
    const Point points[]{{0, 0}, {1, 1}, {0, 2}, {-1, 1}};
    const Point pt{0.5, 1.6};
    ConvexPolygon poly(points);
    ASSERT_FALSE(poly.Inside(pt));
}
