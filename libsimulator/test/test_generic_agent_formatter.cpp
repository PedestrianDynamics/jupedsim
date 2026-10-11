// SPDX-License-Identifier: LGPL-3.0-or-later
#include "generic_agent.hpp"
#include "geometry/geometry.hpp"
#include "geometry_fixtures.hpp"
#include "test_common.hpp"

#include <fmt/format.h>
#include <gtest/gtest.h>

#include <memory>

static GenericAgent make_agent(OperationalModelState model)
{
    static const auto geometry = test_geometries::rectangle({-10, -10}, {10, 10});
    return GenericAgent(
        GenericAgent::ID{},
        jps::UniqueID<Journey>::invalid,
        jps::UniqueID<BaseStage>::invalid,
        *geometry->get_location_near_z(0.0, 0.0, 0.0),
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
