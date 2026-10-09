// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once
#include "Point.hpp"

#include <fmt/core.h>

struct GeneralizedCentrifugalForceModelState {
    Point orientation{1.0, 0.0};
    double speed{};
    Point e0{};
    int orientation_delay{};
    double mass{1.0};
    double tau{0.5};
    double v0{1.2};
    double av{1.0};
    double a_min{0.2};
    double b_min{0.2};
    double b_max{0.4};
};

template <>
struct fmt::formatter<GeneralizedCentrifugalForceModelState> {
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const GeneralizedCentrifugalForceModelState& m, FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(), "GCFM[orientation={}, speed={}])", m.orientation, m.speed);
    }
};
