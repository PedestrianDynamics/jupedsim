// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once
#include "Point.hpp"

struct CollisionFreeSpeedModelV3Update {
    Point position{};
    Point orientation{};
    double headingAngle{};
};
