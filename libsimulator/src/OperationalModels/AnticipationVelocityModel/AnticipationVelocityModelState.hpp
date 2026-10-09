// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once
#include "Point.hpp"

#include <fmt/core.h>

struct AnticipationVelocityModelState {
    Point orientation{0.0, 0.0};
    double strength_neighbor_repulsion{8.0};
    double range_neighbor_repulsion{0.1};
    double wall_buffer_distance{0.1};
    double anticipation_time{1.0};
    double reaction_time{0.3};
    Point velocity{};
    double time_gap{1.06};
    double v0{1.2};
    double radius{0.2};
};

template <>
struct fmt::formatter<AnticipationVelocityModelState> {
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const AnticipationVelocityModelState& m, FormatContext& ctx) const
    {
        return fmt::format_to(
            ctx.out(),
            "AnticipationVelocityModel[orientation={}, strengthNeighborRepulsion={}, "
            "rangeNeighborRepulsion={}, wallBufferDistance={}, "
            "timeGap={}, v0={}, radius={}, reactionTime={}, anticipationTime={}, velocity={}])",
            m.orientation,
            m.strength_neighbor_repulsion,
            m.range_neighbor_repulsion,
            m.wall_buffer_distance,
            m.time_gap,
            m.v0,
            m.radius,
            m.reaction_time,
            m.anticipation_time,
            m.velocity);
    }
};
