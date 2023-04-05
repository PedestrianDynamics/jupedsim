/// Copyright © 2012-2022 Forschungszentrum Jülich GmbH
/// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "Point.hpp"
#include "Polygon.hpp"

#include <variant>
#include <vector>

struct WaypointDescription {
    Point position;
    double distance;
};

struct ExitDescription {
    Polygon polygon;
};

using StageDescription = std::variant<WaypointDescription, ExitDescription>;
