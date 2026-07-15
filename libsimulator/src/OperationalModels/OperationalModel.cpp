// SPDX-License-Identifier: LGPL-3.0-or-later
#include "OperationalModel.hpp"

#include "NeighborhoodSearch.hpp"

void OperationalModel::GetNeighbors(
    const GenericAgent& current,
    const NeighborhoodSearch<GenericAgent>& neighborhoodsearch,
    const CollisionGeometry& geometry,
    StateContainer& neighbor_states) const
{
    neighbor_states = neighborhoodsearch.GetNeighboringAgentStates(
        Pos(current), _cutOffRadius, VisibleNeighborFilter(current, geometry));
}
