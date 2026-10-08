// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "FloorfieldRoutingEngine.hpp"
#include "Geometry/AreaPiece.hpp"
#include "Geometry/Geometry.hpp"
#include "Geometry/Location.hpp"
#include "Point.hpp"
#include "RoutingTarget.hpp"
#include "SurfaceMeshShortestPathRoutingEngine.hpp"

#include <vector>

/// Routes to registered destinations along floor fields, and to single locations along the
/// shortest path on the surface.
class RoutingEngine
{
public:
    /// Borrows @p geometry (non-owning); the caller keeps it alive for the engine's lifetime.
    explicit RoutingEngine(const Geometry& geometry);
    ~RoutingEngine() = default;

    RoutingEngine(const RoutingEngine&) = delete;
    RoutingEngine& operator=(const RoutingEngine&) = delete;
    RoutingEngine(RoutingEngine&&) = delete;
    RoutingEngine& operator=(RoutingEngine&&) = delete;

    /// Registers @p pieces as one destination. Routing heads for whichever piece is nearest in
    /// travel time.
    Destination RegisterDestination(const std::vector<AreaPiece>& pieces);

    /// Unit vector from @p from along the route to @p to, projected to x/y. Zero once @p from
    /// has reached @p to.
    Point GetOrientation(const Location& from, const RoutingTarget& to);

private:
    FloorfieldRoutingEngine _floorfield;
    SurfaceMeshShortestPathRoutingEngine _shortestPath;
};
