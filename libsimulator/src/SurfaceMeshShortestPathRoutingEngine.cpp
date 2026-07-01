// SPDX-License-Identifier: LGPL-3.0-or-later
#include "SurfaceMeshShortestPathRoutingEngine.hpp"

#include "SimulationError.hpp"

#include <CGAL/Surface_mesh_shortest_path.h>

#include <iterator>
#include <tuple>
#include <utility>
#include <vector>

namespace
{
using Traits = CGAL::Surface_mesh_shortest_path_traits<SurfaceKernel, SurfaceMesh>;
using ShortestPath = CGAL::Surface_mesh_shortest_path<Traits>;
} // namespace

////////////////////////////////////////////////////////////////////////////////
// SurfaceMeshShortestPathRoutingEngine
////////////////////////////////////////////////////////////////////////////////
std::tuple<std::vector<Point3D>, double>
SurfaceMeshShortestPathRoutingEngine::get_shortest_path(const Point3D& from, const Location& to)
{
    const auto from_below = face_below(from);
    const auto to_below = face_below(to);
    if(from_below.face == SurfaceMesh::null_face() || to_below.face == SurfaceMesh::null_face()) {
        throw SimulationError(
            "get_shortest_path(): start or destination does not project onto the walkable "
            "surface.");
    }

    ShortestPath shortest_path(_mesh);

    // CGAL needs barycentric coordinates.
    const auto from_loc = shortest_path.locate(from_below.point, *_aabbTree);
    const auto to_loc = shortest_path.locate(to_below.point, *_aabbTree);

    // target location is used as source point for CGAL's pre-calculation.
    shortest_path.add_source_point(to_loc);

    // The point-sequence query also returns the geodesic distance to the source
    // (`.first`), so the cost comes for free -- no separate distance query.
    std::vector<Point3D> path;
    const auto result = shortest_path.shortest_path_points_to_source_points(
        from_loc.first, from_loc.second, std::back_inserter(path));
    const double cost = result.first;
    return {std::move(path), cost};
}
