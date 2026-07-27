// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "CollisionFreeSpeedModelV3State.hpp"
#include "LineSegment.hpp"
#include "OperationalModel.hpp"
#include "OperationalModelType.hpp"
#include "Point.hpp"

class EnvironmentQuery;

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
        const Point& destination,
        const EnvironmentQuery& envQuery) const override;
    void CheckModelConstraint(const OperationalModelState& state, const EnvironmentQuery& envQuery)
        const override;

private:
    double OptimalSpeed(const State& state, double spacing, double time_gap) const;
    double GetSpacing(const State& s1, const State& s2, const Point& direction) const;
    Point BoundaryRepulsion(const State& state, const LineSegment& boundary_segment) const;
};
