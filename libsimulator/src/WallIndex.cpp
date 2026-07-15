// SPDX-License-Identifier: LGPL-3.0-or-later
#include "WallIndex.hpp"

#include <CGAL/intersections.h>
#include <CGAL/squared_distance_2.h>

#include <algorithm>
#include <cassert>
#include <cmath>
#include <variant>

namespace
{
Point2D remove_z(const Point3D& p)
{
    return {p.x(), p.y()};
}

Segment2D flatten(const Segment3D& s)
{
    return {remove_z(s.source()), remove_z(s.target())};
}

/// Height of `seg` at the point of its 2D projection closest to `at`.
double height_at(const Segment3D& seg, const Point2D& at)
{
    const auto source = remove_z(seg.source());
    const auto dir = remove_z(seg.target()) - source;
    const auto len2 = dir.squared_length();
    // Just safety check for debug mode: We should not have any vertical edges via contract
    assert(len2 > 0 && "FATAL: vertical segment detected");
    const auto t = std::clamp(((at - source) * dir) / len2, 0.0, 1.0);
    return seg.source().z() + t * (seg.target().z() - seg.source().z());
}
} // namespace

WallIndex::WallIndex(const SurfaceMesh& mesh)
{
    for(const auto e : mesh.edges()) {
        if(mesh.is_border(e)) {
            _edges.emplace_back(mesh.point(mesh.vertex(e, 0)), mesh.point(mesh.vertex(e, 1)));
        }
    }
}

std::vector<Segment3D>
WallIndex::segments_near(const Point3D& p, double radius, double height) const
{
    std::vector<Segment3D> result{};
    const auto q = remove_z(p);
    for(const auto& edge : _edges) {
        // filter all edges outside the (horizontal) radius
        if(CGAL::squared_distance(q, flatten(edge)) > radius * radius) {
            continue;
        }
        // Now filter on height
        if(std::abs(height_at(edge, q) - p.z()) < height) {
            result.push_back(edge);
        }
    }
    return result;
}

bool WallIndex::is_visible(const Point3D& a, const Point3D& b, double height) const
{
    const Segment3D path3d{a, b};
    const auto path2d = flatten(path3d);
    for(const auto& edge : _edges) {
        const auto crossing = CGAL::intersection(path2d, flatten(edge));
        if(!crossing) {
            continue;
        }

        const auto at = [&]() -> Point2D {
            if(const auto* pt = std::get_if<Point2D>(&*crossing)) {
                // CGAL returned point --> just return it
                return *pt;
            }
            // collinear: CGAL returned a segment
            // Note: This is possible in 3D if a wall is on a different floor, so
            //       differs in height. This should be highly unlikely though.
            //       Checking heights on midpoint in this case.
            const auto& overlap = std::get<Segment2D>(*crossing);
            return CGAL::midpoint(overlap.source(), overlap.target());
        }();
        const auto zLine = height_at(path3d, at);
        const auto zEdge = height_at(edge, at);
        if(std::abs(zEdge - zLine) < height) {
            return false;
        }
    }
    return true;
}
