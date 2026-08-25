// SPDX-License-Identifier: LGPL-3.0-or-later
#include "Geometry/ProjectedBoundary.hpp"

#include "HashCombine.hpp"

#include <CGAL/intersections.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <unordered_map>
#include <vector>

namespace
{
struct GridKey {
    std::int64_t x;
    std::int64_t y;
    bool operator==(const GridKey&) const = default;
};

struct GridKeyHash {
    std::size_t operator()(const GridKey& k) const
    {
        return jps::hash_combine(std::hash<std::int64_t>{}(k.x), std::hash<std::int64_t>{}(k.y));
    }
};

struct BoundarySegment {
    Point2D a;
    Point2D b;
    SurfaceMesh::Vertex_index va;
    SurfaceMesh::Vertex_index vb;
};

/// The two segments' shared mesh vertex, or the null vertex. Two distinct boundary
/// segments can share at most one vertex: an edge's two halfedges are never both
/// boundary, and Surface_mesh has no multi-edges.
SurfaceMesh::Vertex_index shared_mesh_vertex(const BoundarySegment& s, const BoundarySegment& t)
{
    if(s.va == t.va || s.va == t.vb) {
        return s.va;
    }
    if(s.vb == t.va || s.vb == t.vb) {
        return s.vb;
    }
    return SurfaceMesh::null_vertex();
}

/// Violation test for one segment pair. Segments sharing a mesh vertex may meet exactly at
/// that vertex; consecutive collinear segments folding back (zero angle) violate, straight
/// continuation does not. Segments sharing no mesh vertex may not intersect at all.
bool pair_violates(const BoundarySegment& s, const BoundarySegment& t)
{
    const auto q = shared_mesh_vertex(s, t);
    if(q == SurfaceMesh::null_vertex()) {
        return CGAL::do_intersect(K::Segment_2(s.a, s.b), K::Segment_2(t.a, t.b));
    }
    const auto& shared = s.va == q ? s.a : s.b;
    const auto& p1 = s.va == q ? s.b : s.a;
    const auto& p2 = t.va == q ? t.b : t.a;
    // Two segments out of one point overlap beyond it only if they leave it in the same
    // direction: collinear with a zero angle between them.
    return CGAL::collinear(shared, p1, p2) && CGAL::angle(p1, shared, p2) == CGAL::ACUTE;
}
} // namespace

std::vector<SurfaceMesh::Halfedge_index> region_boundary(
    const SurfaceMesh& mesh, const std::function<bool(SurfaceMesh::Face_index)>& keep)
{
    std::vector<SurfaceMesh::Halfedge_index> boundary{};
    for(const auto f : faces(mesh)) {
        if(!keep(f)) {
            continue;
        }
        for(const auto h : CGAL::halfedges_around_face(mesh.halfedge(f), mesh)) {
            const auto opp = mesh.opposite(h);
            if(mesh.is_border(opp) || !keep(mesh.face(opp))) {
                boundary.emplace_back(h);
            }
        }
    }
    return boundary;
}

bool is_projection_strictly_simple(
    const SurfaceMesh& mesh, const std::vector<SurfaceMesh::Halfedge_index>& boundary)
{
    if(boundary.empty()) {
        return true;
    }

    // Every boundary vertex must be entered and left exactly once: a repeated vertex is a
    // pinch, which the strict certificate rejects outright. This also guarantees that two
    // segments sharing a mesh vertex are consecutive along a loop.
    std::unordered_map<SurfaceMesh::Vertex_index, int> degree{};
    degree.reserve(2 * boundary.size());
    std::vector<BoundarySegment> segments{};
    segments.reserve(boundary.size());
    double total_length = 0.;
    for(const auto h : boundary) {
        const auto va = CGAL::source(h, mesh);
        const auto vb = CGAL::target(h, mesh);
        const auto& pa = mesh.point(va);
        const auto& pb = mesh.point(vb);
        const auto& seg = segments.emplace_back(
            BoundarySegment{Point2D{pa.x(), pa.y()}, Point2D{pb.x(), pb.y()}, va, vb});
        if(seg.a == seg.b) {
            return false; // vertical edge: cannot occur on walkable meshes
        }
        if(++degree[va] > 2 || ++degree[vb] > 2) {
            return false;
        }
        total_length += std::sqrt(CGAL::to_double(CGAL::squared_distance(seg.a, seg.b)));
    }

    // Uniform hash grid over the segments' projected bounding boxes; only pairs meeting in
    // a cell are tested. The certificate is an existential over pairs, so neither the
    // unordered cell iteration nor duplicate pair visits can change the result.
    const double cell_size = total_length / static_cast<double>(segments.size());
    std::unordered_map<GridKey, std::vector<std::uint32_t>, GridKeyHash> cells{};
    cells.reserve(2 * segments.size());
    const auto cell_of = [cell_size](double v) {
        return static_cast<std::int64_t>(std::floor(v / cell_size));
    };
    for(std::uint32_t i = 0; i < segments.size(); ++i) {
        const auto& s = segments[i];
        const auto x_lo = cell_of(std::min(s.a.x(), s.b.x()));
        const auto x_hi = cell_of(std::max(s.a.x(), s.b.x()));
        const auto y_lo = cell_of(std::min(s.a.y(), s.b.y()));
        const auto y_hi = cell_of(std::max(s.a.y(), s.b.y()));
        for(auto cx = x_lo; cx <= x_hi; ++cx) {
            for(auto cy = y_lo; cy <= y_hi; ++cy) {
                cells[GridKey{cx, cy}].emplace_back(i);
            }
        }
    }

    for(const auto& [key, members] : cells) {
        for(std::size_t i = 0; i < members.size(); ++i) {
            for(std::size_t j = i + 1; j < members.size(); ++j) {
                if(pair_violates(segments[members[i]], segments[members[j]])) {
                    return false;
                }
            }
        }
    }
    return true;
}
