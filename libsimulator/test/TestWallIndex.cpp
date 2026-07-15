// SPDX-License-Identifier: LGPL-3.0-or-later
#include "WallIndex.hpp"

#include <gtest/gtest.h>

#include <array>

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

SurfaceMesh two_rooms_with_gap()
{
    SurfaceMesh mesh{};
    add_quad(mesh, {Point3D{0, 0, 0}, {4, 0, 0}, {4, 10, 0}, {0, 10, 0}});
    add_quad(mesh, {Point3D{6, 0, 0}, {10, 0, 0}, {10, 10, 0}, {6, 10, 0}});
    return mesh;
}

/// A bridge deck spanning x in [4,6], y in [0,10] at the given height.
SurfaceMesh bridge_at(double z)
{
    SurfaceMesh mesh{};
    add_quad(mesh, {Point3D{4, 0, z}, {6, 0, z}, {6, 10, z}, {4, 10, z}});
    return mesh;
}

/// A ramp climbing from z=0 at y=0 to z=4 at y=10; its left side edge
/// (5,0,0)-(5,10,4) is the climbing border the tests probe.
SurfaceMesh ramp()
{
    SurfaceMesh mesh{};
    add_quad(mesh, {Point3D{5, 0, 0}, {15, 0, 0}, {15, 10, 4}, {5, 10, 4}});
    return mesh;
}
} // namespace

TEST(WallIndex, FlatRoomHasItsFourWalls)
{
    const WallIndex index{flat_room()};
    EXPECT_EQ(index.segments().size(), 4);
}

TEST(WallIndex, SegmentsNearReturnsOnlyWallsInRadius)
{
    const WallIndex index{flat_room()};
    EXPECT_EQ(index.segments_near({1, 5, 0}, 2.0, 2.2).size(), 1);
    EXPECT_TRUE(index.segments_near({5, 5, 0}, 2.0, 2.2).empty()); // center
    EXPECT_EQ(index.segments_near({5, 5, 0}, 6.0, 2.2).size(), 4); // large radius
}

TEST(WallIndex, WallBetweenRoomsBreaksVisibility)
{
    const WallIndex index{two_rooms_with_gap()};
    EXPECT_TRUE(index.is_visible({1, 5, 0}, {3, 5, 0}, 2.2));
    EXPECT_FALSE(index.is_visible({2, 5, 0}, {8, 5, 0}, 2.2));
}

TEST(WallIndex, BridgeAboveDoesNotOccludeGroundPath)
{
    const WallIndex index{bridge_at(3.0)};
    // The path passes underneath: cross it in 2D but height is 3m.
    EXPECT_TRUE(index.is_visible({3, 5, 0}, {7, 5, 0}, 2.2));
    // The same path on bridge level is blocked by the borders.
    EXPECT_FALSE(index.is_visible({3, 5, 3}, {7, 5, 3}, 2.2));
}

TEST(WallIndex, PathCollinearlyUnderBridgeBorderStaysVisible)
{
    const WallIndex index{bridge_at(3.0)};
    // The path runs exactly underneath the x=4 border edge: its 2D projection
    // overlaps it in a whole segment (the collinear intersection case), 3 m below.
    EXPECT_TRUE(index.is_visible({4, 2, 0}, {4, 8, 0}, 2.2));
}

TEST(WallIndex, BridgeAboveIsNoWallForGroundAgents)
{
    // Similar to "BridgeAboveDoesNotOccludeGroundPath", but not for is_visibile()
    // but for segments_near()
    const WallIndex index{bridge_at(3.0)};
    EXPECT_TRUE(index.segments_near({5, 5, 0}, 3.0, 2.2).empty());
    EXPECT_EQ(index.segments_near({5, 5, 3}, 3.0, 2.2).size(), 2);
}

TEST(WallIndex, VisibilityUsesInterpolatedEdgeHeight)
{
    const WallIndex index{ramp()};
    // Ramp blocked
    EXPECT_FALSE(index.is_visible({4, 2, 0}, {6, 2, 0}, 2.2));
    // But underneath its high end (~3.6 m) the crossing is fine.
    EXPECT_TRUE(index.is_visible({4, 9, 0}, {6, 9, 0}, 2.2));
}

TEST(WallIndex, SegmentsNearUsesInterpolatedFootpointHeight)
{
    const WallIndex index{ramp()};
    // Low end of the ramp is treated as a wall.
    EXPECT_EQ(index.segments_near({4, 2, 0}, 1.2, 2.2).size(), 1);
    // Near its high end it is no wall anymore.
    EXPECT_TRUE(index.segments_near({4, 9, 0}, 1.2, 2.2).empty());
}
