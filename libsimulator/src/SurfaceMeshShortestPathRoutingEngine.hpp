// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "RoutingEngine3D.hpp"

#include <CGAL/Surface_mesh_shortest_path.h>

#include <memory>
#include <tuple>
#include <vector>

class SurfaceMeshShortestPathRoutingEngine : public RoutingEngine3D
{
public:
    SurfaceMeshShortestPathRoutingEngine() = default;
    ~SurfaceMeshShortestPathRoutingEngine() override = default;

    void set_target(const Location& target) override;
    std::tuple<std::vector<Point3D>, double> get_shortest_path(const Point3D& source) override;

private:
    using Traits = CGAL::Surface_mesh_shortest_path_traits<SurfaceKernel, SurfaceMesh>;
    using ShortestPath = CGAL::Surface_mesh_shortest_path<Traits>;

    void invalidate_target() override { _shortestPath.reset(); }

    // Cached structure for fast get_shortest_path to common target; null until set_target().
    std::unique_ptr<ShortestPath> _shortestPath{};
};
