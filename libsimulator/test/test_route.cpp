// SPDX-License-Identifier: LGPL-3.0-or-later
#include "generic_agent.hpp"
#include "geometry/geometry.hpp"
#include "geometry_fixtures.hpp"
#include "kernel.hpp"
#include "operational_models/collision_free_speed_model/collision_free_speed_model.hpp"
#include "route.hpp"
#include "routing_engine.hpp"
#include "routing_target.hpp"
#include "simulation_error.hpp"
#include "stage.hpp"
#include "test_common.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <tuple>
#include <vector>

namespace
{
/// An agent on a flat room together with the routing engine of that room.
struct RuleContextFixture {
    std::unique_ptr<Geometry> geometry = test_geometries::rectangle({0, 0}, {10, 10}).geometry;
    RoutingEngine routing{*geometry, FloorfieldOptions{}};
    GenericAgent agent{
        GenericAgent::ID::invalid,
        jps::UniqueID<Route>::invalid,
        jps::UniqueID<BaseStage>::invalid,
        *geometry->get_location_near_z(5, 5, 0),
        CollisionFreeSpeedModel::State{}};
    RuleContext ctx{agent, routing};
};
} // namespace

TEST(FixedRule, NextIsCorrect)
{
    RuleContextFixture fx;
    int stage;

    FixedRule sut(reinterpret_cast<BaseStage*>(&stage));

    for(auto i = 0; i < 20; ++i) {
        ASSERT_EQ(reinterpret_cast<BaseStage*>(&stage), sut.next_stage(fx.ctx));
    }
}

TEST(RoundRobin, SimpleNextIsCorrect)
{
    RuleContextFixture fx;
    int stage1;
    int stage2;
    int stage3;

    std::vector<std::tuple<BaseStage*, uint64_t>> weighted_stages = {
        {reinterpret_cast<BaseStage*>(&stage1), 1},
        {reinterpret_cast<BaseStage*>(&stage2), 1},
        {reinterpret_cast<BaseStage*>(&stage3), 1}};

    RoundRobin sut(weighted_stages);

    for(auto i = 0; i < 5; ++i) {
        for(auto const& [stage, _] : weighted_stages) {
            ASSERT_EQ(stage, sut.next_stage(fx.ctx));
        }
    }
}

TEST(RoundRobin, WeightedRoundRobin)
{
    RuleContextFixture fx;
    int stage1;
    int stage2;
    int stage3;

    std::vector<std::tuple<BaseStage*, uint64_t>> weighted_stages = {
        {reinterpret_cast<BaseStage*>(&stage1), 1},
        {reinterpret_cast<BaseStage*>(&stage2), 2},
        {reinterpret_cast<BaseStage*>(&stage3), 3}};

    RoundRobin sut(weighted_stages);

    for(auto i = 0; i < 5; ++i) {
        ASSERT_EQ(std::get<0>(weighted_stages[0]), sut.next_stage(fx.ctx));

        ASSERT_EQ(std::get<0>(weighted_stages[1]), sut.next_stage(fx.ctx));
        ASSERT_EQ(std::get<0>(weighted_stages[1]), sut.next_stage(fx.ctx));

        ASSERT_EQ(std::get<0>(weighted_stages[2]), sut.next_stage(fx.ctx));
        ASSERT_EQ(std::get<0>(weighted_stages[2]), sut.next_stage(fx.ctx));
        ASSERT_EQ(std::get<0>(weighted_stages[2]), sut.next_stage(fx.ctx));
    }
}

TEST(RoundRobin, ZeroWeightGivesException)
{
    RuleContextFixture fx;
    int stage1;
    std::vector<std::tuple<BaseStage*, uint64_t>> weighted_stages = {
        {reinterpret_cast<BaseStage*>(&stage1), 0}};

    ASSERT_THROW(RoundRobin sut(weighted_stages), SimulationError);
}

TEST(LeastTargeted, NextIsCorrect)
{
    RuleContextFixture fx;
    class MockStage : public BaseStage
    {
    public:
        MockStage(size_t targeting)
        {
            _targeting = targeting;
            ON_CALL(*this, count_targeting).WillByDefault([this]() { return _targeting; });
            ON_CALL(*this, is_completed).WillByDefault([]() { return true; });
        }
        MOCK_METHOD(size_t, count_targeting, (), (const));
        MOCK_METHOD(bool, is_completed, (const GenericAgent& agent), (override));
        MOCK_METHOD(RoutingTarget, target, (const GenericAgent& agent), (override));
        void set_targeting(size_t targeting) { _targeting = targeting; }
    };

    MockStage mockstage1(3);
    MockStage mockstage2(2);
    MockStage mockstage3(1);

    std::vector<BaseStage*> stages = {&mockstage1, &mockstage2, &mockstage3};
    LeastTargeted sut(stages);

    ASSERT_EQ(&mockstage3, sut.next_stage(fx.ctx));

    mockstage1.set_targeting(1);
    mockstage2.set_targeting(1);
    mockstage3.set_targeting(1);
    ASSERT_EQ(&mockstage1, sut.next_stage(fx.ctx));

    mockstage1.set_targeting(5);
    mockstage2.set_targeting(1);
    mockstage3.set_targeting(5);
    ASSERT_EQ(&mockstage2, sut.next_stage(fx.ctx));

    mockstage1.set_targeting(5);
    mockstage2.set_targeting(5);
    mockstage3.set_targeting(2);
    ASSERT_EQ(&mockstage3, sut.next_stage(fx.ctx));
}
