// Copyright © 2012-2023 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "Point.hpp"

struct VelocityModelData {
    double timeGap{1};
    double v0{1.2};
    double radius{0.15};
};

template <>
struct fmt::formatter<VelocityModelData> {

    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const VelocityModelData& m, FormatContext& ctx) const
    {
        return fmt::format_to(
            ctx.out(), "VelocityModel[timeGap={}, v0={}, radius={}])", m.timeGap, m.v0, m.radius);
    }
};
