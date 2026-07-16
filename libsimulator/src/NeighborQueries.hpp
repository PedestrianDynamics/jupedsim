// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "GenericAgent.hpp"
#include "NeighborQuery.hpp"

#include <vector>

class CollisionGeometry;

template <typename T>
class NeighborhoodSearch;

/// Neighbor query for the operational decision path: agents within radius whose line of
/// sight from the queried position is not blocked by a boundary segment.
class VisibleNeighborQuery final : public NeighborQuery
{
private:
    GenericAgent::ID _excludedId;
    const CollisionGeometry& _geometry;
    const NeighborhoodSearch<GenericAgent>& _neighborhoodSearch;

public:
    explicit VisibleNeighborQuery(
        GenericAgent::ID idToExclude,
        const CollisionGeometry& geometry,
        const NeighborhoodSearch<GenericAgent>& neighborhoodSearch);
    std::vector<GenericAgent> operator()(Point position, double radius) const override;
};

/// Neighbor query for constraint checks: agents within radius, deliberately without
/// visibility filtering — overlap checks must see agents through walls. Passing
/// GenericAgent::ID::Invalid excludes nobody; this is the expected value when validating
/// an agent that has not been added to the neighborhood search yet.
class ProximityNeighborQuery final : public NeighborQuery
{
private:
    GenericAgent::ID _excludedId;
    const NeighborhoodSearch<GenericAgent>& _neighborhoodSearch;

public:
    explicit ProximityNeighborQuery(
        GenericAgent::ID idToExclude,
        const NeighborhoodSearch<GenericAgent>& neighborhoodSearch);
    std::vector<GenericAgent> operator()(Point position, double radius) const override;
};
