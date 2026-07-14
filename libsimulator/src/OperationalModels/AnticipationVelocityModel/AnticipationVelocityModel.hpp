// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "AgentJourney.hpp"
#include "CollisionGeometry.hpp"
#include "GenericAgentState.hpp"
#include "LineSegment.hpp"
#include "OperationalModel.hpp"
#include "OperationalModelType.hpp"

#include <fmt/core.h>

#include <cstdint>
#include <random>
#include <vector>

class AnticipationVelocityModel : public OperationalModel
{
public:
    using State = AvmState;

private:
    /// Add a small outward component to maintain minimum distance from walls.
    double _pushoutStrength{0.3};
    double _cutOffRadius{3};
    // Shared sequential RNG: draws must stay on the model to keep simulations deterministic.
    mutable std::mt19937 gen;

public:
    using OperationalModel::GenericState;
    using OperationalModel::StateContainer;

    AnticipationVelocityModel(double pushoutStrength, uint64_t rng_seed);
    ~AnticipationVelocityModel() override = default;
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
    Point CalculateInfluenceDirection(
        const Point& desiredDirection,
        const Point& predictedDirection) const;
    double GetSpacing(const State& state1, const State& state2, const Point& direction) const;
    Point
    NeighborRepulsion(const State& state1, const State& state2, const AgentJourney& journey) const;

    Point HandleWallAvoidance(
        const Point& direction,
        const Point& agentPosition,
        double agentRadius,
        const std::vector<LineSegment>& boundary,
        double wallBufferDistance,
        double pushoutStrength) const;

    Point UpdateDirection(
        const State& state,
        const AgentJourney& journey,
        const Point& calculatedDirection,
        double dt) const;
};
