// SPDX-License-Identifier: LGPL-3.0-or-later
#include "Point.hpp"
#include "TestCommon.hpp"

#include <fmt/printf.h>
#include <gtest/gtest.h>

TEST(Point, IsUnitLength)
{
    EXPECT_TRUE(Point(1, 0).is_unit_length());
    EXPECT_TRUE(Point(-1, 0).is_unit_length());
    EXPECT_TRUE(Point(0, -1).is_unit_length());
    EXPECT_TRUE(Point(0, 1).is_unit_length());
    EXPECT_TRUE(Point(1, 1).normalized().is_unit_length());
    EXPECT_FALSE(Point(-1, -1).is_unit_length());
}
