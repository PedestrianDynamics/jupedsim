// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once
#include "point.hpp"

#include <fmt/core.h>

struct WarpDriverModelState {
    Point orientation{0.0, 0.0};
    double radius{0.15};
    double v0{1.2};
    double stuck_time{0.0};
    double displacement_x{0.0};
    double displacement_y{0.0};
    double detour_time{0.0};
    int detour_side{1};
};

template <>
struct fmt::formatter<WarpDriverModelState> {
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const WarpDriverModelState& m, FormatContext& ctx) const
    {
        return fmt::format_to(
            ctx.out(),
            "WarpDriver[orientation={}, radius={}, v0={}]",
            m.orientation,
            m.radius,
            m.v0);
    }
};
