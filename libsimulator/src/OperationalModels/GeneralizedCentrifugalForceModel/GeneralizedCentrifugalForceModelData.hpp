// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "Point.hpp"
#include "SimulationError.hpp"

#include <fmt/core.h>
struct GeneralizedCentrifugalForceModelData {
    Point orientation{0.0, 0.0};
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

    GeneralizedCentrifugalForceModelData() =
        default; // [RL, TODO] This does not make sense if we need the orientation to be normalized
                 // to length 1.0
    GeneralizedCentrifugalForceModelData(
        Point orientation_,
        double speed_,
        Point e0_,
        double mass_,
        double tau_,
        double v0_,
        double Av_,
        double AMin_,
        double BMin_,
        double BMax_)
        : orientation(orientation_.Normalized())
        , speed(speed_)
        , e0(e0_)
        , mass(mass_)
        , tau(tau_)
        , v0(v0_)
        , Av(Av_)
        , AMin(AMin_)
        , BMin(BMin_)
        , BMax(BMax_)
    {
        if(orientation.isZeroLength()) {
            throw SimulationError("Orientation is invalid: {}. Length should be 1.", orientation);
        }
    }
};

template <>
struct fmt::formatter<GeneralizedCentrifugalForceModelData> {

    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const GeneralizedCentrifugalForceModelData& m, FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(), "GCFM[orientation={}, speed={}])", m.orientation, m.speed);
    }
};
