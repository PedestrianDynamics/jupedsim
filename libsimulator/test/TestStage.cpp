// SPDX-License-Identifier: LGPL-3.0-or-later
#include "FloorfieldRoutingEngine.hpp"
#include "GenericAgent.hpp"
#include "GeometryFixtures.hpp"
#include "Journey.hpp"
#include "Polygon.hpp"
#include "Stage.hpp"
#include "TestCommon.hpp"
#include "gtest/gtest.h"

#include <variant>

namespace
{
Destination DestinationOf(
    FloorfieldRoutingEngine& engine,
    const Geometry& geometry,
    const Polygon& polygon,
    std::size_t region)
{
    return engine.RegisterDestination(geometry.split_into_region_pieces(polygon, region));
}

GenericAgent AgentAt(Location location, BaseStage::ID stageId)
{
    return GenericAgent(
        GenericAgent::ID::Invalid,
        Journey::ID::Invalid,
        stageId,
        location,
        CollisionFreeSpeedModelState{});
}
} // namespace

class StagesTests : public ::testing::Test
{
public:
    std::unique_ptr<Geometry> geometry = test_geometries::rectangle({-10, -10}, {10, 10});
    FloorfieldRoutingEngine engine{*geometry};

    Location At(Point p) const { return geometry->get_location(p.x, p.y, 0); }

    GenericAgent AgentAt(Point p, BaseStage::ID stageId) const { return ::AgentAt(At(p), stageId); }
};

TEST_F(StagesTests, WaypointIsReachedWithinItsDistance)
{
    Waypoint waypoint(DestinationOf(engine, *geometry, Polygon::FromCircle({0, 0}, 1.0), 0));

    EXPECT_TRUE(waypoint.IsCompleted(AgentAt({0.9, 0}, waypoint.Id())));
    EXPECT_FALSE(waypoint.IsCompleted(AgentAt({1.1, 0}, waypoint.Id())));
}

/// Two floors over the same footprint: (x, y) alone no longer says where a stage is.
class StagesOnTwoStoreys : public ::testing::Test
{
public:
    const std::unique_ptr<Geometry> geometry = test_geometries::stacked_floors_with_ramp();
    const std::size_t ground = geometry->get_location_near_z(3, 3, 0.0)->region();
    const std::size_t upper = geometry->get_location_near_z(3, 3, 3.0)->region();
    FloorfieldRoutingEngine engine{*geometry};

    Location At(Point p, std::size_t region) const
    {
        return geometry->get_location(p.x, p.y, region);
    }

    GenericAgent AgentAt(Point p, std::size_t region, BaseStage::ID stageId) const
    {
        return ::AgentAt(At(p, region), stageId);
    }

    Destination CircleAround(Point p, double radius, std::size_t region)
    {
        return DestinationOf(engine, *geometry, Polygon::FromCircle(p, radius), region);
    }
};

TEST_F(StagesOnTwoStoreys, WaypointIsReachedOnlyFromItsOwnFloor)
{
    // A radius larger than the height between the floors.
    Waypoint waypoint(CircleAround({3, 3}, 3.1, upper));

    EXPECT_FALSE(waypoint.IsCompleted(AgentAt({3, 3}, ground, waypoint.Id())));
    EXPECT_TRUE(waypoint.IsCompleted(AgentAt({3, 3}, upper, waypoint.Id())));
}

TEST_F(StagesOnTwoStoreys, PassingOverOrUnderAnExitDoesNotTakeIt)
{
    std::vector<GenericAgent::ID> removed{};
    const Polygon area{test_geometries::rectangle_points({2, 2}, {4, 4})};
    Exit lower(DestinationOf(engine, *geometry, area, ground), removed);
    Exit upper_exit(DestinationOf(engine, *geometry, area, upper), removed);

    EXPECT_FALSE(lower.IsCompleted(AgentAt({3, 3}, upper, lower.Id())));
    EXPECT_FALSE(upper_exit.IsCompleted(AgentAt({3, 3}, ground, upper_exit.Id())));
    EXPECT_TRUE(removed.empty());

    EXPECT_TRUE(lower.IsCompleted(AgentAt({3, 3}, ground, lower.Id())));
    EXPECT_TRUE(upper_exit.IsCompleted(AgentAt({3, 3}, upper, upper_exit.Id())));
    EXPECT_EQ(removed.size(), 2u);
}

TEST_F(StagesOnTwoStoreys, DirectSteeringHandsBackWhereTheAgentWasSteered)
{
    DirectSteering steering{};

    auto agent = AgentAt({1, 1}, ground, steering.Id());
    agent.finalTarget = At({3, 3}, upper);

    const auto target = std::get<Location>(steering.Target(agent));
    EXPECT_EQ(target.xy(), Point(3, 3));
    EXPECT_EQ(target.region(), upper);
}

TEST_F(StagesOnTwoStoreys, WaypointIsReachedFromTheRampLeadingToIt)
{
    // On the upper floor, 1 m past the top of the ramp at x = 14: its radius reaches back onto
    // the ramp, but not down to the ground floor underneath.
    const auto ramp = geometry->get_location_near_z(10, 5, 1.5)->region();
    Waypoint waypoint(CircleAround({15, 5}, 1.5, upper));

    EXPECT_TRUE(waypoint.IsCompleted(AgentAt({13.7, 5}, ramp, waypoint.Id())));
    EXPECT_FALSE(waypoint.IsCompleted(AgentAt({15, 5}, ground, waypoint.Id())));
}
