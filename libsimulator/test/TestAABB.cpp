// Copyright © 2012-2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "AABB.hpp"
#include "SimulationError.hpp"

#include <gtest/gtest.h>

TEST(AABB, CanConstructFromVector)
{
    const std::vector<Point> c{{0, 0}, {1, 1}, {-1, -1}};
    const AABB aabb(c);
    ASSERT_EQ(aabb.xmin, -1);
    ASSERT_EQ(aabb.xmax, 1);
    ASSERT_EQ(aabb.ymin, -1);
    ASSERT_EQ(aabb.ymax, 1);
}

TEST(AABB, CannotConstructFromEmptyVector)
{
    const std::vector<Point> c{};
    ASSERT_THROW(const AABB aabb(c), SimulationError);
}

TEST(AABB, CanConstructFromPositivePoints)
{
    const AABB aabb({0, 0}, {1, 1});
    ASSERT_EQ(aabb.xmin, 0);
    ASSERT_EQ(aabb.xmax, 1);
    ASSERT_EQ(aabb.ymin, 0);
    ASSERT_EQ(aabb.ymax, 1);
}

TEST(AABB, CanConstructFromNegativePoints)
{
    const AABB aabb({-10, -1}, {-4, -5});
    ASSERT_EQ(aabb.xmin, -10);
    ASSERT_EQ(aabb.xmax, -4);
    ASSERT_EQ(aabb.ymin, -5);
    ASSERT_EQ(aabb.ymax, -1);
}

TEST(AABB, CanConstructFromPoints)
{
    const AABB aabb({-2, 1}, {3, -5});
    ASSERT_EQ(aabb.xmin, -2);
    ASSERT_EQ(aabb.xmax, 3);
    ASSERT_EQ(aabb.ymin, -5);
    ASSERT_EQ(aabb.ymax, 1);
}

TEST(AABB, InsidePointIsInside)
{
    const AABB aabb({0, 0}, {1, 1});
    ASSERT_TRUE(aabb.Inside({0.5, 0.5}));
}

TEST(AABB, PointOnBoundaryIsInside)
{
    const AABB aabb({0, 0}, {1, 1});
    ASSERT_TRUE(aabb.Inside({0.0, 0.5}));
    ASSERT_TRUE(aabb.Inside({0.5, 0.0}));
    ASSERT_TRUE(aabb.Inside({1.0, 0.5}));
    ASSERT_TRUE(aabb.Inside({0.5, 1.0}));
}

TEST(AABB, CornersAreInside)
{
    const AABB aabb({0, 0}, {1, 1});
    ASSERT_TRUE(aabb.Inside({0, 0}));
    ASSERT_TRUE(aabb.Inside({1, 0}));
    ASSERT_TRUE(aabb.Inside({0, 1}));
    ASSERT_TRUE(aabb.Inside({1, 1}));
}

TEST(AABB, NonOverlappingDoNotOverlap)
{
    const AABB a({0, 0}, {1, 1});
    const AABB b({2, 0}, {3, 1});
    ASSERT_FALSE(a.Overlap(b));
    ASSERT_FALSE(b.Overlap(a));
}

TEST(AABB, OverlappingDoOverlap)
{
    const AABB a({0, 0}, {1, 1});
    const AABB b({0.5, 0}, {1.5, 1});
    ASSERT_TRUE(a.Overlap(b));
    ASSERT_TRUE(b.Overlap(a));
}

TEST(AABB, OverlappingOnCornerDoOverlap)
{
    const AABB a({0, 0}, {1, 1});
    const AABB b({1, 1}, {2, 2});
    ASSERT_TRUE(a.Overlap(b));
    ASSERT_TRUE(b.Overlap(a));
}

TEST(AABB, OverlappingSidesDoOverlap)
{
    const AABB a({0, 0}, {1, 1});
    const AABB b({1, 0}, {2, 2});
    ASSERT_TRUE(a.Overlap(b));
    ASSERT_TRUE(b.Overlap(a));
}

TEST(AABB, IntersectsDiagonal)
{
    const AABB a({3., 2.}, {6., 4.});
    const LineSegment l({3., 2.}, {6., 4.});
    ASSERT_TRUE(a.Intersects(l));
}

TEST(AABB, IntersectsDiagonalInverted)
{
    const AABB a({3., 2.}, {6., 4.});
    const LineSegment l({6., 4.}, {3., 2.});
    ASSERT_TRUE(a.Intersects(l));
}

TEST(AABB, IntersectsParallelToAxis)
{
    const AABB a(
        {
            -1.,
            -1.,
        },
        {1., 1.});
    const LineSegment l1({-1., -1.}, {-1., 1.});
    ASSERT_TRUE(a.Intersects(l1));
    const LineSegment l2({-1., 1.}, {1., 1.});
    ASSERT_TRUE(a.Intersects(l2));
    const LineSegment l3({1., 1.}, {1., -1.});
    ASSERT_TRUE(a.Intersects(l3));
    const LineSegment l4({1., -1.}, {-1., -1.});
    ASSERT_TRUE(a.Intersects(l4));
}

TEST(AABB, IntersectsTouchesCorner)
{
    const AABB a(
        {
            -1.,
            -1.,
        },
        {1., 1.});
    const LineSegment l1({-1., -1.}, {-2., -2.});
    ASSERT_TRUE(a.Intersects(l1));
    const LineSegment l2({-1., 1.}, {-2., 2.});
    ASSERT_TRUE(a.Intersects(l2));
    const LineSegment l3({1., 1.}, {2., 2.});
    ASSERT_TRUE(a.Intersects(l3));
    const LineSegment l4({1., -1.}, {2., -2.});
    ASSERT_TRUE(a.Intersects(l4));

    const LineSegment l5({-2., 0}, {0., -2.});
    ASSERT_TRUE(a.Intersects(l5));
}

TEST(AABB, IntersectsTouchesEdge)
{
    const AABB a(
        {
            -1.,
            -1.,
        },
        {1., 1.});
    const LineSegment l1({-1., 0.}, {-2., 0.});
    ASSERT_TRUE(a.Intersects(l1));
    const LineSegment l2({0., 1.}, {0., 2.});
    ASSERT_TRUE(a.Intersects(l2));
    const LineSegment l3({1., 0.}, {2., 0.});
    ASSERT_TRUE(a.Intersects(l3));
    const LineSegment l4({0., -1.}, {0., -2.});
    ASSERT_TRUE(a.Intersects(l4));
}

TEST(AABB, IntersectsPartlyInside)
{
    const AABB a(
        {
            -1.,
            -1.,
        },
        {1., 1.});
    const LineSegment l({0., 0.}, {-2., 3.});
    ASSERT_TRUE(a.Intersects(l));
}

TEST(AABB, IntersectsCompletlyInside)
{
    const AABB a(
        {
            -1.,
            -1.,
        },
        {1., 1.});
    const LineSegment l({-0.5, -0.5}, {0.5, 0.5});
    ASSERT_TRUE(a.Intersects(l));
}

TEST(AABB, DoesNotIntersect)
{
    const AABB a(
        {
            -1.,
            -1.,
        },
        {1., 1.});

    const LineSegment l1({a.TopLeft() + Point{0., 1.}, a.TopRight() + Point{0., 1.}});
    ASSERT_FALSE(a.Intersects(l1));

    const LineSegment l2({a.TopRight() + Point{1., 0.}, a.BottomRight() + Point{1., 0.}});
    ASSERT_FALSE(a.Intersects(l2));

    const LineSegment l3({a.BottomRight() - Point{0., 1.}, a.BottomLeft() - Point{0., 1.}});
    ASSERT_FALSE(a.Intersects(l3));

    const LineSegment l4({a.BottomLeft() - Point{1., 0.}, a.TopLeft() - Point{1., 0.}});
    ASSERT_FALSE(a.Intersects(l4));
}
