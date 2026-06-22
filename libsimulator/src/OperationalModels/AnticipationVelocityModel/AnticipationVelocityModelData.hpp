// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "Point.hpp"

#include <fmt/core.h>

struct AnticipationVelocityModelData {
    Point orientation{0.0, 0.0};
    double strengthNeighborRepulsion{8.0};
    double rangeNeighborRepulsion{0.1};
    double wallBufferDistance{0.1}; // buff distance of agent to wall
    double anticipationTime{1.0}; // anticipation time
    double reactionTime{0.3}; // reaction time to update direction
    Point velocity{};
    double timeGap{1.06};
    double v0{1.2};
    double radius{0.2};

    AnticipationVelocityModelData() = default;
    AnticipationVelocityModelData(
        Point orientation_,
        double strengthNeighborRepulsion_,
        double rangeNeighborRepulsion_,
        double wallBufferDistance_,
        double anticipationTime_,
        double reactionTime_,
        double timeGap_,
        double v0_,
        double radius_)
        : orientation(orientation_.Normalized())
        , strengthNeighborRepulsion(strengthNeighborRepulsion_)
        , rangeNeighborRepulsion(rangeNeighborRepulsion_)
        , wallBufferDistance(wallBufferDistance_)
        , anticipationTime(anticipationTime_)
        , reactionTime(reactionTime_)
        , timeGap(timeGap_)
        , v0(v0_)
        , radius(radius_)
    {
    }
};

template <>
struct fmt::formatter<AnticipationVelocityModelData> {

    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const AnticipationVelocityModelData& m, FormatContext& ctx) const
    {
        return fmt::format_to(
            ctx.out(),
            "AnticipationVelocityModel[orientation={}, strengthNeighborRepulsion={}, "
            "rangeNeighborRepulsion={}, wallBufferDistance={}, "
            "timeGap={}, v0={}, radius={}, reactionTime={}, anticipationTime={}, velocity={}])",
            m.orientation,
            m.strengthNeighborRepulsion,
            m.rangeNeighborRepulsion,
            m.wallBufferDistance,
            m.timeGap,
            m.v0,
            m.radius,
            m.reactionTime,
            m.anticipationTime,
            m.velocity);
    }
};
