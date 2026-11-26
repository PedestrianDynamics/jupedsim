// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once
#include "kernel.hpp"

#include <fmt/core.h>

struct CollisionFreeSpeedModelState {
    Vector orientation{0.0, 0.0};
    // Defaults from a joint calibration on the Hermes bottleneck and CrowdQueue gate
    // experiments, see https://pedestriandynamics.org/notes/dakota-defaults/
    double strength_neighbor_repulsion{2.83};
    double range_neighbor_repulsion{0.15};
    double strength_geometry_repulsion{5.95};
    double range_geometry_repulsion{0.021};
    /// Weight in [0, 1] of the vision factor on neighbor repulsion; 0 is isotropic,
    /// 1 ignores neighbors behind the agent.
    double neighbor_repulsion_anisotropy{0.5};

    double time_gap{0.956};
    double v0{1.2};
    double radius{0.155};
};

template <>
struct fmt::formatter<CollisionFreeSpeedModelState> {
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const CollisionFreeSpeedModelState& m, FormatContext& ctx) const
    {
        return fmt::format_to(
            ctx.out(),
            "CollisionFreeSpeedModel[orientation={}, strengthNeighborRepulsion={}, "
            "rangeNeighborRepulsion={}, strengthGeometryRepulsion={}, rangeGeometryRepulsion={}, "
            "neighborRepulsionAnisotropy={}, timeGap={}, v0={}, radius={}]",
            m.orientation,
            m.strength_neighbor_repulsion,
            m.range_neighbor_repulsion,
            m.strength_geometry_repulsion,
            m.range_geometry_repulsion,
            m.neighbor_repulsion_anisotropy,
            m.time_gap,
            m.v0,
            m.radius);
    }
};
