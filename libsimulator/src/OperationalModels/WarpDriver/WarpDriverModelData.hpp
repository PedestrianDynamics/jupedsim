// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include <fmt/core.h>

struct WarpDriverModelData {
    Point orientation{0.0, 0.0};
    double radius{0.15};
    double v0{1.2};
    double stuckTime{0.0}; // elapsed time since anchor was set
    double anchorX{0.0}; // position when stuck tracking began
    double anchorY{0.0};
    double detourTime{0.0}; // remaining time in detour mode
    int detourSide{1}; // +1 = left, -1 = right of desired direction

    WarpDriverModelData() = default;
    WarpDriverModelData(Point orientation_, double radius_, double v0_)
        : orientation(orientation_.Normalized()), radius(radius_), v0(v0_)
    {
    }
};

template <>
struct fmt::formatter<WarpDriverModelData> {

    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const WarpDriverModelData& m, FormatContext& ctx) const
    {
        return fmt::format_to(
            ctx.out(),
            "WarpDriver[orientation={}, radius={}, v0={}]",
            m.orientation,
            m.radius,
            m.v0);
    }
};
