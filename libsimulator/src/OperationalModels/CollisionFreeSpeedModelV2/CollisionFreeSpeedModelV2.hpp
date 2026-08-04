// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "CollisionFreeSpeedModelV2State.hpp"
#include "OperationalModel.hpp"
#include "OperationalModelType.hpp"
#include "Point.hpp"

#include <fmt/core.h>

struct NeighborView;
struct WallView;

class CollisionFreeSpeedModelV2 : public OperationalModel
{
public:
    using State = CollisionFreeSpeedModelV2State;

private:
    double _cutOffRadius{3};

public:
    CollisionFreeSpeedModelV2() = default;
    ~CollisionFreeSpeedModelV2() override = default;
    OperationalModelType Type() const override;
    Point ComputeNextState(
        const OperationalModelState& current,
        OperationalModelState& next,
        const AgentStep& step) const override;
    void CheckModelConstraint(const GenericAgent& agent, const AgentView& view) const override;

private:
    double OptimalSpeed(const State& currState, double spacing, double time_gap) const;
    double
    GetSpacing(const State& currState, const NeighborView& neighbor, const Point& direction) const;
    Point NeighborRepulsion(const State& currState, const NeighborView& neighbor) const;
    Point BoundaryRepulsion(const State& currState, const WallView& boundary) const;
};
