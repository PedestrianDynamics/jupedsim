// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "CollisionGeometry.hpp"
#include "LineSegment.hpp"
#include "OperationalModel.hpp"
#include "OperationalModelType.hpp"
#include "Point.hpp"

#include <fmt/core.h>

class CollisionFreeSpeedModelV3 : public OperationalModel
{
public:
    /// Per-agent state of the collision free speed model v3.
    struct State {
        Point position{};
        Point orientation{1.0, 0.0};
        double strengthNeighborRepulsion{8.0}; // [rad] max steering authority before upper bound
        double rangeNeighborRepulsion{0.1}; // [m] base interaction range for neighbor influence
        double strengthGeometryRepulsion{5.0}; // [-] wall repulsion strength
        double rangeGeometryRepulsion{0.02}; // [m] wall repulsion decay length

        double rangeXScale{20.0}; // [-] forward interaction stretch multiplier
        double rangeYScale{8.0}; // [-] lateral interaction stretch multiplier
        double thetaMaxUpperBound{1.57}; // [rad] hard cap on turn angle per update
        double agentBuffer{0.0}; // [m] stand-off used in speed law: v=0 at s<=buffer

        double timeGap{1};
        double v0{1.2};
        double radius{0.2};
        double headingAngle{0.0}; // [rad] persistent relaxed heading state
    };

private:
    double _cutOffRadius{3};

public:
    CollisionFreeSpeedModelV3() = default;
    ~CollisionFreeSpeedModelV3() override = default;
    OperationalModelType Type() const override;
    void ComputeNextState(
        double dT,
        const GenericAgent& current,
        GenericAgent& next,
        const CollisionGeometry& geometry,
        const NeighborhoodSearch<GenericAgent>& neighborhoodSearch) const override;
    void CheckModelConstraint(
        const GenericAgent& agent,
        const NeighborhoodSearch<GenericAgent>& neighborhoodSearch,
        const CollisionGeometry& geometry) const override;

private:
    double OptimalSpeed(const GenericAgent& ped, double spacing, double time_gap) const;
    double
    GetSpacing(const GenericAgent& ped1, const GenericAgent& ped2, const Point& direction) const;
    Point BoundaryRepulsion(const GenericAgent& ped, const LineSegment& boundary_segment) const;
};

template <>
struct fmt::formatter<CollisionFreeSpeedModelV3::State> {

    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const CollisionFreeSpeedModelV3::State& m, FormatContext& ctx) const
    {
        return fmt::format_to(
            ctx.out(),
            "CollisionFreeSpeedModelV3[orientation={}, strengthNeighborRepulsion={}, "
            "rangeNeighborRepulsion={}, strengthGeometryRepulsion={}, rangeGeometryRepulsion={}, "
            "rangeXScale={}, rangeYScale={}, thetaMaxUpperBound={}, agentBuffer={}, "
            "timeGap={}, v0={}, radius={}, headingAngle={}])",
            m.orientation,
            m.strengthNeighborRepulsion,
            m.rangeNeighborRepulsion,
            m.strengthGeometryRepulsion,
            m.rangeGeometryRepulsion,
            m.rangeXScale,
            m.rangeYScale,
            m.thetaMaxUpperBound,
            m.agentBuffer,
            m.timeGap,
            m.v0,
            m.radius,
            m.headingAngle);
    }
};
