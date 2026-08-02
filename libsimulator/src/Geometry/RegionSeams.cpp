// SPDX-License-Identifier: LGPL-3.0-or-later
#include "Geometry/RegionSeams.hpp"

#include "Point.hpp"

#include <CGAL/boost/graph/iterator.h>

namespace
{
Point xy_of(const Point3D& p)
{
    return Point{p.x(), p.y()};
}
} // namespace

std::vector<RegionSeam> extract_region_seams(const SurfaceMesh& mesh, const RegionMap& region)
{
    std::vector<RegionSeam> seams{};
    for(const auto f : mesh.faces()) {
        const auto r = region[f];
        for(const auto h : CGAL::halfedges_around_face(mesh.halfedge(f), mesh)) {
            const auto opp = mesh.opposite(h);
            if(mesh.is_border(opp)) {
                // A mesh border is a wall, and walls are somebody else's business.
                continue;
            }
            const auto other = region[mesh.face(opp)];
            if(other == r) {
                continue;
            }
            seams.push_back(
                RegionSeam{
                    LineSegment{
                        xy_of(mesh.point(mesh.source(h))), xy_of(mesh.point(mesh.target(h)))},
                    r,
                    other});
        }
    }
    return seams;
}
