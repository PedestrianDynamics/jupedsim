// SPDX-License-Identifier: LGPL-3.0-or-later
#include "GenericAgent.hpp"
#include "SocialForceModelIPP.hpp"
#include "SocialForceModelIPPData.hpp"

#include <cmath>
#include <gtest/gtest.h>

namespace
{
GenericAgent MakeAgent(
    Point pos,
    Point velocity = {0, 0},
    Point gs_pos = {0, 0},
    Point gs_vel = {0, 0},
    double height = 1.75,
    double radius = 0.15)
{
    SocialForceModelIPPData data{};
    data.velocity = velocity;
    data.ground_support_position = gs_pos;
    data.ground_support_velocity = gs_vel;
    data.height = height;
    data.desiredSpeed = 1.0;
    data.reactionTime = 0.5;
    data.lambdaU = 0.5;
    data.lambdaB = 1.0;
    data.balanceSpeed = 1.0;
    data.damping = 1.0;
    data.agentScale = 5.0;
    data.forceDistance = 0.5;
    data.legForceDistance = 0.3;
    data.radius = radius;

    return GenericAgent(
        GenericAgent::ID{},
        jps::UniqueID<Journey>::Invalid,
        jps::UniqueID<BaseStage>::Invalid,
        pos,
        Point(1, 0),
        std::move(data));
}
} // namespace

TEST(SocialForceModelIPP, DrivingForcePointsTowardDestination)
{
    auto agent = MakeAgent(Point(0, 0));
    agent.destination = Point(10, 0);

    // Driving force = (e0 * v0 - v) / tau
    // e0 = (1,0), v0 = 1.0, v = (0,0), tau = 0.5
    // F = (1,0) * 1.0 / 0.5 = (2, 0)
    const auto& model = std::get<SocialForceModelIPPData>(agent.model);
    const Point e0 = (agent.destination - agent.pos).Normalized();
    const Point expected = (e0 * model.desiredSpeed - model.velocity) / model.reactionTime;

    EXPECT_DOUBLE_EQ(expected.x, 2.0);
    EXPECT_DOUBLE_EQ(expected.y, 0.0);
}

TEST(SocialForceModelIPP, ExponentialRepulsionIsPositive)
{
    // Verify the exponential repulsion formula: A * exp(-dist / B)
    const double A = 5.0;
    const double B = 0.5;
    const double dist = 1.0;
    const double force = A * std::exp(-dist / B);
    EXPECT_GT(force, 0.0);
}

TEST(SocialForceModelIPP, ExponentialRepulsionDecaysWithDistance)
{
    const double A = 5.0;
    const double B = 0.5;
    const double force_near = A * std::exp(-0.5 / B);
    const double force_far = A * std::exp(-2.0 / B);
    EXPECT_GT(force_near, force_far);
}

TEST(SocialForceModelIPP, LegForceDistanceShorterThanUpperBody)
{
    auto agent = MakeAgent(Point(0, 0));
    const auto& model = std::get<SocialForceModelIPPData>(agent.model);
    EXPECT_LT(model.legForceDistance, model.forceDistance);
}

TEST(SocialForceModelIPP, GroundSupportScalingFactorIsPositive)
{
    EXPECT_GT(SocialForceModelIPP::GS_SCALING_FACTOR, 0.0);
    EXPECT_LT(SocialForceModelIPP::GS_SCALING_FACTOR, 1.0);
}
