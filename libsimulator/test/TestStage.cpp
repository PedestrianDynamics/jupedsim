// SPDX-License-Identifier: LGPL-3.0-or-later
#include "GenericAgent.hpp"
#include "GeometryFixtures.hpp"
#include "Journey.hpp"
#include "Stage.hpp"
#include "TestCommon.hpp"
#include "gtest/gtest.h"

/// Two floors over the same footprint: (x, y) alone no longer says where a stage is.
class StagesOnTwoStoreys : public ::testing::Test
{
public:
    const std::unique_ptr<Geometry> geometry =
        test_geometries::stacked_floors({0, 0}, {10, 10}, 3.0);

    Location At(Point p, double z) const { return *geometry->get_location_near_z(p.x, p.y, z); }

    GenericAgent AgentAt(Point p, double z, BaseStage::ID stageId) const
    {
        return GenericAgent(
            GenericAgent::ID::Invalid,
            Journey::ID::Invalid,
            stageId,
            At(p, z),
            CollisionFreeSpeedModelState{});
    }
};

TEST_F(StagesOnTwoStoreys, WaypointIsReachedOnlyFromItsOwnFloor)
{
    Waypoint waypoint(At({5, 5}, 3.0), 1.0);

    EXPECT_FALSE(waypoint.IsCompleted(AgentAt({5, 5}, 0.0, waypoint.Id())));
    EXPECT_TRUE(waypoint.IsCompleted(AgentAt({5, 5}, 3.0, waypoint.Id())));
}

TEST_F(StagesOnTwoStoreys, PassingOverOrUnderAnExitDoesNotTakeIt)
{
    std::vector<GenericAgent::ID> removed{};
    const Polygon area{std::vector<Point>{{4, 4}, {6, 4}, {6, 6}, {4, 6}}};
    Exit lower(area, At({5, 5}, 0.0), removed);
    Exit upper(area, At({5, 5}, 3.0), removed);

    EXPECT_FALSE(lower.IsCompleted(AgentAt({5, 5}, 3.0, lower.Id())));
    EXPECT_FALSE(upper.IsCompleted(AgentAt({5, 5}, 0.0, upper.Id())));
    EXPECT_TRUE(removed.empty());

    EXPECT_TRUE(lower.IsCompleted(AgentAt({5, 5}, 0.0, lower.Id())));
    EXPECT_TRUE(upper.IsCompleted(AgentAt({5, 5}, 3.0, upper.Id())));
    EXPECT_EQ(removed.size(), 2u);
}

TEST_F(StagesOnTwoStoreys, TargetCarriesTheFloorItIsOn)
{
    Waypoint upper(At({5, 5}, 3.0), 1.0);
    Waypoint lower(At({5, 5}, 0.0), 1.0);
    const auto agent = AgentAt({1, 1}, 0.0, upper.Id());

    EXPECT_DOUBLE_EQ(upper.Target(agent).z(), 3.0);
    EXPECT_DOUBLE_EQ(lower.Target(agent).z(), 0.0);
}

TEST_F(StagesOnTwoStoreys, DirectSteeringHandsBackWhereTheAgentWasSteered)
{
    DirectSteering steering{};

    auto agent = AgentAt({1, 1}, 0.0, steering.Id());
    agent.finalTarget = At({5, 5}, 3.0);

    EXPECT_EQ(steering.Target(agent).xy(), Point(5, 5));
    EXPECT_DOUBLE_EQ(steering.Target(agent).z(), 3.0);
}

TEST(StagesOnAStair, WaypointIsReachedFromTheStairItStandsOn)
{
    // A stair climbing 3 m over 5 m: an agent 0.8 m short of the waypoint in plan is
    // half a metre below it.
    const auto geometry = test_geometries::two_levels_with_stair();
    Waypoint waypoint(*geometry->get_location_near_z(12.5, 2.0, 1.5), 1.0);

    const GenericAgent agent(
        GenericAgent::ID::Invalid,
        Journey::ID::Invalid,
        waypoint.Id(),
        *geometry->get_location_near_z(11.7, 2.0, 1.02),
        CollisionFreeSpeedModelState{});

    EXPECT_TRUE(waypoint.IsCompleted(agent));
}
