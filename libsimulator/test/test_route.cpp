// SPDX-License-Identifier: LGPL-3.0-or-later
#include "destination.hpp"
#include "generic_agent.hpp"
#include "geometry/geometry.hpp"
#include "geometry_fixtures.hpp"
#include "kernel.hpp"
#include "operational_models/collision_free_speed_model/collision_free_speed_model.hpp"
#include "polygon.hpp"
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
#include <map>
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

    RoundRobin sut({{&stage1, 1}, {&stage2, 2}, {&stage3, 3}});

    // Smooth weighted round-robin: the heaviest target is spread over the cycle, ties go to the
    // earlier target.
    const std::vector<BaseStage*> cycle{&stage3, &stage2, &stage1, &stage3, &stage2, &stage3};
    for(auto i = 0; i < 5; ++i) {
        for(auto* expected : cycle) {
            ASSERT_EQ(expected, sut.next_stage(fx.ctx));
        }
    }
}

TEST(RoundRobin, AllZeroWeightsGiveException)
{
    StubStage stage1;
    StubStage stage2;

    ASSERT_THROW(RoundRobin sut({{&stage1, 0}, {&stage2, 0}}), SimulationError);
}

TEST(Route, SmoothRoundRobinSequence)
{
    RuleContextFixture fx;
    StubStage e1;
    StubStage e2;
    const std::vector<BaseStage*> cycle{&e1, &e2, &e1, &e2, &e1};

    // Only the ratio of the weights matters: 3:2 and 60:40 give the same sequence.
    for(const uint64_t scale : {uint64_t{1}, uint64_t{20}}) {
        RoundRobin sut({{&e1, 3 * scale}, {&e2, 2 * scale}});
        for(auto i = 0; i < 4; ++i) {
            for(auto* expected : cycle) {
                ASSERT_EQ(expected, sut.next_stage(fx.ctx)) << "scale " << scale;
            }
        }
    }
}

TEST(Route, ZeroWeightNeverChosen)
{
    RuleContextFixture fx;
    StubStage a;
    StubStage b;
    StubStage c;

    RoundRobin sut({{&a, 0}, {&b, 1}, {&c, 2}});

    const std::vector<BaseStage*> cycle{&c, &b, &c};
    for(auto i = 0; i < 10; ++i) {
        for(auto* expected : cycle) {
            ASSERT_EQ(expected, sut.next_stage(fx.ctx));
        }
    }
}

TEST(Route, SetWeightsResetsTheCounter)
{
    RuleContextFixture fx;
    StubStage a;
    StubStage b;

    RoundRobin sut({{&a, 3}, {&b, 2}});
    ASSERT_EQ(&a, sut.next_stage(fx.ctx));

    // From the new weights on, the sequence starts afresh, as for a new rule.
    sut.set_weights({{a.id(), 1}, {b.id(), 2}});
    RoundRobin fresh({{&a, 1}, {&b, 2}});
    for(auto i = 0; i < 9; ++i) {
        ASSERT_EQ(fresh.next_stage(fx.ctx), sut.next_stage(fx.ctx));
    }
}

TEST(Route, SetWeightsWithTheCurrentWeightsKeepsTheCounter)
{
    RuleContextFixture fx;
    StubStage a;
    StubStage b;

    RoundRobin sut({{&a, 2}, {&b, 1}});
    RoundRobin untouched({{&a, 2}, {&b, 1}});
    ASSERT_EQ(untouched.next_stage(fx.ctx), sut.next_stage(fx.ctx));

    for(auto i = 0; i < 9; ++i) {
        sut.set_weights({{a.id(), 2}, {b.id(), 1}});
        ASSERT_EQ(untouched.next_stage(fx.ctx), sut.next_stage(fx.ctx));
    }
    EXPECT_THROW(sut.set_weights({{a.id(), 2}}), SimulationError);
}

TEST(Route, SmoothRoundRobinKeepsTheRatioOfOpenTargets)
{
    RuleContextFixture fx;
    StubStage a;
    StubStage b;
    StubStage c;

    RoundRobin sut({{&a, 3}, {&b, 2}, {&c, 1}});
    b.is_closed = true;
    std::size_t to_a = 0;
    std::size_t to_c = 0;
    for(auto i = 0; i < 40; ++i) {
        auto* next = sut.next_stage(fx.ctx);
        ASSERT_NE(&b, next);
        to_a += next == &a ? 1 : 0;
        to_c += next == &c ? 1 : 0;
    }
    EXPECT_EQ(30u, to_a);
    EXPECT_EQ(10u, to_c);
}

TEST(Route, SmoothRoundRobinUnchangedWithoutOpenTarget)
{
    RuleContextFixture fx;
    StubStage a;
    StubStage b;

    RoundRobin sut({{&a, 3}, {&b, 2}});
    RoundRobin twin({{&a, 3}, {&b, 2}});
    for(auto i = 0; i < 3; ++i) {
        ASSERT_EQ(twin.next_stage(fx.ctx), sut.next_stage(fx.ctx));
    }

    // Failed decisions leave no trace: afterwards both rules go on alike.
    a.is_closed = true;
    b.is_closed = true;
    for(auto i = 0; i < 4; ++i) {
        ASSERT_EQ(nullptr, sut.next_stage(fx.ctx));
    }
    a.is_closed = false;
    b.is_closed = false;
    for(auto i = 0; i < 10; ++i) {
        ASSERT_EQ(twin.next_stage(fx.ctx), sut.next_stage(fx.ctx));
    }
}

TEST(Route, RulesKnowTheirTargets)
{
    StubStage a;
    StubStage b;
    StubStage c;

    EXPECT_TRUE(FixedRule(&a).targets(a));
    EXPECT_FALSE(FixedRule(&a).targets(b));
    // A zero weight still names the target.
    EXPECT_TRUE(RoundRobin({{&a, 0}, {&b, 1}}).targets(a));
    EXPECT_FALSE(RoundRobin({{&a, 0}, {&b, 1}}).targets(c));
    EXPECT_TRUE(LeastTargeted({&a, &b}).targets(b));
    EXPECT_FALSE(LeastTargeted({&a, &b}).targets(c));
    EXPECT_TRUE(LeastTravelTime({&a, &b}).targets(b));
    EXPECT_FALSE(LeastTravelTime({&a, &b}).targets(c));
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

    // A closed target is passed over; the others keep their order.
    RoundRobin round_robin({{&a, 1}, {&b, 1}, {&c, 1}});
    EXPECT_EQ(&a, round_robin.next_stage(fx.ctx));
    EXPECT_EQ(&c, round_robin.next_stage(fx.ctx));
    EXPECT_EQ(&a, round_robin.next_stage(fx.ctx));

    // With every target closed there is none. Once all are open again, c and b, passed over
    // while a was chosen, come first.
    a.is_closed = true;
    c.is_closed = true;
    EXPECT_EQ(nullptr, round_robin.next_stage(fx.ctx));
    a.is_closed = false;
    b.is_closed = false;
    c.is_closed = false;
    EXPECT_EQ(&c, round_robin.next_stage(fx.ctx));
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

namespace
{
/// A stage with an area to route to, closed when told so.
class AreaStage : public BaseStage
{
    Destination _destination;

public:
    bool is_closed{false};
    explicit AreaStage(Destination destination) : _destination(destination) {}
    bool is_completed(const GenericAgent&) override { return false; }
    RoutingTarget target(const GenericAgent&) override { return _destination; }
    const Destination* destination() const override { return &_destination; }
    bool closed() const override { return is_closed; }
};

/// A stage with a 1 x 1 m area around @p centre in the fixture's room.
AreaStage area_stage_at(RuleContextFixture& fx, Point centre)
{
    const Vector half{0.5, 0.5};
    const Polygon square{test_geometries::rectangle_points(centre - half, centre + half)};
    return AreaStage{fx.routing.register_destination(fx.geometry->clip_to_region(square, 0))};
}
} // namespace

TEST(Route, LeastTravelTimePicksNearest)
{
    // The agent stands at (5, 5).
    RuleContextFixture fx;
    auto far = area_stage_at(fx, {1, 5});
    auto near = area_stage_at(fx, {7, 5});
    // The same area as far's, so exactly as far.
    auto twin = area_stage_at(fx, {1, 5});

    EXPECT_EQ(&near, LeastTravelTime({&far, &near}).next_stage(fx.ctx));
    EXPECT_EQ(&near, LeastTravelTime({&near, &far}).next_stage(fx.ctx));

    // Ties go to the earlier candidate.
    EXPECT_EQ(&far, LeastTravelTime({&far, &twin}).next_stage(fx.ctx));
    EXPECT_EQ(&twin, LeastTravelTime({&twin, &far}).next_stage(fx.ctx));

    near.is_closed = true;
    EXPECT_EQ(&far, LeastTravelTime({&near, &far}).next_stage(fx.ctx));
    far.is_closed = true;
    EXPECT_EQ(nullptr, LeastTravelTime({&near, &far}).next_stage(fx.ctx));
}

TEST(Route, SetWeightsRejectsInvalidWeightsAndKeepsTheRule)
{
    RuleContextFixture fx;
    StubStage a;
    StubStage b;
    StubStage other;

    RoundRobin sut({{&a, 3}, {&b, 2}});
    RoundRobin twin({{&a, 3}, {&b, 2}});
    ASSERT_EQ(twin.next_stage(fx.ctx), sut.next_stage(fx.ctx));

    const auto too_heavy = RoundRobin::max_weight + 1;
    EXPECT_THROW(sut.set_weights({{a.id(), 1}}), SimulationError);
    EXPECT_THROW(sut.set_weights({{a.id(), 1}, {b.id(), 1}, {other.id(), 1}}), SimulationError);
    EXPECT_THROW(sut.set_weights({{a.id(), 1}, {other.id(), 1}}), SimulationError);
    EXPECT_THROW(sut.set_weights({{a.id(), 0}, {b.id(), 0}}), SimulationError);
    EXPECT_THROW(sut.set_weights({{a.id(), too_heavy}, {b.id(), 1}}), SimulationError);

    // Neither weights nor current weights changed: both rules go on alike.
    for(auto i = 0; i < 10; ++i) {
        ASSERT_EQ(twin.next_stage(fx.ctx), sut.next_stage(fx.ctx));
    }
}

TEST(Route, RoundRobinWeightsAreBounded)
{
    RuleContextFixture fx;
    StubStage a;
    StubStage b;

    EXPECT_THROW(RoundRobin({{&a, RoundRobin::max_weight + 1}, {&b, 1}}), SimulationError);

    // At the limit the current weights stay in range: the ratio still holds.
    RoundRobin sut({{&a, RoundRobin::max_weight}, {&b, RoundRobin::max_weight}});
    for(auto i = 0; i < 10; ++i) {
        ASSERT_EQ(&a, sut.next_stage(fx.ctx));
        ASSERT_EQ(&b, sut.next_stage(fx.ctx));
    }
}

TEST(Route, ClosingATargetMidSequenceKeepsTheRatioOfOpenTargets)
{
    RuleContextFixture fx;
    StubStage a;
    StubStage b;
    StubStage c;

    RoundRobin sut({{&a, 3}, {&b, 2}, {&c, 1}});
    // Stop mid-cycle, so that every target carries a nonzero current weight.
    for(auto i = 0; i < 4; ++i) {
        sut.next_stage(fx.ctx);
    }

    const auto count = [&sut, &fx](int picks) {
        std::map<const BaseStage*, int> counts{};
        for(auto i = 0; i < picks; ++i) {
            ++counts[sut.next_stage(fx.ctx)];
        }
        return counts;
    };

    b.is_closed = true;
    auto counts = count(400);
    EXPECT_EQ(0, counts[&b]);
    EXPECT_NEAR(300, counts[&a], 2);
    EXPECT_NEAR(100, counts[&c], 2);

    b.is_closed = false;
    counts = count(600);
    EXPECT_NEAR(300, counts[&a], 2);
    EXPECT_NEAR(200, counts[&b], 2);
    EXPECT_NEAR(100, counts[&c], 2);
}
