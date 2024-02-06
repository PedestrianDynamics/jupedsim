// Copyright © 2012-2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "Point.hpp"
struct GeneralizedCentrifugalForceModelData {
    double speed{};
    Point e0{};
    int orientationDelay{};
    double mass{};
    double tau{};
    double v0{1.2};
    double Av{};
    double AMin{};
    double BMin{};
    double BMax;
};

template <>
struct fmt::formatter<GeneralizedCentrifugalForceModelData> {

    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const GeneralizedCentrifugalForceModelData& m, FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(), "GCFM[speed={}])", m.speed);
    }
};
