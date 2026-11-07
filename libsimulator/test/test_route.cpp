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

/// A stage that is never completed and is closed when told so.
class StubStage : public BaseStage
{
public:
    bool is_closed{false};
    bool is_completed(const GenericAgent&) override { return false; }
    RoutingTarget target(const GenericAgent& agent) override { return agent.location; }
    bool closed() const override { return is_closed; }
};
} // namespace

TEST(FixedRule, NextIsCorrect)
{
    RuleContextFixture fx;
    StubStage stage;

    FixedRule sut(&stage);

    for(auto i = 0; i < 20; ++i) {
        ASSERT_EQ(&stage, sut.next_stage(fx.ctx));
    }
}

TEST(RoundRobin, SimpleNextIsCorrect)
{
    RuleContextFixture fx;
    StubStage stage1;
    StubStage stage2;
    StubStage stage3;

    std::vector<std::tuple<BaseStage*, uint64_t>> weighted_stages = {
        {&stage1, 1}, {&stage2, 1}, {&stage3, 1}};

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
    StubStage stage1;
    StubStage stage2;
    StubStage stage3;

    std::vector<std::tuple<BaseStage*, uint64_t>> weighted_stages = {
        {&stage1, 1}, {&stage2, 2}, {&stage3, 3}};

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

TEST(Route, RuleSkipsClosedTarget)
{
    RuleContextFixture fx;
    StubStage a;
    StubStage b;
    StubStage c;

    b.is_closed = true;
    FixedRule fixed(&b);
    EXPECT_EQ(nullptr, fixed.next_stage(fx.ctx));

    // A closed target's slots are passed over; the others keep their order.
    RoundRobin round_robin({{&a, 1}, {&b, 1}, {&c, 1}});
    EXPECT_EQ(&a, round_robin.next_stage(fx.ctx));
    EXPECT_EQ(&c, round_robin.next_stage(fx.ctx));
    EXPECT_EQ(&a, round_robin.next_stage(fx.ctx));

    // With every target closed there is none, and the counter is where it was.
    a.is_closed = true;
    c.is_closed = true;
    EXPECT_EQ(nullptr, round_robin.next_stage(fx.ctx));
    a.is_closed = false;
    b.is_closed = false;
    c.is_closed = false;
    EXPECT_EQ(&b, round_robin.next_stage(fx.ctx));

    a.increase_targeting();
    b.increase_targeting();
    b.increase_targeting();
    c.is_closed = true;
    LeastTargeted least_targeted({&a, &b, &c});
    EXPECT_EQ(&a, least_targeted.next_stage(fx.ctx));
    a.is_closed = true;
    EXPECT_EQ(&b, least_targeted.next_stage(fx.ctx));
    b.is_closed = true;
    EXPECT_EQ(nullptr, least_targeted.next_stage(fx.ctx));
}
