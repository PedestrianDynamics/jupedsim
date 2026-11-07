// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "destination.hpp"
#include "floorfield_routing_engine.hpp"
#include "geometry/area_piece.hpp"
#include "geometry/geometry.hpp"
#include "geometry/location.hpp"
#include "kernel.hpp"
#include "routing_target.hpp"
#include "surface_mesh_shortest_path_routing_engine.hpp"

#include <cstddef>
#include <vector>

/// Routes to registered destinations along floor fields, and to single locations along the
/// shortest path on the surface.
class RoutingEngine
{
public:
    /// Borrows @p geometry (non-owning); the caller keeps it alive for the engine's lifetime.
    explicit RoutingEngine(const Geometry& geometry, FloorfieldOptions options);
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
    Vector get_orientation(const Location& from, const RoutingTarget& to);

    /// Travel time in seconds from @p from to @p to, see FloorfieldRoutingEngine::travel_time.
    double travel_time(const Location& from, const Destination& to) const;

    /// Length of the shortest path from @p from to @p to on the surface, infinite if there is
    /// none, see SurfaceMeshShortestPathRoutingEngine::path_length.
    double path_length(const Location& from, const Location& to);

    /// Sequence trees built for shortest paths, see
    /// SurfaceMeshShortestPathRoutingEngine::trees_built.
    std::size_t trees_built() const { return _shortest_path.trees_built(); }

    /// Placement of the floor field grid of @p region, see FloorfieldRoutingEngine::grid_info.
    GridInfo grid_info(std::size_t region) const;
    /// The default speed field of @p region, see FloorfieldRoutingEngine::default_speed.
    SpeedGrid default_speed(std::size_t region) const;
    /// Replaces the default speed field of @p region, see
    /// FloorfieldRoutingEngine::set_default_speed.
    void set_default_speed(std::size_t region, const std::vector<double>& values);

private:
    FloorfieldRoutingEngine _floorfield;
    SurfaceMeshShortestPathRoutingEngine _shortest_path;
};
