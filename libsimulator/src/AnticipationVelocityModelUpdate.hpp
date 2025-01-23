// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once
#include "Point.hpp"

struct AnticipationVelocityModelUpdate {
    Point position{};
    Point velocity{};
    Point orientation{};
};
