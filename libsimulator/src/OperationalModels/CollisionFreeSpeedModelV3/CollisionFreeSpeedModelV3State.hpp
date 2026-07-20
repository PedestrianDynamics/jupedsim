// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "Point.hpp"

#include <fmt/core.h>

struct CollisionFreeSpeedModelV3State {
    Point position{};
    Point orientation{1.0, 0.0};
    double strengthNeighborRepulsion{8.0};
    double rangeNeighborRepulsion{0.1};
    double strengthGeometryRepulsion{5.0};
    double rangeGeometryRepulsion{0.02};
    double rangeXScale{20.0};
    double rangeYScale{8.0};
    double thetaMaxUpperBound{1.57};
    double agentBuffer{0.0};
    double timeGap{1};
    double v0{1.2};
    double radius{0.2};
    double headingAngle{0.0}; // [rad] persistent relaxed heading state

    bool operator==(const CollisionFreeSpeedModelV3State&) const = default;
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
            m.strengthNeighborRepulsion,
            m.rangeNeighborRepulsion,
            m.strengthGeometryRepulsion,
            m.rangeGeometryRepulsion,
            m.rangeXScale,
            m.rangeYScale,
            m.thetaMaxUpperBound,
            m.agentBuffer,
            m.timeGap,
            m.v0,
            m.radius,
            m.headingAngle);
    }
};