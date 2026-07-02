// SPDX-License-Identifier: LGPL-3.0-or-later
#include "SurfaceMeshShortestPathRoutingEngine.hpp"

#include "SimulationError.hpp"

#include <iterator>
#include <tuple>
#include <utility>
#include <vector>

////////////////////////////////////////////////////////////////////////////////
// SurfaceMeshShortestPathRoutingEngine
////////////////////////////////////////////////////////////////////////////////
void SurfaceMeshShortestPathRoutingEngine::set_target(const Location& target)
{
    const auto target_below = face_below(target);
    if(target_below.face == SurfaceMesh::null_face()) {
        throw SimulationError("set_target(): target does not project onto the walkable surface.");
    }

    // Set cache for sub-sequent get_shortest_path() calls.
    _shortestPath = std::make_unique<ShortestPath>(_mesh);
    // CGAL needs barycentric coordinates
    const auto to_loc = _shortestPath->locate(target_below.point, *_aabbTree);
    _shortestPath->add_source_point(to_loc);
    _shortestPath->build_sequence_tree();
}

std::tuple<std::vector<Point3D>, double>
SurfaceMeshShortestPathRoutingEngine::get_shortest_path(const Point3D& source)
{
    if(!_shortestPath) {
        throw SimulationError("get_shortest_path(): no target set; call set_target() first.");
    }

    const auto from_below = face_below(source);
    if(from_below.face == SurfaceMesh::null_face()) {
        throw SimulationError(
            "get_shortest_path(): source does not project onto the walkable "
            "surface.");
    }
    const auto from_loc = _shortestPath->locate(from_below.point, *_aabbTree);

    std::vector<Point3D> path;
    const auto result = _shortestPath->shortest_path_points_to_source_points(
        from_loc.first, from_loc.second, std::back_inserter(path));
    // CGAL returns the cost directly. No separate claculation needed.
    return {std::move(path), result.first};
}
