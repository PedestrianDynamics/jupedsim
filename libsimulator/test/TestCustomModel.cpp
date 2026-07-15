// SPDX-License-Identifier: LGPL-3.0-or-later
#include "GenericAgent.hpp"
#include "GeometryBuilder.hpp"
#include "NeighborhoodSearch.hpp"
#include "OperationalDecisionSystem.hpp"
#include "OperationalModels/AnticipationVelocityModel/AnticipationVelocityModel.hpp"
#include "OperationalModels/CollisionFreeSpeedModel/CollisionFreeSpeedModel.hpp"
#include "OperationalModels/CollisionFreeSpeedModelV2/CollisionFreeSpeedModelV2.hpp"
#include "OperationalModels/CollisionFreeSpeedModelV3/CollisionFreeSpeedModelV3.hpp"
#include "OperationalModels/CustomModel/CustomModel.hpp"
#include "OperationalModels/GeneralizedCentrifugalForceModel/GeneralizedCentrifugalForceModel.hpp"
#include "OperationalModels/OperationalModelState.hpp"
#include "OperationalModels/SocialForceModel/SocialForceModel.hpp"
#include "OperationalModels/WarpDriver/WarpDriverModel.hpp"

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
    void ComputeNextState(
        double dT,
        const GenericState& current,
        GenericState& next,
        const TacticalModelState&,
        const CollisionGeometry&,
        const StateContainer&) const override
    {
        const auto& currentStateData = std::get<CustomModel::State>(current);
        const auto& state = currentStateData.Get<MinimalState>();
        auto& nextStateData = std::get<CustomModel::State>(next);
        auto& nextState = nextStateData.Get<MinimalState>();

        nextStateData.position = currentStateData.position + state.velocity * dT;
        nextState.velocity = state.velocity;
        nextState.applications = state.applications + 1;
    }

    void CheckModelConstraint(
        const GenericAgent&,
        const NeighborhoodSearch<GenericAgent>&,
        const CollisionGeometry&) const override
    {
    }
};

GenericAgent MakeAgent(OperationalModelState state)
{
    return GenericAgent(
        GenericAgent::ID::Invalid,
        jps::UniqueID<Journey>::Invalid,
        jps::UniqueID<BaseStage>::Invalid,
        std::move(state));
}
} // namespace

TEST(CustomModel, TypeIsCustomModel)
{
    const MinimalCustomModel model{};
    ASSERT_EQ(model.Type(), OperationalModelType::CUSTOM_MODEL);
}

TEST(CustomModelState, StoresAndUpdatesTypedPayload)
{
    CustomModel::State data{7};

    ASSERT_EQ(data.Get<int>(), 7);

    data.Set(9);
    ASSERT_EQ(data.Get<int>(), 9);
    ASSERT_THROW((void) data.Get<double>(), std::bad_any_cast);
}

TEST(CustomModelState, FormatsPayload)
{
    const CustomModel::State data{std::string{"custom state"}};

    ASSERT_EQ(fmt::format("{}", data), "custom state");
}

TEST(CustomModelState, FormatsPayloadWithToString)
{
    ASSERT_EQ(fmt::format("{}", CustomModel::State{StringToStringPayload{}}), "string tostring");
    ASSERT_EQ(
        fmt::format("{}", CustomModel::State{StringViewToStringPayload{}}), "string_view tostring");
    ASSERT_EQ(
        fmt::format("{}", CustomModel::State{ConstCharPointerToStringPayload{}}),
        "const char pointer tostring");
}

TEST(CustomModel, FormatsAgentWithCustomModelState)
{
    const auto agent = MakeAgent(CustomModel::State{std::string{"custom state"}});

    ASSERT_NO_THROW((void) fmt::format("{}", agent));
}

TEST(CustomModel, RunsThroughOperationalDecisionSystem)
{
    GeometryBuilder builder{};
    builder.AddAccessibleArea({{-10, -10}, {10, -10}, {10, 10}, {-10, 10}});
    const auto geometry = builder.Build();

    AgentContainer<GenericAgent> agents{};
    agents.emplace_back(MakeAgent(CustomModel::State{MinimalState{Point{2.0, 0.0}, 0}}));

    NeighborhoodSearch<GenericAgent> neighborhoodSearch{2.2};
    neighborhoodSearch.Update(agents);

    OperationalDecisionSystem system{std::make_unique<MinimalCustomModel>()};
    system.Run(0.5, 0.0, neighborhoodSearch, geometry, agents);

    const auto& agent = agents.front();
    const auto& state = std::get<CustomModel::State>(agent.state).Get<MinimalState>();
    ASSERT_EQ(agent.position(), Point(1.0, 0.0));
    ASSERT_EQ(state.applications, 1);
}

TEST(ModelTypeOf, MapsEveryAgentModelDataToItsOperationalModelType)
{
    ASSERT_EQ(
        ModelTypeOf(OperationalModelState{GeneralizedCentrifugalForceModel::State{}}),
        OperationalModelType::GENERALIZED_CENTRIFUGAL_FORCE);
    ASSERT_EQ(
        ModelTypeOf(OperationalModelState{CollisionFreeSpeedModel::State{}}),
        OperationalModelType::COLLISION_FREE_SPEED);
    ASSERT_EQ(
        ModelTypeOf(OperationalModelState{CollisionFreeSpeedModelV2::State{}}),
        OperationalModelType::COLLISION_FREE_SPEED_V2);
    ASSERT_EQ(
        ModelTypeOf(OperationalModelState{CollisionFreeSpeedModelV3::State{}}),
        OperationalModelType::COLLISION_FREE_SPEED_V3);
    ASSERT_EQ(
        ModelTypeOf(OperationalModelState{AnticipationVelocityModel::State{}}),
        OperationalModelType::ANTICIPATION_VELOCITY_MODEL);
    ASSERT_EQ(
        ModelTypeOf(OperationalModelState{SocialForceModel::State{}}),
        OperationalModelType::SOCIAL_FORCE);
    ASSERT_EQ(
        ModelTypeOf(OperationalModelState{WarpDriverModel::State{}}),
        OperationalModelType::WARP_DRIVER);
    ASSERT_EQ(
        ModelTypeOf(OperationalModelState{CustomModel::State{MinimalState{}}}),
        OperationalModelType::CUSTOM_MODEL);
}
