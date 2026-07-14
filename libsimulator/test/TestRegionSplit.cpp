// SPDX-License-Identifier: LGPL-3.0-or-later
#include "CfgCgal.hpp"
#include "Geometry/RegionSplit.hpp"

#include <gtest/gtest.h>

#include <set>

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
