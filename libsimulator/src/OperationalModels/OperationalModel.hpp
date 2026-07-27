// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "OperationalModelState.hpp"
#include "OperationalModelType.hpp"
#include "Point.hpp"
#include "SimulationError.hpp"

#include <fmt/core.h>

#include <string>

class EnvironmentQuery;

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
    OperationalModel() = default;
    virtual ~OperationalModel() = default;

    virtual OperationalModelType Type() const = 0;

    /// Computes the agent state for the next iteration.
    /// "next" arrives as an exact copy of "current"; implementations overwrite only the fields
    /// they change. Other agents must be read exclusively from the frozen current generation,
    /// i.e. via the environment query, never via "next". "destination" is the agent's current
    /// routing waypoint.
    virtual void ComputeNextState(
        double dT,
        const OperationalModelState& current,
        OperationalModelState& next,
        const Point& destination,
        const EnvironmentQuery& envQuery) const = 0;

    virtual void CheckModelConstraint(
        const OperationalModelState& state,
        const EnvironmentQuery& envQuery) const = 0;
};
