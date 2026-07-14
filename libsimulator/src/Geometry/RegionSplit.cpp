// SPDX-License-Identifier: LGPL-3.0-or-later
#include "Geometry/RegionSplit.hpp"

#include "SimulationError.hpp"

#include <CGAL/Bbox_2.h>
#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/intersections.h>

#include <array>
#include <limits>
#include <queue>
#include <utility>
#include <vector>

namespace
{
using P2 = SurfaceKernel::Point_2;
using T2 = SurfaceKernel::Triangle_2;
// The overlap classification must be exact: with inexact constructions,
// CGAL::intersection has classified a mere vertex touch between two flat
// slivers as a triangle-shaped overlap (e.g. BUW floorplan), fragmenting a
// single-valued surface into hundreds of regions.
using EK = CGAL::Exact_predicates_exact_constructions_kernel;
using ET2 = EK::Triangle_2;

/// Orthogonal projection of a triangular face onto the x/y-plane (z dropped).
T2 project(const SurfaceMesh& mesh, SurfaceMesh::Face_index f)
{
    std::array<P2, 3> p{};
    int i = 0;
    for(const auto v : CGAL::vertices_around_face(mesh.halfedge(f), mesh)) {
        const auto& q = mesh.point(v);
        p[i++] = P2(q.x(), q.y());
    }
    return T2(p[0], p[1], p[2]);
}

ET2 exact(const T2& t)
{
    return ET2(
        EK::Point_2(t[0].x(), t[0].y()),
        EK::Point_2(t[1].x(), t[1].y()),
        EK::Point_2(t[2].x(), t[2].y()));
}

/// True iff the two projected triangles share a positive-area region. Faces of
/// one single-valued region only ever touch along shared edges/vertices, whose
/// intersection is a segment/point -- those are deliberately NOT overlaps.
bool overlaps(const T2& a, const T2& b)
{
    const auto result = CGAL::intersection(exact(a), exact(b));
    if(!result) {
        return false;
    }
    if(std::get_if<ET2>(&*result)) {
        return true; // triangle-shaped overlap
    }
    if(const auto* poly = std::get_if<std::vector<EK::Point_2>>(&*result)) {
        return poly->size() >= 3; // convex polygon overlap
    }
    return false; // Point_2 / Segment_2: boundary touch only
}
} // namespace

RegionSplit split_into_regions(SurfaceMesh& mesh)
{
    constexpr auto UNASSIGNED = std::numeric_limits<std::size_t>::max();
    auto [region, added] =
        mesh.add_property_map<SurfaceMesh::Face_index, std::size_t>("f:region", UNASSIGNED);
    if(!added) {
        throw SimulationError("split_into_regions(): mesh already has a region map");
    }

    // Algorithm is like this:
    // Walk over all faces:
    //   If current face is not yet defined to a region:
    //      Increase number of regions
    //      Add current face to a queue
    //      While (queue not empy):
    //        Get next entry from queue
    //        Add all neighboring faces to the queue, that are not yet assigned a region
    //                            and are not neighboring by a "border" edge
    std::size_t next_id = 0;
    for(const auto seed : faces(mesh)) {
        if(region[seed] != UNASSIGNED) {
            continue;
        }
        // first face ("seed") of a new region
        const auto id = next_id++;

        // Projected triangles already accepted into this region, each with its
        // 2D bounding box for a cheap overlap pre-filter.
        std::vector<std::pair<T2, CGAL::Bbox_2>> members{};
        std::queue<SurfaceMesh::Face_index> frontier{};
        const auto join = [&region, id, &members, &frontier, &mesh](SurfaceMesh::Face_index f) {
            region[f] = id;
            const auto t = project(mesh, f);
            members.emplace_back(t, t.bbox());
            frontier.push(f);
        };
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
                if(region[nbr] != UNASSIGNED) {
                    continue;
                }
                const auto t = project(mesh, nbr);
                const auto bb = t.bbox();
                bool clashes = false;
                for(const auto& [member_t, member_bb] : members) {
                    if(CGAL::do_overlap(bb, member_bb) && overlaps(t, member_t)) {
                        clashes = true;
                        break;
                    }
                }
                if(!clashes) {
                    join(nbr);
                }
            }
        }
    }
    return {region, next_id};
}
