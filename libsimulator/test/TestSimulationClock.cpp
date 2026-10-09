// SPDX-License-Identifier: LGPL-3.0-or-later
#include "SimulationClock.hpp"
#include "TestCommon.hpp"

#include <gtest/gtest.h>

TEST(SimulationClock, Construction)
{
    SimulationClock sc{0.5};
    ASSERT_EQ(sc.dt(), 0.5);
    ASSERT_EQ(sc.iteration(), 0);
    ASSERT_EQ(sc.elapsed_time(), 0.0);
}
