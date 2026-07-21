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
