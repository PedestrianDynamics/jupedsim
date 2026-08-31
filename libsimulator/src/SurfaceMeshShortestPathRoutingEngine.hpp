// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "Geometry/Geometry.hpp"
#include "RoutingEngine.hpp"

#include <CGAL/Surface_mesh_shortest_path.h>

#include <map>
#include <tuple>
#include <vector>

class SurfaceMeshShortestPathRoutingEngine : public RoutingEngine
{
public:
    /// Borrows @p geometry (non-owning); the caller keeps it alive for the
    /// engine's lifetime. Ownership lives with the world (later: Simulation),
    /// matching the 2D pipeline where engines never own the geometry.
    explicit SurfaceMeshShortestPathRoutingEngine(
        const Geometry& geometry,
        double wallClearance = 0.2);
    ~SurfaceMeshShortestPathRoutingEngine() override = default;

    bool IsValidLocation(const RoutingTarget& loc) const override;

    std::vector<Point3D>
    GetShortestPath(const Point3D& source, const RoutingTarget& target) override;

    Point GetOrientation(const Point3D& source, const RoutingTarget& target) override;

    Point ComputeWaypoint(const Location& from, const Location& to) override;

private:
    using Traits = CGAL::Surface_mesh_shortest_path_traits<K, SurfaceMesh>;
    using ShortestPath = CGAL::Surface_mesh_shortest_path<Traits>;

    /// The geodesic as the surface hands it over: every point it passes through, and where it
    /// turns on a wall corner, which way is off that corner into the open.
    ///
    /// The distinction is the surface's own, not ours: a geodesic runs straight through a face
    /// and straight across an edge -- it can only turn at a vertex. A turn on a vertex that is on
    /// the border is a corner an agent has to get around, every other point is one the way merely
    /// passes, and a turn on an interior vertex is a fold he walks straight over.
    struct Way {
        std::vector<Point3D> points{};
        /// Parallel to points: unit vector off a wall corner, zero where there is no corner.
        std::vector<Point> intoTheOpen{};
    };

    /// Where @p p sits on the surface. Throws naming @p what if it sits nowhere.
    Geometry::FaceLocation on_surface(const Point3D& p, const char* what) const;

    /// The sequence tree for @p target: built on first use, then kept.
    ShortestPath& tree_for(const RoutingTarget& target);

    Way trace_way(const Point3D& source, const RoutingTarget& target);

    /// @p corner moved into the open by the wall clearance, put back onto the surface.
    Point3D held_off_the_wall(const Point3D& corner, Point into_the_open) const;

    /// Next point of the path from @p source to @p target
    /// Returns @p source itself when @p target is already reached.
    Point next_waypoint(const Point3D& source, const RoutingTarget& target);

    const Geometry& _geometry;

    std::map<RoutingTarget, std::unique_ptr<ShortestPath>> _cache{};
};
