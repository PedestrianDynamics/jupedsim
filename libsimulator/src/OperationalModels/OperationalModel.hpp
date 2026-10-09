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
void validate_constraint(
    T value,
    T value_min,
    T value_max,
    const std::string& name,
    bool exclude_min = false)
{
    if(exclude_min) {
        if(value <= value_min || value > value_max) {
            throw SimulationError(
                "Model constraint violation: {} {} not in allowed range, "
                "{} needs to be in ({},{}]",
                name,
                value,
                name,
                value_min,
                value_max);
        }

    } else {
        if(value < value_min || value > value_max) {
            throw SimulationError(
                "Model constraint violation: {} {} not in allowed range, "
                "{} needs to be in [{},{}]",
                name,
                value,
                name,
                value_min,
                value_max);
        }
    }
}

class OperationalModel
{
public:
    OperationalModel() = default;
    virtual ~OperationalModel() = default;

    virtual OperationalModelType type() const = 0;

    /// Computes the agent's model state for the next iteration and returns how far it
    /// wants to move during this step. "next" arrives as an exact copy of "current";
    /// implementations overwrite only the fields they change. The returned movement is
    /// binding: the framework applies it as is.
    virtual Point compute_next_state(
        const OperationalModelState& current,
        OperationalModelState& next,
        const AgentStep& step) const = 0;

    virtual void check_model_constraint(const GenericAgent& agent, const AgentView& view) const = 0;
};
