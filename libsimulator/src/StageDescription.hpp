// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "Point.hpp"
#include "Polygon.hpp"

#include <cstddef>
#include <variant>
#include <vector>

struct DirectSteeringDescription {
};

struct WaypointDescription {
    Point position;
    double distance;
    /// Region the position lies in, see `Geometry::get_location`.
    std::size_t region_id;
};

struct ExitDescription {
    Polygon polygon;
    /// Region the polygon's centroid lies in, see `Geometry::get_location`.
    std::size_t region_id;
};

using StageDescription =
    std::variant<DirectSteeringDescription, WaypointDescription, ExitDescription>;
