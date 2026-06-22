// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include <fmt/core.h>

struct CollisionFreeSpeedModelV2Data {
    Point orientation{0.0, 0.0};
    double strengthNeighborRepulsion{8.0};
    double rangeNeighborRepulsion{0.1};
    double strengthGeometryRepulsion{5.0};
    double rangeGeometryRepulsion{0.02};

    double timeGap{1};
    double v0{1.2};
    double radius{0.2};

    CollisionFreeSpeedModelV2Data() = default;
    CollisionFreeSpeedModelV2Data(
        Point orientation_,
        double strengthNeighborRepulsion_,
        double rangeNeighborRepulsion_,
        double strengthGeometryRepulsion_,
        double rangeGeometryRepulsion_,
        double timeGap_,
        double v0_,
        double radius_)
        : orientation(orientation_.Normalized())
        , strengthNeighborRepulsion(strengthNeighborRepulsion_)
        , rangeNeighborRepulsion(rangeNeighborRepulsion_)
        , strengthGeometryRepulsion(strengthGeometryRepulsion_)
        , rangeGeometryRepulsion(rangeGeometryRepulsion_)
        , timeGap(timeGap_)
        , v0(v0_)
        , radius(radius_)
    {
    }
};

template <>
struct fmt::formatter<CollisionFreeSpeedModelV2Data> {

    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const CollisionFreeSpeedModelV2Data& m, FormatContext& ctx) const
    {
        return fmt::format_to(
            ctx.out(),
            "CollisionFreeSpeedModelV2[orientation={}, strengthNeighborRepulsion={}, "
            "rangeNeighborRepulsion={}, strengthGeometryRepulsion={}, rangeGeometryRepulsion={}, "
            "timeGap={}, v0={}, radius={}])",
            m.orientation,
            m.strengthNeighborRepulsion,
            m.rangeNeighborRepulsion,
            m.strengthGeometryRepulsion,
            m.rangeGeometryRepulsion,
            m.timeGap,
            m.v0,
            m.radius);
    }
};
