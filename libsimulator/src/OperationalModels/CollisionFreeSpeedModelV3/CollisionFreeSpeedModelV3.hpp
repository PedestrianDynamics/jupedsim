// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "CollisionFreeSpeedModelV3State.hpp"
#include "CollisionGeometry.hpp"
#include "LineSegment.hpp"
#include "OperationalModel.hpp"
#include "OperationalModelType.hpp"
#include "Point.hpp"

class CollisionFreeSpeedModelV3 : public OperationalModel
{
public:
    using State = CollisionFreeSpeedModelV3State;

private:
    double _cutOffRadius{3};

public:
    CollisionFreeSpeedModelV3() = default;
    ~CollisionFreeSpeedModelV3() override = default;
    OperationalModelType Type() const override;
    void ComputeNextState(
        double dT,
        const OperationalModelState& current,
        OperationalModelState& next,
        Point destination,
        const CollisionGeometry& geometry,
        const NeighborQuery& neighborQuery) const override;
    void CheckModelConstraint(
        const OperationalModelState& state,
        const NeighborQuery& neighborQuery,
        const CollisionGeometry& geometry) const override;

private:
    double OptimalSpeed(const State& model, double spacing, double time_gap) const;
    double GetSpacing(const State& model1, const State& model2, const Point& direction) const;
    Point BoundaryRepulsion(const State& model, const LineSegment& boundary_segment) const;
};
