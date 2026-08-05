// SPDX-License-Identifier: LGPL-3.0-or-later
#include "SurfaceMeshShortestPathRoutingEngine.hpp"

#include "Geometry/PolylineMerge.hpp"
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
    const Geometry3D& geometry,
    double wallClearance)
    : RoutingEngine3D(wallClearance)
    , _geometry(geometry)
    , _mergeTolerance(mesh_merge_tolerance(geometry.mesh()))
{
    if(wallClearance != 0.0) {
        throw SimulationError(
            "Routing on a surface does not keep a wall clearance yet, so it cannot be given one "
            "({}).",
            wallClearance);
    }
}

bool SurfaceMeshShortestPathRoutingEngine::IsValidLocation(const RoutingTarget& loc) const
{
    return _geometry.face_below(loc).face != SurfaceMesh::null_face();
}

std::vector<Point3D> SurfaceMeshShortestPathRoutingEngine::GetShortestPath(
    const Point3D& source,
    const RoutingTarget& target)
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
    it->second->shortest_path_points_to_source_points(
        from_loc.first, from_loc.second, std::back_inserter(path));
    return path;
}

Point SurfaceMeshShortestPathRoutingEngine::ComputeWaypoint(
    const Location& from,
    const Location& to)
{
    const auto path = GetShortestPath(from.position_3d(), to.position_3d());

    // CGAL sets a point whenever it crosses a triangle edge. need to do a collinear merge or
    // agents stop at each of those intermediate points as they occur in "agent.nextTarget".
    std::vector<Point> chain{from.xy()};
    for(const auto& p : path) {
        const Point xy{p.x(), p.y()};
        // Points on top of each other would read as a turn: the leading one comes back when the
        // agent stands on an edge.
        if((xy - chain.back()).Norm() > _mergeTolerance) {
            chain.push_back(xy);
        }
    }
    if(chain.size() < 2) {
        // Already there.
        return to.xy();
    }
    const auto runs = straight_runs(chain, _mergeTolerance);
    return runs.size() > 1 ? chain[runs[1]] : chain.back();
}

Point SurfaceMeshShortestPathRoutingEngine::GetOrientation(
    const Point3D& source,
    const RoutingTarget& target)
{
    const auto path = GetShortestPath(source, target);
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
