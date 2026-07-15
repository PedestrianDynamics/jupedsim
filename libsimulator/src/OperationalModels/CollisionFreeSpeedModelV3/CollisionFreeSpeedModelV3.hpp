// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "CollisionFreeSpeedModelV3State.hpp"
#include "CollisionGeometry.hpp"
#include "LineSegment.hpp"
#include "OperationalModel.hpp"
#include "OperationalModelType.hpp"
#include "TacticalModelState.hpp"

#include <fmt/core.h>

class CollisionFreeSpeedModelV3 : public OperationalModel
{
public:
    using State = CollisionFreeSpeedModelV3State;

public:
    using OperationalModel::GenericState;
    using OperationalModel::StateContainer;

    CollisionFreeSpeedModelV3() { _cutOffRadius = 3; }
    ~CollisionFreeSpeedModelV3() override = default;
    OperationalModelType Type() const override;

    void ComputeNextState(
        double dT,
        const GenericState& current,
        GenericState& next,
        const TacticalModelState& tactical,
        const CollisionGeometry& geometry,
        const StateContainer& neighborStates) const override;

    void CheckModelConstraint(
        const GenericAgent& agent,
        const NeighborhoodSearch<GenericAgent>& neighborhoodSearch,
        const CollisionGeometry& geometry) const override;

private:
    double OptimalSpeed(const State& state, double spacing, double time_gap) const;
    double GetSpacing(const State& state1, const State& state2, const Point& direction) const;
    Point BoundaryRepulsion(const State& state, const LineSegment& boundary_segment) const;
};
