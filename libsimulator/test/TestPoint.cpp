// Copyright © 2012-2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "Point.hpp"

#include <fmt/printf.h>
#include <gtest/gtest.h>

TEST(Point, IsUnitLength)
{
    EXPECT_TRUE(Point(1, 0).IsUnitLength());
    EXPECT_TRUE(Point(-1, 0).IsUnitLength());
    EXPECT_TRUE(Point(0, -1).IsUnitLength());
    EXPECT_TRUE(Point(0, 1).IsUnitLength());
    EXPECT_TRUE(Point(1, 1).Normalized().IsUnitLength());
    EXPECT_FALSE(Point(-1, -1).IsUnitLength());
}
