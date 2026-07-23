// SPDX-License-Identifier: LGPL-3.0-or-later
#include "Geometry/Geometry3D.hpp"

#include <CGAL/mark_domain_in_triangulation.h>
#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <set>
#include <utility>
#include <vector>

namespace
{
/// Two triangles sharing a diagonal; corners given counter-clockwise.
void add_quad(SurfaceMesh& mesh, const std::array<Point3D, 4>& corners)
{
    const auto v0 = mesh.add_vertex(corners[0]);
    const auto v1 = mesh.add_vertex(corners[1]);
    const auto v2 = mesh.add_vertex(corners[2]);
    const auto v3 = mesh.add_vertex(corners[3]);
    mesh.add_face(v0, v1, v2);
    mesh.add_face(v0, v2, v3);
}

SurfaceMesh flat_room()
{
    SurfaceMesh mesh{};
    add_quad(mesh, {Point3D{0, 0, 0}, {10, 0, 0}, {10, 10, 0}, {0, 10, 0}});
    return mesh;
}

/// A ramp climbing from z=0 at y=0 to z=4 at y=10 (so z = 0.4*y), one region.
SurfaceMesh ramp()
{
    SurfaceMesh mesh{};
    add_quad(mesh, {Point3D{5, 0, 0}, {15, 0, 0}, {15, 10, 4}, {5, 10, 4}});
    return mesh;
}

/// Two disjoint floors sharing the same (x,y) footprint at different heights:
/// the canonical stacking case that (x,y) alone cannot resolve.
SurfaceMesh stacked_floors()
{
    SurfaceMesh mesh{};
    add_quad(mesh, {Point3D{0, 0, 0}, {10, 0, 0}, {10, 10, 0}, {0, 10, 0}});
    add_quad(mesh, {Point3D{0, 0, 3}, {10, 0, 3}, {10, 10, 3}, {0, 10, 3}});
    return mesh;
}

} // namespace

TEST(Geometry3DLocate, FlatRegionYieldsGroundHeight)
{
    Geometry3D geo{flat_room()};
    ASSERT_EQ(geo.region_count(), 1);

    const auto loc = geo.locate_in_region(0, {5, 5});
    ASSERT_NE(loc.face, SurfaceMesh::null_face());
    EXPECT_NEAR(loc.point.z(), 0.0, 1e-9);
    EXPECT_NEAR(loc.point.x(), 5.0, 1e-9);
    EXPECT_NEAR(loc.point.y(), 5.0, 1e-9);
}

TEST(Geometry3DLocate, PointOutsideRegionFootprintMisses)
{
    Geometry3D geo{flat_room()};
    EXPECT_EQ(geo.locate_in_region(0, {20, 20}).face, SurfaceMesh::null_face());
}

TEST(Geometry3DLocate, RampInterpolatesHeightOnFace)
{
    Geometry3D geo{ramp()};
    ASSERT_EQ(geo.region_count(), 1);

    // by construction: z == 0.4*y
    EXPECT_NEAR(geo.locate_in_region(0, {10, 2}).point.z(), 0.8, 1e-9);
    EXPECT_NEAR(geo.locate_in_region(0, {10, 9}).point.z(), 3.6, 1e-9);
}

TEST(Geometry3DLocate, RegionIdDisambiguatesStackedFloors)
{
    Geometry3D geo{stacked_floors()};
    ASSERT_EQ(geo.region_count(), 2);

    // Same (x,y), two sheets: the region id picks which one.
    EXPECT_NEAR(geo.locate_in_region(0, {5, 5}).point.z(), 0.0, 1e-9);
    EXPECT_NEAR(geo.locate_in_region(1, {5, 5}).point.z(), 3.0, 1e-9);
}

namespace
{
/// 10x10 square with a centred 2x2 hole, the 2D input shape.
PolyWithHoles square_with_hole()
{
    const std::vector<K::Point_2> outer{{0, 0}, {10, 0}, {10, 10}, {0, 10}};
    const std::vector<K::Point_2> hole{{4, 4}, {4, 6}, {6, 6}, {6, 4}};
    PolyWithHoles poly{Poly(outer.begin(), outer.end())};
    poly.add_hole(Poly(hole.begin(), hole.end()));
    return poly;
}
} // namespace

TEST(Geometry3DFromPolygon, HoleIsNotWalkable)
{
    Geometry3D geo{square_with_hole()};

    ASSERT_EQ(geo.region_count(), 1);
    EXPECT_TRUE(geo.is_valid_location({1, 1, 1}));
    EXPECT_FALSE(geo.is_valid_location({5, 5, 1})); // inside the hole
    EXPECT_FALSE(geo.is_valid_location({20, 20, 1})); // outside geometry
    // All lifted vertices sit at z=0.
    for(const auto& v : geo.vertices()) {
        EXPECT_EQ(v[2], 0.0);
    }
}

TEST(Geometry3DFromPolygon, OwnsTheProjected2DView)
{
    Geometry3D geo{square_with_hole()};

    const auto* view = geo.geometry_2d();
    ASSERT_NE(view, nullptr);
    EXPECT_TRUE(view->InsideGeometry({1, 1}));
    EXPECT_FALSE(view->InsideGeometry({5, 5})); // inside the hole
}

TEST(Geometry3DFromMesh, HasNoProjected2DView)
{
    Geometry3D geo{flat_room()};
    // A mesh carries no 2D view -- unlike a polygon-built geometry.
    EXPECT_EQ(geo.geometry_2d(), nullptr);
}

TEST(Geometry3DFromPolygon, LiftReproducesThe2DTriangulation)
{
    const auto poly = square_with_hole();
    Geometry3D geo{poly};

    // The reference: the CDT exactly as the 2D RoutingEngine builds it.
    CDT cdt{};
    cdt.insert_constraint(
        poly.outer_boundary().vertices_begin(), poly.outer_boundary().vertices_end(), true);
    for(const auto& hole : poly.holes()) {
        cdt.insert_constraint(hole.vertices_begin(), hole.vertices_end(), true);
    }
    CGAL::mark_domain_in_triangulation(cdt);

    // Coordinates are copied verbatim (both kernels store doubles), so the
    // centroid sets must match exactly in x/y.
    std::vector<std::pair<double, double>> expected{};
    for(auto f = cdt.finite_faces_begin(); f != cdt.finite_faces_end(); ++f) {
        if(f->get_in_domain()) {
            const auto c =
                CGAL::centroid(f->vertex(0)->point(), f->vertex(1)->point(), f->vertex(2)->point());
            expected.emplace_back(c.x(), c.y());
        }
    }
    const auto vertices = geo.vertices();
    std::vector<std::pair<double, double>> actual{};
    for(const auto& tri : geo.triangles()) {
        double x = 0;
        double y = 0;
        for(const auto v : tri) {
            x += vertices[v][0];
            y += vertices[v][1];
        }
        actual.emplace_back(x / 3.0, y / 3.0);
    }
    // Set equality: the lift produces the same triangles.
    std::sort(expected.begin(), expected.end());
    std::sort(actual.begin(), actual.end());
    EXPECT_EQ(actual, expected);
    // Exactly the triangulation's corner vertices.
    EXPECT_EQ(vertices.size(), cdt.number_of_vertices());
}
