// SPDX-License-Identifier: LGPL-3.0-or-later
#include "agent_rng.hpp"
#include "generic_agent.hpp"
#include "geometry/geometry.hpp"
#include "geometry_fixtures.hpp"
#include "kernel.hpp"
#include "neighborhood_search.hpp"
#include "operational_decision_system.hpp"
#include "operational_model_state.hpp"
#include "operational_models/anticipation_velocity_model/anticipation_velocity_model.hpp"
#include "operational_models/warp_driver/warp_driver_model.hpp"
#include "test_common.hpp"
#include "unique_id.hpp"

#include <boost/random/uniform_int_distribution.hpp>
#include <boost/random/uniform_real_distribution.hpp>
#include <gtest/gtest.h>

#include <cstddef>
#include <memory>
#include <vector>

namespace
{
/// Pairs of agents walking straight at each other, so that both models draw random numbers.
/// The agents take their streams by increasing index and are stored that way or the other way
/// round.
template <typename Model>
AgentContainer<GenericAgent> head_on_pairs(const Geometry& geometry, Model model, bool reverse)
{
    constexpr std::size_t count = 6;
    const auto heading = [](std::size_t index) { return index % 2 == 0 ? 1.0 : -1.0; };
    std::vector<OperationalModelState> states{};
    for(std::size_t index = 0; index < count; ++index) {
        typename Model::State state{};
        state.orientation = Vector{heading(index), 0.0};
        OperationalModelState seeded{state};
        model.seed_agent_state(seeded);
        states.push_back(seeded);
    }
    AgentContainer<GenericAgent> agents{};
    for(std::size_t offset = 0; offset < count; ++offset) {
        const std::size_t index = reverse ? count - 1 - offset : offset;
        const double row = static_cast<double>(index / 2);
        auto& agent = agents.emplace_back(
            GenericAgent::ID::invalid,
            jps::UniqueID<Journey>::invalid,
            jps::UniqueID<BaseStage>::invalid,
            *geometry.get_location_near_z(10.0 - heading(index), 2.0 + row * 8.0, 0.0),
            states[index]);
        agent.route_orientation = Vector{heading(index), 0.0};
    }
    return agents;
}

template <typename Model>
void expect_results_independent_of_step_order(const Model& model_a, const Model& model_b)
{
    auto room = test_geometries::rectangle({0, 0}, {20, 20});
    auto in_order = head_on_pairs(*room.geometry, model_a, false);
    auto reversed = head_on_pairs(*room.geometry, model_b, true);
    const auto initial = in_order;

    NeighborhoodSearch<GenericAgent> in_order_search{2.2};
    NeighborhoodSearch<GenericAgent> reversed_search{2.2};
    OperationalDecisionSystem in_order_system{std::make_unique<Model>(model_a)};
    OperationalDecisionSystem reversed_system{std::make_unique<Model>(model_b)};
    for(int step = 0; step < 10; ++step) {
        in_order_search.update(in_order);
        reversed_search.update(reversed);
        in_order_system.run(0.05, 0.0, in_order_search, *room.geometry, in_order);
        reversed_system.run(0.05, 0.0, reversed_search, *room.geometry, reversed);
    }

    for(std::size_t index = 0; index < in_order.size(); ++index) {
        const auto& first = in_order[index];
        const auto& second = reversed[in_order.size() - 1 - index];
        // Rounding may differ, as neighbors are summed in a different order. A draw that went
        // to another agent shows up far above that.
        EXPECT_NEAR(first.location.xy().x(), second.location.xy().x(), 1e-9);
        EXPECT_NEAR(first.location.xy().y(), second.location.xy().y(), 1e-9);
        const auto& first_rng = std::get<typename Model::State>(first.state).rng;
        EXPECT_TRUE(first_rng == std::get<typename Model::State>(second.state).rng);
        EXPECT_FALSE(first_rng == std::get<typename Model::State>(initial[index].state).rng)
            << "Agent " << index << " drew no random numbers, so this test shows nothing for it";
    }
}
} // namespace

TEST(AgentRng, FitsIntoTheAgentState)
{
    EXPECT_EQ(sizeof(AgentRng), 16u);
}

TEST(AgentRng, TheSameSeedHandsOutTheSameStreams)
{
    AgentRngSeeder a{42};
    AgentRngSeeder b{42};
    for(int agent = 0; agent < 3; ++agent) {
        auto stream_a = a.next();
        auto stream_b = b.next();
        for(int draw = 0; draw < 100; ++draw) {
            EXPECT_EQ(stream_a(), stream_b());
        }
    }
}

TEST(AgentRng, EveryAgentAndEverySeedGetAnotherStream)
{
    AgentRngSeeder sequence{42};
    EXPECT_FALSE(sequence.next() == sequence.next());
    EXPECT_FALSE(AgentRngSeeder{42}.next() == AgentRngSeeder{43}.next());
}

TEST(AgentRng, TheHighBitsOfTheSeedCount)
{
    EXPECT_FALSE(AgentRngSeeder{0xFFFFFFFF00000000ULL}.next() == AgentRngSeeder{0}.next());
}

// The values below are pinned on purpose: the same seed has to give the same numbers on every
// platform and compiler. If one of these fails, simulations stopped being reproducible.
TEST(AgentRng, RawDrawsArePinned)
{
    AgentRngSeeder sequence{42};
    auto first_agent = sequence.next();
    EXPECT_EQ(first_agent(), 3706242782U);
    EXPECT_EQ(first_agent(), 3423337462U);
    EXPECT_EQ(first_agent(), 1593601427U);
    EXPECT_EQ(first_agent(), 917066193U);

    auto second_agent = sequence.next();
    EXPECT_EQ(second_agent(), 368565505U);
    EXPECT_EQ(second_agent(), 1488804179U);
}

TEST(AgentRng, DistributionsArePinned)
{
    auto rng = AgentRngSeeder{42}.next();
    boost::random::uniform_real_distribution<double> real(-0.05, 0.05);
    EXPECT_DOUBLE_EQ(real(rng), 0.03629268924705685);
    EXPECT_DOUBLE_EQ(real(rng), 0.029705786472186455);
    EXPECT_DOUBLE_EQ(real(rng), -0.012896075402386485);

    rng = AgentRngSeeder{42}.next();
    boost::random::uniform_int_distribution<int> coin(0, 1);
    const std::vector<int> expected{1, 1, 0, 0, 0, 0, 0, 1};
    for(const int value : expected) {
        EXPECT_EQ(coin(rng), value);
    }
}

TEST(AgentRng, WarpDriverResultsDoNotDependOnTheStepOrder)
{
    const WarpDriverModel model{0.3};
    expect_results_independent_of_step_order(model, model);
}

TEST(AgentRng, AnticipationVelocityResultsDoNotDependOnTheStepOrder)
{
    const AnticipationVelocityModel model{0.3, 42};
    expect_results_independent_of_step_order(model, model);
}
