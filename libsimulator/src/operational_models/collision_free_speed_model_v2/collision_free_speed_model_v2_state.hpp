// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once
#include "point.hpp"

#include <fmt/core.h>

struct CollisionFreeSpeedModelV2State {
    Point orientation{0.0, 0.0};
    double strength_neighbor_repulsion{8.0};
    double range_neighbor_repulsion{0.1};
    double strength_geometry_repulsion{5.0};
    double range_geometry_repulsion{0.02};

    double time_gap{1};
    double v0{1.2};
    double radius{0.2};
};

template <>
struct fmt::formatter<CollisionFreeSpeedModelV2State> {
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const CollisionFreeSpeedModelV2State& m, FormatContext& ctx) const
    {
        return fmt::format_to(
            ctx.out(),
            "CollisionFreeSpeedModelV2[orientation={}, strengthNeighborRepulsion={}, "
            "rangeNeighborRepulsion={}, strengthGeometryRepulsion={}, rangeGeometryRepulsion={}, "
            "timeGap={}, v0={}, radius={}])",
            m.orientation,
            m.strength_neighbor_repulsion,
            m.range_neighbor_repulsion,
            m.strength_geometry_repulsion,
            m.range_geometry_repulsion,
            m.time_gap,
            m.v0,
            m.radius);
    }
};
