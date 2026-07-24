// SPDX-License-Identifier: LGPL-3.0-or-later
#include "Geometry/RegionView.hpp"

#include "Geometry/Geometry3D.hpp"

#include <cassert>
#include <cmath>
#include <map>
#include <utility>

namespace
{
/// Perpendicular distance of p from the infinite line through a and b.
double perp_distance(Point a, Point b, Point p)
{
    const Point ab = b - a;
    const double len = ab.Norm();
    if(len == 0.0) {
        return (p - a).Norm();
    }
    return std::abs(ab.Determinant(p - a)) / len;
}

/// Greedy Douglas-Peucker split of an ordered polyline. Extends a run as long as the
/// perpendicular distance stays below eps, otherwise starts a new segment.
void split_polyline(const std::vector<Point>& pts, double eps, std::vector<LineSegment>& out)
{
    assert(pts.size() >= 2 && "split_polyline requires at least two points");
    std::size_t start = 0;
    for(std::size_t end = 2; end < pts.size(); ++end) {
        bool collinear = true;
        for(std::size_t k = start + 1; k < end; ++k) {
            if(perp_distance(pts[start], pts[end], pts[k]) > eps) {
                collinear = false;
                break;
            }
        }
        if(!collinear) {
            out.emplace_back(pts[start], pts[end - 1]);
            start = end - 1;
        }
    }
    out.emplace_back(pts[start], pts.back());
}

/// Split a closed cyclic polyline. Broken open at a real corner so a straight
/// side spanning the arbitrary start index is not left over-segmented.
void split_cycle(const std::vector<Point>& cycle, double eps, std::vector<LineSegment>& out)
{
    const std::size_t m = cycle.size();
    // A closed loop of degree-2 vertices has >= 3 of them.
    assert(m >= 3 && "split_cycle requires a non-degenerate loop");
    std::size_t corner = m;
    for(std::size_t i = 0; i < m; ++i) {
        if(perp_distance(cycle[(i + m - 1) % m], cycle[(i + 1) % m], cycle[i]) > eps) {
            corner = i;
            break;
        }
    }
    // It has to turn somewhere.
    assert(corner != m && "split_cycle: closed loop has no corner");
    std::vector<Point> poly{};
    poly.reserve(m + 1);
    for(std::size_t k = 0; k <= m; ++k) {
        poly.push_back(cycle[(corner + k) % m]);
    }
    split_polyline(poly, eps, out);
}
} // namespace

std::vector<LineSegment> merge_collinear(const std::vector<LineSegment>& segments, double eps)
{
    if(segments.empty()) {
        return {};
    }

    // Create a map: Vertex --> all indices of neighboring edges ("incidents")
    const std::size_t n = segments.size();
    std::map<Point, std::vector<std::size_t>> incident{};
    for(std::size_t i = 0; i < n; ++i) {
        incident[segments[i].p1].push_back(i);
        incident[segments[i].p2].push_back(i);
    }
    // lambda function to get the other vertex of the segment
    const auto other_end = [&](std::size_t i, const Point& at) {
        return segments[i].p1 == at ? segments[i].p2 : segments[i].p1;
    };
    const auto continuation = [&](const Point& v, std::size_t from) {
        for(const auto j : incident.at(v)) {
            if(j != from) {
                return j;
            }
        }
        // never happens - caller has to ensure that each vertex has one incoming
        // and one outgoing egde (--> degree 2)
        return from;
    };

    std::vector<bool> used(n, false);
    std::vector<LineSegment> out{};

    // Open chains: start at every junction / dead-end vertex (degree != 2) and
    // walk through degree-2 vertices until the next such vertex.
    for(const auto& [vertex, segs] : incident) {
        if(segs.size() == 2) {
            continue;
        }
        for(const auto start : segs) {
            if(used[start]) {
                continue;
            }
            // found "start" vertex. Collect all points from there along vertices
            // with degree 2.
            used[start] = true;
            std::vector<Point> poly{vertex, other_end(start, vertex)};
            std::size_t prev = start;
            Point cur = poly.back();
            while(incident.at(cur).size() == 2) {
                const auto j = continuation(cur, prev);
                if(used[j]) {
                    break;
                }
                used[j] = true;
                cur = other_end(j, cur);
                poly.push_back(cur);
                prev = j;
            }
            split_polyline(poly, eps, out);
        }
    }

    // Whatever is left forms closed loops (every vertex degree 2).
    for(std::size_t i = 0; i < n; ++i) {
        if(used[i]) {
            continue;
        }
        const Point start = segments[i].p1;
        std::vector<Point> cycle{start};
        used[i] = true;
        std::size_t prev = i;
        Point cur = segments[i].p2;
        while(cur != start) {
            cycle.push_back(cur);
            const auto j = continuation(cur, prev);
            used[j] = true;
            cur = other_end(j, cur);
            prev = j;
        }
        split_cycle(cycle, eps, out);
    }
    return out;
}

RegionView::RegionView(
    std::size_t regionId,
    const Geometry3D* geometry3d,
    std::vector<LineSegment> walls,
    std::vector<LineSegment> seams,
    std::vector<std::size_t> seamNeighbors)
    : _regionId(regionId)
    , _geometry3d(geometry3d)
    , _walls(std::move(walls))
    , _seams(std::move(seams))
    , _seamNeighbors(std::move(seamNeighbors))
{
}

RegionView::LineSegmentRange RegionView::LineSegmentsInDistanceTo(double distance, Point p) const
{
    return _walls.LineSegmentsInDistanceTo(distance, p);
}

const std::vector<LineSegment>& RegionView::LineSegmentsInApproxDistanceTo(Point p) const
{
    return _walls.LineSegmentsInApproxDistanceTo(p);
}

bool RegionView::IntersectsAny(const LineSegment& linesegment) const
{
    return _walls.IntersectsAny(linesegment);
}

bool RegionView::InsideGeometry(Point p) const
{
    return _geometry3d->locate_in_region(_regionId, Point2D{p.x, p.y}).face !=
           SurfaceMesh::null_face();
}

bool RegionView::crosses_seam(Point a, Point b) const
{
    return _seams.IntersectsAny(LineSegment{a, b});
}
