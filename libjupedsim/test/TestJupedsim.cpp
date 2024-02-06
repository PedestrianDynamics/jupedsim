// Copyright © 2012-2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "AgentIterator.hpp"
#include "Point.hpp"
#include "gtest/gtest.h"
#include <ErrorMessage.hpp>
#include <jupedsim/jupedsim.h>

#include <array>
#include <vector>

#include <gtest/gtest.h>

TEST(ErrorMessage, CanGetMessage)
{
    JPS_ErrorMessage_t* msg = new JPS_ErrorMessage_t{};
    const std::string expected{"I am an error"};
    msg->message = expected;
    ASSERT_EQ(JPS_ErrorMessage_GetMessage(msg), expected);
    ASSERT_NO_FATAL_FAILURE(JPS_ErrorMessage_Free(msg));
}

TEST(OperationalModel, CanConstructCollisionFreeSpeedModel)
{
    JPS_ErrorMessage errorMsg{};
    auto builder = JPS_CollisionFreeSpeedModelBuilder_Create(1, 1, 1, 1);
    auto model = JPS_CollisionFreeSpeedModelBuilder_Build(builder, &errorMsg);
    EXPECT_NE(model, nullptr);
    EXPECT_EQ(errorMsg, nullptr);
    JPS_CollisionFreeSpeedModelBuilder_Free(builder);
    JPS_OperationalModel_Free(model);
}

TEST(OperationalModel, DefaultsOfCollisionFreeSpeedModelAgentParameters)
{
    JPS_CollisionFreeSpeedModelAgentParameters agentParameters{};

    ASSERT_DOUBLE_EQ(agentParameters.position.x, 0);
    ASSERT_DOUBLE_EQ(agentParameters.position.y, 0);
    ASSERT_DOUBLE_EQ(agentParameters.journeyId, 0);
    ASSERT_DOUBLE_EQ(agentParameters.stageId, 0);
    ASSERT_DOUBLE_EQ(agentParameters.time_gap, 1);
    ASSERT_DOUBLE_EQ(agentParameters.v0, 1.2);
    ASSERT_DOUBLE_EQ(agentParameters.radius, 0.2);
}

TEST(OperationalModel, CanConstructGeneralizedCentrifugalForceModel)
{
    JPS_ErrorMessage errorMsg{};
    auto builder = JPS_GeneralizedCentrifugalForceModelBuilder_Create(1, 1, 1, 1, 1, 1, 1, 1);
    auto model = JPS_GeneralizedCentrifugalForceModelBuilder_Build(builder, &errorMsg);
    EXPECT_NE(model, nullptr);
    EXPECT_EQ(errorMsg, nullptr);
    JPS_GeneralizedCentrifugalForceModelBuilder_Free(builder);
    JPS_OperationalModel_Free(model);
}

TEST(OperationalModel, DefaultsOfGeneralizedCentrifugalForceModelAgentParameters)
{
    JPS_GeneralizedCentrifugalForceModelAgentParameters agentParameters{};

    ASSERT_DOUBLE_EQ(agentParameters.speed, 0);
    ASSERT_DOUBLE_EQ(agentParameters.e0.x, 0);
    ASSERT_DOUBLE_EQ(agentParameters.e0.y, 0);
    ASSERT_DOUBLE_EQ(agentParameters.position.x, 0);
    ASSERT_DOUBLE_EQ(agentParameters.position.y, 0);
    ASSERT_DOUBLE_EQ(agentParameters.orientation.x, 0);
    ASSERT_DOUBLE_EQ(agentParameters.orientation.y, 0);
    ASSERT_DOUBLE_EQ(agentParameters.journeyId, 0);
    ASSERT_DOUBLE_EQ(agentParameters.stageId, 0);
    ASSERT_DOUBLE_EQ(agentParameters.mass, 1);
    ASSERT_DOUBLE_EQ(agentParameters.tau, 0.5);
    ASSERT_DOUBLE_EQ(agentParameters.v0, 1.2);
    ASSERT_DOUBLE_EQ(agentParameters.a_v, 1.);
    ASSERT_DOUBLE_EQ(agentParameters.a_min, 0.2);
    ASSERT_DOUBLE_EQ(agentParameters.b_min, 0.2);
    ASSERT_DOUBLE_EQ(agentParameters.b_max, 0.4);
}

TEST(GeometryBuilder, CanConstruct)
{
    auto builder = JPS_GeometryBuilder_Create();

    std::vector<std::vector<JPS_Point>> data{
        {{0, 0}, {1, 0}, {1, 1}, {0, 1}},
        {{0, 0}, {0, -1}, {1, -1}, {1, 0}},
        {{0, 0}, {-1, 0}, {-1, -1}, {0, -1}},
        {{0, 0}, {0, 1}, {-1, 1}, {-1, 0}}};

    for(auto& poly : data) {
        JPS_GeometryBuilder_AddAccessibleArea(builder, poly.data(), poly.size());
    }

    std::vector<JPS_Point> box4{{0, 0}, {0.2, 0}, {0.2, 0.2}, {0, 0.2}};
    JPS_GeometryBuilder_ExcludeFromAccessibleArea(builder, box4.data(), box4.size());

    JPS_ErrorMessage message{};
    auto geometry = JPS_GeometryBuilder_Build(builder, &message);
    ASSERT_NE(geometry, nullptr);
    ASSERT_EQ(message, nullptr);
    ASSERT_NO_FATAL_FAILURE(JPS_Geometry_Free(geometry));
    ASSERT_NO_FATAL_FAILURE(JPS_GeometryBuilder_Free(builder));
}

TEST(Simulation, CanSimulate)
{
    auto geo_builder = JPS_GeometryBuilder_Create();

    std::vector<JPS_Point> box1{{0, 0}, {10, 0}, {10, 10}, {0, 10}};
    JPS_GeometryBuilder_AddAccessibleArea(geo_builder, box1.data(), box1.size());

    std::vector<JPS_Point> box2{{10, 4}, {20, 4}, {20, 6}, {10, 6}};
    JPS_GeometryBuilder_AddAccessibleArea(geo_builder, box2.data(), box2.size());

    auto geometry = JPS_GeometryBuilder_Build(geo_builder, nullptr);
    ASSERT_NE(geometry, nullptr);

    auto modelBuilder = JPS_CollisionFreeSpeedModelBuilder_Create(8, 0.1, 5, 0.02);

    auto model = JPS_CollisionFreeSpeedModelBuilder_Build(modelBuilder, nullptr);
    ASSERT_NE(model, nullptr);

    auto simulation = JPS_Simulation_Create(model, geometry, 0.01, nullptr);
    ASSERT_NE(simulation, nullptr);

    std::vector<JPS_Point> box{{18, 4}, {20, 4}, {20, 6}, {18, 6}};
    const auto exitStage = JPS_Simulation_AddStageExit(simulation, box.data(), box.size(), nullptr);
    auto journey = JPS_JourneyDescription_Create();
    JPS_JourneyDescription_AddStage(journey, exitStage);
    auto journeyId = JPS_Simulation_AddJourney(simulation, journey, nullptr);
    JPS_JourneyDescription_Free(journey);

    JPS_CollisionFreeSpeedModelAgentParameters agent_parameters{};
    agent_parameters.journeyId = journeyId;
    agent_parameters.position = JPS_Point{0.0, 0.0};
    agent_parameters.time_gap = 1;
    agent_parameters.v0 = 1.2;
    agent_parameters.radius = 0.3;

    std::vector<JPS_Point> positions{{7, 7}, {1, 3}, {1, 5}, {1, 7}, {2, 7}};
    for(const auto& p : positions) {
        agent_parameters.position = p;
        JPS_Simulation_AddCollisionFreeSpeedModelAgent(simulation, agent_parameters, nullptr);
    }
    while(JPS_Simulation_AgentCount(simulation) > 0) {
        ASSERT_TRUE(JPS_Simulation_Iterate(simulation, nullptr));
    }
    ASSERT_LT(JPS_Simulation_IterationCount(simulation), 2000);
}

struct SimulationTest : public ::testing::Test {
    JPS_Simulation simulation{};
    JPS_JourneyId journey_id{};
    JPS_StageId stage_id{};
    std::array<JPS_CollisionFreeSpeedModelAgentParameters, 2> agent_templates{
        JPS_CollisionFreeSpeedModelAgentParameters{{}, 0, 0, 1, 1.5, 0.3},
        JPS_CollisionFreeSpeedModelAgentParameters{{}, 0, 0, 1, 1.5, 0.3},
    };

    void SetUp() override
    {
        auto geo_builder = JPS_GeometryBuilder_Create();
        std::vector<JPS_Point> box1{{0, 0}, {10, 0}, {10, 10}, {0, 10}};
        JPS_GeometryBuilder_AddAccessibleArea(geo_builder, box1.data(), box1.size());
        auto geometry = JPS_GeometryBuilder_Build(geo_builder, nullptr);
        ASSERT_NE(geometry, nullptr);
        JPS_GeometryBuilder_Free(geo_builder);

        auto modelBuilder = JPS_CollisionFreeSpeedModelBuilder_Create(9, 0.1, 5, 0.02);
        auto model = JPS_CollisionFreeSpeedModelBuilder_Build(modelBuilder, nullptr);

        ASSERT_NE(model, nullptr);

        simulation = JPS_Simulation_Create(model, geometry, 0.01, nullptr);
        ASSERT_NE(simulation, nullptr);

        stage_id = JPS_Simulation_AddStageWaypoint(simulation, {1, 1}, 1, nullptr);

        auto journey = JPS_JourneyDescription_Create();
        JPS_JourneyDescription_AddStage(journey, stage_id);
        journey_id = JPS_Simulation_AddJourney(simulation, journey, nullptr);

        JPS_JourneyDescription_Free(journey);
        ASSERT_NE(journey_id, 0);

        JPS_OperationalModel_Free(model);
        JPS_Geometry_Free(geometry);
        for(auto&& agent : agent_templates) {
            agent.journeyId = journey_id;
            agent.stageId = stage_id;
        }
    }

    void TearDown() override { JPS_Simulation_Free(simulation); }
};

TEST_F(SimulationTest, AgentIteratorIsEmptyForNewSimulation)
{
    ASSERT_EQ(JPS_Simulation_AgentCount(simulation), 0);
    auto iter = JPS_Simulation_AgentIterator(simulation);
    ASSERT_NE(iter, nullptr);
    ASSERT_EQ(JPS_AgentIterator_Next(iter), nullptr);
}

TEST_F(SimulationTest, AgentIteratorCanIterate)
{
    std::vector<JPS_Point> positions{{1, 1}, {2, 1}, {3, 1}};
    std::vector<JPS_CollisionFreeSpeedModelAgentParameters> agent_parameters(
        positions.size(), agent_templates[0]);
    for(size_t index = 0; index < positions.size(); ++index) {
        agent_parameters[index].position = positions[index];
    }

    for(const auto& agent_params : agent_parameters) {
        ASSERT_NE(
            JPS_Simulation_AddCollisionFreeSpeedModelAgent(simulation, agent_params, nullptr), 0);
    }
    ASSERT_EQ(JPS_Simulation_AgentCount(simulation), 3);
    auto iter = JPS_Simulation_AgentIterator(simulation);
    ASSERT_NE(iter, nullptr);
    ASSERT_NE(JPS_AgentIterator_Next(iter), nullptr);
    ASSERT_NE(JPS_AgentIterator_Next(iter), nullptr);
    ASSERT_NE(JPS_AgentIterator_Next(iter), nullptr);
    ASSERT_EQ(JPS_AgentIterator_Next(iter), nullptr);
}

TEST(Regression, Bug1028)
{

    std::vector<std::vector<JPS_Point>> data{
        // CW ordered polygons
        {{103.2, 96.8}, {105.2, 96.8}, {105.2, 94.8}, {103.2, 92.8}},
        {{100, 96.8}, {100, 92.8}, {98, 94.8}, {98, 96.8}},
        // CCW ordered polygons
        {{100.0, 96.8}, {100.0, 92.8}, {103.2, 92.8}, {103.2, 96.8}},
        {{0, 96.8}, {0, 94.8}, {98, 94.8}, {98, 96.8}},
        {{105.2, 96.8}, {105.2, 94.8}, {200, 94.8}, {200, 96.8}}};
    auto geo_builder = JPS_GeometryBuilder_Create();

    for(auto& poly : data) {
        JPS_GeometryBuilder_AddAccessibleArea(geo_builder, poly.data(), poly.size());
    }
    auto geometry = JPS_GeometryBuilder_Build(geo_builder, nullptr);
    ASSERT_NE(geometry, nullptr);
    JPS_GeometryBuilder_Free(geo_builder);
}
