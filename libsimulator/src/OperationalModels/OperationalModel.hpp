// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "CollisionGeometry.hpp"
#include "GenericAgent.hpp"
#include "GeometricFunctions.hpp"
#include "LineSegment.hpp"
#include "OperationalModelState.hpp"
#include "OperationalModelType.hpp"
#include "SimulationError.hpp"
#include "TacticalModelState.hpp"

#include <fmt/core.h>

#include <algorithm>
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
    using GenericState = OperationalModelState;
    using StateContainer = std::vector<GenericState>;

protected:
    /// Radius of the neighborhood query in GetNeighbors().
    /// Models set their value in their respective constructor by accessing this variable
    double _cutOffRadius{3};

    /// Neighbor filter for NeighborhoodSearch::GetNeighboringAgentStates(): keeps only
    /// neighbors that are not the agent itself and whose line of sight to the agent is
    /// not blocked by a boundary wall.
    static auto
    VisibleNeighborFilter(const GenericAgent& current, const CollisionGeometry& geometry)
    {
        const auto& boundary = geometry.LineSegmentsInApproxDistanceTo(Pos(current));
        return [&current, &boundary](const GenericAgent& neighbor) {
            if((current.id) == (neighbor.id)) {
                return false;
            }
            const auto agent_to_neighbor = LineSegment(Pos(current), Pos(neighbor));
            return std::none_of(
                boundary.cbegin(), boundary.cend(), [&agent_to_neighbor](const auto& segment) {
                    return intersects(agent_to_neighbor, segment);
                });
        };
    }

public:
    OperationalModel() = default;
    virtual ~OperationalModel() = default;

    virtual OperationalModelType Type() const = 0;

    /// Collects the frozen neighbor states handed to ComputeNextState(). The default
    /// implementation gathers all states within _cutOffRadius that pass
    /// VisibleNeighborFilter(). Models with different neighbor selection override this.
    virtual void GetNeighbors(
        const GenericAgent& current,
        const NeighborhoodSearch<GenericAgent>& neighborhoodsearch,
        const CollisionGeometry& geometry,
        StateContainer& neighbor_states) const;

    /// Computes the agent state for the next iteration.
    /// "next" arrives as an exact copy of "current"; implementations overwrite only the fields
    /// they change. Other agents must be read exclusively from the frozen current generation,
    /// i.e. via "current" and the neighborhood search, never via "next".
    virtual void ComputeNextState(
        double dT,
        const GenericState& current,
        GenericState& next,
        const TacticalModelState& tactical,
        const CollisionGeometry& geometry,
        const StateContainer& neighborStates) const = 0;

    virtual void CheckModelConstraint(
        const GenericAgent& agent,
        const NeighborhoodSearch<GenericAgent>& neighborhoodSearch,
        const CollisionGeometry& geometry) const = 0;
};
