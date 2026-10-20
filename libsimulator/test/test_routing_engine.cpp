// SPDX-License-Identifier: LGPL-3.0-or-later
#include "geometry_fixtures.hpp"
#include "polygon.hpp"
#include "routing_engine.hpp"
#include "simulation_error.hpp"

#include <gtest/gtest.h>

#include <vector>

TEST(RoutingEngine, LeadsToTheNearestAreaOfADestination)
{
    const auto room = test_geometries::rectangle({0, 0}, {10, 2});
    RoutingEngine engine{*room.geometry, FloorfieldOptions{}};

    const std::vector<AreaPiece> exits{
        {Polygon{test_geometries::rectangle_points({0.5, 0.5}, {1.5, 1.5})}, room.room_id},
        {Polygon{test_geometries::rectangle_points({8.5, 0.5}, {9.5, 1.5})}, room.room_id}};
    const auto destination = engine.register_destination(exits);

    const auto right = room.geometry->get_location(7, 1, room.room_id);
    const auto left = room.geometry->get_location(3, 1, room.room_id);
    EXPECT_GT(engine.get_orientation(right, destination).x, 0.0);
    EXPECT_LT(engine.get_orientation(left, destination).x, 0.0);
}

TEST(RoutingEngine, ADestinationNeedsAnAreaInsideItsRegion)
{
    const auto room = test_geometries::rectangle({0, 0}, {10, 2});
    RoutingEngine engine{*room.geometry, FloorfieldOptions{}};

    EXPECT_THROW(engine.register_destination({}), SimulationError);
    const std::vector<AreaPiece> outside{
        {Polygon{test_geometries::rectangle_points({20, 0}, {21, 1})}, room.room_id}};
    EXPECT_THROW(engine.register_destination(outside), SimulationError);
}
