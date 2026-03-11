// SPDX-License-Identifier: LGPL-3.0-or-later
#include "GenericAgent.hpp"

#include <fmt/format.h>
#include <gtest/gtest.h>

static GenericAgent make_agent(GenericAgent::Model model)
{
    return GenericAgent(
        GenericAgent::ID{},
        jps::UniqueID<Journey>::Invalid,
        jps::UniqueID<BaseStage>::Invalid,
        Point{1.0, 2.0},
        Point{0.0, 1.0},
        std::move(model));
}

TEST(GenericAgentFormatter, FormatsGeneralizedCentrifugalForceModelData)
{
    auto agent = make_agent(GeneralizedCentrifugalForceModelData{});
    ASSERT_NO_THROW((void) fmt::format("{}", agent));
}

TEST(GenericAgentFormatter, FormatsCollisionFreeSpeedModelData)
{
    auto agent = make_agent(CollisionFreeSpeedModelData{});
    ASSERT_NO_THROW((void) fmt::format("{}", agent));
}

TEST(GenericAgentFormatter, FormatsCollisionFreeSpeedModelV2Data)
{
    auto agent = make_agent(CollisionFreeSpeedModelV2Data{});
    ASSERT_NO_THROW((void) fmt::format("{}", agent));
}

TEST(GenericAgentFormatter, FormatsAnticipationVelocityModelData)
{
    auto agent = make_agent(AnticipationVelocityModelData{});
    ASSERT_NO_THROW((void) fmt::format("{}", agent));
}

TEST(GenericAgentFormatter, FormatsSocialForceModelData)
{
    auto agent = make_agent(SocialForceModelData{});
    ASSERT_NO_THROW((void) fmt::format("{}", agent));
}
