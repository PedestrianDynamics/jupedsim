// SPDX-License-Identifier: LGPL-3.0-or-later
#include "Geometry/Geometry3D.hpp"
#include "Geometry/Location.hpp"

#include <gtest/gtest.h>

#include <array>
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

/// Two disjoint floors sharing the same (x,y) footprint at z=0 and z=3.
SurfaceMesh stacked_floors()
{
    SurfaceMesh mesh{};
    add_quad(mesh, {Point3D{0, 0, 0}, {10, 0, 0}, {10, 10, 0}, {0, 10, 0}});
    add_quad(mesh, {Point3D{0, 0, 3}, {10, 0, 3}, {10, 10, 3}, {0, 10, 3}});
    return mesh;
}

/// 10x10 square with a centred 2x2 hole -- a polygon-built geometry.
PolyWithHoles square_with_hole()
{
    const std::vector<K::Point_2> outer{{0, 0}, {10, 0}, {10, 10}, {0, 10}};
    const std::vector<K::Point_2> hole{{4, 4}, {4, 6}, {6, 6}, {6, 4}};
    PolyWithHoles poly{Poly(outer.begin(), outer.end())};
    poly.add_hole(Poly(hole.begin(), hole.end()));
    return poly;
}

/// A flat square split into four triangles by a centre vertex (1,1), so a
/// straight move can pass exactly *through* that vertex from one triangle to a
/// non-adjacent one -- the hits-vertex / start-on-vertex case.
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

/// A connected flat strip of @p n unit quads along x-axis (2*n triangles sharing
/// vertices), for straight moves that cross many faces.
SurfaceMesh flat_strip(int n)
{
    SurfaceMesh mesh{};
    std::vector<SurfaceMesh::Vertex_index> bottom{}, top{};
    for(int i = 0; i <= n; ++i) {
        bottom.push_back(mesh.add_vertex(Point3D{static_cast<double>(i), 0, 0}));
        top.push_back(mesh.add_vertex(Point3D{static_cast<double>(i), 1, 0}));
    }
    for(int i = 0; i < n; ++i) {
        mesh.add_face(bottom[i], bottom[i + 1], top[i + 1]);
        mesh.add_face(bottom[i], top[i + 1], top[i]);
    }
    return mesh;
}

/// A fan of @p n slivers sharing an apex at the origin; rim vertices sit at x=1,
/// y=i*@p spacing. Moving close to the apex crosses all of triangles within a tiny
/// band. "Stress test" for the face walk.
SurfaceMesh sliver_fan(int n, double spacing)
{
    SurfaceMesh mesh{};
    const auto apex = mesh.add_vertex(Point3D{0, 0, 0});
    std::vector<SurfaceMesh::Vertex_index> rim{};
    for(int i = 0; i <= n; ++i) {
        rim.push_back(mesh.add_vertex(Point3D{1, i * spacing, 0}));
    }
    for(int i = 0; i < n; ++i) {
        mesh.add_face(apex, rim[i], rim[i + 1]);
    }
    return mesh;
}

/// Two stacked rows of triangles sharing the horizontal interior edge chain at
/// y=1 (vertices (i,1)). A straight move along y=1 runs exactly along that edge
/// chain -- the collinear "walk along an edge" degenerate case.
SurfaceMesh two_row_strip(int n)
{
    SurfaceMesh mesh{};
    std::vector<SurfaceMesh::Vertex_index> b{}, m{}, t{};
    for(int i = 0; i <= n; ++i) {
        const auto x = static_cast<double>(i);
        b.push_back(mesh.add_vertex(Point3D{x, 0, 0}));
        m.push_back(mesh.add_vertex(Point3D{x, 1, 0}));
        t.push_back(mesh.add_vertex(Point3D{x, 2, 0}));
    }
    for(int i = 0; i < n; ++i) {
        mesh.add_face(b[i], b[i + 1], m[i]); // row 0 (y in [0,1])
        mesh.add_face(b[i + 1], m[i + 1], m[i]);
        mesh.add_face(m[i], m[i + 1], t[i]); // row 1 (y in [1,2])
        mesh.add_face(m[i + 1], t[i + 1], t[i]);
    }
    return mesh;
}

/// Location at @p (x,y) on a single-sheet mesh, disambiguation off (generous z
/// tolerance). Fails the calling test if the point is off the walkable area.
Location location_at(const Geometry3D& geo, double x, double y)
{
    auto loc = geo.get_location(x, y, 0.0, 100.0);
    EXPECT_TRUE(loc.has_value()) << "location (" << x << "," << y << ") is off the surface";
    return *loc;
}
} // namespace

TEST(Location, LocationOnFlatGround)
{
    Geometry3D geo{flat_room()};
    ASSERT_EQ(geo.region_count(), 1);

    const auto loc = geo.get_location(5, 5, 0.0);
    ASSERT_TRUE(loc.has_value());
    EXPECT_DOUBLE_EQ(loc->xy().x, 5.0);
    EXPECT_DOUBLE_EQ(loc->xy().y, 5.0);
    EXPECT_NEAR(loc->z(), 0.0, 1e-9);
    EXPECT_EQ(loc->region(), 0u);
}

TEST(Location, XyIsTheExactAndZCorrectlyCalculated)
{
    Geometry3D geo{ramp()};

    // On the ramp z == 0.4*y; the location keeps the exact (x,y) and caches z.
    const auto loc = geo.get_location(10, 2, 0.0, 1.0);
    ASSERT_TRUE(loc.has_value());
    EXPECT_DOUBLE_EQ(loc->xy().x, 10.0);
    EXPECT_DOUBLE_EQ(loc->xy().y, 2.0);
    EXPECT_NEAR(loc->z(), 0.8, 1e-9);
}

TEST(Location, Position3DCombinesXyAndCachedZ)
{
    Geometry3D geo{ramp()};

    const auto loc = geo.get_location(10, 9, 3.6, 0.1);
    ASSERT_TRUE(loc.has_value());
    EXPECT_NEAR(loc->z(), 3.6, 1e-9);
    const auto p = loc->position_3d();
    EXPECT_DOUBLE_EQ(p.x(), 10.0);
    EXPECT_DOUBLE_EQ(p.y(), 9.0);
    EXPECT_NEAR(p.z(), 3.6, 1e-9);
}

TEST(Location, ZHintDisambiguatesStackedSheets)
{
    Geometry3D geo{stacked_floors()};
    ASSERT_EQ(geo.region_count(), 2);

    const auto lower = geo.get_location(5, 5, 0.0);
    ASSERT_TRUE(lower.has_value());
    EXPECT_NEAR(lower->z(), 0.0, 1e-9);

    const auto upper = geo.get_location(5, 5, 3.0);
    ASSERT_TRUE(upper.has_value());
    EXPECT_NEAR(upper->z(), 3.0, 1e-9);

    // The two sheets are distinct regions.
    EXPECT_NE(lower->region(), upper->region());
}

TEST(Location, ZHintPicksTheNearerSheetWithinTolerance)
{
    Geometry3D geo{stacked_floors()};

    // Hint closer to the upper sheet -- default tolerance still resolves it.
    const auto loc = geo.get_location(5, 5, 2.95);
    ASSERT_TRUE(loc.has_value());
    EXPECT_NEAR(loc->z(), 3.0, 1e-9);
}

TEST(Location, ZHintBeyondToleranceOfAnySheetMisses)
{
    Geometry3D geo{stacked_floors()};

    // Midway between the sheets (z=0 and z=3): 1.4 m from the nearer one,
    // beyond the 0.1 default tolerance -> no location.
    EXPECT_FALSE(geo.get_location(5, 5, 1.4).has_value());
    // A generous explicit tolerance recovers the nearer sheet.
    const auto loc = geo.get_location(5, 5, 1.4, 1.5);
    ASSERT_TRUE(loc.has_value());
    EXPECT_NEAR(loc->z(), 0.0, 1e-9);
}

TEST(Location, PointOutsideFootprintMisses)
{
    Geometry3D geo{flat_room()};
    EXPECT_FALSE(geo.get_location(20, 20, 0.0).has_value());
}

TEST(Location, PointInsideHoleMisses)
{
    Geometry3D geo{square_with_hole()};

    EXPECT_TRUE(geo.get_location(1, 1, 0.0).has_value());
    EXPECT_FALSE(geo.get_location(5, 5, 0.0).has_value()); // inside the hole
}

// -- move_on_surface --

TEST(LocationMove, AcrossInteriorDiagonalKeepsRegionAndInterpolatesHeight)
{
    Geometry3D geo{ramp()};
    ASSERT_EQ(geo.region_count(), 1);

    // (10,2) and (10,8) sit in the two triangles of the ramp quad; the straight
    // move crosses the shared diagonal, stays in one region, z tracks 0.4*y.
    auto loc = location_at(geo, 10, 2);
    loc.move_on_surface(Point{0, 6}); // -> (10,8)
    EXPECT_DOUBLE_EQ(loc.xy().x, 10.0);
    EXPECT_DOUBLE_EQ(loc.xy().y, 8.0);
    EXPECT_EQ(loc.region(), 0u);
    EXPECT_NEAR(loc.z(), 3.2, 1e-9);
}

TEST(LocationMove, HitsVertexWhileWalking)
{
    Geometry3D geo{fan_square()};

    // Target (1,1.7) sits in the top triangle, touching the start (bottom)
    // triangle only at the centre vertex (1,1); the move hits that vertex.
    auto loc = location_at(geo, 1, 0.3);
    loc.move_on_surface(Point{0, 1.4}); // -> (1,1.7)
    EXPECT_DOUBLE_EQ(loc.xy().x, 1.0);
    EXPECT_DOUBLE_EQ(loc.xy().y, 1.7);
    EXPECT_NEAR(loc.z(), 0.0, 1e-9);
}

TEST(LocationMove, StartingExactlyOnVertexResolvesToNeighbour)
{
    Geometry3D geo{fan_square()};

    // The Location sits exactly on the centre vertex (1,1); its cached face is an
    // arbitrary incident one. A move into any of the four fan triangles must
    // resolve to the correct one.
    const std::array<Point, 4> targets{
        Point{1, 0.3}, // bottom triangle
        Point{1.7, 1}, // right triangle
        Point{1, 1.7}, // top triangle
        Point{0.3, 1}, // left triangle
    };
    for(const auto& target : targets) {
        auto loc = location_at(geo, 1, 1);
        loc.move_on_surface(target - Point{1, 1});
        EXPECT_DOUBLE_EQ(loc.xy().x, target.x);
        EXPECT_DOUBLE_EQ(loc.xy().y, target.y);
        EXPECT_NEAR(loc.z(), 0.0, 1e-9);
    }
}

TEST(LocationMove, WithinOneTriangleReturnsTargetHeight)
{
    Geometry3D geo{ramp()};

    auto loc = location_at(geo, 10, 2);
    loc.move_on_surface(Point{1, 1}); // -> (11,3), start/end in one triangle
    EXPECT_DOUBLE_EQ(loc.xy().x, 11.0);
    EXPECT_DOUBLE_EQ(loc.xy().y, 3.0);
    EXPECT_NEAR(loc.z(), 1.2, 1e-9); // 0.4*3
}

TEST(LocationMove, LeavingWalkableAreaThrowsAndLeavesLocationUnchanged)
{
    Geometry3D geo{flat_room()};

    auto loc = location_at(geo, 3, 2);
    // The straight path crosses a border edge -> leaves the walkable area.
    // --> move throws.
    EXPECT_ANY_THROW(loc.move_on_surface(Point{17, 18})); // -> (20,20)
    // Strong exception guarantee: the Location is unchanged.
    EXPECT_DOUBLE_EQ(loc.xy().x, 3.0);
    EXPECT_DOUBLE_EQ(loc.xy().y, 2.0);
    EXPECT_NEAR(loc.z(), 0.0, 1e-9);
}

TEST(LocationMove, LeavingOverABoundaryCornerThrows)
{
    Geometry3D geo{flat_room()}; // [0,10]^2

    // The straight path passes exactly through the boundary corner (10,10) and
    // continues outward. It leaves the walkable area *at the vertex*.
    auto loc = location_at(geo, 4, 3);
    EXPECT_ANY_THROW(loc.move_on_surface(Point{12, 14})); // through (10,10) -> (16,17)
    // Strong exception guarantee: the Location is unchanged.
    EXPECT_DOUBLE_EQ(loc.xy().x, 4.0);
    EXPECT_DOUBLE_EQ(loc.xy().y, 3.0);
}

TEST(LocationMove, AlongAnInteriorEdgeChain)
{
    Geometry3D geo{two_row_strip(4)};
    ASSERT_EQ(geo.region_count(), 1);

    // The path runs exactly along the interior edge chain at y=1, through every
    // vertex (i,1) and collinear with each edge -- the "walk along an edge"
    // degenerate case.
    auto loc = location_at(geo, 0.5, 1.0);
    loc.move_on_surface(Point{3, 0}); // -> (3.5,1.0)
    EXPECT_DOUBLE_EQ(loc.xy().x, 3.5);
    EXPECT_DOUBLE_EQ(loc.xy().y, 1.0);
    EXPECT_EQ(loc.region(), 0u);
    EXPECT_NEAR(loc.z(), 0.0, 1e-9);
}

TEST(LocationMove, NearTheApexOfASliverFan)
{
    constexpr int n = 20;
    constexpr double spacing = 1e-3;
    Geometry3D geo{sliver_fan(n, spacing)};
    ASSERT_EQ(geo.region_count(), 1);

    // A vertical move at x=eps, very close to the apex, crosses every fan
    // edge within a band only spacing*eps wide.
    constexpr double eps = 1e-3;
    const Point start{eps, spacing * eps * 0.5};
    const Point target{eps, spacing * eps * (n - 0.5)};
    auto loc = location_at(geo, start.x, start.y);
    loc.move_on_surface(target - start);
    EXPECT_DOUBLE_EQ(loc.xy().x, eps);
    EXPECT_NEAR(loc.xy().y, target.y, 1e-18);
    EXPECT_NEAR(loc.z(), 0.0, 1e-9);
}

TEST(LocationMove, LongStepCrossesManyFaces)
{
    Geometry3D geo{flat_strip(50)}; // 100 triangles

    // One straight step from the first quad to the last, crossing 2 faces per
    // quad. Checks whether long steps are fine.
    const Point start{0.5, 0.3};
    const Point target{49.5, 0.7};
    auto loc = location_at(geo, start.x, start.y);
    loc.move_on_surface(target - start);
    EXPECT_DOUBLE_EQ(loc.xy().x, 49.5);
    EXPECT_DOUBLE_EQ(loc.xy().y, 0.7);
    EXPECT_EQ(loc.region(), 0u);
    EXPECT_NEAR(loc.z(), 0.0, 1e-9);
}

// -- try_move_on_surface (feasibility variant, non-throwing) --

TEST(LocationTryMove, SucceedsReturnsResultAndLeavesSourceUnchanged)
{
    Geometry3D geo{ramp()};

    const auto loc = location_at(geo, 10, 2);
    const auto moved = loc.try_move_on_surface(Point{1, 1}); // -> (11,3), z=1.2
    ASSERT_TRUE(moved.has_value());
    EXPECT_DOUBLE_EQ(moved->xy().x, 11.0);
    EXPECT_DOUBLE_EQ(moved->xy().y, 3.0);
    EXPECT_NEAR(moved->z(), 1.2, 1e-9);
    EXPECT_EQ(moved->region(), 0u);
    // The source Location is const and must be untouched.
    EXPECT_DOUBLE_EQ(loc.xy().x, 10.0);
    EXPECT_DOUBLE_EQ(loc.xy().y, 2.0);
}

TEST(LocationTryMove, LeavingAcrossBorderEdgeReturnsNullopt)
{
    Geometry3D geo{flat_room()};

    const auto loc = location_at(geo, 3, 2);
    // Straight path crosses a border edge -> no result (instead of throwing).
    EXPECT_FALSE(loc.try_move_on_surface(Point{17, 18}).has_value()); // -> (20,20)
    // Source unchanged.
    EXPECT_DOUBLE_EQ(loc.xy().x, 3.0);
    EXPECT_DOUBLE_EQ(loc.xy().y, 2.0);
}

TEST(LocationTryMove, LeavingOverBoundaryCornerReturnsNullopt)
{
    Geometry3D geo{flat_room()}; // [0,10]^2

    // Straight path passes exactly through the boundary corner (10,10) and
    // continues outward -> leaves the area at the vertex.
    const auto loc = location_at(geo, 4, 3);
    EXPECT_FALSE(loc.try_move_on_surface(Point{12, 14}).has_value()); // through (10,10)
}
