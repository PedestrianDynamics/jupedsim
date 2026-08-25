// SPDX-License-Identifier: LGPL-3.0-or-later
#include "Geometry/RegionSplit.hpp"

#include "Visitor.hpp"

#include <CGAL/Bbox_2.h>
#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/Polygon_mesh_processing/compute_normal.h>
#include <CGAL/boost/graph/iterator.h>
#include <CGAL/intersections.h>

#include <array>
#include <cassert>
#include <limits>
#include <queue>
#include <tuple>
#include <vector>

namespace
{
// The overlap classification must be exact: with inexact constructions,
// CGAL::intersection has classified a mere vertex touch between two flat
// slivers as a triangle-shaped overlap (e.g. BUW floorplan), fragmenting a
// single-valued surface into hundreds of regions.
using EK = CGAL::Exact_predicates_exact_constructions_kernel;
using ET2 = EK::Triangle_2;
using EP2 = EK::Point_2;
using EL2 = EK::Segment_2;

/// Orthogonal projection of a triangular face onto the x/y-plane (z dropped).
ET2 project(const SurfaceMesh& mesh, SurfaceMesh::Face_index f)
{
    std::array<EP2, 3> p{};
    int i = 0;
    for(const auto v : CGAL::vertices_around_face(mesh.halfedge(f), mesh)) {
        const auto& q = mesh.point(v);
        p[i++] = EP2(q.x(), q.y());
    }
    return ET2(p[0], p[1], p[2]);
}

bool shares_vertex(const SurfaceMesh& mesh, SurfaceMesh::Face_index a, SurfaceMesh::Face_index b)
{
    for(const auto v_a : mesh.vertices_around_face(mesh.halfedge(a))) {
        for(const auto v_b : mesh.vertices_around_face(mesh.halfedge(b))) {
            if(v_a == v_b) {
                return true;
            }
        }
    }
    return false;
}

bool shares_edge(const SurfaceMesh& mesh, SurfaceMesh::Face_index a, SurfaceMesh::Face_index b)
{
    for(const auto h : CGAL::halfedges_around_face(mesh.halfedge(a), mesh)) {
        if(mesh.face(mesh.opposite(h)) == b) {
            return true;
        }
    }
    return false;
}

enum class OverlapResult {
    // Triangles are fully disjoint
    Disjoint,
    // Triangles overlap with area
    Overlap,
    // Triangles touch along a segment
    TouchAlongSegment,
    // Triangles touch in a single point
    TouchInPoint
};

/// Classifies the intersection of two projected triangles. Two triangles are
/// convex, so their intersection is one convex piece and the variant kind alone
/// tells its dimension: area (triangle/polygon), segment, or point.
OverlapResult overlaps(const ET2& a, const ET2& b)
{
    const auto result = CGAL::intersection(a, b);
    if(!result) {
        return OverlapResult::Disjoint;
    }
    return std::visit(
        overloaded{
            [](const ET2&) { return OverlapResult::Overlap; },
            [](const std::vector<EP2>&) { return OverlapResult::Overlap; },
            [](const EP2&) { return OverlapResult::TouchInPoint; },
            [](const EL2&) { return OverlapResult::TouchAlongSegment; }},
        *result);
}
} // namespace

RegionSplit split_into_regions(const SurfaceMesh& mesh)
{
    constexpr auto UNASSIGNED = std::numeric_limits<std::size_t>::max();
    // number_of_faces() counts live faces only; with garbage, face indices exceed it
    // and the vector would be indexed out of bounds.
    assert(!mesh.has_garbage());
    std::vector<size_t> region_map(mesh.number_of_faces(), UNASSIGNED);

    std::vector<std::tuple<SurfaceMesh::Face_index, ET2, CGAL::Bbox_2>> members{};
    std::size_t next_id = 0;
    for(const auto seed : faces(mesh)) {
        if(region_map[seed] != UNASSIGNED) {
            continue;
        }
        // first face ("seed") of a new region
        const auto id = next_id++;

        // Projected triangles already accepted into this region, each with its
        // 2D bounding box for a cheap overlap pre-filter.
        std::queue<SurfaceMesh::Face_index> frontier{};
        const auto join = [&region_map, id, &members, &frontier, &mesh](SurfaceMesh::Face_index f) {
            region_map[f] = id;
            const auto t = project(mesh, f);
            members.emplace_back(f, t, t.bbox());
            frontier.push(f);
        };
        members.clear();
        join(seed);

        while(!frontier.empty()) {
            const auto g = frontier.front();
            frontier.pop();
            for(const auto h : CGAL::halfedges_around_face(mesh.halfedge(g), mesh)) {
                const auto opp = mesh.opposite(h);
                if(mesh.is_border(opp)) {
                    continue;
                }
                const auto nbr = mesh.face(opp);
                if(region_map[nbr] != UNASSIGNED) {
                    continue;
                }
                const auto t = project(mesh, nbr);
                const auto bb = t.bbox();
                // A contact is only allowed where the two faces share the mesh element
                // of the same dimension: a segment contact must be the shared edge, a
                // point contact the shared vertex (the intersection of two convex
                // triangles is one convex piece, so a lone touch point of two
                // vertex-sharing faces IS that vertex). Anything beyond -- an area
                // overlap even between edge-neighbors (a fold hinged at the shared
                // edge), a segment contact without the shared edge (coincident walls),
                // a point contact between unrelated faces -- clashes.
                bool clashes = false;
                for(const auto& [member_f, member_t, member_bb] : members) {
                    if(CGAL::do_overlap(bb, member_bb)) {
                        switch(overlaps(t, member_t)) {
                            case OverlapResult::Disjoint:
                                break;
                            case OverlapResult::Overlap:
                                clashes = true;
                                break;
                            case OverlapResult::TouchAlongSegment:
                                clashes = !shares_edge(mesh, nbr, member_f);
                                break;
                            case OverlapResult::TouchInPoint:
                                clashes = !shares_vertex(mesh, nbr, member_f);
                                break;
                        }
                    }
                    if(clashes) {
                        break;
                    }
                }
                if(!clashes) {
                    join(nbr);
                }
            }
        }
    }
    return {region_map, next_id};
}
