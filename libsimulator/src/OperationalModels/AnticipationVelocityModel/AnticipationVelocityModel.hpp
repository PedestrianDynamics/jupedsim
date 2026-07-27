// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "AnticipationVelocityModelState.hpp"
#include "LineSegment.hpp"
#include "OperationalModel.hpp"
#include "OperationalModelType.hpp"
#include "Point.hpp"

class EnvironmentQuery;

#include <cstdint>
#include <random>
#include <vector>

class AnticipationVelocityModel : public OperationalModel
{
public:
    using State = AnticipationVelocityModelState;

private:
    double _pushoutStrength{0.3};
    double _cutOffRadius{3};
    mutable std::mt19937 gen;

public:
    AnticipationVelocityModel(double pushoutStrength, uint64_t rng_seed);
    ~AnticipationVelocityModel() override = default;
    OperationalModelType Type() const override;
    void ComputeNextState(
        double dT,
        const OperationalModelState& current,
        OperationalModelState& next,
        const Point& destination,
        const EnvironmentQuery& envQuery) const override;
    void CheckModelConstraint(const OperationalModelState& state, const EnvironmentQuery& envQuery)
        const override;

private:
    double OptimalSpeed(const State& state, double spacing, double time_gap) const;
    Point CalculateInfluenceDirection(
        const Point& desiredDirection,
        const Point& predictedDirection) const;
    double GetSpacing(const State& s1, const State& s2, const Point& direction) const;
    Point NeighborRepulsion(const State& s1, const State& s2, const Point& destination) const;

    Point HandleWallAvoidance(
        const Point& direction,
        const Point& agentPosition,
        double agentRadius,
        const auto& boundary,
        double wallBufferDistance,
        double pushoutStrength) const;

    Point UpdateDirection(
        const State& state,
        const Point& destination,
        const Point& calculatedDirection,
        double dt) const;
};
