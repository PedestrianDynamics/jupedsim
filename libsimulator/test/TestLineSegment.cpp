// SPDX-License-Identifier: LGPL-3.0-or-later
#include "GeometricFunctions.hpp"
#include "LineSegment.hpp"
#include "Point.hpp"
#include "TestCommon.hpp"

#include <gtest/gtest.h>

#include <random>

const double pi = acos(-1);

TEST(LineSegment, ScalarProduct)
{
    for(int i : {-5, -4, -3, -2, -1, 1, 2, 3, 4}) {
        Point p1(pi / i, pi * i);
        Point p2(i, std::sin(pi / i));
        LineSegment l1(p1, p2);
        Point normal = l1.normal_vec();
        Point diff = p2 - p1;
        ASSERT_NEAR(normal.scalar_product(diff), 0.0, 1.0e-12);
    }
}

TEST(LineSegment, ShortestPoint)
{
    Point pa(-2, 4);
    Point pb(14, 9);
    LineSegment l1(pa, pb);
    const Point& dpab = pa - pb;
    for(float i = -20; i < 20; ++i) {
        i = (i == 0) ? 0.5 : i;
        Point p1(i, std::sin(pi / i));
        Point p2 = l1.shortest_point(p1);
        double lambda = (p1 - pb).scalar_product(dpab) / dpab.scalar_product(dpab);
        if(lambda > 1) {
            ASSERT_EQ(p2, pa);
        } else if(lambda < 0) {
            ASSERT_EQ(p2, pb);
        } else {
            ASSERT_NEAR((p2 - p1).scalar_product(dpab), 0.0, 1.0e-12);
        }
    }
}

TEST(LineSegment, DistTo)
{
    LineSegment l1(Point(-10, 2), Point(10, 2));
    for(int i = -10; i < 11; ++i) {
        ASSERT_DOUBLE_EQ(l1.dist_to(Point(i, i)), abs(i - 2));
    }
}

TEST(LineSegment, SameLineSegmentsIntersect)
{
    const LineSegment l{{0, 0}, {1, 6}};
    ASSERT_TRUE(intersects(l, l));
}

TEST(LineSegment, CollinearLineSegmentsTouchingInEndpointsIntersect)
{
    const LineSegment l1{{0, 0}, {0, 6}};
    const LineSegment l2{{0, 6}, {0, 8}};
    ASSERT_TRUE(intersects(l1, l2));
}

TEST(LineSegment, CollinearLineSegmentsNotTouchingDoNotIntersect)
{
    const LineSegment l1{{0, 0}, {0, 6}};
    const LineSegment l2{{1, 0}, {1, 6}};
    ASSERT_FALSE(intersects(l1, l2));
}

TEST(LineSegment, ItersectsWithEndpointsTouchingNotCollinear)
{
    const LineSegment l1{{-1, -1}, {1, 1}};
    const LineSegment l2{{1, 1}, {0, 0}};
    ASSERT_TRUE(intersects(l1, l2));
}

TEST(LineSegment, ItersectsWithOneEndpointTouchingInTheMiddle)
{
    const LineSegment l1{{-1, -1}, {1, 1}};
    const LineSegment l2{{1, -1}, {0, 0}};
    ASSERT_TRUE(intersects(l1, l2));
}

TEST(LineSegment, OperatorLTCaseA)
{
    const LineSegment a{{0, 0}, {1, 1}};
    const LineSegment b{{1, 1}, {0, 0}};
    ASSERT_TRUE(a < b);
    ASSERT_FALSE(b < a);
    ASSERT_FALSE(a < a);
}

TEST(LineSegment, OperatorLTCaseB)
{
    const LineSegment a{{0, 0}, {2, 2}};
    const LineSegment b{{1, 1}, {2, 2}};
    ASSERT_TRUE(a < b);
    ASSERT_FALSE(b < a);
    ASSERT_FALSE(a < a);
}

TEST(LineSegment, OperatorLTCaseC)
{
    const LineSegment a{{0, 0}, {1, 1}};
    const LineSegment b{{0, 0}, {2, 2}};
    ASSERT_TRUE(a < b);
    ASSERT_FALSE(b < a);
    ASSERT_FALSE(a < a);
}
