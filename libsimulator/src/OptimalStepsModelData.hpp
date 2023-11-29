// Copyright © 2012-2023 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "Point.hpp"

struct OptimalStepsModelData {
    double v0{1.2};
    double radius{0.2};

    double nextTimeToAct{0};

    double intimateSpaceWidth{0.45}; // \delta_{i}
    double intimateSpacePower{1};
    double intimateSpaceFactor{1.2};
    double personalSpaceWidth{1.2}; // \delta_{per}
    double personalSpacePower{1};
    double repulsionIntensity{30}; // \mu_p

    // TODO rename
    double geometryWidth{0.8};
    double geometryHeight{6};

    OptimalStepsModelData(double v0_, double radius_) : v0(v0_), radius(radius_){};
};

template <>
struct fmt::formatter<OptimalStepsModelData> {

    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const OptimalStepsModelData& m, FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(), "OptimalStepsModel[v0={}, radius={}])", m.v0, m.radius);
    }
};
