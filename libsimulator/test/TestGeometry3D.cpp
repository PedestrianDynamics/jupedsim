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

/// A flat square split into four triangles by a centre vertex, so a straight
/// walk can pass exactly *through* that vertex from the bottom triangle to the
/// non-adjacent top triangle -- the corner/fan case for walk_on_surface.
SurfaceMesh fan_square()
{
    SurfaceMesh mesh{};
    const auto c00 = mesh.add_vertex(Point3D{0, 0, 0});
    const auto c20 = mesh.add_vertex(Point3D{2, 0, 0});
    const auto c22 = mesh.add_vertex(Point3D{2, 2, 0});
    const auto c02 = mesh.add_vertex(Point3D{0, 2, 0});
    const auto ctr = mesh.add_vertex(Point3D{1, 1, 0});
    mesh.add_face(c00, c20, ctr); // bottom
    mesh.add_face(c20, c22, ctr); // right
    mesh.add_face(c22, c02, ctr); // top
    mesh.add_face(c02, c00, ctr); // left
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

TEST(Geometry3DWalk, WalkAcrossInteriorDiagonalKeepsRegionAndInterpolatesHeight)
{
    Geometry3D geo{ramp()};
    ASSERT_EQ(geo.region_count(), 1);

    // (10,2) and (10,8) sit in the two triangles of the ramp quad; the straight
    // walk crosses the shared diagonal, stays in the one region, z tracks 0.4*y.
    const auto loc = geo.walk_on_surface(0, {10, 2}, {10, 8});
    ASSERT_NE(loc.face, SurfaceMesh::null_face());
    EXPECT_EQ(geo.region_of(loc.face), 0u);
    EXPECT_NEAR(loc.point.x(), 10.0, 1e-9);
    EXPECT_NEAR(loc.point.y(), 8.0, 1e-9);
    EXPECT_NEAR(loc.point.z(), 3.2, 1e-9);
}

TEST(Geometry3DWalk, WalkReachesVertexNeighbourFace)
{
    Geometry3D geo{fan_square()};
    ASSERT_EQ(geo.region_count(), 1);

    // Target (1,1.7) sits in the top triangle, which touches the start (bottom)
    // triangle only at the centre vertex (1,1) -- an edge search would miss it;
    // the vertex 1-ring covers it.
    const auto loc = geo.walk_on_surface(0, {1, 0.3}, {1, 1.7});
    ASSERT_NE(loc.face, SurfaceMesh::null_face());
    EXPECT_NEAR(loc.point.x(), 1.0, 1e-9);
    EXPECT_NEAR(loc.point.y(), 1.7, 1e-9);
    EXPECT_NEAR(loc.point.z(), 0.0, 1e-9);
}

TEST(Geometry3DWalk, WalkStartingExactlyOnVertexResolvesToNeighbour)
{
    Geometry3D geo{fan_square()};

    // 'from' is exactly the centre vertex (1,1). This is a corner case as
    // locate_in_region returns an triangle. We try walking to any of the
    // surrounding triangles to make sure this works as expected.
    const std::array<Point2D, 4> targets{
        Point2D{1, 0.3}, // bottom triangle
        Point2D{1.7, 1}, // right triangle
        Point2D{1, 1.7}, // top triangle
        Point2D{0.3, 1}, // left triangle
    };
    for(const auto& to : targets) {
        const auto loc = geo.walk_on_surface(0, {1, 1}, to);
        // Each target is strictly interior to exactly one fan triangle, so a
        // non-null result can only mean the correct face was found.
        ASSERT_NE(loc.face, SurfaceMesh::null_face());
        EXPECT_NEAR(loc.point.x(), to.x(), 1e-9);
        EXPECT_NEAR(loc.point.y(), to.y(), 1e-9);
        EXPECT_NEAR(loc.point.z(), 0.0, 1e-9);
    }
}

TEST(Geometry3DWalk, WalkWithinOneTriangleReturnsTargetHeight)
{
    Geometry3D geo{ramp()};

    // Both endpoints in the same triangle
    const auto loc = geo.walk_on_surface(0, {10, 2}, {11, 3});
    ASSERT_NE(loc.face, SurfaceMesh::null_face());
    EXPECT_NEAR(loc.point.x(), 11.0, 1e-9);
    EXPECT_NEAR(loc.point.y(), 3.0, 1e-9);
    EXPECT_NEAR(loc.point.z(), 1.2, 1e-9); // 0.4*3
}

TEST(Geometry3DWalk, WalkTargetOutsideNeighbourhoodThrows)
{
    Geometry3D geo{flat_room()};

    // 'to' far outside the start face and its 1-ring: a step too large for the
    // mesh resolution (or off-surface) -- rejected rather than silently wrong.
    EXPECT_ANY_THROW(geo.walk_on_surface(0, {3, 2}, {20, 20}));
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

    const auto* view = geo.collision_geometry();
    ASSERT_NE(view, nullptr);
    EXPECT_TRUE(view->InsideGeometry({1, 1}));
    EXPECT_FALSE(view->InsideGeometry({5, 5})); // inside the hole
}

TEST(Geometry3DFromMesh, HasNoProjected2DView)
{
    Geometry3D geo{flat_room()};
    // A mesh carries no 2D view -- unlike a polygon-built geometry.
    EXPECT_EQ(geo.collision_geometry(), nullptr);
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
