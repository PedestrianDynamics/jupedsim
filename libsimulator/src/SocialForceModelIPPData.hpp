// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "Point.hpp"
struct SocialForceModelIPPData {
    Point velocity{}; // v
    Point ground_support_position{}; // position of ground support circle
    Point ground_support_velocity{}; // velocity of ground support circle
    double height{}; // height of the agent [m]
    double desiredSpeed{}; // v0 [m/s]
    double reactionTime{}; // tau [s]
    double lambdaU{}; // unbalancing rate [1/s]
    double lambdaB{}; // balancing rate [1/s]
    double balanceSpeed{}; // v_s coupling speed [m/s]
    double damping{}; // lambda velocity dissipation [1/s]
    double agentScale{}; // A repulsion amplitude [N]
    double forceDistance{}; // B upper body interaction range [m]
    double legForceDistance{}; // B_leg leg interaction range [m]
    double radius{}; // r upper body radius [m]
};

template <>
struct fmt::formatter<SocialForceModelIPPData> {

    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const SocialForceModelIPPData& m, FormatContext& ctx) const
    {
        return fmt::format_to(
            ctx.out(),
            "IPP[velocity={}, gs_pos={}, gs_vel={}, h={}, v0={}, tau={}, "
            "lambda_u={}, lambda_b={}, v_s={}, lambda={}, A={}, B={}, B_leg={}, r={}])",
            m.velocity,
            m.ground_support_position,
            m.ground_support_velocity,
            m.height,
            m.desiredSpeed,
            m.reactionTime,
            m.lambdaU,
            m.lambdaB,
            m.balanceSpeed,
            m.damping,
            m.agentScale,
            m.forceDistance,
            m.legForceDistance,
            m.radius);
    }
};
