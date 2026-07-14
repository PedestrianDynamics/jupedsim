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
    explicit SurfaceMeshShortestPathRoutingEngine(const Geometry3D& geometry);
    ~SurfaceMeshShortestPathRoutingEngine() override = default;

    bool IsValidLocation(const Location& loc) const override;

    std::tuple<std::vector<Point3D>, double>
    GetShortestPath(const Point3D& source, const Location& target) override;

    Point GetOrientation(const Point3D& source, const Location& target) override;

private:
    const Geometry3D& _geometry;

    // cache
    using Traits = CGAL::Surface_mesh_shortest_path_traits<SurfaceKernel, SurfaceMesh>;
    using ShortestPath = CGAL::Surface_mesh_shortest_path<Traits>;
    std::map<Location, std::unique_ptr<ShortestPath>> _cache{};
};
