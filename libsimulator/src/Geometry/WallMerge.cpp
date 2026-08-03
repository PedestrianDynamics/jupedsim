// SPDX-License-Identifier: LGPL-3.0-or-later
#include "Geometry/WallMerge.hpp"

#include "Geometry/PolylineMerge.hpp"
#include "Point.hpp"

#include <CGAL/boost/graph/iterator.h>

#include <set>
#include <unordered_set>

namespace
{
using Halfedge = SurfaceMesh::Halfedge_index;

Point xy_of(const Point3D& p)
{
    return Point{p.x(), p.y()};
}

/// Stores one step of the border walk: the vertex it starts at and the region behind the wall.
struct BorderStep {
    Point3D from;
    std::size_t region;
};

} // namespace

std::vector<MergedWall>
merge_border_walls(const SurfaceMesh& mesh, const RegionMap& region, double eps)
{
    std::vector<MergedWall> out{};
    std::unordered_set<std::size_t> visited{};

    // Every border cycle exactly once: the walk below marks all of its halfedges, so any
    // later halfedge of the same cycle is skipped here.
    for(const auto h : mesh.halfedges()) {
        if(!mesh.is_border(h) || visited.count(static_cast<std::size_t>(h)) > 0) {
            continue;
        }

        // Border halfedges form closed cycles; next() walks one of them. The wall itself is
        // on the other side, where the face is.
        std::vector<BorderStep> steps{};
        for(Halfedge b = h;;) {
            visited.insert(static_cast<std::size_t>(b));
            steps.push_back(
                BorderStep{mesh.point(mesh.source(b)), region[mesh.face(mesh.opposite(b))]});
            b = mesh.next(b);
            if(b == h) {
                break;
            }
        }
        if(steps.empty()) {
            continue;
        }

        // The walk started at an arbitrary halfedge, and cutting a closed loop there would
        // split whichever straight side happens to contain it into two walls. So open it at
        // a corner instead, and rotate the cycle to begin there.
        const std::size_t m = steps.size();
        std::vector<Point> cycle{};
        cycle.reserve(m);
        for(const auto& step : steps) {
            cycle.push_back(xy_of(step.from));
        }
        const std::size_t corner = first_corner(cycle, eps);

        std::vector<BorderStep> rotated{};
        std::vector<Point> chain{};
        rotated.reserve(m);
        chain.reserve(m + 1);
        for(std::size_t k = 0; k < m; ++k) {
            rotated.push_back(steps[(corner + k) % m]);
            chain.push_back(cycle[(corner + k) % m]);
        }
        // Closing the loop: the chain ends where it started, so there is one more point than
        // there are steps.
        chain.push_back(chain.front());

        // Turn the half-open range of steps [first, last) into one wall. The regions come
        // from the steps rather than the points, because a region sits behind an edge, not
        // at a vertex.
        const auto emit = [&](std::size_t first, std::size_t last) {
            std::set<std::size_t> regions{};
            for(std::size_t i = first; i < last; ++i) {
                regions.insert(rotated[i].region);
            }
            out.push_back(
                // End to start, so a wall faces the way the polygon path faced it in 2D.
                // If provided the other way round, `ShortestPoint` to an agent might change
                // in the last bit.
                MergedWall{
                    LineSegment{chain[last], chain[first]},
                    static_cast<std::uint32_t>(out.size()),
                    {regions.begin(), regions.end()}});
        };

        const auto starts = straight_runs(chain, eps);
        for(std::size_t i = 0; i + 1 < starts.size(); ++i) {
            emit(starts[i], starts[i + 1]);
        }
        emit(starts.back(), m);
    }
    return out;
}
