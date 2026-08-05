// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "Geometry/Geometry3D.hpp"
#include "RoutingEngine3D.hpp"

#include <CGAL/Surface_mesh_shortest_path.h>

#include <map>
#include <tuple>
#include <vector>

class SurfaceMeshShortestPathRoutingEngine : public RoutingEngine3D
{
public:
    /// Borrows @p geometry (non-owning); the caller keeps it alive for the
    /// engine's lifetime. Ownership lives with the world (later: Simulation),
    /// matching the 2D pipeline where engines never own the geometry.
    /// @param wallClearance has to be zero: this engine does not keep a distance from wall
    /// corners yet, and would silently ignore one.
    explicit SurfaceMeshShortestPathRoutingEngine(
        const Geometry3D& geometry,
        double wallClearance = 0.0);
    ~SurfaceMeshShortestPathRoutingEngine() override = default;

    bool IsValidLocation(const RoutingTarget& loc) const override;

    std::vector<Point3D>
    GetShortestPath(const Point3D& source, const RoutingTarget& target) override;

    Point GetOrientation(const Point3D& source, const RoutingTarget& target) override;

    Point ComputeWaypoint(const Location& from, const Location& to) override;

private:
    const Geometry3D& _geometry;
    /// Tolerance needed for the collinear merge, calculated on the underlying mesh itself.
    double _mergeTolerance;

    // cache
    using Traits = CGAL::Surface_mesh_shortest_path_traits<K, SurfaceMesh>;
    using ShortestPath = CGAL::Surface_mesh_shortest_path<Traits>;
    std::map<RoutingTarget, std::unique_ptr<ShortestPath>> _cache{};
};
