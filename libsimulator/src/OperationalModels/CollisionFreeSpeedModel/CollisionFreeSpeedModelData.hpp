// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include <fmt/core.h>

struct CollisionFreeSpeedModelData {
    Point orientation{0.0, 0.0};
    double timeGap{1};
    double v0{1.2};
    double radius{0.2};

    CollisionFreeSpeedModelData() = default;
    CollisionFreeSpeedModelData(Point orientation_, double timeGap_, double v0_, double radius_)
        : orientation(orientation_.Normalized()), timeGap(timeGap_), v0(v0_), radius(radius_)
    {
    }
};

template <>
struct fmt::formatter<CollisionFreeSpeedModelData> {

    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const CollisionFreeSpeedModelData& m, FormatContext& ctx) const
    {
        return fmt::format_to(
            ctx.out(),
            "CollisionFreeSpeedModel[orientation={}, timeGap={}, v0={}, radius={}])",
            m.orientation,
            m.timeGap,
            m.v0,
            m.radius);
    }
};
