// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once
#include "Point.hpp"

#include <fmt/core.h>

struct WarpDriverModelState {
    Point orientation{0.0, 0.0};
    double radius{0.15};
    double v0{1.2};
    double stuckTime{0.0};
    double displacementX{0.0};
    double displacementY{0.0};
    double detourTime{0.0};
    int detourSide{1};
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
