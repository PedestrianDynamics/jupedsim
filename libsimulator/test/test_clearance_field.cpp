// SPDX-License-Identifier: LGPL-3.0-or-later
#include "geometry/clearance_field.hpp"
#include "geometry_fixtures.hpp"
#include "line_segment.hpp"
#include "point.hpp"
#include "simulation_error.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <cmath>
#include <limits>
#include <random>
#include <vector>

namespace
{
/// The walls of a 10 x 10 room with a 2 x 2 hole in its middle.
std::vector<LineSegment> room_with_hole_walls()
{
    return {
        {{0, 0}, {10, 0}},
        {{10, 0}, {10, 10}},
        {{10, 10}, {0, 10}},
        {{0, 10}, {0, 0}},
        {{4, 4}, {6, 4}},
        {{6, 4}, {6, 6}},
        {{6, 6}, {4, 6}},
        {{4, 6}, {4, 4}}};
}

double true_clearance(const std::vector<LineSegment>& segments, Point p)
{
    double d = std::numeric_limits<double>::max();
    for(const auto& s : segments) {
        d = std::min(d, s.dist_to(p));
    }
    return d;
}

bool crosses_any(const std::vector<LineSegment>& segments, const LineSegment& chord)
{
    return std::any_of(
        segments.begin(), segments.end(), [&](const auto& s) { return intersects(chord, s); });
}
} // namespace

TEST(ClearanceField, NeverExceedsTheTrueDistanceAndStaysClose)
{
    const auto walls = room_with_hole_walls();
    constexpr double cell = 0.5;
    constexpr double cap = 3.0;
    const ClearanceField field({walls}, cell, cap);
    const double slack = 2 * cell * std::sqrt(0.5) + 1e-4;

    std::mt19937 gen(42);
    std::uniform_real_distribution<double> coord(0.0, 10.0);
    for(int i = 0; i < 10000; ++i) {
        const Point p{coord(gen), coord(gen)};
        const double truth = true_clearance(walls, p);
        const double bound = field.clearance(0, p);
        ASSERT_LE(bound, truth) << "at (" << p.x << ", " << p.y << ")";
        ASSERT_GE(bound, std::min(truth, cap) - slack) << "at (" << p.x << ", " << p.y << ")";
    }
}

TEST(ClearanceField, IsCappedFarFromAllSegments)
{
    const std::vector<LineSegment> square{
        {{0, 0}, {100, 0}}, {{100, 0}, {100, 100}}, {{100, 100}, {0, 100}}, {{0, 100}, {0, 0}}};
    const ClearanceField field({square}, 0.5, 2.0);
    EXPECT_DOUBLE_EQ(field.clearance(0, {50, 50}), 2.0);
    EXPECT_EQ(field.clearance(0, {50, 0.1}), 0.0);
    EXPECT_GT(field.clearance(0, {50, 1.5}), 0.5);
    EXPECT_LT(field.clearance(0, {50, 1.5}), 1.5);
}

TEST(ClearanceField, RejectsAnUnknownRegion)
{
    const ClearanceField field({room_with_hole_walls()});
    EXPECT_THROW(field.clearance(1, {2, 2}), SimulationError);
}

TEST(ClearanceField, RejectsAPointOutsideTheRegionsBoundingBox)
{
    const ClearanceField field({room_with_hole_walls()});
    EXPECT_THROW(field.clearance(0, {-1, 5}), SimulationError);
    EXPECT_THROW(field.clearance(0, {5, 11}), SimulationError);
    EXPECT_THROW(field.clearance(0, {std::nan(""), 2}), SimulationError);
}

TEST(ClearanceField, CoversTheWholeBoundingBox)
{
    // The far edges map into the last row and column, not past them.
    const ClearanceField field({room_with_hole_walls()});
    for(const Point p : {Point{0, 0}, Point{10, 0}, Point{0, 10}, Point{10, 10}}) {
        EXPECT_NO_THROW(field.clearance(0, p)) << "at (" << p.x << ", " << p.y << ")";
    }
}

TEST(ClearanceField, GeometryAgreesWithBruteForceSight)
{
    // Single region: whether the straight line between two points is clear is exactly whether
    // it crosses no wall. The early exit must not change a single answer.
    const auto room = test_geometries::rectangle_with_hole({0, 0}, {10, 10}, {4, 4}, {6, 6});
    const auto walls = room_with_hole_walls();
    const auto& geo = *room.geometry;

    std::mt19937 gen(7);
    std::uniform_real_distribution<double> coord(0.05, 9.95);
    const auto random_location = [&] {
        while(true) {
            const Point p{coord(gen), coord(gen)};
            if(p.x > 4 && p.x < 6 && p.y > 4 && p.y < 6) {
                continue; // inside the hole
            }
            return geo.get_location(p.x, p.y, room.room_id);
        }
    };
    int shortcut_taken = 0;
    for(int i = 0; i < 5000; ++i) {
        const auto a = random_location();
        // Mostly short sight lines, so the early exit gets exercised.
        std::uniform_real_distribution<double> offset(-3.0, 3.0);
        const Point target{
            std::clamp(a.xy().x + offset(gen), 0.05, 9.95),
            std::clamp(a.xy().y + offset(gen), 0.05, 9.95)};
        if(target.x > 4 && target.x < 6 && target.y > 4 && target.y < 6) {
            continue;
        }
        const auto b = geo.get_location(target.x, target.y, room.room_id);
        const bool expected = !crosses_any(walls, LineSegment{a.xy(), b.xy()});
        if(geo.clearance(a) > distance(a.xy(), b.xy())) {
            ++shortcut_taken;
        }
        ASSERT_EQ(geo.no_geometry_between(a, b), expected);
        ASSERT_EQ(geo.no_geometry_between(a, b.xy() - a.xy()), expected);
        // The wall query answers nothing exactly when no wall comes within reach.
        const double r = 0.3;
        ASSERT_EQ(geo.line_segments_in_range(a, r).empty(), true_clearance(walls, a.xy()) > r);
    }
    EXPECT_GT(shortcut_taken, 100);
}

TEST(ClearanceField, StackedFloorsAreNotSeenThroughTheCeiling)
{
    // Right above each other and far from any wall: the early exit fires, and still the agent
    // on the upper floor is not in sight -- it stands in another region.
    const auto floors = test_geometries::stacked_floors({0, 0}, {10, 10}, 3.0);
    const auto& geo = *floors.geometry;
    const auto below = geo.get_location(5, 5, floors.room_1);
    const auto above = geo.get_location(5, 5.5, floors.room_2);
    ASSERT_GT(geo.clearance(below), 0.5);
    EXPECT_FALSE(geo.no_geometry_between(below, above));
    EXPECT_TRUE(geo.no_geometry_between(below, geo.get_location(5, 5.5, floors.room_1)));
    EXPECT_TRUE(geo.line_segments_in_range(below, 0.3).empty());
}
