// Copyright © 2012-2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "GeometricFunctions.hpp"
#include "LineSegment.hpp"
#include "Point.hpp"

#include <gtest/gtest.h>
#include <random>

const double PI = acos(-1);

TEST(LineSegment, ScalarProduct)
{
    for(int i : {-5, -4, -3, -2, -1, 1, 2, 3, 4}) {
        Point P1(PI / i, PI * i);
        Point P2(i, std::sin(PI / i));
        LineSegment L1(P1, P2);
        Point normal = L1.NormalVec();
        Point diff = P2 - P1;
        ASSERT_NEAR(normal.ScalarProduct(diff), 0.0, 1.0e-12);
    }
}

TEST(LineSegment, ShortestPoint)
{
    Point PA(-2, 4);
    Point PB(14, 9);
    LineSegment L1(PA, PB);
    const Point& DPAB = PA - PB;
    for(float i = -20; i < 20; ++i) {
        i = (i == 0) ? 0.5 : i;
        Point P1(i, std::sin(PI / i));
        Point P2 = L1.ShortestPoint(P1);
        double lambda = (P1 - PB).ScalarProduct(DPAB) / DPAB.ScalarProduct(DPAB);
        if(lambda > 1) {
            ASSERT_EQ(P2, PA);
        } else if(lambda < 0) {
            ASSERT_EQ(P2, PB);
        } else {
            ASSERT_NEAR((P2 - P1).ScalarProduct(DPAB), 0.0, 1.0e-12);
        }
    }
}

TEST(LineSegment, DistTo)
{
    LineSegment L1(Point(-10, 2), Point(10, 2));
    for(int i = -10; i < 11; ++i) {
        ASSERT_DOUBLE_EQ(L1.DistTo(Point(i, i)), abs(i - 2));
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
