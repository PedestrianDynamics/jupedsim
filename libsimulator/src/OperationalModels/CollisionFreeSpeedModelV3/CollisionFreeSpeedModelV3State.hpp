// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once
#include "Point.hpp"

#include <fmt/core.h>

struct CollisionFreeSpeedModelV3State {
    Point orientation{1.0, 0.0};
    double strength_neighbor_repulsion{8.0};
    double range_neighbor_repulsion{0.1};
    double strength_geometry_repulsion{5.0};
    double range_geometry_repulsion{0.02};

    double range_x_scale{20.0};
    double range_y_scale{8.0};
    double theta_max_upper_bound{1.57};
    double agent_buffer{0.0};

    double time_gap{1};
    double v0{1.2};
    double radius{0.2};
    double heading_angle{0.0};
};

template <>
struct fmt::formatter<CollisionFreeSpeedModelV3State> {
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const CollisionFreeSpeedModelV3State& m, FormatContext& ctx) const
    {
        return fmt::format_to(
            ctx.out(),
            "CollisionFreeSpeedModelV3[orientation={}, strengthNeighborRepulsion={}, "
            "rangeNeighborRepulsion={}, strengthGeometryRepulsion={}, rangeGeometryRepulsion={}, "
            "rangeXScale={}, rangeYScale={}, thetaMaxUpperBound={}, agentBuffer={}, "
            "timeGap={}, v0={}, radius={}, headingAngle={}])",
            m.orientation,
            m.strength_neighbor_repulsion,
            m.range_neighbor_repulsion,
            m.strength_geometry_repulsion,
            m.range_geometry_repulsion,
            m.range_x_scale,
            m.range_y_scale,
            m.theta_max_upper_bound,
            m.agent_buffer,
            m.time_gap,
            m.v0,
            m.radius,
            m.heading_angle);
    }
};
