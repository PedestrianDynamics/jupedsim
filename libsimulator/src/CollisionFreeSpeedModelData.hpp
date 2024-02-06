// Copyright © 2012-2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "Point.hpp"

struct CollisionFreeSpeedModelData {
    double timeGap{1};
    double v0{1.2};
    double radius{0.15};
};

template <>
struct fmt::formatter<CollisionFreeSpeedModelData> {

    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const CollisionFreeSpeedModelData& m, FormatContext& ctx) const
    {
        return fmt::format_to(
            ctx.out(),
            "CollisionFreeSpeedModel[timeGap={}, v0={}, radius={}])",
            m.timeGap,
            m.v0,
            m.radius);
    }
};
