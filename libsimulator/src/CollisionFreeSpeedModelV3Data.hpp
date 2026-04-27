// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include <fmt/core.h>

struct CollisionFreeSpeedModelV3Data {
    double strengthNeighborRepulsion{}; // [rad] max steering authority before upper bound
    double rangeNeighborRepulsion{}; // [m] base interaction range for neighbor influence
    double strengthGeometryRepulsion{}; // [-] wall repulsion strength
    double rangeGeometryRepulsion{}; // [m] wall repulsion decay length

    double rangeXScale{20.0}; // [-] forward interaction stretch multiplier
    double rangeYScale{8.0}; // [-] lateral interaction stretch multiplier
    double thetaMaxUpperBound{1.57}; // [rad] hard cap on turn angle per update
    double agentBuffer{0.0}; // [m] stand-off used in speed law: v=0 at s<=buffer

    double timeGap{1};
    double v0{1.2};
    double radius{0.15};
    double headingAngle{0.0}; // [rad] persistent relaxed heading state
};

template <>
struct fmt::formatter<CollisionFreeSpeedModelV3Data> {

    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const CollisionFreeSpeedModelV3Data& m, FormatContext& ctx) const
    {
        return fmt::format_to(
            ctx.out(),
            "CollisionFreeSpeedModelV3[strengthNeighborRepulsion={}, "
            "rangeNeighborRepulsion={}, strengthGeometryRepulsion={}, rangeGeometryRepulsion={}, "
            "rangeXScale={}, rangeYScale={}, thetaMaxUpperBound={}, agentBuffer={}, "
            "timeGap={}, v0={}, radius={}, headingAngle={}])",
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
