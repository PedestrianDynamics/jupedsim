// SPDX-License-Identifier: LGPL-3.0-or-later
#include "Journey.hpp"
#include "TestCommon.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

TEST(FixedTransition, NextIsCorrect)
{
    int stage;

    FixedTransition sut(reinterpret_cast<BaseStage*>(&stage));

    for(auto i = 0; i < 20; ++i) {
        ASSERT_EQ(reinterpret_cast<BaseStage*>(&stage), sut.next_stage());
    }
}

TEST(RoundRobinTransition, SimpleNextIsCorrect)
{
    int stage1;
    int stage2;
    int stage3;

    std::vector<std::tuple<BaseStage*, uint64_t>> weighted_stages = {
        {reinterpret_cast<BaseStage*>(&stage1), 1},
        {reinterpret_cast<BaseStage*>(&stage2), 1},
        {reinterpret_cast<BaseStage*>(&stage3), 1}};

    RoundRobinTransition sut(weighted_stages);

    for(auto i = 0; i < 5; ++i) {
        for(auto const& [stage, _] : weighted_stages) {
            ASSERT_EQ(stage, sut.next_stage());
        }
    }
}

TEST(RoundRobinTransition, WeightedRoundRobin)
{
    int stage1;
    int stage2;
    int stage3;

    std::vector<std::tuple<BaseStage*, uint64_t>> weighted_stages = {
        {reinterpret_cast<BaseStage*>(&stage1), 1},
        {reinterpret_cast<BaseStage*>(&stage2), 2},
        {reinterpret_cast<BaseStage*>(&stage3), 3}};

    RoundRobinTransition sut(weighted_stages);

    for(auto i = 0; i < 5; ++i) {
        ASSERT_EQ(std::get<0>(weighted_stages[0]), sut.next_stage());

        ASSERT_EQ(std::get<0>(weighted_stages[1]), sut.next_stage());
        ASSERT_EQ(std::get<0>(weighted_stages[1]), sut.next_stage());

        ASSERT_EQ(std::get<0>(weighted_stages[2]), sut.next_stage());
        ASSERT_EQ(std::get<0>(weighted_stages[2]), sut.next_stage());
        ASSERT_EQ(std::get<0>(weighted_stages[2]), sut.next_stage());
    }
}

TEST(RoundRobinTransition, ZeroWeightGivesException)
{
    int stage1;
    std::vector<std::tuple<BaseStage*, uint64_t>> weighted_stages = {
        {reinterpret_cast<BaseStage*>(&stage1), 0}};

    ASSERT_THROW(RoundRobinTransition sut(weighted_stages), SimulationError);
}

TEST(LeastTargetedTransition, NextIsCorrect)
{
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
        MOCK_METHOD(StageProxy, proxy, (Simulation* sim), (override));
        void set_targeting(size_t targeting) { _targeting = targeting; }
    };

    MockStage mockstage1(3);
    MockStage mockstage2(2);
    MockStage mockstage3(1);

    std::vector<BaseStage*> stages = {&mockstage1, &mockstage2, &mockstage3};
    LeastTargetedTransition sut(stages);

    ASSERT_EQ(&mockstage3, sut.next_stage());

    mockstage1.set_targeting(1);
    mockstage2.set_targeting(1);
    mockstage3.set_targeting(1);
    ASSERT_EQ(&mockstage1, sut.next_stage());

    mockstage1.set_targeting(5);
    mockstage2.set_targeting(1);
    mockstage3.set_targeting(5);
    ASSERT_EQ(&mockstage2, sut.next_stage());

    mockstage1.set_targeting(5);
    mockstage2.set_targeting(5);
    mockstage3.set_targeting(2);
    ASSERT_EQ(&mockstage3, sut.next_stage());
}
