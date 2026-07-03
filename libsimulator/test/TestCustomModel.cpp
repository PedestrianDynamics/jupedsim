// SPDX-License-Identifier: LGPL-3.0-or-later
#include "GenericAgent.hpp"
#include "GeometryBuilder.hpp"
#include "NeighborhoodSearch.hpp"
#include "OperationalDecisionSystem.hpp"
#include "OperationalModels/CustomModel/CustomModel.hpp"

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
        const GenericAgent& current,
        GenericAgent& next,
        const CollisionGeometry&,
        const NeighborhoodSearch<GenericAgent>&) const override
    {
        const auto& state = std::get<CustomModel::State>(current.model).Get<MinimalState>();
        auto& nextState = std::get<CustomModel::State>(next.model).Get<MinimalState>();

        next.pos = current.pos + state.velocity * dT;
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

GenericAgent MakeAgent(GenericAgent::ModelState model)
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
    const auto& state = std::get<CustomModel::State>(agent.model).Get<MinimalState>();
    ASSERT_EQ(agent.pos, Point(1.0, 0.0));
    ASSERT_EQ(state.applications, 1);
}

TEST(ModelTypeOf, MapsEveryAgentModelDataToItsOperationalModelType)
{
    ASSERT_EQ(
        ModelTypeOf(GenericAgent::ModelState{GeneralizedCentrifugalForceModel::State{}}),
        OperationalModelType::GENERALIZED_CENTRIFUGAL_FORCE);
    ASSERT_EQ(
        ModelTypeOf(GenericAgent::ModelState{CollisionFreeSpeedModel::State{}}),
        OperationalModelType::COLLISION_FREE_SPEED);
    ASSERT_EQ(
        ModelTypeOf(GenericAgent::ModelState{CollisionFreeSpeedModelV2::State{}}),
        OperationalModelType::COLLISION_FREE_SPEED_V2);
    ASSERT_EQ(
        ModelTypeOf(GenericAgent::ModelState{CollisionFreeSpeedModelV3::State{}}),
        OperationalModelType::COLLISION_FREE_SPEED_V3);
    ASSERT_EQ(
        ModelTypeOf(GenericAgent::ModelState{AnticipationVelocityModel::State{}}),
        OperationalModelType::ANTICIPATION_VELOCITY_MODEL);
    ASSERT_EQ(
        ModelTypeOf(GenericAgent::ModelState{SocialForceModel::State{}}),
        OperationalModelType::SOCIAL_FORCE);
    ASSERT_EQ(
        ModelTypeOf(GenericAgent::ModelState{WarpDriverModel::State{}}),
        OperationalModelType::WARP_DRIVER);
    ASSERT_EQ(
        ModelTypeOf(GenericAgent::ModelState{CustomModel::State{MinimalState{}}}),
        OperationalModelType::CUSTOM_MODEL);
}
