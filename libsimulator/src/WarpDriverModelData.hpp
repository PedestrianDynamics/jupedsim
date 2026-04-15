// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include <fmt/core.h>

struct WarpDriverModelData {
    double radius{0.15};
    double v0{1.2};
    double stuckTime{0.0}; // elapsed time since anchor was set
    double anchorX{0.0}; // position when stuck tracking began
    double anchorY{0.0};
    double detourTime{0.0}; // remaining time in detour mode
    int detourSide{1}; // +1 = left, -1 = right of desired direction
};

template <>
struct fmt::formatter<WarpDriverModelData> {

    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const WarpDriverModelData& m, FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(), "WarpDriver[radius={}, v0={}]", m.radius, m.v0);
    }
};
