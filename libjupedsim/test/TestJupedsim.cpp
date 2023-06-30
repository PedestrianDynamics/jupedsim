/// Copyright © 2012-2022 Forschungszentrum Jülich GmbH
/// SPDX-License-Identifier: LGPL-3.0-or-later
#include "AgentIterator.hpp"
#include "Point.hpp"
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

TEST(OperationalModel, CanConstructVelocityModel)
{
    JPS_ErrorMessage errorMsg{};
    auto builder = JPS_VelocityModelBuilder_Create(1, 1, 1, 1);
    JPS_VelocityModelBuilder_AddParameterProfile(builder, 1, 1, 1, 1, 0.3);
    auto model = JPS_VelocityModelBuilder_Build(builder, &errorMsg);
    EXPECT_NE(model, nullptr);
    EXPECT_EQ(errorMsg, nullptr);
    JPS_VelocityModelBuilder_Free(builder);
    JPS_OperationalModel_Free(model);
}

TEST(OperationalModel, CanConstructGCFMModel)
{
    JPS_ErrorMessage errorMsg{};
    auto builder = JPS_GCFMModelBuilder_Create(1, 1, 1, 1, 1, 1, 1, 1);
    JPS_GCFMModelBuilder_AddParameterProfile(builder, 1, 1, 0.5, 1.2, 0.53, 0.15, 0.15, 0.15);
    auto model = JPS_GCFMModelBuilder_Build(builder, &errorMsg);
    EXPECT_NE(model, nullptr);
    EXPECT_EQ(errorMsg, nullptr);
    JPS_GCFMModelBuilder_Free(builder);
    JPS_OperationalModel_Free(model);
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

    auto modelBuilder = JPS_VelocityModelBuilder_Create(8, 0.1, 5, 0.02);
    const JPS_ModelParameterProfileId profile_id = 1;
    JPS_VelocityModelBuilder_AddParameterProfile(modelBuilder, profile_id, 1, 0.5, 1.2, 0.3);

    auto model = JPS_VelocityModelBuilder_Build(modelBuilder, nullptr);
    ASSERT_NE(model, nullptr);

    auto simulation = JPS_Simulation_Create(model, geometry, 0.01, nullptr);
    ASSERT_NE(simulation, nullptr);

    auto journey = JPS_JourneyDescription_Create();
    std::vector<JPS_Point> box{{18, 4}, {20, 4}, {20, 6}, {18, 6}};
    auto success =
        JPS_JourneyDescription_AddExit(journey, box.data(), box.size(), nullptr, nullptr);
    ASSERT_TRUE(success);
    auto journeyId = JPS_Simulation_AddJourney(simulation, journey, nullptr);
    JPS_JourneyDescription_Free(journey);

    JPS_VelocityModelAgentParameters agent_parameters{};
    agent_parameters.journeyId = journeyId;
    agent_parameters.orientation = JPS_Point{1.0, 0.0};
    agent_parameters.position = JPS_Point{0.0, 0.0};
    agent_parameters.profileId = profile_id;

    std::vector<JPS_Point> positions{{7, 7}, {1, 3}, {1, 5}, {1, 7}, {2, 7}};
    for(const auto& p : positions) {
        agent_parameters.position = p;
        JPS_Simulation_AddVelocityModelAgent(simulation, agent_parameters, nullptr);
    }
    while(JPS_Simulation_AgentCount(simulation) > 0) {
        ASSERT_TRUE(JPS_Simulation_Iterate(simulation, nullptr));
    }
    ASSERT_LT(JPS_Simulation_IterationCount(simulation), 2000);
}

struct SimulationTest : public ::testing::Test {
    JPS_Simulation simulation{};
    JPS_JourneyId journey_id{};
    std::array<JPS_ModelParameterProfileId, 2> model_paramater_profile_id{1, 2};

    void SetUp() override
    {
        auto geo_builder = JPS_GeometryBuilder_Create();
        std::vector<JPS_Point> box1{{0, 0}, {10, 0}, {10, 10}, {0, 10}};
        JPS_GeometryBuilder_AddAccessibleArea(geo_builder, box1.data(), box1.size());
        auto geometry = JPS_GeometryBuilder_Build(geo_builder, nullptr);
        ASSERT_NE(geometry, nullptr);
        JPS_GeometryBuilder_Free(geo_builder);

        auto modelBuilder = JPS_VelocityModelBuilder_Create(9, 0.1, 5, 0.02);
        JPS_VelocityModelBuilder_AddParameterProfile(
            modelBuilder, model_paramater_profile_id[0], 1, 0.5, 1.5, 0.3);
        JPS_VelocityModelBuilder_AddParameterProfile(
            modelBuilder, model_paramater_profile_id[1], 1, 0.3, 1.2, 0.13);
        auto model = JPS_VelocityModelBuilder_Build(modelBuilder, nullptr);

        ASSERT_NE(model, nullptr);

        simulation = JPS_Simulation_Create(model, geometry, 0.01, nullptr);
        ASSERT_NE(simulation, nullptr);

        auto journey = JPS_JourneyDescription_Create();
        ASSERT_TRUE(JPS_JourneyDescription_AddWaypoint(journey, {1, 1}, 1, nullptr, nullptr));
        journey_id = JPS_Simulation_AddJourney(simulation, journey, nullptr);

        JPS_JourneyDescription_Free(journey);
        ASSERT_NE(journey_id, 0);

        JPS_OperationalModel_Free(model);
        JPS_Geometry_Free(geometry);
    }

    void TearDown() override { JPS_Simulation_Free(simulation); }
};

TEST_F(SimulationTest, AgentIteratorIsEmptyForNewSimulation)
{
    ASSERT_EQ(JPS_Simulation_AgentCount(simulation), 0);
    auto iter = JPS_Simulation_VelocityModelAgentIterator(simulation);
    ASSERT_NE(iter, nullptr);
    ASSERT_EQ(JPS_VelocityModelAgentIterator_Next(iter), nullptr);
}

TEST_F(SimulationTest, AgentIteratorCanIterate)
{
    std::vector<JPS_VelocityModelAgentParameters> agent_parameters{
        {{}, {1.0, 1.0}, {1.0, 1.0}, journey_id, model_paramater_profile_id[0], 0},
        {{}, {2.0, 1.0}, {1.0, 1.0}, journey_id, model_paramater_profile_id[0], 0},
        {{}, {3.0, 1.0}, {1.0, 1.0}, journey_id, model_paramater_profile_id[0], 0}};
    for(const auto& agent_params : agent_parameters) {
        ASSERT_NE(JPS_Simulation_AddVelocityModelAgent(simulation, agent_params, nullptr), 0);
    }
    ASSERT_EQ(JPS_Simulation_AgentCount(simulation), 3);
    auto iter = JPS_Simulation_VelocityModelAgentIterator(simulation);
    ASSERT_NE(iter, nullptr);
    ASSERT_NE(JPS_VelocityModelAgentIterator_Next(iter), nullptr);
    ASSERT_NE(JPS_VelocityModelAgentIterator_Next(iter), nullptr);
    ASSERT_NE(JPS_VelocityModelAgentIterator_Next(iter), nullptr);
    ASSERT_EQ(JPS_VelocityModelAgentIterator_Next(iter), nullptr);
}

TEST_F(SimulationTest, CanChangeModelParameterProfiles)
{
    JPS_VelocityModelAgentParameters agent_params{
        {}, {1, 1}, {1, 0}, journey_id, model_paramater_profile_id[0], 0};
    const auto agent_id = JPS_Simulation_AddVelocityModelAgent(simulation, agent_params, nullptr);
    ASSERT_NE(agent_id, 0);
    ASSERT_EQ(JPS_Simulation_AgentCount(simulation), 1);
    JPS_ErrorMessage errorMsg{};
    EXPECT_EQ(JPS_Simulation_Iterate(simulation, &errorMsg), true);
    if(errorMsg) {
        std::cerr << JPS_ErrorMessage_GetMessage(errorMsg) << std::endl;
        JPS_ErrorMessage_Free(errorMsg);
    }
    ASSERT_EQ(
        JPS_Simulation_SwitchAgentProfile(
            simulation, agent_id, model_paramater_profile_id[1], nullptr),
        true);
    ASSERT_EQ(JPS_Simulation_Iterate(simulation, nullptr), true);
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
