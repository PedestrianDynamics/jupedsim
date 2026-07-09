// SPDX-License-Identifier: LGPL-3.0-or-later
#include "SurfaceMeshShortestPathRoutingEngine.hpp"

#include "SimulationError.hpp"

#include <cassert>
#include <cstddef>
#include <iterator>
#include <tuple>
#include <utility>
#include <variant>
#include <vector>

////////////////////////////////////////////////////////////////////////////////
// SurfaceMeshShortestPathRoutingEngine
////////////////////////////////////////////////////////////////////////////////
SurfaceMeshShortestPathRoutingEngine::SurfaceMeshShortestPathRoutingEngine(SurfaceMesh&& mesh)
    : _mesh(std::move(mesh)), _aabbTree(faces(_mesh).begin(), faces(_mesh).end(), _mesh)
{
}

SurfaceMeshShortestPathRoutingEngine::FaceLocation
SurfaceMeshShortestPathRoutingEngine::face_below(const Point3D& p) const
{
    // first_intersection along -z returns the hit nearest to the ray
    // source, i.e. the face directly below the query point.
    const SurfaceKernel::Ray_3 ray(p, SurfaceKernel::Direction_3(0, 0, -1));
    const auto hit = _aabbTree.first_intersection(ray);
    if(!hit) {
        return {SurfaceMesh::null_face(), SurfaceKernel::Point_3{}};
    }

    const auto* projected = std::get_if<SurfaceKernel::Point_3>(&hit->first);
    // A -z ray meets a face in a single point unless the face is vertical
    assert(projected && "face_below: Invalid vertical face");

    return {hit->second, *projected};
}

bool SurfaceMeshShortestPathRoutingEngine::IsValidLocation(const Location& loc) const
{
    return face_below(loc).face != SurfaceMesh::null_face();
}

std::tuple<std::vector<Point3D>, double> SurfaceMeshShortestPathRoutingEngine::GetShortestPath(
    const Point3D& source,
    const Location& target)
{
    const auto from_below = face_below(source);
    if(from_below.face == SurfaceMesh::null_face()) {
        throw SimulationError(
            "GetShortestPath(): source does not project onto the walkable surface.");
    }
    const auto target_below = face_below(target);
    if(target_below.face == SurfaceMesh::null_face()) {
        throw SimulationError(
            "GetShortestPath(): target does not project onto the walkable surface.");
    }

    // TODO: Implement cache for sequence tree.
    ShortestPath shortest_path(_mesh);
    const auto to_loc = shortest_path.locate(target_below.point, _aabbTree);
    shortest_path.add_source_point(to_loc);
    shortest_path.build_sequence_tree();

    const auto from_loc = shortest_path.locate(from_below.point, _aabbTree);
    std::vector<Point3D> path;
    const auto result = shortest_path.shortest_path_points_to_source_points(
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
    // Direction towards the next waypoint, projected onto x/y (z dropped).
    // A query point sitting exactly on a triangle edge makes CGAL emit a
    // duplicate leading waypoint, so skip any waypoints coinciding with the
    // start and steer towards the first one that actually differs.
    for(std::size_t i = 1; i < path.size(); ++i) {
        const Point dir(path[i].x() - path[0].x(), path[i].y() - path[0].y());
        if(!dir.isZeroLength()) {
            return dir.Normalized();
        }
    }
    // Already at the destination (or degenerate) -> no direction.
    return {0, 0};
}
