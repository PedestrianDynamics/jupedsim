// SPDX-License-Identifier: LGPL-3.0-or-later
#include "GenericAgent.hpp"
#include "OperationalModels/AnticipationVelocityModel/AnticipationVelocityModel.hpp"
#include "OperationalModels/CollisionFreeSpeedModel/CollisionFreeSpeedModel.hpp"
#include "OperationalModels/CollisionFreeSpeedModelV2/CollisionFreeSpeedModelV2.hpp"
#include "OperationalModels/GeneralizedCentrifugalForceModel/GeneralizedCentrifugalForceModel.hpp"
#include "OperationalModels/SocialForceModel/SocialForceModel.hpp"

#include <fmt/format.h>
#include <gtest/gtest.h>

static GenericAgent make_agent(GenericAgent::ModelState state)
{
    return GenericAgent(
        GenericAgent::ID{},
        jps::UniqueID<Journey>::Invalid,
        jps::UniqueID<BaseStage>::Invalid,
        std::move(state));
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
