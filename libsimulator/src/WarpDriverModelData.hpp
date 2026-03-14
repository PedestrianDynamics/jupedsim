// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include <fmt/core.h>

struct WarpDriverModelData {
    double radius{0.15};
    double v0{1.2};
    int jamCounter{0};
};

template <>
struct fmt::formatter<WarpDriverModelData> {

    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const WarpDriverModelData& m, FormatContext& ctx) const
    {
        return fmt::format_to(
            ctx.out(),
            "WarpDriver[radius={}, v0={}, jamCounter={}]",
            m.radius,
            m.v0,
            m.jamCounter);
    }
};
