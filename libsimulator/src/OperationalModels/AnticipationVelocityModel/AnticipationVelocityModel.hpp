// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "AnticipationVelocityModelState.hpp"
#include "OperationalModel.hpp"
#include "OperationalModelType.hpp"
#include "Point.hpp"

#include <fmt/core.h>

#include <cstdint>
#include <random>
#include <vector>

struct NeighborView;
struct WallView;

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
    Point ComputeNextState(
        const OperationalModelState& current,
        OperationalModelState& next,
        const AgentStep& step) const override;
    void CheckModelConstraint(const GenericAgent& agent, const AgentView& view) const override;

private:
    double OptimalSpeed(const State& currState, double spacing, double time_gap) const;
    Point CalculateInfluenceDirection(
        const Point& desiredDirection,
        const Point& predictedDirection) const;
    double
    GetSpacing(const State& currState, const NeighborView& neighbor, const Point& direction) const;
    Point NeighborRepulsion(
        const State& currState,
        Point toNextTarget,
        const NeighborView& neighbor) const;

    Point HandleWallAvoidance(
        const Point& direction,
        double agentRadius,
        const auto& boundaries,
        double wallBufferDistance,
        double pushoutStrength) const;

    Point UpdateDirection(
        const State& currState,
        Point toNextTarget,
        const Point& calculatedDirection,
        double dt) const;
};
