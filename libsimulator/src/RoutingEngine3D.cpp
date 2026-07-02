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
    if(path.size() < 2) {
        // Already at the destination (or no next waypoint) -> no direction.
        return {0, 0};
    }
    // Direction towards the next waypoint, projected onto x/y (z dropped).
    const Point dir(path[1].x() - path[0].x(), path[1].y() - path[0].y());
    if(dir.isZeroLength()) { // can this happen? just for safety
        return {0, 0};
    }
    return dir.Normalized();
}
