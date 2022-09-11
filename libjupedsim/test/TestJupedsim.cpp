#include "AgentIterator.hpp"
#include "Point.hpp"
#include <ErrorMessage.hpp>
#include <jupedsim/jupedsim.h>

#include <gtest/gtest.h>
#include <vector>

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
    JPS_VelocityModelBuilder_AddParameterProfile(builder, 1, 1, 1);
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
    JPS_GCFMModelBuilder_AddParameterProfile(builder, 1, 1, 1, 1);
    auto model = JPS_GCFMModelBuilder_Build(builder, &errorMsg);
    EXPECT_NE(model, nullptr);
    EXPECT_EQ(errorMsg, nullptr);
    JPS_GCFMModelBuilder_Free(builder);
    JPS_OperationalModel_Free(model);
}

TEST(GeometryBuilder, CanConstruct)
{
    auto builder = JPS_GeometryBuilder_Create();

    std::vector<double> box1{0, 0, 1, 0, 1, 1, 0, 1};
    JPS_GeometryBuilder_AddAccessibleArea(builder, box1.data(), box1.size() / 2);

    std::vector<double> box2{1, 0, 2, 0, 2, 1, 1, 1};
    JPS_GeometryBuilder_AddAccessibleArea(builder, box2.data(), box2.size() / 2);

    std::vector<double> box3{0, 0.25, 2, 0.25, 2, 0.75, 0, 0.75};
    JPS_GeometryBuilder_AddAccessibleArea(builder, box3.data(), box3.size() / 2);

    std::vector<double> box4{0, 0, 0.2, 0, 0.2, 0.2, 0, 0.2};
    JPS_GeometryBuilder_ExcludeFromAccessibleArea(builder, box4.data(), box4.size() / 2);

    JPS_ErrorMessage message{};
    auto geometry = JPS_GeometryBuilder_Build(builder, &message);
    ASSERT_NE(geometry, nullptr);
    ASSERT_EQ(message, nullptr);
    ASSERT_NO_FATAL_FAILURE(JPS_Geometry_Free(geometry));
    ASSERT_NO_FATAL_FAILURE(JPS_GeometryBuilder_Free(builder));
}

TEST(Areas, CanConstruct)
{
    auto builder = JPS_AreasBuilder_Create();

    std::vector<double> box{0, 0, 1, 0, 1, 1, 0, 1};
    std::vector<const char*> labels{"exit", "carpet", "deathtrap"};
    JPS_AreasBuilder_AddArea(builder, 1, box.data(), box.size() / 2, labels.data(), labels.size());

    JPS_ErrorMessage message{};
    auto areas = JPS_AreasBuilder_Build(builder, &message);
    ASSERT_NE(areas, nullptr);
    ASSERT_EQ(message, nullptr);
    ASSERT_NO_FATAL_FAILURE(JPS_Areas_Free(areas));
    ASSERT_NO_FATAL_FAILURE(JPS_AreasBuilder_Free(builder));
}

TEST(Simulation, CanSimulate)
{
    auto geo_builder = JPS_GeometryBuilder_Create();

    std::vector<double> box1{0, 0, 10, 0, 10, 10, 0, 10};
    JPS_GeometryBuilder_AddAccessibleArea(geo_builder, box1.data(), box1.size() / 2);

    std::vector<double> box2{10, 4, 20, 4, 20, 6, 10, 6};
    JPS_GeometryBuilder_AddAccessibleArea(geo_builder, box2.data(), box2.size() / 2);

    auto geometry = JPS_GeometryBuilder_Build(geo_builder, nullptr);
    ASSERT_NE(geometry, nullptr);

    auto areas_builder = JPS_AreasBuilder_Create();

    const uint16_t destinationId = 1;
    std::vector<double> box{18, 4, 20, 4, 20, 6, 18, 6};
    std::vector<const char*> labels{"exit"};
    JPS_AreasBuilder_AddArea(
        areas_builder, destinationId, box.data(), box.size() / 2, labels.data(), labels.size());

    auto areas = JPS_AreasBuilder_Build(areas_builder, nullptr);
    ASSERT_NE(areas, nullptr);

    auto modelBuilder = JPS_VelocityModelBuilder_Create(8, 0.1, 5, 0.02);
    const JPS_ModelParameterProfileId profile_id = 1;
    JPS_VelocityModelBuilder_AddParameterProfile(modelBuilder, profile_id, 1, 0.5);

    auto model = JPS_VelocityModelBuilder_Build(modelBuilder, nullptr);
    ASSERT_NE(model, nullptr);

    auto simulation = JPS_Simulation_Create(model, geometry, areas, 0.01, nullptr);
    ASSERT_NE(simulation, nullptr);

    JPS_Waypoint waypoints[] = {{{19, 5}, 1}};
    auto journey = JPS_Journey_Create_SimpleJourney(waypoints, sizeof(waypoints));
    auto journeyId = JPS_Simulation_AddJourney(simulation, journey, nullptr);

    JPS_AgentParameters agent_parameters{};
    agent_parameters.v0 = 1.0;
    agent_parameters.AMin = 0.15;
    agent_parameters.BMax = 0.15;
    agent_parameters.BMin = 0.15;
    agent_parameters.Av = 0.53;
    agent_parameters.journeyId = journeyId;
    agent_parameters.orientationX = 1.0;
    agent_parameters.orientationY = 0.0;
    agent_parameters.positionX = 0.0;
    agent_parameters.positionY = 0.0;
    agent_parameters.profileId = profile_id;

    std::vector<Point> positions{Point(7, 7), Point(1, 3), Point(1, 5), Point(1, 7), Point(2, 7)};
    for(const auto& p : positions) {
        agent_parameters.positionX = p.x;
        agent_parameters.positionY = p.y;
        JPS_Simulation_AddAgent(simulation, agent_parameters, nullptr);
    }
    while(JPS_Simulation_AgentCount(simulation) > 0) {
        ASSERT_TRUE(JPS_Simulation_Iterate(simulation, nullptr));
    }
    ASSERT_LT(JPS_Simulation_IterationCount(simulation), 2000);
}

struct SimulationTest : public ::testing::Test {
    JPS_Simulation simulation{};
    JPS_JourneyId journey_id{};
    JPS_ModelParameterProfileId model_paramater_profile_id{1};

    void SetUp() override
    {
        auto geo_builder = JPS_GeometryBuilder_Create();
        std::vector<double> box1{0, 0, 10, 0, 10, 10, 0, 10};
        JPS_GeometryBuilder_AddAccessibleArea(geo_builder, box1.data(), box1.size() / 2);
        auto geometry = JPS_GeometryBuilder_Build(geo_builder, nullptr);
        ASSERT_NE(geometry, nullptr);
        JPS_GeometryBuilder_Free(geo_builder);

        auto areas_builder = JPS_AreasBuilder_Create();

        const uint16_t destinationId = 1;
        std::vector<double> box{18, 4, 20, 4, 20, 6, 18, 6};
        std::vector<const char*> labels{"exit"};
        JPS_AreasBuilder_AddArea(
            areas_builder, destinationId, box.data(), box.size() / 2, labels.data(), labels.size());

        auto areas = JPS_AreasBuilder_Build(areas_builder, nullptr);
        ASSERT_NE(areas, nullptr);
        JPS_AreasBuilder_Free(areas_builder);

        auto modelBuilder = JPS_VelocityModelBuilder_Create(9, 0.1, 5, 0.02);
        JPS_VelocityModelBuilder_AddParameterProfile(
            modelBuilder, model_paramater_profile_id, 1, 1);
        auto model = JPS_VelocityModelBuilder_Build(modelBuilder, nullptr);

        ASSERT_NE(model, nullptr);

        simulation = JPS_Simulation_Create(model, geometry, areas, 0.01, nullptr);
        ASSERT_NE(simulation, nullptr);

        const std::vector<JPS_Waypoint> waypoints{{{1, 1}, 1}};
        auto journey = JPS_Journey_Create_SimpleJourney(waypoints.data(), waypoints.size());
        journey_id = JPS_Simulation_AddJourney(simulation, journey, nullptr);
        ASSERT_NE(journey_id, 0);

        JPS_OperationalModel_Free(model);
        JPS_Geometry_Free(geometry);
        JPS_Areas_Free(areas);
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
    std::vector<JPS_AgentParameters> agent_parameters{
        {1.0, 1.0, 1.0, 1.0, 1.0, 1, 0.5, 0.53, 0.15, journey_id, model_paramater_profile_id},
        {2.0, 1.0, 1.0, 1.0, 1.0, 1, 0.5, 0.53, 0.15, journey_id, model_paramater_profile_id},
        {3.0, 1.0, 1.0, 1.0, 1.0, 1, 0.5, 0.53, 0.15, journey_id, model_paramater_profile_id}};
    for(const auto& agent_params : agent_parameters) {
        ASSERT_NE(JPS_Simulation_AddAgent(simulation, agent_params, nullptr), 0);
    }
    ASSERT_EQ(JPS_Simulation_AgentCount(simulation), 3);
    auto iter = JPS_Simulation_AgentIterator(simulation);
    ASSERT_NE(iter, nullptr);
    ASSERT_NE(JPS_AgentIterator_Next(iter), nullptr);
    ASSERT_NE(JPS_AgentIterator_Next(iter), nullptr);
    ASSERT_NE(JPS_AgentIterator_Next(iter), nullptr);
    ASSERT_EQ(JPS_AgentIterator_Next(iter), nullptr);
}

TEST_F(SimulationTest, CanQueryAgentProperties)
{
    const double pos_x = 1.123;
    const double pos_y = 0.999;
    const double orientation_x = 1;
    const double orientation_y = 0;
    JPS_AgentParameters agent_params{
        pos_x,
        pos_y,
        orientation_x,
        orientation_y,
        1.0,
        1,
        0.5,
        0.53,
        0.15,
        journey_id,
        model_paramater_profile_id};
    const auto agent_id = JPS_Simulation_AddAgent(simulation, agent_params, nullptr);
    ASSERT_NE(agent_id, 0);
    ASSERT_EQ(JPS_Simulation_AgentCount(simulation), 1);
    auto iter = JPS_Simulation_AgentIterator(simulation);
    ASSERT_NE(iter, nullptr);
    const auto agent = JPS_AgentIterator_Next(iter);
    ASSERT_NE(agent, nullptr);
    ASSERT_EQ(JPS_AgentIterator_Next(iter), nullptr);

    ASSERT_EQ(JPS_Agent_Id(agent), agent_id);
    ASSERT_EQ(JPS_Agent_PositionX(agent), pos_x);
    ASSERT_EQ(JPS_Agent_PositionY(agent), pos_y);
    ASSERT_EQ(JPS_Agent_OrientationX(agent), orientation_x);
    ASSERT_EQ(JPS_Agent_OrientationY(agent), orientation_y);
}
