// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "RoutingEngine3D.hpp"

class SurfaceMeshShortestPathRoutingEngine : public RoutingEngine3D
{
public:
    SurfaceMeshShortestPathRoutingEngine() = default;
    virtual ~SurfaceMeshShortestPathRoutingEngine() override = default;

    // no copy and assignments
    SurfaceMeshShortestPathRoutingEngine(const SurfaceMeshShortestPathRoutingEngine& other) =
        delete;
    SurfaceMeshShortestPathRoutingEngine&
    operator=(const SurfaceMeshShortestPathRoutingEngine& other) = delete;

    // move is supported
    SurfaceMeshShortestPathRoutingEngine(SurfaceMeshShortestPathRoutingEngine&& other) = default;
    SurfaceMeshShortestPathRoutingEngine&
    operator=(SurfaceMeshShortestPathRoutingEngine&& other) = default;

    /// Checks whether the provided location (3D-point or polygon)
    /// is on walkable surface taking wall clearance into account.
    /// @param loc location (Point or Polygon) to check
    /// @return path between from and to, includes from as
    ///         first element and to as last element
    // bool is_valid_location(const Location& loc) override;

    /// Compute shortest path on walkable surface.
    /// @param from where to route (Point)
    /// @param to route to (Location: Point or Polygon)
    /// @return path between from and to, includes from as
    ///         first element and to as last element
    std::tuple<std::vector<Point3D>, double>
    get_shortest_path(const Point3D& from, const Location& to) override;

    /// Get orientation to next point of shortest path on walkable surface projectted to x/y.
    /// @param from where to route
    /// @param to route to (Location: Point or Polygon)
    /// @return 2D orientation to next point of shortest path between from and to
    // Point get_orientation(const Point3D& from, const Location& to) override;

    /// "Distance" (--> costs) of shortest path on walkable surface.
    /// @param from where to route
    /// @param to route to
    /// @return cost of path between from and to
    // double get_distance(const Point3D& from, const Location& to) override;
};
