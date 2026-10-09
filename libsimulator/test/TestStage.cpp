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
    std::unique_ptr<Geometry> geometry = test_geometries::rectangle({-10, -10}, {10, 10});
    FloorfieldRoutingEngine engine{*geometry};

    Location at(Point p) const { return geometry->get_location(p.x, p.y, 0); }

    GenericAgent agent_at(Point p, BaseStage::ID stage_id) const
    {
        return ::agent_at(at(p), stage_id);
    }
};

TEST_F(StagesTests, WaypointIsReachedWithinItsDistance)
{
    Waypoint waypoint(destination_of(engine, *geometry, Polygon::from_circle({0, 0}, 1.0), 0));

    EXPECT_TRUE(waypoint.is_completed(agent_at({0.9, 0}, waypoint.id())));
    EXPECT_FALSE(waypoint.is_completed(agent_at({1.1, 0}, waypoint.id())));
}

/// Two floors over the same footprint: (x, y) alone no longer says where a stage is.
class StagesOnTwoStoreys : public ::testing::Test
{
public:
    const std::unique_ptr<Geometry> geometry = test_geometries::stacked_floors_with_ramp();
    const std::size_t ground = geometry->get_location_near_z(3, 3, 0.0)->region();
    const std::size_t upper = geometry->get_location_near_z(3, 3, 3.0)->region();
    FloorfieldRoutingEngine engine{*geometry};

    Location at(Point p, std::size_t region) const
    {
        return geometry->get_location(p.x, p.y, region);
    }

    GenericAgent agent_at(Point p, std::size_t region, BaseStage::ID stage_id) const
    {
        return ::agent_at(at(p, region), stage_id);
    }

    Destination circle_around(Point p, double radius, std::size_t region)
    {
        return destination_of(engine, *geometry, Polygon::from_circle(p, radius), region);
    }
};

TEST_F(StagesOnTwoStoreys, WaypointIsReachedOnlyFromItsOwnFloor)
{
    // A radius larger than the height between the floors.
    Waypoint waypoint(circle_around({3, 3}, 3.1, upper));

    EXPECT_FALSE(waypoint.is_completed(agent_at({3, 3}, ground, waypoint.id())));
    EXPECT_TRUE(waypoint.is_completed(agent_at({3, 3}, upper, waypoint.id())));
}

TEST_F(StagesOnTwoStoreys, PassingOverOrUnderAnExitDoesNotTakeIt)
{
    std::vector<GenericAgent::ID> removed{};
    const Polygon area{test_geometries::rectangle_points({2, 2}, {4, 4})};
    Exit lower(destination_of(engine, *geometry, area, ground), removed);
    Exit upper_exit(destination_of(engine, *geometry, area, upper), removed);

    EXPECT_FALSE(lower.is_completed(agent_at({3, 3}, upper, lower.id())));
    EXPECT_FALSE(upper_exit.is_completed(agent_at({3, 3}, ground, upper_exit.id())));
    EXPECT_TRUE(removed.empty());

    EXPECT_TRUE(lower.is_completed(agent_at({3, 3}, ground, lower.id())));
    EXPECT_TRUE(upper_exit.is_completed(agent_at({3, 3}, upper, upper_exit.id())));
    EXPECT_EQ(removed.size(), 2u);
}

TEST_F(StagesOnTwoStoreys, DirectSteeringHandsBackWhereTheAgentWasSteered)
{
    DirectSteering steering{};

    auto agent = agent_at({1, 1}, ground, steering.id());
    agent.final_target = at({3, 3}, upper);

    const auto target = std::get<Location>(steering.target(agent));
    EXPECT_EQ(target.xy(), Point(3, 3));
    EXPECT_EQ(target.region(), upper);
}

TEST_F(StagesOnTwoStoreys, WaypointIsReachedFromTheRampLeadingToIt)
{
    // On the upper floor, 1 m past the top of the ramp at x = 14: its radius reaches back onto
    // the ramp, but not down to the ground floor underneath.
    const auto ramp = geometry->get_location_near_z(10, 5, 1.5)->region();
    Waypoint waypoint(circle_around({15, 5}, 1.5, upper));

    EXPECT_TRUE(waypoint.is_completed(agent_at({13.7, 5}, ramp, waypoint.id())));
    EXPECT_FALSE(waypoint.is_completed(agent_at({15, 5}, ground, waypoint.id())));
}
