// SPDX-License-Identifier: LGPL-3.0-or-later
#include "GenericAgent.hpp"
#include "Geometry/Geometry3D.hpp"
#include "GeometryBuilder.hpp"
#include "NeighborhoodSearch.hpp"
#include "OperationalDecisionSystem.hpp"
#include "OperationalModels/CustomModel/CustomModel.hpp"

#include <gtest/gtest.h>

#include <memory>

namespace
{
struct ConstantVelocity {
    Point velocity{};
};

/// Minimal model: advances the position by `velocity * dT` each step. Enough to
/// exercise the Location change in OperationalDecisionSystem::Run.
class ConstantVelocityModel : public CustomModel
{
public:
    void ComputeNextState(
        double dT,
        const GenericAgent& current,
        GenericAgent& next,
        const Geometry2D&,
        const NeighborhoodSearch<GenericAgent>&) const override
    {
        const auto& cur = std::get<CustomModel::State>(current.model);
        auto& nxt = std::get<CustomModel::State>(next.model);
        nxt.position = cur.position + cur.Get<ConstantVelocity>().velocity * dT;
    }

    void CheckModelConstraint(
        const GenericAgent&,
        const NeighborhoodSearch<GenericAgent>&,
        const Geometry2D&) const override
    {
    }
};

GenericAgent make_agent(Point start, Point velocity)
{
    auto agent = GenericAgent(
        GenericAgent::ID::Invalid,
        jps::UniqueID<Journey>::Invalid,
        jps::UniqueID<BaseStage>::Invalid,
        CustomModel::State{ConstantVelocity{velocity}});
    agent.position() = start;
    return agent;
}

/// A flat square [-10,10]^2 with its 3D twin over the same polygon.
std::unique_ptr<Geometry3D> flat_square()
{
    GeometryBuilder builder{};
    builder.AddAccessibleArea({{-10, -10}, {10, -10}, {10, 10}, {-10, 10}});
    return std::make_unique<Geometry3D>(builder.Build().Polygon());
}
} // namespace

TEST(OperationalDecisionSystemReAnchor, LocationTracksPositionOnFlatGeometry)
{
    auto geo = flat_square();

    AgentContainer<GenericAgent> agents{};
    const Point start{0.0, 0.0};
    agents.emplace_back(make_agent(start, Point{2.0, 0.0}));
    agents.front().location = geo->get_location(start.x, start.y, 0.0);
    ASSERT_TRUE(agents.front().location.has_value());

    NeighborhoodSearch<GenericAgent> neighborhoodSearch{2.2};
    neighborhoodSearch.Update(agents);

    OperationalDecisionSystem system{std::make_unique<ConstantVelocityModel>()};
    system.Run(0.5, 0.0, neighborhoodSearch, *geo->geometry_2d(), agents);

    const auto& agent = agents.front();
    ASSERT_TRUE(agent.location.has_value());
    // Check invariant: the Location's (x,y) equals the model position.
    // velocity=2.0, dt=0.5 --> move x by 1.0
    EXPECT_NEAR(agent.location->xy().x, agent.position().x, 1e-9);
    EXPECT_NEAR(agent.location->xy().y, agent.position().y, 1e-9);
    EXPECT_NEAR(agent.position().x, start.x + 1.0, 1e-9);
    EXPECT_NEAR(agent.position().y, start.y, 1e-9);
    EXPECT_NEAR(agent.location->z(), 0.0, 1e-9);
    EXPECT_EQ(agent.location->region(), 0u);
}

TEST(OperationalDecisionSystemReAnchor, InvariantHoldsOverManySteps)
{
    auto geo = flat_square();

    AgentContainer<GenericAgent> agents{};
    const Point start{-5.0, -3.0};
    const Point velocity{1.0, 0.5};
    const double dT = 0.1;
    const int steps = 20;
    agents.emplace_back(make_agent(start, velocity));
    agents.front().location = geo->get_location(start.x, start.y, 0.0);
    ASSERT_TRUE(agents.front().location.has_value());

    NeighborhoodSearch<GenericAgent> neighborhoodSearch{2.2};
    OperationalDecisionSystem system{std::make_unique<ConstantVelocityModel>()};

    for(int step = 0; step < steps; ++step) {
        neighborhoodSearch.Update(agents);
        system.Run(dT, 0.0, neighborhoodSearch, *geo->geometry_2d(), agents);
        const auto& agent = agents.front();
        ASSERT_TRUE(agent.location.has_value());
        EXPECT_NEAR(agent.location->xy().x, agent.position().x, 1e-9);
        EXPECT_NEAR(agent.location->xy().y, agent.position().y, 1e-9);
        EXPECT_NEAR(agent.location->z(), 0.0, 1e-9);
        EXPECT_EQ(agent.location->region(), 0u);
    }
    // After `steps` steps the agent advanced by velocity * dT * steps.
    EXPECT_NEAR(agents.front().position().x, start.x + velocity.x * dT * steps, 1e-9);
    EXPECT_NEAR(agents.front().position().y, start.y + velocity.y * dT * steps, 1e-9);
}

TEST(OperationalDecisionSystemReAnchor, AgentWithoutLocationIsUnaffected)
{
    auto geo = flat_square();

    AgentContainer<GenericAgent> agents{};
    const Point start{0.0, 0.0};
    const Point velocity{2.0, 0.0};
    const double dT = 0.5;
    agents.emplace_back(make_agent(start, velocity)); // no location -> nullopt

    NeighborhoodSearch<GenericAgent> neighborhoodSearch{2.2};
    neighborhoodSearch.Update(agents);

    OperationalDecisionSystem system{std::make_unique<ConstantVelocityModel>()};
    ASSERT_NO_THROW(system.Run(dT, 0.0, neighborhoodSearch, *geo->geometry_2d(), agents));

    const auto& agent = agents.front();
    EXPECT_FALSE(agent.location.has_value());
    EXPECT_EQ(agent.position(), start + velocity * dT);
}
