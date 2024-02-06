#pragma once

#include "Point.hpp"
struct SocialForceModelData {
    Point velocity{}; // v
    double mass{}; // m
    double desiredSpeed{}; // v0
    double reactionTime{}; // T
    double agentScale{}; // A for other agents
    double obstacleScale{}; // A for obstacles
    double forceDistance{}; // B
    double radius{}; // r
};

template <>
struct fmt::formatter<SocialForceModelData> {

    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const SocialForceModelData& m, FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(), "SFM[velocity={}])", m.velocity);
    }
};