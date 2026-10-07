// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "Point.hpp"
#include "Polygon.hpp"

#include <variant>
#include <vector>

struct DirectSteeringDescription {
};

struct WaypointDescription {
    Point position;
    double distance;
};

struct ExitDescription {
    Polygon polygon;
};

using StageDescription =
    std::variant<DirectSteeringDescription, WaypointDescription, ExitDescription>;
