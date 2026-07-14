// SPDX-License-Identifier: LGPL-3.0-or-later
#include "SurfaceMeshShortestPathRoutingEngine.hpp"

#include "SimulationError.hpp"

#include <cstddef>
#include <iterator>
#include <tuple>
#include <utility>
#include <vector>

////////////////////////////////////////////////////////////////////////////////
// SurfaceMeshShortestPathRoutingEngine
////////////////////////////////////////////////////////////////////////////////
SurfaceMeshShortestPathRoutingEngine::SurfaceMeshShortestPathRoutingEngine(
    const Geometry3D& geometry)
    : _geometry(geometry)
{
}

bool SurfaceMeshShortestPathRoutingEngine::IsValidLocation(const Location& loc) const
{
    return _geometry.face_below(loc).face != SurfaceMesh::null_face();
}

std::tuple<std::vector<Point3D>, double>
SurfaceMeshShortestPathRoutingEngine::GetShortestPath(const Point3D& source, const Location& target)
{
    const auto from_below = _geometry.face_below(source);
    if(from_below.face == SurfaceMesh::null_face()) {
        throw SimulationError(
            "GetShortestPath(): source does not project onto the walkable surface.");
    }
    const auto target_below = _geometry.face_below(target);
    if(target_below.face == SurfaceMesh::null_face()) {
        throw SimulationError(
            "GetShortestPath(): target does not project onto the walkable surface.");
    }

    auto it = _cache.find(target);
    if(it == _cache.end()) {
        auto shortest_path = std::make_unique<ShortestPath>(_geometry.mesh());
        const auto to_loc = shortest_path->locate(target_below.point, _geometry.aabb_tree());
        shortest_path->add_source_point(to_loc);
        shortest_path->build_sequence_tree();
        it = _cache.emplace(target, std::move(shortest_path)).first;
    }

    const auto from_loc = it->second->locate(from_below.point, _geometry.aabb_tree());
    std::vector<Point3D> path;
    const auto result = it->second->shortest_path_points_to_source_points(
        from_loc.first, from_loc.second, std::back_inserter(path));
    // CGAL returns the cost directly. No separate calculation needed.
    return {std::move(path), result.first};
}

Point SurfaceMeshShortestPathRoutingEngine::GetOrientation(
    const Point3D& source,
    const Location& target)
{
    const auto result = GetShortestPath(source, target);
    const auto& path = std::get<0>(result);
    // Heading to the next waypoint, projected onto x/y (z dropped). If a query
    // point sits exactly on a triangle edge, CGAL emits a duplicate leading
    // waypoint, so return the first non-zero direction along the shortest path.
    for(std::size_t i = 1; i < path.size(); ++i) {
        const Point dir(path[i].x() - path[0].x(), path[i].y() - path[0].y());
        if(!dir.isZeroLength()) {
            return dir.Normalized();
        }
    }
    // Already at the destination (or degenerate) -> no heading.
    return {0, 0};
}
