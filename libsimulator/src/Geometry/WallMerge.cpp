// SPDX-License-Identifier: LGPL-3.0-or-later
#include "Geometry/WallMerge.hpp"

#include "Point.hpp"

#include <CGAL/boost/graph/iterator.h>

#include <algorithm>
#include <cmath>
#include <limits>
#include <set>
#include <unordered_set>
#include <utility>

namespace
{
using Halfedge = SurfaceMesh::Halfedge_index;

Point xy_of(const Point3D& p)
{
    return Point{p.x(), p.y()};
}

/// Perpendicular distance of p from the infinite line through a and b, in the horizontal
/// plane. Height is ignored --> see `merge_border_walls`.
double perp_distance(Point a, Point b, Point p)
{
    const Point ab = b - a;
    const double len = ab.Norm();
    if(len == 0.0) {
        return (p - a).Norm();
    }
    return std::abs(ab.Determinant(p - a)) / len;
}

/// True iff the step onto @p c turns back along the step that arrived at @p b. (Turning by
/// exactly 180 degrees is invisible to `perp_distance`.)
bool doubles_back(Point a, Point b, Point c)
{
    return (b - a).ScalarProduct(c - b) <= 0.0;
}

/// Stores one step of the border walk: the vertex it starts at and the region behind the wall.
struct BorderStep {
    Point3D from;
    std::size_t region;
};

/// Greedily fuse a run of border steps. A run keeps growing while it does not turn back on
/// itself and every vertex it swallowed stays within eps of the run's chord -- the chord,
/// so that the deviation of the whole run is bounded, not just that of neighbouring pairs.
///
/// This is an "anchor and float" simplification in the spirit of Lang (1969): anchor at the
/// start, push the far end outwards, cut back one step at the first violation.
void split_run(
    const std::vector<BorderStep>& steps,
    const Point3D& last_point,
    double eps,
    std::vector<MergedWall>& out)
{
    // The steps come from a closed cycle, so index n is the point the walk started from:
    // there is one more point than there are steps.
    const std::size_t n = steps.size();
    const auto point_at = [&](std::size_t i) { return i < n ? steps[i].from : last_point; };

    // Turn the half-open range of steps [first, last) into one wall. The regions come from
    // the steps rather than the points, because a region sits behind an edge, not at a
    // vertex.
    const auto emit = [&](std::size_t first, std::size_t last) {
        std::set<std::size_t> regions{};
        for(std::size_t i = first; i < last; ++i) {
            regions.insert(steps[i].region);
        }
        out.push_back(
            // End to start, so a wall faces the way the polygon path faced it in 2D.
            // If provided the other way round, `ShortestPoint` to an agent might change
            // in the last bit.
            MergedWall{
                LineSegment{xy_of(point_at(last)), xy_of(point_at(first))},
                static_cast<std::uint32_t>(out.size()),
                {regions.begin(), regions.end()}});
    };

    // Anchor at "start", float "end" outwards. Both tests are applied to the run as a
    // whole, so the run is only ever extended while it stays a single straight wall.
    std::size_t start = 0;
    for(std::size_t end = 2; end <= n; ++end) {
        bool fuse =
            !doubles_back(xy_of(point_at(end - 2)), xy_of(point_at(end - 1)), xy_of(point_at(end)));
        for(std::size_t k = start + 1; fuse && k < end; ++k) {
            if(perp_distance(xy_of(point_at(start)), xy_of(point_at(end)), xy_of(point_at(k))) >
               eps) {
                fuse = false;
            }
        }
        if(!fuse) {
            // One step too far: emit what still fit and re-anchor on its last point, so
            // consecutive walls share a corner instead of leaving a gap.
            emit(start, end - 1);
            start = end - 1;
        }
    }
    // Whatever is left runs to the point the cycle started from.
    emit(start, n);
}

} // namespace

double wall_merge_tolerance(const SurfaceMesh& mesh)
{
    double lo = std::numeric_limits<double>::max();
    double hi = std::numeric_limits<double>::lowest();
    for(const auto v : mesh.vertices()) {
        const auto& p = mesh.point(v);
        lo = std::min({lo, p.x(), p.y(), p.z()});
        hi = std::max({hi, p.x(), p.y(), p.z()});
    }
    if(lo > hi) {
        return 0.0;
    }
    // A few ULP of the coordinate magnitude: invisible next to the error the input
    // coordinates already carry, and orders of magnitude below any real corner.
    const double scale = std::max(std::abs(lo), std::abs(hi));
    return 8.0 * std::max(scale, 1.0) * std::numeric_limits<double>::epsilon();
}

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

        // Find a corner to open the cycle at. The walk started at an arbitrary halfedge,
        // and cutting a closed loop there would split whichever straight side happens to
        // contain it into two walls.
        //
        // A corner is where the border turns: either away from the line through its two
        // neighbours, or back onto it.
        const std::size_t m = steps.size();
        std::size_t corner = m;
        for(std::size_t i = 0; i < m; ++i) {
            const auto& prev = steps[(i + m - 1) % m].from;
            const auto& next = steps[(i + 1) % m].from;
            if(perp_distance(xy_of(prev), xy_of(next), xy_of(steps[i].from)) > eps ||
               doubles_back(xy_of(prev), xy_of(steps[i].from), xy_of(next))) {
                corner = i;
                break;
            }
        }
        if(corner == m) {
            // No corner at all: a degenerate cycle with nothing to align to. Break it
            // anywhere; every split is as good as any other.
            corner = 0;
        }

        // Rotate the cycle so it begins at that corner, which turns it into an open chain.
        std::vector<BorderStep> rotated{};
        rotated.reserve(m);
        for(std::size_t k = 0; k < m; ++k) {
            rotated.push_back(steps[(corner + k) % m]);
        }
        split_run(rotated, rotated.front().from, eps, out);
    }
    return out;
}
