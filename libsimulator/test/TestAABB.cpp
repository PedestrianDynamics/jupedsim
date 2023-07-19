/// Copyright © 2012-2022 Forschungszentrum Jülich GmbH
/// SPDX-License-Identifier: LGPL-3.0-or-later
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
