// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "cfg_cgal.hpp"
#include "geometry/geometry.hpp"
#include "geometry/location.hpp"
#include "kernel.hpp"

#include <CGAL/Surface_mesh_shortest_path.h>

#include <cstddef>
#include <memory>
#include <vector>

/// Point-to-point shortest paths along the surface.
class SurfaceMeshShortestPathRoutingEngine
{
public:
    /// Borrows @p geometry (non-owning); the caller keeps it alive for the
    /// engine's lifetime. Ownership lives with the world (later: Simulation),
    /// matching the 2D pipeline where engines never own the geometry.
    /// @param wallClearance how far a route is held off the wall corners it turns on.
    explicit SurfaceMeshShortestPathRoutingEngine(
        const Geometry& geometry,
        double wall_clearance = 0.2);
    ~SurfaceMeshShortestPathRoutingEngine() = default;

    SurfaceMeshShortestPathRoutingEngine(const SurfaceMeshShortestPathRoutingEngine&) = delete;
    SurfaceMeshShortestPathRoutingEngine&
    operator=(const SurfaceMeshShortestPathRoutingEngine&) = delete;
    SurfaceMeshShortestPathRoutingEngine(SurfaceMeshShortestPathRoutingEngine&&) = delete;
    SurfaceMeshShortestPathRoutingEngine&
    operator=(SurfaceMeshShortestPathRoutingEngine&&) = delete;

    /// True iff @p loc projects onto the walkable surface.
    bool is_valid_location(const Point3D& loc) const;

    /// Compute the shortest path from @p source to @p target, held off wall corners by the
    /// engine's wall clearance.
    /// @param source where to route from
    /// @param target where to route to
    /// @return the path, including source as first and target as last element
    std::vector<Point3D> get_shortest_path(const Point3D& source, const Point3D& target);

    /// Unit vector from @p from along the route to @p to, projected to x/y. Zero once @p from
    /// has reached @p to.
    Vector get_orientation(const Location& from, const Location& to);

    /// Length of the geodesic from @p from to @p to over the surface, infinite if @p to cannot
    /// be reached from @p from.
    double path_length(const Location& from, const Location& to);

    double wall_clearance() const { return _wall_clearance; }

    /// How many sequence trees the engine has built, one per change of target.
    std::size_t trees_built() const { return _trees_built; }

private:
    using Traits = CGAL::Surface_mesh_shortest_path_traits<K, SurfaceMesh>;
    using ShortestPath = CGAL::Surface_mesh_shortest_path<Traits>;

    struct Step {
        Point3D point;
        /// Unit vector off a wall corner, zero where there is no corner.
        Vector into_the_open;
    };
    using Way = std::vector<Step>;

    /// Where @p p sits on the surface. Throws naming @p what if it sits nowhere.
    Geometry::FaceLocation on_surface(const Point3D& p, const char* what) const;

    /// The sequence tree for @p target. The last one built is cached. Therefore asking
    /// for the same target again in a row does not rebuild it.
    ShortestPath& tree_for(const Point3D& target);

    Way trace_way(const Point3D& source, const Point3D& target);

    /// @p corner moved into the open by the wall clearance, put back onto the surface.
    Point3D held_off_the_wall(const Point3D& corner, Vector into_the_open) const;

    /// Next point of the path from @p source to @p target
    /// Returns @p source itself when @p target is already reached.
    Point next_waypoint(const Point3D& source, const Point3D& target);

    const Geometry& _geometry;
    double _wall_clearance;

    /// The tree last built by `tree_for`, and its target. Makes `GetShortestPath` and
    /// `GetOrientation` non-reentrant.
    Point3D _last_target{};
    std::unique_ptr<ShortestPath> _last_tree{};
    std::size_t _trees_built{0};
};
