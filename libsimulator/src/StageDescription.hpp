/// Copyright © 2012-2022 Forschungszentrum Jülich GmbH
/// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "Point.hpp"

#include <variant>
#include <vector>

struct WaypointDescription {
    Point position;
    double distance;
};

struct ExitDescription {
    /// Constraints:
    /// * last to first point linesegment is implied
    /// * lineloop has to form ccw orienetd convex polygon
    std::vector<Point> lineloop;
};

using StageDescription = std::variant<WaypointDescription, ExitDescription>;
