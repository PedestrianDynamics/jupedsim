// SPDX-License-Identifier: LGPL-3.0-or-later
#include "GenericAgent.hpp"

#include <fmt/format.h>
#include <gtest/gtest.h>

static GenericAgent make_agent(GenericAgent::ModelState model)
{
    return GenericAgent(
        GenericAgent::ID{},
        jps::UniqueID<Journey>::Invalid,
        jps::UniqueID<BaseStage>::Invalid,
        Point{1.0, 2.0},
        std::move(model));
}

TEST(GenericAgentFormatter, FormatsGeneralizedCentrifugalForceModelAgent)
{
    auto agent = make_agent(GeneralizedCentrifugalForceModel::State{});
    ASSERT_NO_THROW((void) fmt::format("{}", agent));
}

TEST(GenericAgentFormatter, FormatsCollisionFreeSpeedModelAgent)
{
    auto agent = make_agent(CollisionFreeSpeedModel::State{});
    ASSERT_NO_THROW((void) fmt::format("{}", agent));
}

TEST(GenericAgentFormatter, FormatsCollisionFreeSpeedModelV2Agent)
{
    auto agent = make_agent(CollisionFreeSpeedModelV2::State{});
    ASSERT_NO_THROW((void) fmt::format("{}", agent));
}

TEST(GenericAgentFormatter, FormatsAnticipationVelocityModelAgent)
{
    auto agent = make_agent(AnticipationVelocityModel::State{});
    ASSERT_NO_THROW((void) fmt::format("{}", agent));
}

TEST(GenericAgentFormatter, FormatsSocialForceModelAgent)
{
    auto agent = make_agent(SocialForceModel::State{});
    ASSERT_NO_THROW((void) fmt::format("{}", agent));
}
