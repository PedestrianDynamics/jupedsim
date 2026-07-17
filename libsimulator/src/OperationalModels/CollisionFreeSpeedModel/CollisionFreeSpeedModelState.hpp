// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "Point.hpp"

#include <fmt/core.h>

/// Per-agent state of the collision free speed model.
struct CollisionFreeSpeedModelState {
    Point position{};
    Point orientation{0.0, 0.0};
    double timeGap{1};
    double v0{1.2};
    double radius{0.2};

    bool operator==(const CollisionFreeSpeedModelState&) const = default;
};

template <>
struct fmt::formatter<CollisionFreeSpeedModelState> {

    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const CollisionFreeSpeedModelState& m, FormatContext& ctx) const
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
