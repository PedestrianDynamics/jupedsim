// Copyright © 2012-2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "Journey.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

TEST(FixedTransition, NextIsCorrect)
{
    int stage;

    FixedTransition sut(reinterpret_cast<BaseStage*>(&stage));

    for(auto i = 0; i < 20; ++i) {
        ASSERT_EQ(reinterpret_cast<BaseStage*>(&stage), sut.NextStage());
    }
}

TEST(RoundRobinTransition, SimpleNextIsCorrect)
{
    int stage1;
    int stage2;
    int stage3;

    std::vector<std::tuple<BaseStage*, uint64_t>> weightedStages = {
        {reinterpret_cast<BaseStage*>(&stage1), 1},
        {reinterpret_cast<BaseStage*>(&stage2), 1},
        {reinterpret_cast<BaseStage*>(&stage3), 1}};

    RoundRobinTransition sut(weightedStages);

    for(auto i = 0; i < 5; ++i) {
        for(auto const& [stage, _] : weightedStages) {
            ASSERT_EQ(stage, sut.NextStage());
        }
    }
}

TEST(RoundRobinTransition, WeightedRoundRobin)
{
    int stage1;
    int stage2;
    int stage3;

    std::vector<std::tuple<BaseStage*, uint64_t>> weightedStages = {
        {reinterpret_cast<BaseStage*>(&stage1), 1},
        {reinterpret_cast<BaseStage*>(&stage2), 2},
        {reinterpret_cast<BaseStage*>(&stage3), 3}};

    RoundRobinTransition sut(weightedStages);

    for(auto i = 0; i < 5; ++i) {
        ASSERT_EQ(std::get<0>(weightedStages[0]), sut.NextStage());

        ASSERT_EQ(std::get<0>(weightedStages[1]), sut.NextStage());
        ASSERT_EQ(std::get<0>(weightedStages[1]), sut.NextStage());

        ASSERT_EQ(std::get<0>(weightedStages[2]), sut.NextStage());
        ASSERT_EQ(std::get<0>(weightedStages[2]), sut.NextStage());
        ASSERT_EQ(std::get<0>(weightedStages[2]), sut.NextStage());
    }
}

TEST(RoundRobinTransition, ZeroWeightGivesException)
{
    int stage1;
    std::vector<std::tuple<BaseStage*, uint64_t>> weightedStages = {
        {reinterpret_cast<BaseStage*>(&stage1), 0}};

    ASSERT_THROW(RoundRobinTransition sut(weightedStages), SimulationError);
}

TEST(LeastTargetedTransition, NextIsCorrect)
{
    class MockStage : public BaseStage
    {
    public:
        MockStage(size_t targeting_)
        {
            targeting = targeting_;
            ON_CALL(*this, CountTargeting).WillByDefault([this]() { return targeting; });
            ON_CALL(*this, IsCompleted).WillByDefault([]() { return true; });
        }
        MOCK_METHOD(size_t, CountTargeting, (), (const));
        MOCK_METHOD(bool, IsCompleted, (const GenericAgent& agent), (override));
        MOCK_METHOD(Point, Target, (const GenericAgent& agent), (override));
        MOCK_METHOD(StageProxy, Proxy, (Simulation * simulation_), (override));
        void SetTargeting(size_t targeting_) { targeting = targeting_; }
    };

    MockStage mockstage1(3);
    MockStage mockstage2(2);
    MockStage mockstage3(1);

    std::vector<BaseStage*> stages = {&mockstage1, &mockstage2, &mockstage3};
    LeastTargetedTransition sut(stages);

    ASSERT_EQ(&mockstage3, sut.NextStage());

    mockstage1.SetTargeting(1);
    mockstage2.SetTargeting(1);
    mockstage3.SetTargeting(1);
    ASSERT_EQ(&mockstage1, sut.NextStage());

    mockstage1.SetTargeting(5);
    mockstage2.SetTargeting(1);
    mockstage3.SetTargeting(5);
    ASSERT_EQ(&mockstage2, sut.NextStage());

    mockstage1.SetTargeting(5);
    mockstage2.SetTargeting(5);
    mockstage3.SetTargeting(2);
    ASSERT_EQ(&mockstage3, sut.NextStage());
}
