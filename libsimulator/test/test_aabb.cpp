// SPDX-License-Identifier: LGPL-3.0-or-later
#include "aabb.hpp"
#include "simulation_error.hpp"
#include "test_common.hpp"

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
    ASSERT_TRUE(aabb.inside({0.5, 0.5}));
}

TEST(AABB, PointOnBoundaryIsInside)
{
    const AABB aabb({0, 0}, {1, 1});
    ASSERT_TRUE(aabb.inside({0.0, 0.5}));
    ASSERT_TRUE(aabb.inside({0.5, 0.0}));
    ASSERT_TRUE(aabb.inside({1.0, 0.5}));
    ASSERT_TRUE(aabb.inside({0.5, 1.0}));
}

TEST(AABB, CornersAreInside)
{
    const AABB aabb({0, 0}, {1, 1});
    ASSERT_TRUE(aabb.inside({0, 0}));
    ASSERT_TRUE(aabb.inside({1, 0}));
    ASSERT_TRUE(aabb.inside({0, 1}));
    ASSERT_TRUE(aabb.inside({1, 1}));
}

TEST(AABB, NonOverlappingDoNotOverlap)
{
    const AABB a({0, 0}, {1, 1});
    const AABB b({2, 0}, {3, 1});
    ASSERT_FALSE(a.overlap(b));
    ASSERT_FALSE(b.overlap(a));
}

TEST(AABB, OverlappingDoOverlap)
{
    const AABB a({0, 0}, {1, 1});
    const AABB b({0.5, 0}, {1.5, 1});
    ASSERT_TRUE(a.overlap(b));
    ASSERT_TRUE(b.overlap(a));
}

TEST(AABB, OverlappingOnCornerDoOverlap)
{
    const AABB a({0, 0}, {1, 1});
    const AABB b({1, 1}, {2, 2});
    ASSERT_TRUE(a.overlap(b));
    ASSERT_TRUE(b.overlap(a));
}

TEST(AABB, OverlappingSidesDoOverlap)
{
    const AABB a({0, 0}, {1, 1});
    const AABB b({1, 0}, {2, 2});
    ASSERT_TRUE(a.overlap(b));
    ASSERT_TRUE(b.overlap(a));
}

TEST(AABB, IntersectsDiagonal)
{
    const AABB a({3., 2.}, {6., 4.});
    const LineSegment l({3., 2.}, {6., 4.});
    ASSERT_TRUE(a.intersects(l));
}

TEST(AABB, IntersectsDiagonalInverted)
{
    const AABB a({3., 2.}, {6., 4.});
    const LineSegment l({6., 4.}, {3., 2.});
    ASSERT_TRUE(a.intersects(l));
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
    ASSERT_TRUE(a.intersects(l1));
    const LineSegment l2({-1., 1.}, {1., 1.});
    ASSERT_TRUE(a.intersects(l2));
    const LineSegment l3({1., 1.}, {1., -1.});
    ASSERT_TRUE(a.intersects(l3));
    const LineSegment l4({1., -1.}, {-1., -1.});
    ASSERT_TRUE(a.intersects(l4));
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
    ASSERT_TRUE(a.intersects(l1));
    const LineSegment l2({-1., 1.}, {-2., 2.});
    ASSERT_TRUE(a.intersects(l2));
    const LineSegment l3({1., 1.}, {2., 2.});
    ASSERT_TRUE(a.intersects(l3));
    const LineSegment l4({1., -1.}, {2., -2.});
    ASSERT_TRUE(a.intersects(l4));

    const LineSegment l5({-2., 0}, {0., -2.});
    ASSERT_TRUE(a.intersects(l5));
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
    ASSERT_TRUE(a.intersects(l1));
    const LineSegment l2({0., 1.}, {0., 2.});
    ASSERT_TRUE(a.intersects(l2));
    const LineSegment l3({1., 0.}, {2., 0.});
    ASSERT_TRUE(a.intersects(l3));
    const LineSegment l4({0., -1.}, {0., -2.});
    ASSERT_TRUE(a.intersects(l4));
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
    ASSERT_TRUE(a.intersects(l));
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
    ASSERT_TRUE(a.intersects(l));
}

TEST(AABB, DoesNotIntersect)
{
    const AABB a(
        {
            -1.,
            -1.,
        },
        {1., 1.});

    const LineSegment l1({a.top_left() + Point{0., 1.}, a.top_right() + Point{0., 1.}});
    ASSERT_FALSE(a.intersects(l1));

    const LineSegment l2({a.top_right() + Point{1., 0.}, a.bottom_right() + Point{1., 0.}});
    ASSERT_FALSE(a.intersects(l2));

    const LineSegment l3({a.bottom_right() - Point{0., 1.}, a.bottom_left() - Point{0., 1.}});
    ASSERT_FALSE(a.intersects(l3));

    const LineSegment l4({a.bottom_left() - Point{1., 0.}, a.top_left() - Point{1., 0.}});
    ASSERT_FALSE(a.intersects(l4));
}
