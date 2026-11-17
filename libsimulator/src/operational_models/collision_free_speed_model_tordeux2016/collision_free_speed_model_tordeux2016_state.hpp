// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once
#include "kernel.hpp"

#include <fmt/core.h>

struct CollisionFreeSpeedModelTordeux2016State {
    Vector orientation{0.0, 0.0};
    double time_gap{1};
    double v0{1.2};
    double radius{0.2};
};

template <>
struct fmt::formatter<CollisionFreeSpeedModelTordeux2016State> {
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const CollisionFreeSpeedModelTordeux2016State& m, FormatContext& ctx) const
    {
        return fmt::format_to(
            ctx.out(),
            "CollisionFreeSpeedModelTordeux2016[orientation={}, timeGap={}, v0={}, radius={}])",
            m.orientation,
            m.time_gap,
            m.v0,
            m.radius);
    }
};
