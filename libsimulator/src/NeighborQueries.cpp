// SPDX-License-Identifier: LGPL-3.0-or-later
#include "NeighborQueries.hpp"

#include "CollisionGeometry.hpp"
#include "GeometricFunctions.hpp"
#include "LineSegment.hpp"
#include "NeighborhoodSearch.hpp"

#include <algorithm>
#include <iterator>

namespace
{
std::vector<OperationalModelState> intoStates(const std::vector<GenericAgent>& agents)
{
    std::vector<OperationalModelState> states;
    states.reserve(agents.size());
    std::transform(
        std::begin(agents), std::end(agents), std::back_inserter(states), [](const auto& agent) {
            return agent.model;
        });
    return states;
}
} // namespace

//==============================================================================
// VisibleNeighborQuery
//==============================================================================

VisibleNeighborQuery::VisibleNeighborQuery(
    GenericAgent::ID idToExclude,
    const CollisionGeometry& geometry,
    const NeighborhoodSearch<GenericAgent>& neighborhoodSearch)
    : _excludedId(idToExclude), _geometry(geometry), _neighborhoodSearch(neighborhoodSearch)
{
}

std::vector<OperationalModelState>
VisibleNeighborQuery::operator()(Point position, double radius) const
{
    auto neighborhood = _neighborhoodSearch.GetNeighboringAgents(position, radius);
    const auto& boundary = _geometry.LineSegmentsInApproxDistanceTo(position);

    // Drop the excluded agent itself and any neighbor whose line of sight from `position`
    // is blocked by a boundary segment.
    std::erase_if(neighborhood, [this, &position, &boundary](const auto& neighbor) {
        if(_excludedId == neighbor.id) {
            return true;
        }
        const auto agent_to_neighbor = LineSegment(position, neighbor.position());
        return std::any_of(
            boundary.cbegin(), boundary.cend(), [&agent_to_neighbor](const auto& boundary_segment) {
                return intersects(agent_to_neighbor, boundary_segment);
            });
    });
    return intoStates(neighborhood);
}

//==============================================================================
// ProximityNeighborQuery
//==============================================================================

ProximityNeighborQuery::ProximityNeighborQuery(
    GenericAgent::ID idToExclude,
    const NeighborhoodSearch<GenericAgent>& neighborhoodSearch)
    : _excludedId(idToExclude), _neighborhoodSearch(neighborhoodSearch)
{
}

std::vector<OperationalModelState>
ProximityNeighborQuery::operator()(Point position, double radius) const
{
    auto neighborhood = _neighborhoodSearch.GetNeighboringAgents(position, radius);
    std::erase_if(
        neighborhood, [this](const auto& neighbor) { return _excludedId == neighbor.id; });
    return intoStates(neighborhood);
}
