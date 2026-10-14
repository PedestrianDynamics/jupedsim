// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "polygon.hpp"

#include <cstddef>
#include <variant>
#include <vector>

struct DirectSteeringDescription {};

/// Input area of a stage
struct StageArea {
    Polygon polygon;
    std::size_t region_id;
};

struct WaypointDescription {
    std::vector<StageArea> areas;
};

struct ExitDescription {
    std::vector<StageArea> areas;
};

using StageDescription =
    std::variant<DirectSteeringDescription, WaypointDescription, ExitDescription>;
