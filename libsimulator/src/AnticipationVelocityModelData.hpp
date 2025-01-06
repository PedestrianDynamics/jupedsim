// Copyright © 2012-2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "Point.hpp"

struct AnticipationVelocityModelData {
    double strengthNeighborRepulsion{};
    double rangeNeighborRepulsion{};
    double strengthGeometryRepulsion{};
    double rangeGeometryRepulsion{};
    double anticipationTime{1.0}; // t^a
    double reactionTime{0.3}; // tau
    Point velocity{}; // v
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
            "rangeNeighborRepulsion={}, strengthGeometryRepulsion={}, rangeGeometryRepulsion={}, "
            "timeGap={}, v0={}, radius={}, reactionTime={}, anticipationTime={}, velocity={}])",
            m.strengthNeighborRepulsion,
            m.rangeNeighborRepulsion,
            m.strengthGeometryRepulsion,
            m.rangeGeometryRepulsion,
            m.timeGap,
            m.v0,
            m.radius,
            m.reactionTime,
            m.anticipationTime,
            m.velocity);
    }
};
