// SPDX-License-Identifier: LGPL-3.0-or-later
#include "Geometry/WalkableSurface.hpp"
#include "SimulationError.hpp"

#include <gtest/gtest.h>

#include <vector>

namespace
{
std::vector<Point> get_square(double min_xy, double max_xy)
{
    return {{min_xy, min_xy}, {min_xy, max_xy}, {max_xy, max_xy}, {max_xy, min_xy}};
}
const std::vector<Point> square{get_square(0, 10)};
} // namespace

TEST(WalkableSurface, AcceptsBoundaryOnly)
{
    WalkableSurface surface{};
    EXPECT_EQ(surface.AddRegion({square, {}}, 0.0), 0);
}

TEST(WalkableSurface, AcceptsSimpleWithHole)
{
    WalkableSurface surface{};
    const std::vector<Point> hole{get_square(2, 4)};
    EXPECT_EQ(surface.AddRegion({square, {hole}}, 0.0), 0);
}

TEST(WalkableSurface, RejectsSelfIntersecting)
{
    WalkableSurface surface{};
    const std::vector<Point> bowtie{{0, 0}, {10, 0}, {0, 10}, {10, 10}};
    EXPECT_THROW(surface.AddRegion({bowtie, {}}, 0.0), SimulationError);
}
