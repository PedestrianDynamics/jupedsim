// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "Point.hpp"

#include <fmt/core.h>

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
    double strengthNeighborRepulsion{0.3};
    double strengthGeometryRepulsion{0.2};
    double maxNeighborInteractionDistance{2};
    double maxGeometryInteractionDistance{2};
    double maxNeighborInterpolationDistance{0.1};
    double maxGeometryInterpolationDistance{0.1};
    double maxNeighborRepulsionForce{9};
    double maxGeometryRepulsionForce{3};
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