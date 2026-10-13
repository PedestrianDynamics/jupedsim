// SPDX-License-Identifier: LGPL-3.0-or-later
#include "floorfield_routing_engine.hpp"
#include "generic_agent.hpp"
#include "geometry_fixtures.hpp"
#include "gtest/gtest.h"
#include "journey.hpp"
#include "polygon.hpp"
#include "stage.hpp"
#include "test_common.hpp"

#include <variant>

namespace
{
Destination destination_of(
    FloorfieldRoutingEngine& engine,
    const Geometry& geometry,
    const Polygon& polygon,
    std::size_t region)
{
    return engine.register_destination(geometry.split_into_region_pieces(polygon, region));
}

GenericAgent agent_at(Location location, BaseStage::ID stage_id)
{
    return GenericAgent(
        GenericAgent::ID::invalid,
        Journey::ID::invalid,
        stage_id,
        location,
        CollisionFreeSpeedModelState{});
}
} // namespace

class StagesTests : public ::testing::Test
{
public:
    const test_geometries::SingleRoom room = test_geometries::rectangle({-10, -10}, {10, 10});
    FloorfieldRoutingEngine engine{*room.geometry};

    Location at(Point p) const { return room.geometry->get_location(p.x, p.y, room.room_id); }

    GenericAgent agent_at(Point p, BaseStage::ID stage_id) const
    {
        return ::agent_at(at(p), stage_id);
    }
};

TEST_F(StagesTests, WaypointIsReachedWithinItsDistance)
{
    Waypoint waypoint(
        destination_of(engine, *room.geometry, Polygon::from_circle({0, 0}, 1.0), room.room_id));

    EXPECT_TRUE(waypoint.is_completed(agent_at({0.9, 0}, waypoint.id())));
    EXPECT_FALSE(waypoint.is_completed(agent_at({1.1, 0}, waypoint.id())));
}

/// Two floors over the same footprint: (x, y) alone no longer says where a stage is.
class StagesOnTwoStoreys : public ::testing::Test
{
public:
    const test_geometries::TwoConnectedRegions rooms = test_geometries::stacked_floors_with_ramp();
    FloorfieldRoutingEngine engine{*rooms.geometry};

    Location at(Point p, std::size_t region) const
    {
        return rooms.geometry->get_location(p.x, p.y, region);
    }

    GenericAgent agent_at(Point p, std::size_t region, BaseStage::ID stage_id) const
    {
        return ::agent_at(at(p, region), stage_id);
    }

    Destination circle_around(Point p, double radius, std::size_t region)
    {
        return destination_of(engine, *rooms.geometry, Polygon::from_circle(p, radius), region);
    }
};

TEST_F(StagesOnTwoStoreys, WaypointIsReachedOnlyFromItsOwnFloor)
{
    // A radius larger than the height between the floors.
    Waypoint waypoint(circle_around({3, 3}, 3.1, rooms.room_2));

    EXPECT_FALSE(waypoint.is_completed(agent_at({3, 3}, rooms.room_1, waypoint.id())));
    EXPECT_TRUE(waypoint.is_completed(agent_at({3, 3}, rooms.room_2, waypoint.id())));
}

TEST_F(StagesOnTwoStoreys, PassingOverOrUnderAnExitDoesNotTakeIt)
{
    std::vector<GenericAgent::ID> removed{};
    const Polygon area{test_geometries::rectangle_points({2, 2}, {4, 4})};
    Exit lower(destination_of(engine, *rooms.geometry, area, rooms.room_1), removed);
    Exit upper_exit(destination_of(engine, *rooms.geometry, area, rooms.room_2), removed);

    EXPECT_FALSE(lower.is_completed(agent_at({3, 3}, rooms.room_2, lower.id())));
    EXPECT_FALSE(upper_exit.is_completed(agent_at({3, 3}, rooms.room_1, upper_exit.id())));
    EXPECT_TRUE(removed.empty());

    EXPECT_TRUE(lower.is_completed(agent_at({3, 3}, rooms.room_1, lower.id())));
    EXPECT_TRUE(upper_exit.is_completed(agent_at({3, 3}, rooms.room_2, upper_exit.id())));
    EXPECT_EQ(removed.size(), 2u);
}

TEST_F(StagesOnTwoStoreys, DirectSteeringHandsBackWhereTheAgentWasSteered)
{
    DirectSteering steering{};

    auto agent = agent_at({1, 1}, rooms.room_1, steering.id());
    agent.final_target = at({3, 3}, rooms.room_2);

    const auto target = std::get<Location>(steering.target(agent));
    EXPECT_EQ(target.xy(), Point(3, 3));
    EXPECT_EQ(target.region(), rooms.room_2);
}

TEST_F(StagesOnTwoStoreys, WaypointIsReachedFromTheRampLeadingToIt)
{
    // On the upper floor, 1 m past the top of the ramp at x = 14: its radius reaches back onto
    // the ramp, but not down to the ground floor underneath.
    Waypoint waypoint(circle_around({15, 5}, 1.5, rooms.room_2));

    EXPECT_TRUE(waypoint.is_completed(agent_at({13.7, 5}, rooms.connector, waypoint.id())));
    EXPECT_FALSE(waypoint.is_completed(agent_at({15, 5}, rooms.room_1, waypoint.id())));
}
