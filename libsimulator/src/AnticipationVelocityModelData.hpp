// Copyright © 2012-2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "Point.hpp"

struct AnticipationVelocityModelData {
    double strengthNeighborRepulsion{};
    double rangeNeighborRepulsion{};
    double wallBufferDistance{0.1}; // buff distance of agent to wall
    double anticipationTime{1.0}; // anticipation time
    double reactionTime{0.3}; // reaction time to update direction
    Point velocity{};
    double timeGap{1.06};
    double v0{1.2};
    double radius{0.15};
};

template <>
struct fmt::formatter<AnticipationVelocityModelData> {

    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const AnticipationVelocityModelData& m, FormatContext& ctx) const
    {
        return fmt::format_to(
            ctx.out(),
            "AnticipationVelocityModel[strengthNeighborRepulsion={}, "
            "rangeNeighborRepulsion={}, wallBufferDistance={}, "
            "timeGap={}, v0={}, radius={}, reactionTime={}, anticipationTime={}, velocity={}])",
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
