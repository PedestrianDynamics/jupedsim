// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "AnticipationVelocityModelState.hpp"
#include "CollisionGeometry.hpp"
#include "LineSegment.hpp"
#include "OperationalModel.hpp"
#include "OperationalModelType.hpp"
#include "Point.hpp"

#include <cstdint>
#include <random>
#include <vector>

class AnticipationVelocityModel : public OperationalModel
{
public:
    using State = AnticipationVelocityModelState;

private:
    /// Add a small outward component to maintain minimum distance from walls.
    double _pushoutStrength{0.3};
    double _cutOffRadius{3};
    // Shared sequential RNG: draws must stay on the model to keep simulations deterministic.
    mutable std::mt19937 gen;

public:
    AnticipationVelocityModel(double pushoutStrength, uint64_t rng_seed);
    ~AnticipationVelocityModel() override = default;
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
    Point CalculateInfluenceDirection(
        const Point& desiredDirection,
        const Point& predictedDirection) const;
    double GetSpacing(const State& model1, const State& model2, const Point& direction) const;
    Point NeighborRepulsion(const State& model1, Point destination, const State& model2) const;

    Point HandleWallAvoidance(
        const Point& direction,
        const Point& agentPosition,
        double agentRadius,
        const std::vector<LineSegment>& boundary,
        double wallBufferDistance,
        double pushoutStrength) const;

    Point UpdateDirection(
        const State& model,
        Point destination,
        const Point& calculatedDirection,
        double dt) const;
};
