// SPDX-License-Identifier: LGPL-3.0-or-later
#include "Geometry/RegionSeams.hpp"

#include "Geometry/PolylineMerge.hpp"
#include "Point.hpp"

#include <CGAL/boost/graph/iterator.h>

#include <map>
#include <set>
#include <utility>
#include <vector>

namespace
{
using Vertex = SurfaceMesh::Vertex_index;

Point xy_of(const Point3D& p)
{
    return Point{p.x(), p.y()};
}

/// One seam edge, still as mesh vertices. Chaining them needs vertex identity rather than
/// coordinates: two edges that merely touch at equal coordinates are not the same boundary.
struct SeamEdge {
    Vertex from;
    Vertex to;
};

/// The vertices a run of seam edges passes through, in order.
using Chain = std::vector<Vertex>;

/// Put one region pair's edges in order, end to end.
///
/// Walls need nothing of the sort: border halfedges already form cycles and the mesh walks
/// them. Seam edges come out of a scan over faces with no order at all, and a straight run
/// cannot be found in a heap of segments.
std::vector<Chain> chains_of(const std::vector<SeamEdge>& edges)
{
    std::map<Vertex, std::size_t> leaving{};
    std::set<Vertex> arrived_at{};
    for(std::size_t i = 0; i < edges.size(); ++i) {
        leaving.emplace(edges[i].from, i);
        arrived_at.insert(edges[i].to);
    }

    std::vector<bool> used(edges.size(), false);
    std::vector<Chain> chains{};
    const auto walk = [&](std::size_t first) {
        Chain chain{edges[first].from};
        for(std::size_t e = first;;) {
            used[e] = true;
            chain.push_back(edges[e].to);
            const auto next = leaving.find(edges[e].to);
            if(next == leaving.end() || used[next->second]) {
                break;
            }
            e = next->second;
        }
        chains.push_back(std::move(chain));
    };

    // Start where nothing leads in, so an open chain comes out in one piece rather than as
    // two halves of itself. Whatever is left after that is a loop, with no end to start from.
    for(std::size_t i = 0; i < edges.size(); ++i) {
        if(!used[i] && arrived_at.count(edges[i].from) == 0) {
            walk(i);
        }
    }
    for(std::size_t i = 0; i < edges.size(); ++i) {
        if(!used[i]) {
            walk(i);
        }
    }
    return chains;
}

void emit_chain(
    const SurfaceMesh& mesh,
    const Chain& chain,
    std::size_t region,
    std::size_t neighbor,
    double eps,
    std::vector<RegionSeam>& out)
{
    std::vector<Point> points{};
    points.reserve(chain.size());
    for(const auto v : chain) {
        points.push_back(xy_of(mesh.point(v)));
    }

    if(chain.size() > 2 && chain.front() == chain.back()) {
        // A loop, so there is no natural place to begin. Cutting it just anywhere would
        // leave whichever straight side contains the cut as two seams instead of one.
        const std::vector<Point> cycle{points.begin(), points.end() - 1};
        const std::size_t corner = first_corner(cycle, eps);
        points.clear();
        for(std::size_t k = 0; k < cycle.size(); ++k) {
            points.push_back(cycle[(corner + k) % cycle.size()]);
        }
        points.push_back(points.front());
    }

    const auto starts = straight_runs(points, eps);
    for(std::size_t i = 0; i < starts.size(); ++i) {
        const std::size_t last = i + 1 < starts.size() ? starts[i + 1] : points.size() - 1;
        out.push_back(RegionSeam{LineSegment{points[starts[i]], points[last]}, region, neighbor});
    }
}

} // namespace

std::vector<RegionSeam>
extract_region_seams(const SurfaceMesh& mesh, const RegionMap& region, double eps)
{
    // Grouped by the pair, because a run may only be fused while it leads to the same place:
    // two collinear stretches of boundary towards different neighbours are two seams.
    std::map<std::pair<std::size_t, std::size_t>, std::vector<SeamEdge>> by_pair{};
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
            by_pair[{r, other}].push_back(SeamEdge{mesh.source(h), mesh.target(h)});
        }
    }

    std::vector<RegionSeam> seams{};
    for(const auto& [pair, edges] : by_pair) {
        for(const auto& chain : chains_of(edges)) {
            emit_chain(mesh, chain, pair.first, pair.second, eps, seams);
        }
    }
    return seams;
}
