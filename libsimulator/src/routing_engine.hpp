// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "floorfield_routing_engine.hpp"
#include "geometry/area_piece.hpp"
#include "geometry/geometry.hpp"
#include "geometry/location.hpp"
#include "point.hpp"
#include "routing_target.hpp"
#include "surface_mesh_shortest_path_routing_engine.hpp"

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
    Destination register_destination(const std::vector<AreaPiece>& pieces);

    /// Unit vector from @p from along the route to @p to, projected to x/y. Zero once @p from
    /// has reached @p to.
    Point get_orientation(const Location& from, const RoutingTarget& to);

private:
    FloorfieldRoutingEngine _floorfield;
    SurfaceMeshShortestPathRoutingEngine _shortest_path;
};
