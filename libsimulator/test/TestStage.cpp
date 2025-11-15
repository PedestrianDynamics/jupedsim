// SPDX-License-Identifier: LGPL-3.0-or-later
#include "GenericAgent.hpp"
#include "GeometryBuilder.hpp"
#include "Journey.hpp"
#include "Stage.hpp"
#include "gtest/gtest.h"

class StagesTests : public ::testing::Test
{
public:
    NeighborhoodSearch<GenericAgent> neighborhoodSearch{2};
    std::unique_ptr<CollisionGeometry> collisionGeometry{};

    void SetUp() override
    {
        GeometryBuilder b{};
        b.AddAccessibleArea({{-10, -10}, {10, -10}, {10, 10}, {-10, 10}});
        collisionGeometry = std::make_unique<CollisionGeometry>(b.Build());
    }

    void TearDown() override {}
};

TEST_F(StagesTests, NotifiableWaitingSetTargetIsCorrect)
{
    std::vector<Point> waitingPoints = {{-9, -9}, {9, -9}, {9, 9}, {-9, 9}};
    NotifiableWaitingSet waitingSet(waitingPoints);

    // Each agent gets the next target of the provided waiting points until all positions are
    // occupied
    for(size_t i = 0; i < waitingPoints.size(); ++i) {
        GenericAgent agent(
            GenericAgent::ID::Invalid,
            Journey::ID::Invalid,
            waitingSet.Id(),
            waitingPoints[i],
            {},
            CollisionFreeSpeedModelData{});
        neighborhoodSearch.AddAgent(agent);

        const auto& target = waitingSet.Target(agent);
        ASSERT_EQ(target, waitingPoints[i]);

        waitingSet.Update(neighborhoodSearch, *collisionGeometry);
    }

    // Each next agent gets the last slot
    for(size_t i = 0; i < 2; ++i) {
        GenericAgent agentToLastWaitingSetPos(
            GenericAgent::ID::Invalid,
            Journey::ID::Invalid,
            waitingSet.Id(),
            {},
            {},
            CollisionFreeSpeedModelData{});
        neighborhoodSearch.AddAgent(agentToLastWaitingSetPos);
        const auto& target = waitingSet.Target(agentToLastWaitingSetPos);
        ASSERT_EQ(target, waitingPoints.back());
    }
}
