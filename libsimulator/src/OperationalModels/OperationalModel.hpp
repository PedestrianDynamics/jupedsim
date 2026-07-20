// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "CollisionGeometry.hpp"
#include "OperationalModelState.hpp"
#include "OperationalModelType.hpp"
#include "Point.hpp"
#include "SimulationError.hpp"

#include <fmt/core.h>

#include <string>

class NeighborQuery;

template <typename T>
void validateConstraint(
    T value,
    T valueMin,
    T valueMax,
    const std::string& name,
    bool excludeMin = false)
{
    if(excludeMin) {
        if(value <= valueMin || value > valueMax) {
            throw SimulationError(
                "Model constraint violation: {} {} not in allowed range, "
                "{} needs to be in ({},{}]",
                name,
                value,
                name,
                valueMin,
                valueMax);
        }

    } else {
        if(value < valueMin || value > valueMax) {
            throw SimulationError(
                "Model constraint violation: {} {} not in allowed range, "
                "{} needs to be in [{},{}]",
                name,
                value,
                name,
                valueMin,
                valueMax);
        }
    }
}

class OperationalModel
{
public:
    using StateContainer = std::vector<OperationalModelState>;

    OperationalModel() = default;
    virtual ~OperationalModel() = default;

    virtual OperationalModelType Type() const = 0;

    /// Computes the agent state for the next iteration.
    /// "next" arrives as an exact copy of "current"; implementations overwrite only the fields
    /// they change. Other agents must be read exclusively from the frozen current generation,
    /// i.e. via the neighbor query, which returns their states and already excludes the agent
    /// itself as well as neighbors whose line of sight to the agent is blocked by a boundary
    /// wall. "destination" is the agent's current routing waypoint.
    virtual void ComputeNextState(
        double dT,
        const OperationalModelState& current,
        OperationalModelState& next,
        const Point& destination,
        const CollisionGeometry& geometry,
        const NeighborQuery& neighborQuery) const = 0;

    /// Validates the state of an agent about to be added to the simulation. The neighbor
    /// query is bound to that agent and deliberately not visibility-filtered: overlap checks
    /// must see agents through walls.
    virtual void CheckModelConstraint(
        const OperationalModelState& state,
        const NeighborQuery& neighborQuery,
        const CollisionGeometry& geometry) const = 0;
};
