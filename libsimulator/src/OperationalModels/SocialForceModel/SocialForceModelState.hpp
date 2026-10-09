// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once
#include "Point.hpp"

#include <fmt/core.h>

struct SocialForceModelState {
    Point velocity{};
    double mass{80.0};
    double desired_speed{0.8};
    double reaction_time{0.5};
    double agent_scale{2000.0};
    double obstacle_scale{2000.0};
    double force_distance{0.08};
    double radius{0.3};
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
            m.desired_speed,
            m.reaction_time,
            m.agent_scale,
            m.obstacle_scale,
            m.force_distance,
            m.radius);
    }
};
