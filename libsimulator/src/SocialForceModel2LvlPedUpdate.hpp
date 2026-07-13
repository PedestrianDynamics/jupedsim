// SPDX-License-Identifier: LGPL-3.0-or-later
#include "Point.hpp"

#include <optional>

struct SocialForceModel2LvlPedUpdate {
    Point position{};
    Point ground_support_position{};
    Point ground_support_velocity{};
    Point velocity{};
};
