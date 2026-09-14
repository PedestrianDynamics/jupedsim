// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "Geometry/Geometry.hpp"
#include "GeometryBuilder.hpp"
#include "MeshFixtures.hpp"
#include "Point.hpp"

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
        builder.AddAccessibleArea(loop);
    }
    for(const auto& loop : holes) {
        builder.ExcludeFromAccessibleArea(loop);
    }
    return std::make_unique<Geometry>(builder.Build());
}

/// Flat rectangle at z = 0.
inline std::unique_ptr<Geometry> rectangle(Point lower_left, Point upper_right)
{
    return from_polygons({rectangle_points(lower_left, upper_right)});
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

/// A long flat corridor with door recesses set into its sides.
inline std::unique_ptr<Geometry> corridor_with_door_recesses()
{
    return std::make_unique<Geometry>(fixtures::corridor_with_door_recesses());
}

} // namespace test_geometries
