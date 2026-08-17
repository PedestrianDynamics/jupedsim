// SPDX-License-Identifier: LGPL-3.0-or-later
#include "GenericAgent.hpp"
#include "GeometryBuilder.hpp"
#include "Journey.hpp"
#include "MeshFixtures.hpp"
#include "Stage.hpp"
#include "TestCommon.hpp"
#include "gtest/gtest.h"

class StagesTests : public ::testing::Test
{
public:
    NeighborhoodSearch<GenericAgent> neighborhoodSearch{2};
    std::unique_ptr<Geometry3D> geometry{};

    void SetUp() override
    {
        GeometryBuilder b{};
        b.AddAccessibleArea({{-10, -10}, {10, -10}, {10, 10}, {-10, 10}});
        geometry = std::make_unique<Geometry3D>(b.Build().Polygon());
    }

    void TearDown() override {}

    Location At(Point p) const { return *geometry->get_location(p.x, p.y, 0.0); }

    std::vector<Location> AllAt(const std::vector<Point>& points) const
    {
        std::vector<Location> locations{};
        for(const auto& p : points) {
            locations.push_back(At(p));
        }
        return locations;
    }

    GenericAgent AgentAt(Point p, BaseStage::ID stageId) const
    {
        return GenericAgent(
            GenericAgent::ID::Invalid,
            Journey::ID::Invalid,
            stageId,
            At(p),
            CollisionFreeSpeedModelState{});
    }
};

TEST_F(StagesTests, NotifiableWaitingSetTargetIsCorrect)
{
    std::vector<Point> waitingPoints = {{-9, -9}, {9, -9}, {9, 9}, {-9, 9}};
    NotifiableWaitingSet waitingSet(AllAt(waitingPoints));

    // Each agent gets the next target of the provided waiting points until all positions are
    // occupied
    for(size_t i = 0; i < waitingPoints.size(); ++i) {
        GenericAgent agent = AgentAt(waitingPoints[i], waitingSet.Id());
        neighborhoodSearch.AddAgent(agent);

        ASSERT_EQ(waitingSet.Target(agent).xy(), waitingPoints[i]);
        EnvironmentQuery envQuery(*geometry, neighborhoodSearch);
        waitingSet.Update(envQuery);
    }

    // Each next agent gets the last slot
    for(size_t i = 0; i < 2; ++i) {
        GenericAgent agentToLastWaitingSetPos = AgentAt(Point{}, waitingSet.Id());
        neighborhoodSearch.AddAgent(agentToLastWaitingSetPos);
        ASSERT_EQ(waitingSet.Target(agentToLastWaitingSetPos).xy(), waitingPoints.back());
    }
}

/// Two floors over the same footprint: (x, y) alone no longer says where a stage is.
class StagesOnTwoStoreys : public ::testing::Test
{
public:
    Geometry3D geometry{fixtures::stacked_floors()};

    Location At(Point p, double z) const { return *geometry.get_location(p.x, p.y, z); }

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

TEST_F(StagesOnTwoStoreys, WaitingSetIsCompletedOnlyOnItsOwnFloor)
{
    NotifiableWaitingSet waitingSet(std::vector<Location>{At({5, 5}, 3.0)});
    waitingSet.State(WaitingSetState::Inactive);

    EXPECT_FALSE(waitingSet.IsCompleted(AgentAt({5, 5}, 0.0, waitingSet.Id())));
    EXPECT_TRUE(waitingSet.IsCompleted(AgentAt({5, 5}, 3.0, waitingSet.Id())));
}

TEST_F(StagesOnTwoStoreys, QueueEnqueuesOnlyAgentsOnItsOwnFloor)
{
    NotifiableQueue queue(std::vector<Location>{At({5, 5}, 3.0)});
    NeighborhoodSearch<GenericAgent> search{5.0};
    const EnvironmentQuery query{geometry, search};

    AgentContainer<GenericAgent> agents{};
    agents.push_back(AgentAt({5, 5}, 0.0, queue.Id()));
    search.Update(agents);
    queue.Update(query);
    EXPECT_TRUE(queue.Occupants().empty());

    agents.push_back(AgentAt({5, 5}, 3.0, queue.Id()));
    search.Update(agents);
    queue.Update(query);
    ASSERT_EQ(queue.Occupants().size(), 1u);
    EXPECT_EQ(queue.Occupants().front(), agents[1].id);
}

TEST_F(StagesOnTwoStoreys, WaitingSetSeatsOnlyAgentsOnItsOwnFloor)
{
    NotifiableWaitingSet waitingSet(std::vector<Location>{At({5, 5}, 3.0)});
    NeighborhoodSearch<GenericAgent> search{5.0};
    const EnvironmentQuery query{geometry, search};

    AgentContainer<GenericAgent> agents{};
    agents.push_back(AgentAt({5, 5}, 0.0, waitingSet.Id()));
    search.Update(agents);
    waitingSet.Update(query);
    EXPECT_TRUE(waitingSet.Occupants().empty());

    agents.push_back(AgentAt({5, 5}, 3.0, waitingSet.Id()));
    search.Update(agents);
    waitingSet.Update(query);
    ASSERT_EQ(waitingSet.Occupants().size(), 1u);
    EXPECT_EQ(waitingSet.Occupants().front(), agents[1].id);
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
    Geometry3D geometry{fixtures::two_levels_with_stair()};
    Waypoint waypoint(*geometry.get_location(12.5, 2.0, 1.5), 1.0);

    const GenericAgent agent(
        GenericAgent::ID::Invalid,
        Journey::ID::Invalid,
        waypoint.Id(),
        *geometry.get_location(11.7, 2.0, 1.02),
        CollisionFreeSpeedModelState{});

    EXPECT_TRUE(waypoint.IsCompleted(agent));
}
