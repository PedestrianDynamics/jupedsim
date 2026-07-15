// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "Point.hpp"

#include <fmt/core.h>

struct WarpDriverModelState {
    Point position{};
    Point orientation{0.0, 0.0};
    double radius{0.15};
    double v0{1.2};
    double stuckTime{0.0};
    double anchorX{0.0};
    double anchorY{0.0};
    double detourTime{0.0};
    int detourSide{1};
    double timeHorizon{2.0};
    double stepSize{0.5};
    double timeUncertainty{0.5};
    double velocityUncertaintyX{0.2};
    double velocityUncertaintyY{0.2};
    int numSamples{20};
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