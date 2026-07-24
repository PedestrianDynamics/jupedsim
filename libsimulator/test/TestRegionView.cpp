// SPDX-License-Identifier: LGPL-3.0-or-later
#include "Geometry/Geometry2D.hpp"
#include "Geometry/Geometry3D.hpp"
#include "Geometry/RegionView.hpp"
#include "LineSegment.hpp"
#include "Point.hpp"

#include <gtest/gtest.h>

#include <array>
#include <set>
#include <utility>
#include <vector>

namespace
{
/// A canonical (undirected) form of a segment so two wall sets can be compared
/// regardless of per-segment orientation.
LineSegment canonical(const LineSegment& s)
{
    return s.p1 < s.p2 ? s : LineSegment{s.p2, s.p1};
}

std::set<LineSegment> as_set(const std::vector<LineSegment>& segments)
{
    std::set<LineSegment> out{};
    for(const auto& s : segments) {
        out.insert(canonical(s));
    }
    return out;
}

/// Every wall of a RegionView (a huge query radius returns all of them).
std::vector<LineSegment> all_walls(const RegionView& view)
{
    std::vector<LineSegment> out{};
    for(const auto& s : view.LineSegmentsInDistanceTo(1e12, {0, 0})) {
        out.push_back(s);
    }
    return out;
}

/// Given a side view ("profile"), add y coordinates to turn it full 3D.
/// Uniform winding keeps the orientation consistent across folds.
SurfaceMesh extrude_profile(const std::vector<std::array<double, 2>>& profile, double width)
{
    SurfaceMesh mesh{};
    std::vector<SurfaceMesh::Vertex_index> bottom{};
    std::vector<SurfaceMesh::Vertex_index> top{};
    for(const auto& [x, z] : profile) {
        bottom.push_back(mesh.add_vertex(Point3D{x, 0.0, z}));
        top.push_back(mesh.add_vertex(Point3D{x, width, z}));
    }
    for(std::size_t i = 0; i + 1 < profile.size(); ++i) {
        mesh.add_face(bottom[i], bottom[i + 1], top[i + 1]);
        mesh.add_face(bottom[i], top[i + 1], top[i]);
    }
    return mesh;
}

/// Floor (z=0) -> ramp up -> mezzanine (z=3) folding back over the floor. The
/// floor+ramp stay one region; the mezzanine overlaps the floor in (x,y) and
/// becomes a second region. The fold edge between ramp and mezzanine is a seam.
SurfaceMesh folded_ribbon()
{
    return extrude_profile({{0, 0}, {10, 0}, {12, 3}, {2, 3}}, 10.0);
}

/// A flat 10x10 rectangle built from a mesh, with the y=0 and y=10 sides each
/// subdivided by a collinear mid vertex (x=5): the merge must fuse them back.
SurfaceMesh flat_rectangle_mesh()
{
    return extrude_profile({{0, 0}, {5, 0}, {10, 0}}, 10.0);
}

PolyWithHoles rectangle_poly()
{
    const std::vector<K::Point_2> outer{{0, 0}, {10, 0}, {10, 10}, {0, 10}};
    return PolyWithHoles{Poly(outer.begin(), outer.end())};
}
} // namespace

// -- merge_collinear (standalone) -------------------------------------------

TEST(MergeCollinear, EmptyStaysEmpty)
{
    EXPECT_TRUE(merge_collinear({}, 1e-9).empty());
}

TEST(MergeCollinear, SingleSegmentSurvives)
{
    const std::vector<LineSegment> in{{{0, 0}, {10, 0}}};
    EXPECT_EQ(as_set(merge_collinear(in, 1e-9)), as_set(in));
}

TEST(MergeCollinear, CollinearRunFusesToOne)
{
    const std::vector<LineSegment> in{{{0, 0}, {5, 0}}, {{5, 0}, {10, 0}}};
    const auto out = merge_collinear(in, 1e-9);
    ASSERT_EQ(out.size(), 1u);
    EXPECT_EQ(as_set(out), as_set({{{0, 0}, {10, 0}}}));
}

TEST(MergeCollinear, CornerIsPreserved)
{
    const std::vector<LineSegment> in{{{0, 0}, {5, 0}}, {{5, 0}, {5, 5}}};
    EXPECT_EQ(as_set(merge_collinear(in, 1e-9)), as_set(in));
}

TEST(MergeCollinear, JunctionBreaksTheChain)
{
    // A T-junction at (5,0): three spokes, none collinear-mergeable across it.
    const std::vector<LineSegment> in{{{0, 0}, {5, 0}}, {{5, 0}, {10, 0}}, {{5, 0}, {5, 5}}};
    EXPECT_EQ(as_set(merge_collinear(in, 1e-9)), as_set(in));
}

// -- RegionView on a flat mesh (A/B vs polygon build) -----------------------

TEST(RegionViewFlat, SingleRegionNoSeams)
{
    Geometry3D geo{flat_rectangle_mesh()};
    ASSERT_EQ(geo.region_count(), 1u);
    EXPECT_TRUE(geo.region_view(0).seam_neighbors().empty());
    EXPECT_FALSE(geo.region_view(0).crosses_seam({0, 5}, {10, 5}));
}

TEST(RegionViewFlat, MergedWallsEqualPolygonWalls)
{
    Geometry3D mesh_geo{flat_rectangle_mesh()};
    Geometry3D poly_geo{rectangle_poly()};
    ASSERT_NE(poly_geo.geometry_2d(), nullptr);

    // Collinear-subdivided mesh borders merge back to the 4 rectangle sides,
    // matching the polygon build's walls exactly (undirected).
    const auto mesh_walls = as_set(all_walls(mesh_geo.region_view(0)));
    const auto poly_walls = as_set(all_walls(poly_geo.region_view(0)));
    EXPECT_EQ(mesh_walls.size(), 4u);
    EXPECT_EQ(mesh_walls, poly_walls);
}

TEST(RegionViewFlat, InsideGeometryMatchesTheMeshFootprint)
{
    Geometry3D geo{flat_rectangle_mesh()};
    const auto& view = geo.region_view(0);
    EXPECT_TRUE(view.InsideGeometry({5, 5}));
    EXPECT_FALSE(view.InsideGeometry({20, 20}));
}

// -- RegionView on the folded two-region mesh -------------------------------

TEST(RegionViewFolded, SeamSplitsIntoTwoRegions)
{
    Geometry3D geo{folded_ribbon()};
    ASSERT_EQ(geo.region_count(), 2u);
    // Each region contains the other one as neighbor.
    EXPECT_EQ(geo.region_view(0).seam_neighbors(), (std::vector<std::size_t>{1}));
    EXPECT_EQ(geo.region_view(1).seam_neighbors(), (std::vector<std::size_t>{0}));
}

TEST(RegionViewFolded, SeamIsInSeamGridNotWallGrid)
{
    Geometry3D geo{folded_ribbon()};
    // The fold edge sits at x=12; a chord crossing it must register as a seam
    // crossing but not as a wall hit.
    const Point a{11, 5};
    const Point b{13, 5};
    EXPECT_TRUE(geo.region_view(0).crosses_seam(a, b));
    EXPECT_FALSE(geo.region_view(0).IntersectsAny(LineSegment{a, b}));

    // The seam is never among the walls.
    for(const auto& w : all_walls(geo.region_view(0))) {
        EXPECT_FALSE(canonical(w) == canonical(LineSegment{{12, 0}, {12, 10}}));
    }
}

TEST(RegionViewFolded, InsideGeometryCorrectOnBothSidesOfTheSeam)
{
    Geometry3D geo{folded_ribbon()};
    // (6,5): floor (region 0, z=0) AND mezzanine (region 1, z=3) both cover it.
    EXPECT_TRUE(geo.region_view(0).InsideGeometry({6, 5}));
    EXPECT_TRUE(geo.region_view(1).InsideGeometry({6, 5}));
    // (1,5): only the floor reaches x<2; the mezzanine starts at x=2.
    EXPECT_TRUE(geo.region_view(0).InsideGeometry({1, 5}));
    EXPECT_FALSE(geo.region_view(1).InsideGeometry({1, 5}));
    // Outside every footprint.
    EXPECT_FALSE(geo.region_view(0).InsideGeometry({20, 5}));
    EXPECT_FALSE(geo.region_view(1).InsideGeometry({20, 5}));
}
