// SPDX-License-Identifier: LGPL-3.0-or-later
#include "AgentView.hpp"
#include "GenericAgent.hpp"
#include "GeometryFixtures.hpp"
#include "OperationalDecisionSystem.hpp"
#include "OperationalModels/CustomModel/CustomModel.hpp"
#include "TestCommon.hpp"

#include <fmt/format.h>
#include <gtest/gtest.h>

#include <any>
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>

namespace
{
struct MinimalState {
    Point velocity{};
    int applications{};
};

struct StringToStringPayload {
    std::string to_string() const { return "string tostring"; }
};

struct StringViewToStringPayload {
    std::string_view to_string() const { return "string_view tostring"; }
};

struct ConstCharPointerToStringPayload {
    const char* to_string() const { return "const char pointer tostring"; }
};

class MinimalCustomModel : public CustomModel
{
public:
    Point compute_next_state(
        const OperationalModelState& current,
        OperationalModelState& next,
        const AgentStep& step) const override
    {
        const auto& current_model_data = std::get<CustomModel::State>(current);
        const auto& state = current_model_data.get<MinimalState>();
        auto& next_model_data = std::get<CustomModel::State>(next);
        auto& next_state = next_model_data.get<MinimalState>();

        next_state.velocity = state.velocity;
        next_state.applications = state.applications + 1;
        return state.velocity * step.dt();
    }

    void check_model_constraint(const GenericAgent&, const AgentView&) const override {}
};

/// The flat square every agent in this file stands on.
const Geometry& flat_square()
{
    static const auto geometry = test_geometries::rectangle({-10, -10}, {10, 10});
    return *geometry;
}

GenericAgent make_agent(OperationalModelState model, Point position = {})
{
    return GenericAgent(
        GenericAgent::ID::invalid,
        jps::UniqueID<Journey>::invalid,
        jps::UniqueID<BaseStage>::invalid,
        *flat_square().get_location_near_z(position.x, position.y, 0.0),
        std::move(model));
}
} // namespace

TEST(CustomModel, TypeIsCustomModel)
{
    const MinimalCustomModel model{};
    ASSERT_EQ(model.type(), OperationalModelType::CustomModel);
}

TEST(CustomModelState, DoesNotSwallowArbitraryTypesImplicitly)
{
    // Ensure only explicit construction of CustomModelState and no implicit conversion
    // from any type.
    EXPECT_FALSE((std::is_convertible_v<Point, CustomModel::State>) );
    EXPECT_FALSE((std::is_convertible_v<Point, OperationalModelState>) );
    EXPECT_FALSE((std::is_convertible_v<GenericAgent, OperationalModelState>) );

    EXPECT_TRUE((std::is_constructible_v<CustomModel::State, int>) );
}

TEST(CustomModelState, StoresAndUpdatesTypedPayload)
{
    CustomModel::State data{7};

    ASSERT_EQ(data.get<int>(), 7);

    data.set(9);
    ASSERT_EQ(data.get<int>(), 9);
    ASSERT_THROW((void) data.get<double>(), std::bad_any_cast);
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
    const auto agent = make_agent(CustomModel::State{std::string{"custom state"}});

    ASSERT_NO_THROW((void) fmt::format("{}", agent));
}

TEST(CustomModel, RunsThroughOperationalDecisionSystem)
{
    AgentContainer<GenericAgent> agents{};
    agents.emplace_back(make_agent(CustomModel::State{MinimalState{Point{2.0, 0.0}, 0}}));

    NeighborhoodSearch<GenericAgent> neighborhood_search{2.2};
    neighborhood_search.update(agents);

    OperationalDecisionSystem system{std::make_unique<MinimalCustomModel>()};
    system.run(0.5, 0.0, neighborhood_search, flat_square(), agents);

    const auto& agent = agents.front();
    const auto& state = std::get<CustomModel::State>(agent.state).get<MinimalState>();
    ASSERT_EQ(agent.location.xy(), Point(1.0, 0.0));
    ASSERT_EQ(state.applications, 1);
}

TEST(ModelTypeOf, MapsEveryAgentModelDataToItsOperationalModelType)
{
    ASSERT_EQ(
        model_type_of(OperationalModelState{GeneralizedCentrifugalForceModelState{}}),
        OperationalModelType::GeneralizedCentrifugalForce);
    ASSERT_EQ(
        model_type_of(OperationalModelState{CollisionFreeSpeedModelState{}}),
        OperationalModelType::CollisionFreeSpeed);
    ASSERT_EQ(
        model_type_of(OperationalModelState{CollisionFreeSpeedModelV2State{}}),
        OperationalModelType::CollisionFreeSpeedV2);
    ASSERT_EQ(
        model_type_of(OperationalModelState{CollisionFreeSpeedModelV3State{}}),
        OperationalModelType::CollisionFreeSpeedV3);
    ASSERT_EQ(
        model_type_of(OperationalModelState{AnticipationVelocityModelState{}}),
        OperationalModelType::AnticipationVelocityModel);
    ASSERT_EQ(
        model_type_of(OperationalModelState{SocialForceModelState{}}),
        OperationalModelType::SocialForce);
    ASSERT_EQ(
        model_type_of(OperationalModelState{WarpDriverModelState{}}),
        OperationalModelType::WarpDriver);
    ASSERT_EQ(
        model_type_of(OperationalModelState{CustomModelState{MinimalState{}}}),
        OperationalModelType::CustomModel);
}
