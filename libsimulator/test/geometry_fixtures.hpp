// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "geometry/geometry.hpp"
#include "geometry/walkable_surface.hpp"
#include "geometry_builder.hpp"
#include "mesh_fixtures.hpp"
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

/// Flat geometry at z = 0.
inline std::unique_ptr<Geometry> from_polygons(
    const std::vector<std::vector<Point>>& boundaries,
    const std::vector<std::vector<Point>>& holes = {})
{
    GeometryBuilder builder{};
    for(const auto& loop : boundaries) {
        builder.add_accessible_area(loop);
    }
    for(const auto& loop : holes) {
        builder.exclude_from_accessible_area(loop);
    }
    WalkableSurface surface{};
    surface.add_region(builder.build(), 0.0);
    return surface.create_geometry();
}

/// Flat rectangle at z = 0.
inline std::unique_ptr<Geometry> rectangle(Point lower_left, Point upper_right)
{
    return from_polygons({rectangle_points(lower_left, upper_right)});
}

inline std::unique_ptr<Geometry> two_rooms()
{
    WalkableSurface surface{};
    const auto a =
        surface.add_region(WalkableSurface::Polygon{rectangle_points({0, 0}, {10, 10}), {}}, 0.0);
    const auto b =
        surface.add_region(WalkableSurface::Polygon{rectangle_points({11, 0}, {21, 10}), {}}, 0.0);
    surface.connect_regions(a, {{10, 0}, {10, 10}}, b, {{11, 0}, {11, 10}});
    return surface.create_geometry();
}

/// Two floors over the same 20 x 10 footprint, the ground floor at z = 0 and the upper one at
/// z = 3, joined by a ramp in the middle. The ramp climbs along x in [6, 14], y in [4, 6],
/// through an opening of that size in both floors: its foot on the ground floor's opening edge at
/// x = 6, its top on the upper floor's at x = 14. Built from regions, so that areas are cut
/// along their footprints.
inline std::unique_ptr<Geometry> stacked_floors_with_ramp()
{
    const auto footprint = rectangle_points({0, 0}, {20, 10});
    const auto opening = rectangle_points({6, 4}, {14, 6});
    WalkableSurface surface{};
    const auto ground = surface.add_region(WalkableSurface::Polygon{footprint, {opening}}, 0.0);
    const auto upper = surface.add_region(WalkableSurface::Polygon{footprint, {opening}}, 3.0);
    surface.connect_regions(ground, {{6, 4}, {6, 6}}, upper, {{14, 4}, {14, 6}});
    return surface.create_geometry();
}

/// Flat rectangle with rectangular hole, z = 0.
inline std::unique_ptr<Geometry> rectangle_with_hole(
    Point lower_left,
    Point upper_right,
    Point hole_lower_left,
    Point hole_upper_right)
{
    return from_polygons(
        {rectangle_points(lower_left, upper_right)},
        {rectangle_points(hole_lower_left, hole_upper_right)});
}

/// Ground and upper floor over the same 20 x 20 footprint, joined through a stairwell by a
/// U-stair: one flight up to a landing, one back to the upper floor.
struct UStair {
    std::unique_ptr<Geometry> geometry;
    std::size_t ground;
    std::size_t upper;
    std::size_t landing;
};

inline UStair u_stair()
{
    // (8, 9) and (13, 9) split the stairwell sides so each flight has an edge of its own.
    const WalkableSurface::Ring stairwell{{8, 6}, {14, 6}, {14, 12}, {8, 12}, {8, 9}};
    WalkableSurface surface{};
    const auto ground = surface.add_region({rectangle_points({0, 0}, {20, 20}), {stairwell}}, 0.0);
    const auto upper = surface.add_region({rectangle_points({0, 0}, {20, 20}), {stairwell}}, 3.0);
    const auto landing =
        surface.add_region({{{13, 6}, {14, 6}, {14, 12}, {13, 12}, {13, 9}}, {}}, 1.5);
    surface.connect_regions(ground, {{8, 6}, {8, 9}}, landing, {{13, 6}, {13, 9}});
    surface.connect_regions(landing, {{13, 9}, {13, 12}}, upper, {{8, 9}, {8, 12}});
    return {surface.create_geometry(), ground, upper, landing};
}

/// A ramp climbing along y, from z = 0 to "height".
inline std::unique_ptr<Geometry> ramp(Point lower_left, Point upper_right, double height)
{
    return std::make_unique<Geometry>(fixtures::ramp(lower_left, upper_right, height));
}

/// A flat floor welded to a ramp rising away from it: one region, with a seam across it.
inline std::unique_ptr<Geometry> floor_with_ramp()
{
    return std::make_unique<Geometry>(fixtures::floor_with_ramp());
}

/// Two unconnected floors.
inline std::unique_ptr<Geometry>
stacked_floors(Point lower_left, Point upper_right, double floor_height)
{
    return std::make_unique<Geometry>(
        fixtures::stacked_floors(lower_left, upper_right, floor_height));
}

/// Ground floor, a flight climbing away from it, a landing, and the upper floor turning back
/// over it.
inline std::unique_ptr<Geometry> switchback_stair(bool upper_first = false)
{
    return std::make_unique<Geometry>(fixtures::switchback_stair(upper_first));
}

/// Ground floor, a flight climbing 3 m over 5 m, and the upper level it leads to.
inline std::unique_ptr<Geometry> two_levels_with_stair()
{
    return std::make_unique<Geometry>(fixtures::two_levels_with_stair());
}

/// A straight climb across the full width to a landing, with nothing lying over anything.
inline std::unique_ptr<Geometry> straight_stair_to_a_landing()
{
    return std::make_unique<Geometry>(fixtures::straight_stair_to_a_landing());
}

/// A flight to a landing where the way on turns off to the side.
inline std::unique_ptr<Geometry> stair_turning_on_a_landing()
{
    return std::make_unique<Geometry>(fixtures::stair_turning_on_a_landing());
}

/// A corridor 45 m long and 2 m wide with door recesses down one side:
///
///          ┌┐    ┌┐    ┌┐    ┌┐    ┌┐    ┌┐    ┌┐
///     ┌────┘└────┘└────┘└────┘└────┘└────┘└────┘└───┐
///     │                                             │
///     └─────────────────────────────────────────────┘
///
/// The long wall below has no corner along the way, so that triangulation creates slivers.
inline std::unique_ptr<Geometry> corridor_with_door_recesses()
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
