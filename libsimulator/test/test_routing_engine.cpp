// SPDX-License-Identifier: LGPL-3.0-or-later
#include "collision_free_speed_model_state.hpp"
#include "floorfield_routing_engine.hpp"
#include "generic_agent.hpp"
#include "geometry/area_piece.hpp"
#include "geometry/location.hpp"
#include "geometry_fixtures.hpp"
#include "kernel.hpp"
#include "polygon.hpp"
#include "route.hpp"
#include "routing_engine.hpp"
#include "simulation_error.hpp"
#include "stage.hpp"
#include "tactical_decision_system.hpp"

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
    EXPECT_GT(engine.get_orientation(right, destination).x(), 0.0);
    EXPECT_LT(engine.get_orientation(left, destination).x(), 0.0);
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

TEST(TacticalDecisionSystem, HeldAndStrandedAgentsStayOffTheNavigationMesh)
{
    const auto room = test_geometries::rectangle({0, 0}, {10, 10});
    RoutingEngine engine{*room.geometry, FloorfieldOptions{}};
    std::vector<GenericAgent> agents{};
    for(const double x : {2.0, 4.0, 6.0}) {
        agents.emplace_back(
            GenericAgent::ID::invalid,
            Route::ID::invalid,
            BaseStage::ID::invalid,
            room.geometry->get_location(x, 5, room.room_id),
            CollisionFreeSpeedModelState{});
    }
    agents[0].held = true;
    agents[1].stranded = true;
    agents[2].held = true;
    // Pushed off the point where it stopped.
    agents[2].location = room.geometry->get_location(6, 4, room.room_id);

    TacticalDecisionSystem{}.run(engine, agents);

    EXPECT_EQ(engine.trees_built(), 0U);
    EXPECT_TRUE(agents[0].route_orientation.is_zero_length());
    EXPECT_TRUE(agents[1].route_orientation.is_zero_length());
    EXPECT_NEAR(agents[2].route_orientation.x(), 0.0, 1e-12);
    EXPECT_NEAR(agents[2].route_orientation.y(), 1.0, 1e-12);
}
