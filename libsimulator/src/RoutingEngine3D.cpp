// SPDX-License-Identifier: LGPL-3.0-or-later
#include "RoutingEngine3D.hpp"

#include "SimulationError.hpp"

#include <tuple>
#include <variant>
#include <vector>

////////////////////////////////////////////////////////////////////////////////
// RoutingEngine3D
////////////////////////////////////////////////////////////////////////////////
void RoutingEngine3D::set_geometry(SurfaceMesh&& mesh)
{
    _mesh = std::move(mesh);
    _aabbTree = std::make_unique<AABBTree>(faces(_mesh).begin(), faces(_mesh).end(), _mesh);
    invalidate_target(); // inform engine to clear target cache
}

RoutingEngine3D::FaceLocation RoutingEngine3D::face_below(const Point3D& p) const
{
    if(!_aabbTree) {
        throw SimulationError("RoutingEngine does not have a geometry defined.");
    }

    // first_intersection along -z returns the hit nearest to the ray
    // source, i.e. the face directly below the query point.
    const SurfaceKernel::Ray_3 ray(p, SurfaceKernel::Direction_3(0, 0, -1));
    const auto hit = _aabbTree->first_intersection(ray);
    if(!hit) {
        return {SurfaceMesh::null_face(), SurfaceKernel::Point_3{}};
    }

    const auto* projected = std::get_if<SurfaceKernel::Point_3>(&hit->first);
    if(!projected) {
        return {SurfaceMesh::null_face(), SurfaceKernel::Point_3{}};
    }

    return {hit->second, *projected};
}

bool RoutingEngine3D::is_valid_location(const Location& loc) const
{
    return face_below(loc).face != SurfaceMesh::null_face();
}

Point RoutingEngine3D::get_orientation(const Point3D& source)
{
    const auto result = get_shortest_path(source);
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
