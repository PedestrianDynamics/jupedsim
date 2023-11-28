// Copyright © 2012-2023 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "Point.hpp"

struct OptimalStepsModelData {
    double nextTimeToAct{0};
    double timeGap{1};
    double v0{1.2};
    double radius{0.15};

    double intimateSpaceWidth{}; // \delta_{i}
    double intimateSpacePower{};
    double intimateSpaceFactor{};
    double personalSpaceWidth{}; // \delta_{per}
    double personalSpacePower{};
    double repulsionIntensity{}; // \mu_p

    double geometryWidth{};
    double geometryHeight{};
};

template <>
struct fmt::formatter<OptimalStepsModelData> {

    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const OptimalStepsModelData& m, FormatContext& ctx) const
    {
        return fmt::format_to(
            ctx.out(),
            "OptimalStepsModel[timeGap={}, v0={}, radius={}])",
            m.timeGap,
            m.v0,
            m.radius);
    }
};
