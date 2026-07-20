// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "CollisionFreeSpeedModelState.hpp"
#include "CollisionGeometry.hpp"
#include "GenericAgent.hpp"
#include "LineSegment.hpp"
#include "NeighborQuery.hpp"
#include "OperationalModel.hpp"
#include "OperationalModelType.hpp"
#include "Point.hpp"
#include "TacticalModelState.hpp"

#include <fmt/core.h>

class CollisionFreeSpeedModel : public OperationalModel
{
public:
    using State = CollisionFreeSpeedModelState;

private:
    double strengthNeighborRepulsion{8.0};
    double rangeNeighborRepulsion{0.1};
    double strengthGeometryRepulsion{5.0};
    double rangeGeometryRepulsion{0.02};

    double _cutOffRadius{3.0};

public:
    CollisionFreeSpeedModel() { _cutOffRadius = 3; }
    CollisionFreeSpeedModel(
        double strengthNeighborRepulsion,
        double rangeNeighborRepulsion,
        double strengthGeometryRepulsion,
        double rangeGeometryRepulsion);
    ~CollisionFreeSpeedModel() override = default;
    OperationalModelType Type() const override;

    void ComputeNextState(
        double dT,
        const OperationalModelState& current,
        OperationalModelState& next,
        const Point& destination,
        const CollisionGeometry& geometry,
        const NeighborQuery& neighborQuery) const override;

    void CheckModelConstraint(
        const OperationalModelState& state,
        const NeighborQuery& neighborQuery,
        const CollisionGeometry& geometry) const override;

private:
    double OptimalSpeed(const State& state, double spacing, double time_gap) const;
    double GetSpacing(const State& state1, const State& state2, const Point& direction) const;
    Point NeighborRepulsion(const State& state1, const State& state2) const;
    Point BoundaryRepulsion(const State& state, const LineSegment& boundary_segment) const;
};
