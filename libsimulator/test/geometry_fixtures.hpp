// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "geometry/geometry.hpp"
#include "geometry/walkable_surface.hpp"
#include "geometry_builder.hpp"
#include "point.hpp"

#include <array>
#include <memory>
#include <vector>

/// Shared geometries across tests. Decouples generation of geometry from usage.
namespace test_geometries
{

/// Convenience function to get all rectangle corner points.
inline std::vector<Point> rectangle_points(Point lower_left, Point upper_right)
{
    return {lower_left, {upper_right.x, lower_left.y}, upper_right, {lower_left.x, upper_right.y}};
}

/// A structure to hold a single region and its geometry.
struct SingleRoom {
    std::unique_ptr<Geometry> geometry;
    std::size_t room_id;
};

/// Flat geometry at z = 0.
inline SingleRoom from_polygons(
    const std::vector<std::vector<Point>>& boundaries,
    const std::vector<std::vector<Point>>& holes = {})
{
    SingleRoom room{};
    GeometryBuilder builder{};
    for(const auto& loop : boundaries) {
        builder.add_accessible_area(loop);
    }
    for(const auto& loop : holes) {
        builder.exclude_from_accessible_area(loop);
    }
    WalkableSurface surface{};
    room.room_id = surface.add_region(builder.build(), 0.0);
    room.geometry = surface.create_geometry();
    return room;
}

/// Flat rectangle at z = 0.
inline SingleRoom rectangle(Point lower_left, Point upper_right)
{
    return from_polygons({rectangle_points(lower_left, upper_right)});
}

/// A structure to hold two connected regions and their geometry.
/// By convention room_1 is the lower region and room_2 is the upper region, if two floors are
/// constructed.
struct TwoConnectedRegions {
    std::unique_ptr<Geometry> geometry;
    std::size_t room_1;
    std::size_t room_2;
    std::size_t connector; // connecting the two rooms
};

/// Two rooms side by side, joined by a connector: seams at x = 10 and x = 11.
/// room_1: x in [0, 10], y in [0, 10], z = 0. room_2: x in [11, 21], y in [0, 10], z = 0.
inline TwoConnectedRegions two_rooms()
{
    TwoConnectedRegions out{};
    WalkableSurface surface{};
    out.room_1 =
        surface.add_region(WalkableSurface::Polygon{rectangle_points({0, 0}, {10, 10}), {}}, 0.0);
    out.room_2 =
        surface.add_region(WalkableSurface::Polygon{rectangle_points({11, 0}, {21, 10}), {}}, 0.0);
    out.connector =
        surface.connect_regions(out.room_1, {{10, 0}, {10, 10}}, out.room_2, {{11, 0}, {11, 10}});
    out.geometry = surface.create_geometry();
    return out;
}

/// Ground floor (room_1, x in [0, 5], z = 0) and upper floor (room_2, x in [10, 15], z = 3) side
/// by side, joined by stairs (connector) rising 3 m over 5 m; y in [0, 5] throughout. Seams at
/// x = 5 and x = 10.
inline TwoConnectedRegions floors_joined_by_stairs()
{
    TwoConnectedRegions out{};
    WalkableSurface surface{};
    out.room_1 =
        surface.add_region(WalkableSurface::Polygon{rectangle_points({0, 0}, {5, 5}), {}}, 0.0);
    out.room_2 =
        surface.add_region(WalkableSurface::Polygon{rectangle_points({10, 0}, {15, 5}), {}}, 3.0);
    out.connector =
        surface.connect_regions(out.room_1, {{5, 0}, {5, 5}}, out.room_2, {{10, 0}, {10, 5}});
    out.geometry = surface.create_geometry();
    return out;
}

/// Two floors over the same 20 x 10 footprint, the ground floor at z = 0 (room_1) and the upper
/// one at z = 3 (room_2), joined by a ramp in the middle (connector). The ramp climbs along
/// x in [6, 14], y in [4, 6], through an opening of that size in both floors: its foot on the
/// ground floor's opening edge at x = 6, its top on the upper floor's at x = 14. Built from
/// regions, so that areas are cut along their footprints.
inline TwoConnectedRegions stacked_floors_with_ramp()
{
    TwoConnectedRegions out{};
    const auto footprint = rectangle_points({0, 0}, {20, 10});
    const auto opening = rectangle_points({6, 4}, {14, 6});
    WalkableSurface surface{};
    out.room_1 = surface.add_region(WalkableSurface::Polygon{footprint, {opening}}, 0.0);
    out.room_2 = surface.add_region(WalkableSurface::Polygon{footprint, {opening}}, 3.0);
    out.connector =
        surface.connect_regions(out.room_1, {{6, 4}, {6, 6}}, out.room_2, {{14, 4}, {14, 6}});
    out.geometry = surface.create_geometry();
    return out;
}

/// Flat rectangle with rectangular hole, z = 0.
inline SingleRoom rectangle_with_hole(
    Point lower_left,
    Point upper_right,
    Point hole_lower_left,
    Point hole_upper_right)
{
    return from_polygons(
        {rectangle_points(lower_left, upper_right)},
        {rectangle_points(hole_lower_left, hole_upper_right)});
}

/// A structure to hold two regions joined by way of a landing, and their geometry.
/// By convention room_1 is the lower region and room_2 is the upper region.
struct TwoConnectedRegionsWithLanding {
    std::unique_ptr<Geometry> geometry;
    std::size_t room_1;
    std::size_t room_2;
    std::size_t landing;
};

/// Ground floor (room_1) and upper floor (room_2) over the same 20 x 20 footprint, joined
/// through a stairwell by a U-stair: one flight up to a landing, one back to the upper floor.
inline TwoConnectedRegionsWithLanding u_stair()
{
    TwoConnectedRegionsWithLanding out{};
    // (8, 9) and (13, 9) split the stairwell sides so each flight has an edge of its own.
    const WalkableSurface::Ring stairwell{{8, 6}, {14, 6}, {14, 12}, {8, 12}, {8, 9}};
    WalkableSurface surface{};
    out.room_1 = surface.add_region({rectangle_points({0, 0}, {20, 20}), {stairwell}}, 0.0);
    out.room_2 = surface.add_region({rectangle_points({0, 0}, {20, 20}), {stairwell}}, 3.0);
    out.landing = surface.add_region({{{13, 6}, {14, 6}, {14, 12}, {13, 12}, {13, 9}}, {}}, 1.5);
    surface.connect_regions(out.room_1, {{8, 6}, {8, 9}}, out.landing, {{13, 6}, {13, 9}});
    surface.connect_regions(out.landing, {{13, 9}, {13, 12}}, out.room_2, {{8, 9}, {8, 12}});
    out.geometry = surface.create_geometry();
    return out;
}

/// A ramp (connector) over the rectangle from "lower_left" to "upper_right", climbing along y
/// from z = 0 to "height". A 1 m deep floor at either end (room_1 below, room_2 on top) gives
/// it something to connect.
inline TwoConnectedRegions ramp(Point lower_left, Point upper_right, double height)
{
    TwoConnectedRegions out{};
    WalkableSurface surface{};
    out.room_1 = surface.add_region(
        {rectangle_points({lower_left.x, lower_left.y - 1}, {upper_right.x, lower_left.y}), {}},
        0.0);
    out.room_2 = surface.add_region(
        {rectangle_points({lower_left.x, upper_right.y}, {upper_right.x, upper_right.y + 1}), {}},
        height);
    out.connector = surface.connect_regions(
        out.room_1,
        {lower_left, {upper_right.x, lower_left.y}},
        out.room_2,
        {{lower_left.x, upper_right.y}, upper_right});
    out.geometry = surface.create_geometry();
    return out;
}

/// A flat floor (room_1, y in [0, 10], z = 0) and a ramp (connector) rising away from it at
/// 45 deg (y in [10, 15], z up to 5), with a 1 m deep floor on top (room_2). The ramp reaches
/// beyond the floor rather than back over it, so the surface bends without folding: a seam at y =
/// 10 to carry a path across.
inline TwoConnectedRegions floor_with_ramp()
{
    TwoConnectedRegions out{};
    WalkableSurface surface{};
    out.room_1 = surface.add_region({rectangle_points({0, 0}, {10, 10}), {}}, 0.0);
    out.room_2 = surface.add_region({rectangle_points({0, 15}, {10, 16}), {}}, 5.0);
    out.connector =
        surface.connect_regions(out.room_1, {{0, 10}, {10, 10}}, out.room_2, {{0, 15}, {10, 15}});
    out.geometry = surface.create_geometry();
    return out;
}

/// Two floors over the same footprint, the ground floor at z = 0 (room_1) and the upper one
/// at "floor_height" (room_2). Over the footprint nothing joins them. Only outside it, east
/// of "upper_right.x", does a 1 m wide stair (connector) climb from the ground floor's east
/// wall to an arm of the upper floor that reaches round to meet it. The surface has to be
/// connected.
inline TwoConnectedRegions stacked_floors(Point lower_left, Point upper_right, double floor_height)
{
    constexpr double width = 1.0;
    const double run = 2.0 * floor_height;
    const double stair_top = upper_right.x + run;
    TwoConnectedRegions out{};
    WalkableSurface surface{};
    // (upper_right.x, lower_left.y + width) splits the east wall so the stair has an edge.
    out.room_1 = surface.add_region(
        {{lower_left,
          {upper_right.x, lower_left.y},
          {upper_right.x, lower_left.y + width},
          upper_right,
          {lower_left.x, upper_right.y}},
         {}},
        0.0);
    // The footprint, an arm east along its north side, and a column back down to the stair's
    // top.
    out.room_2 = surface.add_region(
        {{lower_left,
          {upper_right.x, lower_left.y},
          {upper_right.x, upper_right.y - width},
          {stair_top, upper_right.y - width},
          {stair_top, lower_left.y + width},
          {stair_top, lower_left.y},
          {stair_top + width, lower_left.y},
          {stair_top + width, upper_right.y},
          {lower_left.x, upper_right.y}},
         {}},
        floor_height);
    out.connector = surface.connect_regions(
        out.room_1,
        {{upper_right.x, lower_left.y}, {upper_right.x, lower_left.y + width}},
        out.room_2,
        {{stair_top, lower_left.y}, {stair_top, lower_left.y + width}});
    out.geometry = surface.create_geometry();
    return out;
}

/// Ground floor (room_1, z = 0, x in [0, 10]), a flight climbing 3 m over 4 m away from it
/// along y in [0, 4], a landing at x in [14, 18], and the upper floor turning back over the
/// ground floor's far half, y in [4, 8]. Landing and upper floor are one L-shaped region
/// (room_2, z = 3); the flight joins them (connector).
inline TwoConnectedRegions switchback_stair()
{
    TwoConnectedRegions out{};
    WalkableSurface surface{};
    // (10, 4) splits the ground floor's east wall so the flight has an edge of its own.
    out.room_1 = surface.add_region({{{0, 0}, {10, 0}, {10, 4}, {10, 8}, {0, 8}}, {}}, 0.0);
    out.room_2 =
        surface.add_region({{{0, 4}, {14, 4}, {14, 0}, {18, 0}, {18, 8}, {0, 8}}, {}}, 3.0);
    out.connector =
        surface.connect_regions(out.room_1, {{10, 0}, {10, 4}}, out.room_2, {{14, 0}, {14, 4}});
    out.geometry = surface.create_geometry();
    return out;
}

/// Ground floor (room_1, x in [0, 10], z = 0), a flight (connector) climbing 3 m over 5 m across
/// the full width (x in [10, 15]), and a landing at the top (room_2, x in [15, 20], z = 3);
/// y in [0, 8] throughout.
/// Nothing lies over anything.
inline TwoConnectedRegions straight_stair_to_a_landing()
{
    TwoConnectedRegions out{};
    WalkableSurface surface{};
    out.room_1 = surface.add_region({rectangle_points({0, 0}, {10, 8}), {}}, 0.0);
    out.room_2 = surface.add_region({rectangle_points({15, 0}, {20, 8}), {}}, 3.0);
    out.connector =
        surface.connect_regions(out.room_1, {{10, 0}, {10, 8}}, out.room_2, {{15, 0}, {15, 8}});
    out.geometry = surface.create_geometry();
    return out;
}

/// A stair that turns on a landing and climbs on: from a floor at z = 0 (room_1, x in [-1, 0],
/// y in [0, 2]) the first flight climbs 3 m out along y in [0, 2] to the landing (landing,
/// x in [6, 8], y in [0, 5], z = 3), and the second flight climbs 3 m on back along y in [3, 5]
/// to a floor at z = 6 (room_2, x in [-1, 0]). Between the flights the stairwell, open at the
/// near end.
///
/// The flights lie beside each other in plan and overlap nowhere -- and still the far wall of
/// the stairwell stands 3 m up and climbs to 6, one metre in plan from someone at the foot of
/// the first flight.
inline TwoConnectedRegionsWithLanding stair_turning_on_a_landing()
{
    TwoConnectedRegionsWithLanding out{};
    WalkableSurface surface{};
    out.room_1 = surface.add_region({rectangle_points({-1, 0}, {0, 2}), {}}, 0.0);
    // (6, 2) and (6, 3) split the landing's west side so each flight has an edge of its own.
    out.landing = surface.add_region({{{6, 0}, {8, 0}, {8, 5}, {6, 5}, {6, 3}, {6, 2}}, {}}, 3.0);
    out.room_2 = surface.add_region({rectangle_points({-1, 3}, {0, 5}), {}}, 6.0);
    surface.connect_regions(out.room_1, {{0, 0}, {0, 2}}, out.landing, {{6, 0}, {6, 2}});
    surface.connect_regions(out.landing, {{6, 3}, {6, 5}}, out.room_2, {{0, 3}, {0, 5}});
    out.geometry = surface.create_geometry();
    return out;
}

/// A corridor 45 m long and 2 m wide with door recesses down one side:
///
///          ┌┐    ┌┐    ┌┐    ┌┐    ┌┐    ┌┐    ┌┐
///     ┌────┘└────┘└────┘└────┘└────┘└────┘└────┘└───┐
///     │                                             │
///     └─────────────────────────────────────────────┘
///
/// The long wall below has no corner along the way, so that triangulation creates slivers.
inline SingleRoom corridor_with_door_recesses()
{
    constexpr double length = 45.0;
    constexpr double width = 2.0;
    constexpr double depth = 0.3;
    // Walked back along the far wall, so the ring stays counter-clockwise.
    constexpr std::array<std::array<double, 2>, 7> doors{
        {{42, 41}, {36, 35}, {30, 29}, {24, 23}, {18, 17}, {12, 11}, {6, 5}}};

    std::vector<Point> ring{{0, 0}, {length, 0}, {length, width}};
    for(const auto& [near, far] : doors) {
        ring.push_back({near, width});
        ring.push_back({near, width + depth});
        ring.push_back({far, width + depth});
        ring.push_back({far, width});
    }
    ring.push_back({0, width});
    return from_polygons({ring});
}

} // namespace test_geometries
