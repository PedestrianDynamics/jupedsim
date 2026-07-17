// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "Point.hpp"

#include <fmt/core.h>

/// Per-agent state of the generalized centrifugal force model.
struct GeneralizedCentrifugalForceModelState {
    Point position{};
    Point orientation{1.0, 0.0};
    double speed{};
    Point e0{};
    int orientationDelay{};
    double mass{1.0};
    double tau{0.5};
    double v0{1.2};
    double Av{1.0};
    double AMin{0.2};
    double BMin{0.2};
    double BMax{0.4};

    bool operator==(const GeneralizedCentrifugalForceModelState&) const = default;
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
