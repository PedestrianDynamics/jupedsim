// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "OperationalModelState.hpp"
#include "OperationalModelType.hpp"
#include "Point.hpp"
#include "SimulationError.hpp"

#include <fmt/core.h>

#include <string>

class AgentStep;
class AgentView;
struct GenericAgent;

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

    /// Computes the agent's model state for the next iteration and returns how far it
    /// wants to move during this step. "next" arrives as an exact copy of "current";
    /// implementations overwrite only the fields they change. The returned movement is
    /// binding: the framework applies it as is.
    virtual Point ComputeNextState(
        const OperationalModelState& current,
        OperationalModelState& next,
        const AgentStep& step) const = 0;

    virtual void CheckModelConstraint(const GenericAgent& agent, const AgentView& view) const = 0;
};
