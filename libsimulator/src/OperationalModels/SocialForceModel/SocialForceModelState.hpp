// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "Point.hpp"

#include <fmt/core.h>

struct SocialForceModelState {
    Point position{};
    Point velocity{};
    double mass{80.0};
    double desiredSpeed{0.8};
    double reactionTime{0.5};
    double agentScale{2000.0};
    double obstacleScale{2000.0};
    double forceDistance{0.08};
    double radius{0.3};
    double bodyForce{120000};
    double friction{240000};
};

template <>
struct fmt::formatter<SocialForceModelState> {
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }
    template <typename FormatContext>
    auto format(const SocialForceModelState& m, FormatContext& ctx) const
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