// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "AgentJourney.hpp"
#include "CollisionGeometry.hpp"
#include "GenericAgentState.hpp"
#include "LineSegment.hpp"
#include "OperationalModel.hpp"
#include "OperationalModelType.hpp"

#include <fmt/core.h>

class CollisionFreeSpeedModelV2 : public OperationalModel
{
public:
    using State = CfsmV2State;

private:
    double _cutOffRadius{3};

public:
    using OperationalModel::GenericState;
    using OperationalModel::StateContainer;

    CollisionFreeSpeedModelV2() = default;
    ~CollisionFreeSpeedModelV2() override = default;
    OperationalModelType Type() const override;

    void GetNeighbors(
        const GenericState& current,
        const NeighborhoodSearch<GenericAgent>& neighborhoodsearch,
        const CollisionGeometry& geometry,
        StateContainer& neighbor_states) const override;

    void ComputeNextState(
        double dT,
        const GenericState& current,
        GenericState& next,
        const AgentJourney& journey,
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
