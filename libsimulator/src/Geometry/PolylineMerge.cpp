// SPDX-License-Identifier: LGPL-3.0-or-later
#include "Geometry/PolylineMerge.hpp"

#include <algorithm>
#include <cmath>
#include <limits>

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

/// True iff the step onto @p c turns back along the step that arrived at @p b. (Turning by
/// exactly 180 degrees is invisible to `perp_distance`.)
bool doubles_back(Point a, Point b, Point c)
{
    return (b - a).ScalarProduct(c - b) <= 0.0;
}

} // namespace

std::vector<std::size_t> straight_runs(const std::vector<Point>& points, double eps)
{
    if(points.size() < 2) {
        return {0};
    }

    std::vector<std::size_t> starts{0};
    const std::size_t last = points.size() - 1;
    std::size_t start = 0;
    for(std::size_t end = 2; end <= last; ++end) {
        bool fuse = !doubles_back(points[end - 2], points[end - 1], points[end]);
        for(std::size_t k = start + 1; fuse && k < end; ++k) {
            if(perp_distance(points[start], points[end], points[k]) > eps) {
                fuse = false;
            }
        }
        if(!fuse) {
            // One step too far: the run that still fit ends at the previous point, and the
            // next one re-anchors there so the two share that corner.
            start = end - 1;
            starts.push_back(start);
        }
    }
    return starts;
}

double mesh_merge_tolerance(const SurfaceMesh& mesh)
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
