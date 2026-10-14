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
    return engine.register_destination(geometry.clip_to_region(polygon, region));
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

TEST_F(StagesTests, WaypointIsReachedWithinItsArea)
{
    const Polygon area{test_geometries::rectangle_points({-1, -1}, {1, 1})};
    Waypoint waypoint(destination_of(engine, *room.geometry, area, room.room_id));

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

    Destination square_around(Point p, double half_width, std::size_t region)
    {
        const Point half{half_width, half_width};
        const Polygon area{test_geometries::rectangle_points(p - half, p + half)};
        return destination_of(engine, *rooms.geometry, area, region);
    }
};

TEST_F(StagesOnTwoStoreys, WaypointIsReachedOnlyFromItsOwnFloor)
{
    // Reaches further than the height between the floors.
    Waypoint waypoint(square_around({3, 3}, 3.1, rooms.room_2));

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
