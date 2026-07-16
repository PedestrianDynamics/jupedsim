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
        std::move(model));
}

TEST(GenericAgentFormatter, FormatsGeneralizedCentrifugalForceModelAgent)
{
    auto agent = make_agent(GeneralizedCentrifugalForceModelState{});
    ASSERT_NO_THROW((void) fmt::format("{}", agent));
}

TEST(GenericAgentFormatter, FormatsCollisionFreeSpeedModelAgent)
{
    auto agent = make_agent(CollisionFreeSpeedModelState{});
    ASSERT_NO_THROW((void) fmt::format("{}", agent));
}

TEST(GenericAgentFormatter, FormatsCollisionFreeSpeedModelV2Agent)
{
    auto agent = make_agent(CollisionFreeSpeedModelV2State{});
    ASSERT_NO_THROW((void) fmt::format("{}", agent));
}

TEST(GenericAgentFormatter, FormatsAnticipationVelocityModelAgent)
{
    auto agent = make_agent(AnticipationVelocityModelState{});
    ASSERT_NO_THROW((void) fmt::format("{}", agent));
}

TEST(GenericAgentFormatter, FormatsSocialForceModelAgent)
{
    auto agent = make_agent(SocialForceModelState{});
    ASSERT_NO_THROW((void) fmt::format("{}", agent));
}
