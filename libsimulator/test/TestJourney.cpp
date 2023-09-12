// Copyright © 2012-2023 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "Journey.hpp"

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