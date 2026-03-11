// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "Point.hpp"

#include <fmt/core.h>
struct SocialForceModelData {
    Point velocity{}; // v
    double mass{80.0}; // m
    double desiredSpeed{0.8}; // v0
    double reactionTime{0.5}; // tau
    double agentScale{2000.0}; // A for other agents
    double obstacleScale{2000.0}; // A for obstacles
    double forceDistance{0.08}; // B
    double radius{0.3}; // r
};

template <>
struct fmt::formatter<SocialForceModelData> {

    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const SocialForceModelData& m, FormatContext& ctx) const
    {
        return fmt::format_to(
            ctx.out(),
            "SFM[velocity={}, m={}, v0={}, tau={}, A_ped={}, A_obst={}, B={}, r={}])",
            m.velocity,
            m.mass,
            m.desiredSpeed,
            m.reactionTime,
            m.agentScale,
            m.obstacleScale,
            m.forceDistance,
            m.radius);
    }
};
