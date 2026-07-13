// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "Point.hpp"
struct SocialForceModel2LvlPedData {
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
    double agentScale{}; // A, social repulsion amplitude vs agents [N]
    double obstacleScale{}; // A_w repulsion amplitude vs walls [N]
    double legScale{}; // A_leg repulsion amplitude for ground support [N]
    double forceDistance{}; // B social interaction range [m]
    double obstacleForceDistance{}; // B_w wall interaction range [m]
    double legForceDistance{}; // B_leg leg interaction range [m]
    double radius{}; // r upper body radius [m]
    double bodyForce{}; // stiffness for hard contact [N]
    double friction{}; // friction coefficient [kg/s]
};

template <>
struct fmt::formatter<SocialForceModel2LvlPedData> {

    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const SocialForceModel2LvlPedData& m, FormatContext& ctx) const
    {
        return fmt::format_to(
            ctx.out(),
            "2LvlPed[velocity={}, gs_pos={}, gs_vel={}, h={}, v0={}, tau={}, "
            "lambda_u={}, lambda_b={}, v_s={}, lambda={}, A={}, A_w={}, A_leg={}, B={}, B_w={}, B_leg={}, "
            "r={}, k={}, mu={}])",
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
            m.obstacleScale,
            m.legScale,
            m.forceDistance,
            m.obstacleForceDistance,
            m.legForceDistance,
            m.radius,
            m.bodyForce,
            m.friction);
    }
};
