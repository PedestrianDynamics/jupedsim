// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "CollisionFreeSpeedModelState.hpp"
#include "CollisionGeometry.hpp"
#include "LineSegment.hpp"
#include "OperationalModel.hpp"
#include "OperationalModelType.hpp"
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

public:
    using OperationalModel::GenericState;
    using OperationalModel::StateContainer;

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
    Point NeighborRepulsion(const State& state1, const State& state2) const;
    Point BoundaryRepulsion(const State& state, const LineSegment& boundary_segment) const;
};
