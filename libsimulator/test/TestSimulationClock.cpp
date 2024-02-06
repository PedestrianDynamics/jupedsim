// Copyright © 2012-2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "SimulationClock.hpp"

#include <gtest/gtest.h>

TEST(SimulationClock, Construction)
{
    SimulationClock sc{0.5};
    ASSERT_EQ(sc.dT(), 0.5);
    ASSERT_EQ(sc.Iteration(), 0);
    ASSERT_EQ(sc.ElapsedTime(), 0.0);
}
