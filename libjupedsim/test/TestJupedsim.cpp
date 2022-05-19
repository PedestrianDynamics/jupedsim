#include "Point.hpp"
#include <ErrorMessage.hpp>
#include <jupedsim/jupedsim.h>

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
    auto model = JPS_OperationalModel_Create_VelocityModel(1, 1, 1, 1, &errorMsg);
    EXPECT_NE(model, nullptr);
    EXPECT_EQ(errorMsg, nullptr);
}

TEST(OperationalModel, CanConstructGCFMModel)
{
    JPS_ErrorMessage errorMsg{};
    auto model = JPS_OperationalModel_Create_GCFMModel(1, 1, 1, 1, 1, 1, 1, 1, &errorMsg);
    EXPECT_NE(model, nullptr);
    EXPECT_EQ(errorMsg, nullptr);
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
    ;
    std::vector<double> box{18, 4, 20, 4, 20, 6, 18, 6};
    std::vector<const char*> labels{"exit"};
    JPS_AreasBuilder_AddArea(
        areas_builder, destinationId, box.data(), box.size() / 2, labels.data(), labels.size());

    auto areas = JPS_AreasBuilder_Build(areas_builder, nullptr);
    ASSERT_NE(areas, nullptr);

    auto model = JPS_OperationalModel_Create_VelocityModel(8, 0.1, 5, 0.02, nullptr);
    ASSERT_NE(model, nullptr);

    auto simulation = JPS_Simulation_Create(model, geometry, areas, 0.01, nullptr);
    ASSERT_NE(simulation, nullptr);

    JPS_AgentParameters agent_parameters{};
    agent_parameters.v0 = 1.0;
    agent_parameters.AMin = 0.15;
    agent_parameters.BMax = 0.15;
    agent_parameters.BMin = 0.15;
    agent_parameters.Av = 0.53;
    agent_parameters.T = 1;
    agent_parameters.Tau = 0.5;
    agent_parameters.destinationAreaId = destinationId;
    agent_parameters.orientationX = 1.0;
    agent_parameters.orientationY = 0.0;
    agent_parameters.positionX = 0.0;
    agent_parameters.positionY = 0.0;

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
