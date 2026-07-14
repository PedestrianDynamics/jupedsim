// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "AgentJourney.hpp"
#include "CollisionGeometry.hpp"
#include "GenericAgentState.hpp"
#include "OperationalModelType.hpp"
#include "SimulationError.hpp"

#include <fmt/core.h>

#include <string>
#include <vector>

template <typename T>
class NeighborhoodSearch;

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
    using GenericState = GenericAgentModel;
    using StateContainer = std::vector<GenericState>;
    virtual void GetNeighbors(
        const GenericState& current,
        const NeighborhoodSearch<GenericAgent>& neighborhoodsearch,
        const CollisionGeometry& geometry,
        StateContainer& neighbor_states) const = 0;

    /// Computes the agent state for the next iteration.
    /// "next" arrives as an exact copy of "current"; implementations overwrite only the fields
    /// they change. Other agents must be read exclusively from the frozen current generation,
    /// i.e. via "current" and the neighborhood search, never via "next".
    virtual void ComputeNextState(
        double dT,
        const GenericState& current,
        GenericState& next,
        const AgentJourney& journey,
        const CollisionGeometry& geometry,
        const StateContainer& neighborStates) const = 0;

    virtual void CheckModelConstraint(
        const GenericAgent& agent,
        const NeighborhoodSearch<GenericAgent>& neighborhoodSearch,
        const CollisionGeometry& geometry) const = 0;
};
