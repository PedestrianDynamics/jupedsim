// Copyright © 2012-2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "Point.hpp"

struct CollisionFreeSpeedModelIndividualData {
    double strengthNeighborRepulsion{};
    double rangeNeighborRepulsion{};
    double strengthGeometryRepulsion{};
    double rangeGeometryRepulsion{};

    double timeGap{1};
    double v0{1.2};
    double radius{0.15};
};

template <>
struct fmt::formatter<CollisionFreeSpeedModelIndividualData> {

    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const CollisionFreeSpeedModelIndividualData& m, FormatContext& ctx) const
    {
        return fmt::format_to(
            ctx.out(),
            "CollisionFreeSpeedModelIndividual[strengthNeighborRepulsion={}, "
            "rangeNeighborRepulsion={}, strengthGeometryRepulsion={}, rangeGeometryRepulsion={}, "
            "timeGap={}, v0={}, radius={}])",
            m.strengthNeighborRepulsion,
            m.rangeNeighborRepulsion,
            m.strengthGeometryRepulsion,
            m.rangeGeometryRepulsion,
            m.timeGap,
            m.v0,
            m.radius);
    }
};
