// SPDX-License-Identifier: LGPL-3.0-or-later
#include "Geometry/Geometry.hpp"
#include "LineSegment.hpp"
#include "MeshFixtures.hpp"
#include "TestCommon.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <set>
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

/// True iff some segment of @p answer lies on @p wall.
///
/// An answer is made of the visible stretches of a wall, clipped to the query radius, so a
/// wall is named by where it runs and not by a segment to compare against.
bool sees_part_of(const std::vector<LineSegment>& answer, const LineSegment& wall)
{
    const Point along = wall.p2 - wall.p1;
    const auto on_wall = [&](Point p) {
        const Point offset = p - wall.p1;
        if(std::abs(along.CrossProduct(offset)) > 1e-9) {
            return false;
        }
        const double t = offset.ScalarProduct(along) / along.ScalarProduct(along);
        return t >= -1e-9 && t <= 1.0 + 1e-9;
    };
    return std::any_of(answer.begin(), answer.end(), [&](const LineSegment& piece) {
        return on_wall(piece.p1) && on_wall(piece.p2);
    });
}

/// How many answered pieces run along the horizontal line at @p y. Where two storeys carry a
/// wall on the same line in plan, counting is what tells one storey's answer from two.
std::ptrdiff_t pieces_along(const std::vector<LineSegment>& answer, double y)
{
    return std::count_if(answer.begin(), answer.end(), [y](const LineSegment& piece) {
        return piece.p1.y == y && piece.p2.y == y;
    });
}

} // namespace

TEST(GeometryLocate, FlatRegionYieldsGroundHeight)
{
    Geometry geo{flat_room()};
    ASSERT_EQ(geo.region_count(), 1);

    const auto loc = geo.locate_in_region(0, {5, 5});
    ASSERT_NE(loc.face, SurfaceMesh::null_face());
    EXPECT_NEAR(loc.point.z(), 0.0, 1e-9);
    EXPECT_NEAR(loc.point.x(), 5.0, 1e-9);
    EXPECT_NEAR(loc.point.y(), 5.0, 1e-9);
}

TEST(GeometryLocate, PointOutsideRegionFootprintMisses)
{
    Geometry geo{flat_room()};
    EXPECT_EQ(geo.locate_in_region(0, {20, 20}).face, SurfaceMesh::null_face());
}

TEST(GeometryLocate, RampInterpolatesHeightOnFace)
{
    Geometry geo{ramp()};
    ASSERT_EQ(geo.region_count(), 1);

    // by construction: z == 0.4*y
    EXPECT_NEAR(geo.locate_in_region(0, {10, 2}).point.z(), 0.8, 1e-9);
    EXPECT_NEAR(geo.locate_in_region(0, {10, 9}).point.z(), 3.6, 1e-9);
}

TEST(GeometryLocate, RegionIdDisambiguatesStackedFloors)
{
    Geometry geo{stacked_floors()};
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

TEST(GeometryFromPolygon, HoleIsNotWalkable)
{
    Geometry geo{square_with_hole()};

    ASSERT_EQ(geo.region_count(), 1);
    EXPECT_TRUE(geo.is_valid_location({1, 1, 1}));
    EXPECT_FALSE(geo.is_valid_location({5, 5, 1})); // inside the hole
    EXPECT_FALSE(geo.is_valid_location({20, 20, 1})); // outside geometry
    // All lifted vertices sit at z=0.
    for(const auto& v : geo.vertices()) {
        EXPECT_EQ(v[2], 0.0);
    }
}

TEST(GeometryFromPolygon, KeepsThePolygonItWasLiftedFrom)
{
    Geometry geo{square_with_hole()};

    const auto* poly = geo.polygon();
    ASSERT_NE(poly, nullptr);
    EXPECT_EQ(poly->outer_boundary().size(), 4u);
    EXPECT_EQ(poly->holes().size(), 1u);
}

TEST(GeometryFromMesh, HasNoPolygon)
{
    Geometry geo{flat_room()};
    // A surface that may fold over itself has no polygon underneath.
    EXPECT_EQ(geo.polygon(), nullptr);
}

TEST(GeometryModelQueries, EverythingAnsweredIsWithinTheRadius)
{
    Geometry geo{square_with_hole()};
    const auto who = geo.get_location(2, 5, 0.0);
    ASSERT_TRUE(who.has_value());

    const auto walls = geo.line_segments_in_range(*who, 5.0);
    ASSERT_FALSE(walls.empty());
    for(const auto& wall : walls) {
        EXPECT_LE(wall.DistTo(who->xy()), 5.0 + 1e-9);
    }
    // The hole's near side and the room's west wall are both 2 m away and in plain sight.
    EXPECT_TRUE(sees_part_of(walls, LineSegment{{4, 4}, {4, 6}}));
    EXPECT_TRUE(sees_part_of(walls, LineSegment{{0, 0}, {0, 10}}));
    // The hole's far side is 4 m away and stands behind its near side.
    EXPECT_FALSE(sees_part_of(walls, LineSegment{{6, 4}, {6, 6}}));
}

TEST(GeometryModelQueries, NoGeometryBetweenMatchesFlatView)
{
    Geometry geo{square_with_hole()};
    const auto left = geo.get_location(2, 5, 0.0);
    const auto right = geo.get_location(8, 5, 0.0);
    const auto below = geo.get_location(2, 2, 0.0);
    ASSERT_TRUE(left.has_value() && right.has_value() && below.has_value());

    // Straight line 2,5 -> 8,5 runs through the central hole: blocked.
    EXPECT_FALSE(geo.no_geometry_between(*left, right->xy() - left->xy()));
    // 2,5 -> 2,2 stays clear of the hole: visible.
    EXPECT_TRUE(geo.no_geometry_between(*left, below->xy() - left->xy()));
}

TEST(GeometryModelQueries, AStepIsJudgedByTheWayThereNotByWhereItLands)
{
    Geometry geo{square_with_hole()};
    const auto who = geo.get_location(2, 5, 0.0);
    ASSERT_TRUE(who.has_value());

    const Point to_free{1, 1}; // free
    const Point to_hole{5, 5}; // inside the central hole
    const Point to_outside{20, 20}; // outside geometry
    const Point beyond_hole{8, 5}; // free again, but only reachable around the hole
    EXPECT_TRUE(geo.no_geometry_between(*who, to_free - who->xy()));
    EXPECT_FALSE(geo.no_geometry_between(*who, to_hole - who->xy()));
    EXPECT_FALSE(geo.no_geometry_between(*who, to_outside - who->xy()));
    EXPECT_FALSE(geo.no_geometry_between(*who, beyond_hole - who->xy()));

    // The last one is where the two questions part ways: it lands on walkable ground, and
    // there is still no straight step to it.
    EXPECT_TRUE(geo.is_valid_location({beyond_hole.x, beyond_hole.y, 0.0}));
}

TEST(GeometryVisibility, WithinOneRegionTheFlatTestAnswers)
{
    Geometry geo{fixtures::wavy_terrain()};
    const auto who = geo.get_location(10.0, 5.0, 0.0, 2.0);
    ASSERT_TRUE(who.has_value());

    // A field has no seams, so every chord stays on the one sheet and the flat test decides.
    EXPECT_TRUE(geo.no_geometry_between(*who, {4.0, 0.0}));
    // Out past the edge of the field, across its boundary.
    EXPECT_FALSE(geo.no_geometry_between(*who, {40.0, 0.0}));
}

TEST(GeometryVisibility, TheFloorAboveIsNotInSightThoughNothingStandsBetween)
{
    Geometry geo{stacked_floors()};
    const auto below = geo.get_location(2, 5, 0.0);
    const auto beside = geo.get_location(8, 5, 0.0);
    const auto above = geo.get_location(8, 5, 3.0);
    ASSERT_TRUE(below.has_value() && beside.has_value() && above.has_value());

    // The two floors are not joined, so the chord crosses no seam and meets no wall either
    // way. What separates them is which sheet each end is on, and nothing else.
    EXPECT_TRUE(geo.no_geometry_between(*below, *beside));
    EXPECT_FALSE(geo.no_geometry_between(*below, *above));
}

TEST(GeometryVisibility, LeavingTheSurfaceOverASeamBlocksTheView)
{
    Geometry geo{fixtures::two_levels_with_stair()};

    // Standing on the stair, two metres short of its head at x = 15.
    const auto who = geo.get_location(13.0, 2.0, 1.8, 0.5);
    ASSERT_TRUE(who.has_value());

    // Back down the stair: same region, no seam, and nothing in the way.
    EXPECT_TRUE(geo.no_geometry_between(*who, {-2.0, 0.0}));

    // On past the head, where the surface folds back over itself. The chord crosses the
    // seam, so the walk decides - and it runs off the end.
    EXPECT_FALSE(geo.no_geometry_between(*who, {4.0, 0.0}));
}

TEST(GeometryVisibility, CrossingASeamOntoWalkableSurfaceDoesNotBlock)
{
    Geometry geo{fixtures::switchback_stair()};

    // Flight and landing meet at x = 14, which is also where the region overlay cuts (the
    // coplanar landing + upper floor fuse into one region). Walking across the seam stays
    // on the surface all the way.
    const auto who = geo.get_location(12.0, 2.0, 1.5, 0.5);
    ASSERT_TRUE(who.has_value());
    EXPECT_TRUE(geo.no_geometry_between(*who, {4.0, 0.0}));

    // And someone standing where that walk comes out is in sight, even though the query
    // started in a different region than the one they are in.
    const auto beyond = geo.get_location(16.0, 2.0, 3.0, 0.5);
    ASSERT_TRUE(beyond.has_value());
    ASSERT_NE(who->region(), beyond->region());
    EXPECT_TRUE(geo.no_geometry_between(*who, *beyond));
}

TEST(GeometryModelQueries, AMeshAnswersWithItsOwnRegionsWalls)
{
    Geometry geo{fixtures::wavy_terrain()};
    const auto who = geo.get_location(10.0, 5.0, 0.0, 2.0);
    ASSERT_TRUE(who.has_value());

    // One region and nothing in the way, so all four sides of the field answer. Its border
    // is meshed on a 2 m grid, so each side comes back in pieces.
    const auto walls = geo.line_segments_in_range(*who, 100.0);
    EXPECT_TRUE(sees_part_of(walls, LineSegment{{0, 0}, {20, 0}}));
    EXPECT_TRUE(sees_part_of(walls, LineSegment{{0, 10}, {20, 10}}));
    EXPECT_TRUE(sees_part_of(walls, LineSegment{{0, 0}, {0, 10}}));
    EXPECT_TRUE(sees_part_of(walls, LineSegment{{20, 0}, {20, 10}}));
}

TEST(GeometryModelQueries, ASeamBringsTheNextRegionsWallsIntoTheAnswer)
{
    Geometry geo{fixtures::switchback_stair()};

    // On the flight, a little short of x = 14 where it meets the landing and the region
    // changes. The wall along y = 0 runs straight on past that boundary, where it is the
    // next region's -- and an answer that stopped at the boundary would leave the agent
    // with half the wall he is walking along.
    const auto who = geo.get_location(11.5, 1.0, 1.5, 0.5);
    ASSERT_TRUE(who.has_value());

    const auto walls = geo.line_segments_in_range(*who, 4.0);
    const bool reaches_across = std::any_of(walls.begin(), walls.end(), [](const LineSegment& w) {
        return w.p1.y == 0.0 && w.p2.y == 0.0 && std::max(w.p1.x, w.p2.x) > 14.0;
    });
    EXPECT_TRUE(reaches_across) << "the answer stopped at the region boundary";
}

TEST(GeometryModelQueries, AWallBorderingTwoRegionsIsDeliveredOnce)
{
    Geometry geo{fixtures::switchback_stair()};

    // The wall along y = 8 runs across landing and upper floor, and the agent stands where
    // both are in sight. Nothing may be answered twice, however many ways lead to it.
    const auto who = geo.get_location(13.0, 7.0, 3.0, 0.5);
    ASSERT_TRUE(who.has_value());

    const auto delivered = geo.line_segments_in_range(*who, 100.0);
    const std::set<LineSegment> distinct{delivered.begin(), delivered.end()};
    EXPECT_EQ(delivered.size(), distinct.size()) << "the same wall came back more than once";
}

TEST(GeometryModelQueries, AWallOfTheStoreyAboveIsNotInTheAnswer)
{
    Geometry geo{fixtures::switchback_stair()};

    // The far side at y = 8 carries two walls, one above the other: the ground floor's, which
    // ends at x = 10, and the upper floor's, which runs on to the landing at x = 18. In plan
    // they fall on the same line, so what tells them apart is how many answers come back from
    // it -- the one the agent stands under, and not the one three metres over his head.
    const auto who = geo.get_location(5.0, 6.0, 0.0, 0.5);
    ASSERT_TRUE(who.has_value());

    const auto walls = geo.line_segments_in_range(*who, 2.2);
    EXPECT_TRUE(sees_part_of(walls, LineSegment{{0, 8}, {10, 8}})) << "its own wall, 2 m away";
    EXPECT_EQ(pieces_along(walls, 8.0), 1) << "the upper floor's wall answered as well";
}

TEST(GeometryModelQueries, TheFlightAboveIsNotAWallEvenInTheSameRegion)
{
    Geometry geo{fixtures::stair_turning_on_a_landing()};
    ASSERT_EQ(geo.region_count(), 1u);

    // Standing at the foot of the first flight, with the stair well beside him: its near wall
    // at y = 2 is his own flight's and 1 m away, its far wall at y = 3 belongs to the flight
    // above and stands 3 m up, 2 m away in plan. What keeps the far one out is the near one:
    // to reach across the well a sight line has to pass through the wall along its near side.
    const auto who = geo.get_location(1.0, 1.0, 0.5, 0.2);
    ASSERT_TRUE(who.has_value());

    const auto walls = geo.line_segments_in_range(*who, 2.2);
    const auto side_of_the_well = [&walls](double y) {
        return std::any_of(walls.begin(), walls.end(), [y](const LineSegment& w) {
            return w.p1.y == y && w.p2.y == y;
        });
    };
    EXPECT_TRUE(side_of_the_well(2.0)) << "his own flight's wall, right beside him";
    EXPECT_FALSE(side_of_the_well(3.0)) << "repelled by a wall belonging to the flight above";
}

TEST(GeometryModelQueries, AWallRisingFromTheAgentsLevelStaysInTheAnswer)
{
    Geometry geo{fixtures::switchback_stair()};

    // The wall along y = 0 runs from the ground floor up the flight to the landing, so it
    // starts at the agent's feet and ends 3 m up. Nothing may drop it: how far a wall reaches
    // up is not in the surface, and it is right there next to him.
    const auto who = geo.get_location(5.0, 1.0, 0.0, 0.5);
    ASSERT_TRUE(who.has_value());

    const auto walls = geo.line_segments_in_range(*who, 2.2);
    EXPECT_TRUE(sees_part_of(walls, LineSegment{{0, 0}, {18, 0}}));
}

TEST(GeometryModelQueries, AWallOfTheStoreyBelowIsNotInTheAnswer)
{
    Geometry geo{fixtures::switchback_stair()};

    // Upstairs, over the ground floor. Its east wall (10,4)-(10,8) stands three metres below and
    // comes within reach over the seam -- and it is what the upper floor's own floor rests on, so
    // it cannot reach up here.
    const auto who = geo.get_location(10.5, 6.0, 3.0, 0.5);
    ASSERT_TRUE(who.has_value());

    const auto walls = geo.line_segments_in_range(*who, 4.0);
    EXPECT_TRUE(sees_part_of(walls, LineSegment{{0, 4}, {14, 4}})) << "its own storey's wall";
    EXPECT_FALSE(sees_part_of(walls, LineSegment{{10, 4}, {10, 8}}))
        << "the ground floor's wall, three metres below";
    // The ground floor's far wall lies on y = 8 just as this storey's own does.
    EXPECT_EQ(pieces_along(walls, 8.0), 1) << "the ground floor's wall answered as well";
}

TEST(GeometryModelQueries, AWallOfTheFlightBelowIsNotInTheAnswerEither)
{
    Geometry geo{fixtures::stair_turning_on_a_landing()};
    ASSERT_EQ(geo.region_count(), 1u);

    // At the top of the second flight, six metres up. The foot of the first flight lies in the
    // same region, two metres away in plan, and on the same line x = 0 -- and still across the
    // well, so the wall along its far side stands between the two.
    const auto who = geo.get_location(0.5, 4.0, 5.75, 0.5);
    ASSERT_TRUE(who.has_value());

    const auto walls = geo.line_segments_in_range(*who, 4.0);
    EXPECT_TRUE(sees_part_of(walls, LineSegment{{0, 3}, {0, 5}}))
        << "the wall right in front of him";
    EXPECT_FALSE(sees_part_of(walls, LineSegment{{0, 0}, {0, 2}}))
        << "the foot of the flight below";
}
