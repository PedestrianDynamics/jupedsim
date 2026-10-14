// SPDX-License-Identifier: LGPL-3.0-or-later
#include "geometry/area_piece.hpp"
#include "geometry/geometry.hpp"
#include "geometry/walkable_surface.hpp"
#include "geometry_fixtures.hpp"
#include "line_segment.hpp"
#include "polygon.hpp"
#include "simulation_error.hpp"
#include "test_common.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <set>
#include <vector>

namespace
{
/// True iff some segment of @p answer lies on @p wall.
///
/// An answer is made of the visible stretches of a wall, clipped to the query radius, so a
/// wall is named by where it runs and not by a segment to compare against.
bool sees_part_of(const std::vector<LineSegment>& answer, const LineSegment& wall)
{
    const Point along = wall.p2 - wall.p1;
    const auto on_wall = [&](Point p) {
        const Point offset = p - wall.p1;
        if(std::abs(along.cross_product(offset)) > 1e-9) {
            return false;
        }
        const double t = offset.scalar_product(along) / along.scalar_product(along);
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

double total_area(const std::vector<AreaPiece>& pieces)
{
    double area = 0.0;
    for(const auto& piece : pieces) {
        area += piece.polygon.area();
    }
    return area;
}

} // namespace

TEST(GeometryLocate, FlatRegionYieldsGroundHeight)
{
    const auto room = test_geometries::rectangle({0, 0}, {10, 10});
    ASSERT_EQ(room.geometry->region_count(), 1);

    const auto loc = room.geometry->locate_in_region(room.room_id, {5, 5});
    ASSERT_NE(loc.face, SurfaceMesh::null_face());
    EXPECT_NEAR(loc.point.z(), 0.0, 1e-9);
    EXPECT_NEAR(loc.point.x(), 5.0, 1e-9);
    EXPECT_NEAR(loc.point.y(), 5.0, 1e-9);
}

TEST(GeometryLocate, PointOutsideRegionFootprintMisses)
{
    const auto floors = test_geometries::stacked_floors({0, 0}, {10, 10}, 3.0);
    EXPECT_EQ(
        floors.geometry->locate_in_region(floors.room_1, {20, 20}).face, SurfaceMesh::null_face());
}

TEST(GeometryLocate, RampInterpolatesHeightOnFace)
{
    const auto ramp = test_geometries::ramp({5, 0}, {15, 10}, 4.0);
    const auto& geo = ramp.geometry;

    // by construction: z == 0.4*y
    EXPECT_NEAR(geo->locate_in_region(ramp.connector, {10, 2}).point.z(), 0.8, 1e-9);
    EXPECT_NEAR(geo->locate_in_region(ramp.connector, {10, 9}).point.z(), 3.6, 1e-9);
}

TEST(GeometryLocate, RegionIdDisambiguatesStackedFloors)
{
    const auto rooms = test_geometries::stacked_floors({0, 0}, {10, 10}, 3.0);
    ASSERT_EQ(rooms.geometry->region_count(), 3); // both floors and the stair joining them

    // Same (x,y), two sheets: the region id picks which one.
    EXPECT_NEAR(rooms.geometry->locate_in_region(rooms.room_1, {5, 5}).point.z(), 0.0, 1e-9);
    EXPECT_NEAR(rooms.geometry->locate_in_region(rooms.room_2, {5, 5}).point.z(), 3.0, 1e-9);
}

TEST(GeometryGetLocation, OffTheSurfaceThrows)
{
    const auto room = test_geometries::rectangle_with_hole({0, 0}, {10, 10}, {4, 4}, {6, 6});
    EXPECT_THROW(room.geometry->get_location(20, 20, room.room_id), SimulationError);
    EXPECT_THROW(room.geometry->get_location(5, 5, room.room_id), SimulationError);
}

TEST(GeometryGetLocation, RegionIdPicksAmongFloorsOnTopOfEachOther)
{
    // Over (5, 6) the upper floor lies above the ground floor.
    const auto stair = test_geometries::switchback_stair();
    const auto loc = stair.geometry->get_location(5, 6, stair.room_2);
    EXPECT_EQ(loc.region(), stair.room_2);
    EXPECT_NEAR(loc.z(), 3.0, 1e-9);
}

TEST(GeometryGetLocation, PointOffTheGivenRegionThrows)
{
    const auto stair = test_geometries::straight_stair_to_a_landing();
    EXPECT_THROW(stair.geometry->get_location(5, 4, stair.room_2), SimulationError);
}

TEST(GeometryGetLocation, UnknownRegionIdThrows)
{
    const auto stair = test_geometries::straight_stair_to_a_landing();
    ASSERT_EQ(stair.geometry->region_count(), 3u);
    EXPECT_THROW(stair.geometry->get_location(5, 4, 3), SimulationError);
}

TEST(GeometryGetLocation, NearZOnASeamTakesTheLowestRegionId)
{
    // Seams at the foot (x = 10) and the head (x = 15) of the flight.
    const auto stair = test_geometries::straight_stair_to_a_landing();

    const auto foot = stair.geometry->get_location_near_z(10, 4, 0.0);
    ASSERT_TRUE(foot.has_value());
    EXPECT_EQ(foot->region(), std::min(stair.room_1, stair.connector));
    EXPECT_NEAR(foot->z(), 0.0, 1e-9);

    const auto head = stair.geometry->get_location_near_z(15, 4, 3.0);
    ASSERT_TRUE(head.has_value());
    EXPECT_EQ(head->region(), std::min(stair.connector, stair.room_2));
    EXPECT_NEAR(head->z(), 3.0, 1e-9);
}

TEST(GeometryGetLocation, OnASeamEitherRegionIdIsAccepted)
{
    // The foot of the flight, x = 10, lies on both the ground floor and the flight.
    const auto stair = test_geometries::straight_stair_to_a_landing();
    EXPECT_EQ(stair.geometry->get_location(10, 4, stair.room_1).region(), stair.room_1);
    EXPECT_EQ(stair.geometry->get_location(10, 4, stair.connector).region(), stair.connector);
}

TEST(GeometryFromPolygon, HoleIsNotWalkable)
{
    const auto room = test_geometries::rectangle_with_hole({0, 0}, {10, 10}, {4, 4}, {6, 6});
    ASSERT_EQ(room.geometry->region_count(), 1);
    EXPECT_TRUE(room.geometry->is_valid_location({1, 1, 1}));
    EXPECT_FALSE(room.geometry->is_valid_location({5, 5, 1})); // inside the hole
    EXPECT_FALSE(room.geometry->is_valid_location({20, 20, 1})); // outside geometry
    // All lifted vertices sit at z=0.
    for(const auto& v : room.geometry->vertices()) {
        EXPECT_EQ(v[2], 0.0);
    }
}

TEST(GeometryFromPolygon, KeepsThePolygonItWasLiftedFrom)
{
    const auto room = test_geometries::rectangle_with_hole({0, 0}, {10, 10}, {4, 4}, {6, 6});
    const auto poly = room.geometry->polygon(room.room_id);
    EXPECT_EQ(poly.outer_boundary().size(), 4u);
    EXPECT_EQ(poly.holes().size(), 1u);
}

TEST(GeometryFromPolygon, RegionIdOutOfRangeThrows)
{
    const auto room = test_geometries::rectangle_with_hole({0, 0}, {10, 10}, {4, 4}, {6, 6});
    EXPECT_THROW(room.geometry->polygon(1), SimulationError);
}

TEST(GeometryModelQueries, EverythingAnsweredIsWithinTheRadius)
{
    const auto room = test_geometries::rectangle_with_hole({0, 0}, {10, 10}, {4, 4}, {6, 6});
    const auto who = room.geometry->get_location_near_z(2, 5, 0.0);
    ASSERT_TRUE(who.has_value());

    const auto walls = room.geometry->line_segments_in_range(*who, 5.0);
    ASSERT_FALSE(walls.empty());
    for(const auto& wall : walls) {
        EXPECT_LE(wall.dist_to(who->xy()), 5.0 + 1e-9);
    }
    // The hole's near side and the room's west wall are both 2 m away and in plain sight.
    EXPECT_TRUE(sees_part_of(walls, LineSegment{{4, 4}, {4, 6}}));
    EXPECT_TRUE(sees_part_of(walls, LineSegment{{0, 0}, {0, 10}}));
    // The hole's far side is 4 m away and stands behind its near side.
    EXPECT_FALSE(sees_part_of(walls, LineSegment{{6, 4}, {6, 6}}));
}

TEST(GeometryModelQueries, NoGeometryBetweenMatchesFlatView)
{
    const auto room = test_geometries::rectangle_with_hole({0, 0}, {10, 10}, {4, 4}, {6, 6});
    const auto left = room.geometry->get_location_near_z(2, 5, 0.0);
    const auto right = room.geometry->get_location_near_z(8, 5, 0.0);
    const auto below = room.geometry->get_location_near_z(2, 2, 0.0);
    ASSERT_TRUE(left.has_value() && right.has_value() && below.has_value());

    // Straight line 2,5 -> 8,5 runs through the central hole: blocked.
    EXPECT_FALSE(room.geometry->no_geometry_between(*left, right->xy() - left->xy()));
    // 2,5 -> 2,2 stays clear of the hole: visible.
    EXPECT_TRUE(room.geometry->no_geometry_between(*left, below->xy() - left->xy()));
}

TEST(GeometryModelQueries, AStepIsJudgedByTheWayThereNotByWhereItLands)
{
    const auto room = test_geometries::rectangle_with_hole({0, 0}, {10, 10}, {4, 4}, {6, 6});
    const auto who = room.geometry->get_location_near_z(2, 5, 0.0);
    ASSERT_TRUE(who.has_value());

    const Point to_free{1, 1}; // free
    const Point to_hole{5, 5}; // inside the central hole
    const Point to_outside{20, 20}; // outside geometry
    const Point beyond_hole{8, 5}; // free again, but only reachable around the hole
    EXPECT_TRUE(room.geometry->no_geometry_between(*who, to_free - who->xy()));
    EXPECT_FALSE(room.geometry->no_geometry_between(*who, to_hole - who->xy()));
    EXPECT_FALSE(room.geometry->no_geometry_between(*who, to_outside - who->xy()));
    EXPECT_FALSE(room.geometry->no_geometry_between(*who, beyond_hole - who->xy()));

    // The last one is where the two questions part ways: it lands on walkable ground, and
    // there is still no straight step to it.
    EXPECT_TRUE(room.geometry->is_valid_location({beyond_hole.x, beyond_hole.y, 0.0}));
}

TEST(GeometryVisibility, TheFloorAboveIsNotInSightThoughNothingStandsBetween)
{
    const auto floors = test_geometries::stacked_floors({0, 0}, {10, 10}, 3.0);
    const auto below = floors.geometry->get_location_near_z(2, 5, 0.0);
    const auto beside = floors.geometry->get_location_near_z(8, 5, 0.0);
    const auto above = floors.geometry->get_location_near_z(8, 5, 3.0);
    ASSERT_TRUE(below.has_value() && beside.has_value() && above.has_value());

    // The two floors are joined only outside their footprint, so the chord crosses no seam and
    // meets no wall either way. What separates them is which sheet each end is on, and nothing
    // else.
    EXPECT_TRUE(floors.geometry->no_geometry_between(*below, *beside));
    EXPECT_FALSE(floors.geometry->no_geometry_between(*below, *above));
}

TEST(GeometryVisibility, CrossingASeamOntoWalkableSurfaceDoesNotBlock)
{
    const auto floors = test_geometries::switchback_stair();
    const auto who = floors.geometry->get_location_near_z(12.0, 2.0, 1.5, 0.5);
    ASSERT_TRUE(who.has_value());
    EXPECT_TRUE(floors.geometry->no_geometry_between(*who, {4.0, 0.0}));
    // And someone standing where that walk comes out is in sight, even though the query
    // started in a different region than the one they are in.
    const auto beyond = floors.geometry->get_location_near_z(16.0, 2.0, 3.0, 0.5);
    ASSERT_TRUE(beyond.has_value());
    ASSERT_NE(who->region(), beyond->region());
    EXPECT_TRUE(floors.geometry->no_geometry_between(*who, *beyond));
}

TEST(GeometryVisibility, LeavingTheSurfaceOverASeamBlocksTheView)
{
    const auto stair = test_geometries::u_stair();

    // On the first flight, two metres short of the landing at x = 13.
    const auto who = stair.geometry->get_location_near_z(11.0, 7.5, 0.9, 0.5);
    ASSERT_TRUE(who.has_value());

    // Back down the flight: same region, no seam, and nothing in the way.
    EXPECT_TRUE(stair.geometry->no_geometry_between(*who, {-2.0, 0.0}));

    // On over the seam and onto the landing: still on the surface.
    EXPECT_TRUE(stair.geometry->no_geometry_between(*who, {2.5, 0.0}));

    // On over the landing and off its far edge at x = 14. The chord crosses the seam onto the
    // landing, so the walk decides -- and it runs off the end, although in plan the ground
    // floor lies below and the upper floor above where it comes out.
    EXPECT_FALSE(stair.geometry->no_geometry_between(*who, {4.0, 0.0}));
}

TEST(GeometryModelQueries, ASeamBringsTheNextRegionsWallsIntoTheAnswer)
{
    const auto floors = test_geometries::switchback_stair();

    // On the flight, a little short of x = 14 where it meets the landing and the region
    // changes. The wall along y = 0 runs straight on past that boundary, where it is the
    // next region's -- and an answer that stopped at the boundary would leave the agent
    // with half the wall he is walking along.
    const auto who = floors.geometry->get_location_near_z(11.5, 1.0, 1.5, 0.5);
    ASSERT_TRUE(who.has_value());

    const auto walls = floors.geometry->line_segments_in_range(*who, 4.0);
    const bool reaches_across = std::any_of(walls.begin(), walls.end(), [](const LineSegment& w) {
        return w.p1.y == 0.0 && w.p2.y == 0.0 && std::max(w.p1.x, w.p2.x) > 14.0;
    });
    EXPECT_TRUE(reaches_across) << "the answer stopped at the region boundary";
}

TEST(GeometryModelQueries, AWallBorderingTwoRegionsIsDeliveredOnce)
{
    const auto floors = test_geometries::switchback_stair();

    // The wall along y = 8 runs across landing and upper floor, and the agent stands where
    // both are in sight. Nothing may be answered twice, however many ways lead to it.
    const auto who = floors.geometry->get_location_near_z(13.0, 7.0, 3.0, 0.5);
    ASSERT_TRUE(who.has_value());

    const auto delivered = floors.geometry->line_segments_in_range(*who, 100.0);
    const std::set<LineSegment> distinct{delivered.begin(), delivered.end()};
    EXPECT_EQ(delivered.size(), distinct.size()) << "the same wall came back more than once";
}

TEST(GeometryModelQueries, AWallOfTheStoreyAboveIsNotInTheAnswer)
{
    const auto floors = test_geometries::switchback_stair();

    // The far side at y = 8 carries two walls, one above the other: the ground floor's, which
    // ends at x = 10, and the upper floor's, which runs on to the landing at x = 18. In plan
    // they fall on the same line, so what tells them apart is how many answers come back from
    // it -- the one the agent stands under, and not the one three metres over his head.
    const auto who = floors.geometry->get_location_near_z(5.0, 6.0, 0.0, 0.5);
    ASSERT_TRUE(who.has_value());

    const auto walls = floors.geometry->line_segments_in_range(*who, 2.2);
    EXPECT_TRUE(sees_part_of(walls, LineSegment{{0, 8}, {10, 8}})) << "its own wall, 2 m away";
    EXPECT_EQ(pieces_along(walls, 8.0), 1) << "the upper floor's wall answered as well";
}

TEST(GeometryModelQueries, TheFlightAboveIsNotAWall)
{
    const auto floors = test_geometries::stair_turning_on_a_landing();

    // Standing at the foot of the first flight, with the stairwell beside him: its near wall at
    // y = 2 is his own flight's, its far wall at y = 3 belongs to the flight above and stands
    // 3 m up. The radius takes in the whole stair, so the query looks on through the seam at
    // x = 6 onto the landing. The flight above lies behind a second seam on that same line
    // x = 6, and no straight sight line through the first seam comes back through the second.
    const auto who = floors.geometry->get_location_near_z(1.0, 1.0, 0.5, 0.2);
    ASSERT_TRUE(who.has_value());

    const auto walls = floors.geometry->line_segments_in_range(*who, 10.0);
    const auto side_of_the_well = [&walls](double y) {
        return std::any_of(walls.begin(), walls.end(), [y](const LineSegment& w) {
            return w.p1.y == y && w.p2.y == y;
        });
    };
    EXPECT_TRUE(side_of_the_well(2.0)) << "his own flight's wall, right beside him";
    EXPECT_TRUE(sees_part_of(walls, LineSegment{{6, 0}, {8, 0}}))
        << "the landing's wall, seen through the seam at x = 6";
    EXPECT_FALSE(side_of_the_well(3.0)) << "repelled by a wall belonging to the flight above";
}

TEST(GeometryModelQueries, AWallRisingFromTheAgentsLevelStaysInTheAnswer)
{
    const auto floors = test_geometries::switchback_stair();

    // The wall along y = 0 runs from the ground floor up the flight to the landing, so it
    // starts at the agent's feet and ends 3 m up. Nothing may drop it: how far a wall reaches
    // up is not in the surface, and it is right there next to him.
    const auto who = floors.geometry->get_location_near_z(5.0, 1.0, 0.0, 0.5);
    ASSERT_TRUE(who.has_value());

    const auto walls = floors.geometry->line_segments_in_range(*who, 2.2);
    EXPECT_TRUE(sees_part_of(walls, LineSegment{{0, 0}, {18, 0}}));
}

TEST(GeometryModelQueries, AWallOfTheStoreyBelowIsNotInTheAnswer)
{
    const auto floors = test_geometries::switchback_stair();

    // Upstairs, over the ground floor. Its east wall (10,4)-(10,8) stands three metres below and
    // comes within reach over the seam -- and it is what the upper floor's own floor rests on, so
    // it cannot reach up here.
    const auto who = floors.geometry->get_location_near_z(10.5, 6.0, 3.0, 0.5);
    ASSERT_TRUE(who.has_value());

    const auto walls = floors.geometry->line_segments_in_range(*who, 4.0);
    EXPECT_TRUE(sees_part_of(walls, LineSegment{{0, 4}, {14, 4}})) << "its own storey's wall";
    EXPECT_FALSE(sees_part_of(walls, LineSegment{{10, 4}, {10, 8}}))
        << "the ground floor's wall, three metres below";
    // The ground floor's far wall lies on y = 8 just as this storey's own does.
    EXPECT_EQ(pieces_along(walls, 8.0), 1) << "the ground floor's wall answered as well";
}

TEST(GeometryModelQueries, AWallOfTheFlightBelowIsNotInTheAnswerEither)
{
    const auto floors = test_geometries::stair_turning_on_a_landing();

    // At the top of the second flight, six metres up. The floor at the foot of the first flight
    // lies two metres away in plan, its end wall on the same line x = -1 as the top floor's.
    // The radius takes in the whole stair, so the query looks on through the seam at x = 6 onto
    // the landing -- but the way down from there turns back through a second seam on that same
    // line x = 6, and no straight sight line passes both.
    const auto who = floors.geometry->get_location_near_z(0.5, 4.0, 5.75, 0.5);
    ASSERT_TRUE(who.has_value());

    const auto walls = floors.geometry->line_segments_in_range(*who, 10.0);
    EXPECT_TRUE(sees_part_of(walls, LineSegment{{-1, 3}, {-1, 5}}))
        << "the wall right in front of him";
    EXPECT_TRUE(sees_part_of(walls, LineSegment{{6, 5}, {8, 5}}))
        << "the landing's wall, seen through the seam at x = 6";
    EXPECT_FALSE(sees_part_of(walls, LineSegment{{-1, 0}, {-1, 2}}))
        << "the foot of the flight below";
}

/*******************************************************************
 *  Test clip_to_region()
 ********************************************************************/

TEST(GeometryClipToRegion, CompletelyInsideSurface)
{
    const auto room = test_geometries::rectangle({0, 0}, {10, 10});
    const Polygon area{test_geometries::rectangle_points({2, 2}, {4, 4})};

    const auto pieces = room.geometry->clip_to_region(area, room.room_id);

    ASSERT_EQ(pieces.size(), 1u);
    EXPECT_EQ(pieces[0].region_id, room.room_id);
    EXPECT_NEAR(total_area(pieces), 4.0, 1e-9);
}

TEST(GeometryClipToRegion, TrimmedAtSurfaceBorder)
{
    const auto room = test_geometries::rectangle({0, 0}, {10, 10});
    const Polygon area{test_geometries::rectangle_points({8, 2}, {12, 4})};

    const auto pieces = room.geometry->clip_to_region(area, room.room_id);

    EXPECT_NEAR(total_area(pieces), 2.0 * 2.0, 1e-9);
}

TEST(GeometryClipToRegion, ClippedAtSeam)
{
    // Seam between room_1 and the connector at x = 10.
    const auto rooms = test_geometries::two_rooms();
    const Polygon area{test_geometries::rectangle_points({9, 2}, {12, 4})};

    const auto pieces = rooms.geometry->clip_to_region(area, rooms.room_1);

    EXPECT_NEAR(total_area(pieces), 1.0 * 2.0, 1e-9);
}

TEST(GeometryClipToRegion, ConcaveCreatesSeveralPieces)
{
    // A U open at the top. The area runs across both arms and the gap between them.
    const auto room = test_geometries::from_polygons(
        {{{0, 0}, {9, 0}, {9, 9}, {6, 9}, {6, 3}, {3, 3}, {3, 9}, {0, 9}}});
    const Polygon area{test_geometries::rectangle_points({0, 6}, {9, 8})};

    const auto pieces = room.geometry->clip_to_region(area, room.room_id);

    EXPECT_EQ(pieces.size(), 2u);
    EXPECT_NEAR(total_area(pieces), 2.0 * (3.0 * 2.0), 1e-9);
}

TEST(GeometryClipToRegion, RejectedOutsideBoundary)
{
    const auto room = test_geometries::rectangle({0, 0}, {10, 10});
    const Polygon area{test_geometries::rectangle_points({12, 2}, {14, 4})};

    EXPECT_THROW(room.geometry->clip_to_region(area, room.room_id), SimulationError);
}
