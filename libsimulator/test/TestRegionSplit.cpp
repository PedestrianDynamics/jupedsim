// SPDX-License-Identifier: LGPL-3.0-or-later
#include "CfgCgal.hpp"
#include "Geometry/RegionSplit.hpp"
#include "MeshFixtures.hpp"
#include "TestCommon.hpp"

#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/intersections.h>

#include <gtest/gtest.h>

#include <array>
#include <set>
#include <string>
#include <vector>

namespace
{
/// A single flat 10x10 square at z=0, split into two triangles.
SurfaceMesh flat_square()
{
    SurfaceMesh mesh{};
    const auto a = mesh.add_vertex({0, 0, 0});
    const auto b = mesh.add_vertex({10, 0, 0});
    const auto c = mesh.add_vertex({10, 10, 0});
    const auto d = mesh.add_vertex({0, 10, 0});
    mesh.add_face(a, b, c);
    mesh.add_face(a, c, d);
    return mesh;
}

/// Flat floor (y in [0,10], z=0) joined at the seam y=10 to a ramp rising
/// *forward* (y in [10,15], z = y-10). Ramp does NOT overlap the floor
/// -> the whole surface is single-valued -> one region.
SurfaceMesh ramp_forward()
{
    SurfaceMesh mesh{};
    const auto v0 = mesh.add_vertex({0, 0, 0});
    const auto v1 = mesh.add_vertex({10, 0, 0});
    const auto v2 = mesh.add_vertex({10, 10, 0}); // seam
    const auto v3 = mesh.add_vertex({0, 10, 0}); // seam
    const auto v4 = mesh.add_vertex({10, 15, 5}); // ramp top, beyond the floor
    const auto v5 = mesh.add_vertex({0, 15, 5});
    mesh.add_face(v0, v1, v2);
    mesh.add_face(v0, v2, v3);
    mesh.add_face(v3, v2, v4);
    mesh.add_face(v3, v4, v5);
    return mesh;
}

/// Flat floor (y in [0,10], z=0) joined at the seam y=10 to a ramp that folds
/// *back* over the floor (y in [5,10], z rising to 5). Its (x,y)-footprint
/// overlaps the floor -> not single-valued -> floor and ramp must split.
SurfaceMesh ramp_back_over_floor()
{
    SurfaceMesh mesh{};
    const auto v0 = mesh.add_vertex({0, 0, 0});
    const auto v1 = mesh.add_vertex({10, 0, 0});
    const auto v2 = mesh.add_vertex({10, 10, 0}); // seam
    const auto v3 = mesh.add_vertex({0, 10, 0}); // seam
    const auto v4 = mesh.add_vertex({10, 5, 5}); // ramp top, back over the floor
    const auto v5 = mesh.add_vertex({0, 5, 5});
    mesh.add_face(v0, v1, v2);
    mesh.add_face(v0, v2, v3);
    mesh.add_face(v3, v2, v4);
    mesh.add_face(v3, v4, v5);
    return mesh;
}

/// Two stacked floors joined by a ramp, all edge-welded (shared vertices form
/// shared edges at every seam). Lower floor y in [0,5] z=0; ramp y in [5,10]
/// rising z 0->5; upper floor y in [0,10] at z=5, folding back over the lower
/// floor + ramp.
/// lower+ramp is single-valued and MUST merge; only the overlapping upper floor
/// forces a second region. Expected: 2 regions, {lower, ramp} | {upper}.
SurfaceMesh stacked_floors_via_ramp()
{
    SurfaceMesh mesh{};
    const auto v0 = mesh.add_vertex({0, 0, 0});
    const auto v1 = mesh.add_vertex({10, 0, 0});
    const auto v2 = mesh.add_vertex({10, 5, 0}); // seam lower<->ramp
    const auto v3 = mesh.add_vertex({0, 5, 0}); // seam lower<->ramp
    const auto v4 = mesh.add_vertex({10, 10, 5}); // seam ramp<->upper
    const auto v5 = mesh.add_vertex({0, 10, 5}); // seam ramp<->upper
    const auto v6 = mesh.add_vertex({10, 0, 5}); // upper, back over the lower floor
    const auto v7 = mesh.add_vertex({0, 0, 5});
    mesh.add_face(v0, v1, v2); // lower
    mesh.add_face(v0, v2, v3);
    mesh.add_face(v3, v2, v4); // ramp (welded to lower along v2-v3)
    mesh.add_face(v3, v4, v5);
    mesh.add_face(v5, v4, v6); // upper (welded to ramp along v4-v5)
    mesh.add_face(v5, v6, v7);
    return mesh;
}

/// Mean z of a face's vertices -- used to distinguish floor faces (z=0) from ramp faces.
double mean_z(const SurfaceMesh& mesh, SurfaceMesh::Face_index f)
{
    double sum = 0;
    int n = 0;
    for(const auto v : CGAL::vertices_around_face(mesh.halfedge(f), mesh)) {
        sum += mesh.point(v).z();
        ++n;
    }
    return sum / n;
}

/// Flat ring around a stairwell hole, plus a flight welded to one hole edge and rising
/// through the hole footprint: the flight's remaining border edges coincide in (x, y) with
/// the ring's other hole walls WITHOUT sharing those mesh edges -- the essence of
/// examples/geometry/multi_level_u_stair.obj.
SurfaceMesh ring_with_flight_in_the_stairwell()
{
    SurfaceMesh mesh{};
    const auto o0 = mesh.add_vertex({0, 0, 0});
    const auto o1 = mesh.add_vertex({12, 0, 0});
    const auto o2 = mesh.add_vertex({12, 12, 0});
    const auto o3 = mesh.add_vertex({0, 12, 0});
    const auto h0 = mesh.add_vertex({4, 4, 0});
    const auto h1 = mesh.add_vertex({8, 4, 0});
    const auto h2 = mesh.add_vertex({8, 8, 0});
    const auto h3 = mesh.add_vertex({4, 8, 0});
    mesh.add_face(o0, o1, h1);
    mesh.add_face(o0, h1, h0);
    mesh.add_face(o1, o2, h2);
    mesh.add_face(o1, h2, h1);
    mesh.add_face(o2, o3, h3);
    mesh.add_face(o2, h3, h2);
    mesh.add_face(o3, o0, h0);
    mesh.add_face(o3, h0, h3);
    const auto t0 = mesh.add_vertex({4, 8, 2});
    const auto t1 = mesh.add_vertex({8, 8, 2});
    mesh.add_face(h0, h1, t1); // flight, welded to the ring along h0-h1 only
    mesh.add_face(h0, t1, t0);
    return mesh;
}

/// A floor, a ramp up, an upper floor -- all welded -- and a wing whose single vertex
/// (8, 2, 1) projects onto the interior of the floor's border edge y=2: exactly one
/// isolated (x, y) coincidence between distinct surface points, shared with nothing.
SurfaceMesh wing_tip_over_a_floor_edge()
{
    SurfaceMesh mesh{};
    const auto a0 = mesh.add_vertex({0, 0, 0});
    const auto a1 = mesh.add_vertex({10, 0, 0});
    const auto a2 = mesh.add_vertex({10, 2, 0});
    const auto a3 = mesh.add_vertex({0, 2, 0});
    mesh.add_face(a0, a1, a2); // floor
    mesh.add_face(a0, a2, a3);
    const auto r2 = mesh.add_vertex({12, 0, 1});
    const auto r3 = mesh.add_vertex({12, 2, 1});
    mesh.add_face(a1, r2, r3); // ramp, welded to the floor along x=10
    mesh.add_face(a1, r3, a2);
    const auto b3 = mesh.add_vertex({12, 3, 1});
    const auto b4 = mesh.add_vertex({12, 4, 1});
    const auto b5 = mesh.add_vertex({20, 0, 1});
    const auto b6 = mesh.add_vertex({20, 4, 1});
    mesh.add_face(r2, b5, r3); // upper floor, welded to the ramp along x=12
    mesh.add_face(r3, b5, b3);
    mesh.add_face(b3, b5, b6);
    mesh.add_face(b3, b6, b4);
    const auto w = mesh.add_vertex({8, 2, 1});
    mesh.add_face(w, b3, b4); // the wing, welded to the upper floor along b3-b4
    return mesh;
}

using EK = CGAL::Exact_predicates_exact_constructions_kernel;

EK::Triangle_2 project_exact(const SurfaceMesh& mesh, SurfaceMesh::Face_index f)
{
    std::array<EK::Point_2, 3> p{};
    int i = 0;
    for(const auto v : CGAL::vertices_around_face(mesh.halfedge(f), mesh)) {
        const auto& q = mesh.point(v);
        p[i++] = EK::Point_2(q.x(), q.y());
    }
    return {p[0], p[1], p[2]};
}

bool faces_share_edge(const SurfaceMesh& mesh, SurfaceMesh::Face_index a, SurfaceMesh::Face_index b)
{
    for(const auto h : CGAL::halfedges_around_face(mesh.halfedge(a), mesh)) {
        if(mesh.face(mesh.opposite(h)) == b) {
            return true;
        }
    }
    return false;
}

bool faces_share_vertex(
    const SurfaceMesh& mesh,
    SurfaceMesh::Face_index a,
    SurfaceMesh::Face_index b)
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

/// Independent oracle for the invariant split_into_regions() promises: within one region, a
/// face pair's projected intersection may not exceed the dimension of the mesh element the
/// two faces share -- a segment only across the shared edge, a point only at a shared vertex.
void expect_valid_split(const SurfaceMesh& mesh, const RegionSplit& split)
{
    std::vector<SurfaceMesh::Face_index> all{};
    all.reserve(mesh.number_of_faces());
    for(const auto f : faces(mesh)) {
        all.push_back(f);
    }
    for(std::size_t i = 0; i < all.size(); ++i) {
        const auto ti = project_exact(mesh, all[i]);
        const auto bbi = ti.bbox();
        for(std::size_t j = i + 1; j < all.size(); ++j) {
            if(split.region[all[i]] != split.region[all[j]]) {
                continue;
            }
            const auto tj = project_exact(mesh, all[j]);
            if(!CGAL::do_overlap(bbi, tj.bbox())) {
                continue;
            }
            const auto hit = CGAL::intersection(ti, tj);
            if(!hit) {
                continue;
            }
            bool allowed = false;
            if(std::get_if<EK::Point_2>(&*hit) != nullptr) {
                allowed = faces_share_vertex(mesh, all[i], all[j]);
            } else if(std::get_if<EK::Segment_2>(&*hit) != nullptr) {
                allowed = faces_share_edge(mesh, all[i], all[j]);
            }
            EXPECT_TRUE(allowed) << "faces " << all[i] << " and " << all[j] << " of region "
                                 << split.region[all[i]] << " coincide beyond their shared "
                                 << "mesh elements";
        }
    }
}
} // namespace

TEST(RegionSplit, FlatSliverNeighborsStayOneRegion)
{
    // Safeguard against regression seen when transforming BUW.wkt into 3D:
    // Three flat triangles from the BUW floorplan lift, two of
    // them slim slivers that touch each other in exactly ONE point
    // ((11.48, 20.42)). With inexact constructions CGAL::intersection
    // classified that vertex touch as a full triangle-shaped overlap, which
    // fragmented the flat (single-valued!) BUW lift into 218 regions.
    SurfaceMesh mesh{};
    const auto a = mesh.add_vertex({11.56, 18.62, 0});
    const auto b = mesh.add_vertex({11.48, 20.42, 0});
    const auto c = mesh.add_vertex({11.48, 18.62, 0});
    const auto d = mesh.add_vertex({11.56, 20.42, 0});
    const auto e = mesh.add_vertex({8.48, 8.44, 0});
    mesh.add_face(a, b, c);
    mesh.add_face(a, d, b);
    mesh.add_face(b, e, c);
    ASSERT_EQ(mesh.number_of_faces(), 3u);
    const auto [region, count] = split_into_regions(mesh);
    EXPECT_EQ(count, 1u);
}

TEST(RegionSplit, FlatSquareIsSingleRegion)
{
    auto mesh = flat_square();
    const auto [region, count] = split_into_regions(mesh);
    EXPECT_EQ(count, 1u);
}

TEST(RegionSplit, SlopeChangeWithoutOverlapStaysOneRegion)
{
    // A tilted ramp is NOT a reason to split -- only (x,y)-overlap is.
    auto mesh = ramp_forward();
    const auto [region, count] = split_into_regions(mesh);
    EXPECT_EQ(count, 1u);
}

TEST(RegionSplit, SurfaceFoldingOverItselfSplits)
{
    auto mesh = ramp_back_over_floor();
    const auto [region, count] = split_into_regions(mesh);
    ASSERT_EQ(count, 2u);

    // The partition must be exactly {floor faces} | {ramp faces}.
    std::set<std::size_t> floor_ids{};
    std::set<std::size_t> ramp_ids{};
    for(const auto f : faces(mesh)) {
        (mean_z(mesh, f) < 1e-9 ? floor_ids : ramp_ids).insert(region[f]);
    }
    EXPECT_EQ(floor_ids.size(), 1u);
    EXPECT_EQ(ramp_ids.size(), 1u);
    EXPECT_NE(*floor_ids.begin(), *ramp_ids.begin());
}

TEST(RegionSplit, RampMergesWithAFloorAcrossWeldedSeams)
{
    // Ground floor + ramp form a single region, the upper floor another one.
    auto mesh = stacked_floors_via_ramp();
    const auto [region, count] = split_into_regions(mesh);
    ASSERT_EQ(count, 2u);

    // Lower (z=0) + ramp (0<z<5) share one region; the upper floor (z=5) is the
    // other. Merging the ramp into the lower floor is the load-bearing assertion.
    std::set<std::size_t> lower_ramp_ids{};
    std::set<std::size_t> upper_ids{};
    for(const auto f : faces(mesh)) {
        (mean_z(mesh, f) > 5.0 - 1e-9 ? upper_ids : lower_ramp_ids).insert(region[f]);
    }
    EXPECT_EQ(lower_ramp_ids.size(), 1u); // lower AND ramp in a single region
    EXPECT_EQ(upper_ids.size(), 1u);
    EXPECT_NE(*lower_ramp_ids.begin(), *upper_ids.begin());
}

TEST(RegionSplit, AFlightCoincidingWithTheStairwellWallsSplitsOff)
{
    // No positive-area overlap anywhere: the flight fills the hole's footprint. What forces
    // the split is boundary coincidence alone -- the flight's side and top edges lie exactly
    // over the ring's hole walls without sharing those mesh edges.
    const auto mesh = ring_with_flight_in_the_stairwell();
    ASSERT_EQ(mesh.number_of_faces(), 10u);
    const auto split = split_into_regions(mesh);
    EXPECT_EQ(split.count, 2u);

    // Faces 0..7 are the ring, 8..9 the flight (add order on a fresh mesh).
    std::set<std::size_t> ring_ids{};
    std::set<std::size_t> flight_ids{};
    for(const auto f : faces(mesh)) {
        (static_cast<std::size_t>(f) < 8 ? ring_ids : flight_ids).insert(split.region[f]);
    }
    EXPECT_EQ(ring_ids.size(), 1u);
    EXPECT_EQ(flight_ids.size(), 1u);
    EXPECT_NE(*ring_ids.begin(), *flight_ids.begin());
    expect_valid_split(mesh, split);
}

TEST(RegionSplit, AForeignPointContactSplits)
{
    // The wing touches the floor's border edge in exactly one (x, y) point, one metre up and
    // welded to nothing there. Distinct surface points may not share a projection, so the
    // wing cannot stay in the floor's region.
    const auto mesh = wing_tip_over_a_floor_edge();
    ASSERT_EQ(mesh.number_of_faces(), 9u);
    const auto split = split_into_regions(mesh);
    EXPECT_EQ(split.count, 2u);

    // Faces 0..7 are floor+ramp+upper (one welded, coincidence-free surface), face 8 the wing.
    std::set<std::size_t> surface_ids{};
    for(const auto f : faces(mesh)) {
        if(static_cast<std::size_t>(f) < 8) {
            surface_ids.insert(split.region[f]);
        }
    }
    EXPECT_EQ(surface_ids.size(), 1u);
    EXPECT_NE(split.region[8], *surface_ids.begin());
    expect_valid_split(mesh, split);
}

TEST(RegionSplit, EveryFixtureSplitsValidly)
{
    const std::vector<std::pair<std::string, SurfaceMesh>> meshes{
        {"flat_square", flat_square()},
        {"ramp_forward", ramp_forward()},
        {"ramp_back_over_floor", ramp_back_over_floor()},
        {"stacked_floors_via_ramp", stacked_floors_via_ramp()},
        {"ring_with_flight_in_the_stairwell", ring_with_flight_in_the_stairwell()},
        {"wing_tip_over_a_floor_edge", wing_tip_over_a_floor_edge()},
        {"flat_rectangle", fixtures::flat_rectangle()},
        {"two_levels_with_stair", fixtures::two_levels_with_stair()},
        {"two_levels_with_wide_seam", fixtures::two_levels_with_wide_seam()},
        {"stacked_floors", fixtures::stacked_floors()},
        {"switchback_stair", fixtures::switchback_stair()},
        {"switchback_stair_upper_first", fixtures::switchback_stair(true)},
        {"straight_stair_to_a_landing", fixtures::straight_stair_to_a_landing()},
        {"stair_turning_on_a_landing", fixtures::stair_turning_on_a_landing()},
        {"wavy_terrain", fixtures::wavy_terrain()},
        {"corridor_with_door_recesses", fixtures::corridor_with_door_recesses()},
    };
    for(const auto& [name, mesh] : meshes) {
        SCOPED_TRACE(name);
        expect_valid_split(mesh, split_into_regions(mesh));
    }
}
