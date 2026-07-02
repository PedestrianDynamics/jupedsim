// SPDX-License-Identifier: LGPL-3.0-or-later
#include "GenericAgent.hpp"
#include "GeometryBuilder.hpp"
#include "NeighborhoodSearch.hpp"
#include "OperationalDecisionSystem.hpp"
#include "OperationalModels/CustomModel/CustomModel.hpp"
#include "OperationalModels/CustomModel/CustomModelData.hpp"
#include "OperationalModels/CustomModel/CustomModelUpdate.hpp"

#include <fmt/format.h>
#include <gtest/gtest.h>

#include <any>
#include <memory>
#include <string>
#include <string_view>

namespace
{
struct MinimalState {
    Point velocity{};
    int applications{};
};

struct MinimalUpdate {
    Point position{};
    Point orientation{};
    Point velocity{};
    int applications{};
};

struct StringToStringPayload {
    std::string ToString() const { return "string tostring"; }
};

struct StringViewToStringPayload {
    std::string_view ToString() const { return "string_view tostring"; }
};

struct ConstCharPointerToStringPayload {
    const char* ToString() const { return "const char pointer tostring"; }
};

class MinimalCustomModel : public CustomModel
{
public:
    OperationalModelUpdate ComputeNewPosition(
        double dT,
        const GenericAgent& agent,
        const CollisionGeometry&,
        const NeighborhoodSearch<GenericAgent>&) const override
    {
        const auto& state = std::get<CustomModelData>(agent.model).Get<MinimalState>();
        const auto position = agent.pos + state.velocity * dT;
        const auto orientation = state.velocity.Normalized();
        return CustomModelUpdate{
            MinimalUpdate{position, orientation, state.velocity, state.applications + 1}};
    }

    void ApplyUpdate(const OperationalModelUpdate& update, GenericAgent& agent) const override
    {
        const auto& customUpdate = std::get<CustomModelUpdate>(update).Get<MinimalUpdate>();
        auto& state = std::get<CustomModelData>(agent.model).Get<MinimalState>();

        agent.pos = customUpdate.position;
        state.velocity = customUpdate.velocity;
        state.applications = customUpdate.applications;
    }

    void CheckModelConstraint(
        const GenericAgent&,
        const NeighborhoodSearch<GenericAgent>&,
        const CollisionGeometry&) const override
    {
    }
};

GenericAgent MakeAgent(GenericAgent::Model model)
{
    return GenericAgent(
        GenericAgent::ID::Invalid,
        jps::UniqueID<Journey>::Invalid,
        jps::UniqueID<BaseStage>::Invalid,
        Point{0.0, 0.0},
        std::move(model));
}
} // namespace

TEST(CustomModel, TypeIsCustomModel)
{
    const MinimalCustomModel model{};
    ASSERT_EQ(model.Type(), OperationalModelType::CUSTOM_MODEL);
}

TEST(CustomModelData, StoresAndUpdatesTypedPayload)
{
    CustomModelData data{7};

    ASSERT_EQ(data.Get<int>(), 7);

    data.Set(9);
    ASSERT_EQ(data.Get<int>(), 9);
    ASSERT_THROW((void) data.Get<double>(), std::bad_any_cast);
}

TEST(CustomModelData, FormatsPayload)
{
    const CustomModelData data{std::string{"custom state"}};

    ASSERT_EQ(fmt::format("{}", data), "custom state");
}

TEST(CustomModelData, FormatsPayloadWithToString)
{
    ASSERT_EQ(fmt::format("{}", CustomModelData{StringToStringPayload{}}), "string tostring");
    ASSERT_EQ(
        fmt::format("{}", CustomModelData{StringViewToStringPayload{}}), "string_view tostring");
    ASSERT_EQ(
        fmt::format("{}", CustomModelData{ConstCharPointerToStringPayload{}}),
        "const char pointer tostring");
}

TEST(CustomModelUpdate, StoresAndUpdatesTypedPayload)
{
    CustomModelUpdate update{13};

    ASSERT_EQ(update.Get<int>(), 13);

    update.Set(17);
    ASSERT_EQ(update.Get<int>(), 17);
    ASSERT_THROW((void) update.Get<double>(), std::bad_any_cast);
}

TEST(CustomModelUpdate, FormatsPayload)
{
    const CustomModelUpdate update{std::string{"custom update"}};

    ASSERT_EQ(fmt::format("{}", update), "custom update");
}

TEST(CustomModelUpdate, FormatsPayloadWithToString)
{
    ASSERT_EQ(fmt::format("{}", CustomModelUpdate{StringToStringPayload{}}), "string tostring");
    ASSERT_EQ(
        fmt::format("{}", CustomModelUpdate{StringViewToStringPayload{}}), "string_view tostring");
    ASSERT_EQ(
        fmt::format("{}", CustomModelUpdate{ConstCharPointerToStringPayload{}}),
        "const char pointer tostring");
}

TEST(CustomModel, FormatsAgentWithCustomModelData)
{
    const auto agent = MakeAgent(CustomModelData{std::string{"custom state"}});

    ASSERT_NO_THROW((void) fmt::format("{}", agent));
}

TEST(CustomModel, RunsThroughOperationalDecisionSystem)
{
    GeometryBuilder builder{};
    builder.AddAccessibleArea({{-10, -10}, {10, -10}, {10, 10}, {-10, 10}});
    const auto geometry = builder.Build();

    AgentContainer<GenericAgent> agents{};
    agents.emplace_back(MakeAgent(CustomModelData{MinimalState{Point{2.0, 0.0}, 0}}));

    NeighborhoodSearch<GenericAgent> neighborhoodSearch{2.2};
    neighborhoodSearch.Update(agents);

    OperationalDecisionSystem system{std::make_unique<MinimalCustomModel>()};
    system.Run(0.5, 0.0, neighborhoodSearch, geometry, agents);

    const auto& agent = agents.front();
    const auto& state = std::get<CustomModelData>(agent.model).Get<MinimalState>();
    ASSERT_EQ(agent.pos, Point(1.0, 0.0));
    ASSERT_EQ(state.applications, 1);
}

TEST(ModelTypeOf, MapsEveryAgentModelDataToItsOperationalModelType)
{
    ASSERT_EQ(
        ModelTypeOf(GenericAgent::Model{GeneralizedCentrifugalForceModelData{}}),
        OperationalModelType::GENERALIZED_CENTRIFUGAL_FORCE);
    ASSERT_EQ(
        ModelTypeOf(GenericAgent::Model{CollisionFreeSpeedModelData{}}),
        OperationalModelType::COLLISION_FREE_SPEED);
    ASSERT_EQ(
        ModelTypeOf(GenericAgent::Model{CollisionFreeSpeedModelV2Data{}}),
        OperationalModelType::COLLISION_FREE_SPEED_V2);
    ASSERT_EQ(
        ModelTypeOf(GenericAgent::Model{CollisionFreeSpeedModelV3Data{}}),
        OperationalModelType::COLLISION_FREE_SPEED_V3);
    ASSERT_EQ(
        ModelTypeOf(GenericAgent::Model{AnticipationVelocityModelData{}}),
        OperationalModelType::ANTICIPATION_VELOCITY_MODEL);
    ASSERT_EQ(
        ModelTypeOf(GenericAgent::Model{SocialForceModelData{}}),
        OperationalModelType::SOCIAL_FORCE);
    ASSERT_EQ(
        ModelTypeOf(GenericAgent::Model{WarpDriverModelData{}}), OperationalModelType::WARP_DRIVER);
    ASSERT_EQ(
        ModelTypeOf(GenericAgent::Model{CustomModelData{MinimalState{}}}),
        OperationalModelType::CUSTOM_MODEL);
}
