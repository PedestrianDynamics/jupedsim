// SPDX-License-Identifier: LGPL-3.0-or-later
#include "AgentView.hpp"
#include "GenericAgent.hpp"
#include "Geometry/Geometry.hpp"
#include "GeometryFixtures.hpp"
#include "NeighborhoodSearch.hpp"
#include "OperationalDecisionSystem.hpp"
#include "OperationalModels/CustomModel/CustomModel.hpp"
#include "TestCommon.hpp"

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
    Point ComputeNextState(
        const OperationalModelState& current,
        OperationalModelState&,
        const AgentStep& step) const override
    {
        const auto& cur = std::get<CustomModel::State>(current);
        return cur.Get<ConstantVelocity>().velocity * step.dt();
    }

    void CheckModelConstraint(const GenericAgent&, const AgentView&) const override {}
};

GenericAgent make_agent(const Geometry& geometry, Point start, Point velocity)
{
    return GenericAgent(
        GenericAgent::ID::Invalid,
        jps::UniqueID<Journey>::Invalid,
        jps::UniqueID<BaseStage>::Invalid,
        *geometry.get_location(start.x, start.y, 0.0),
        CustomModel::State{ConstantVelocity{velocity}});
}

/// A flat square [-10,10]^2 with its 3D twin over the same polygon.
std::unique_ptr<Geometry> flat_square()
{
    return test_geometries::rectangle({-10, -10}, {10, 10});
}
} // namespace

TEST(OperationalDecisionSystemLocation, WalkingTheStepLandsWhereDeadReckoningSays)
{
    auto geo = flat_square();

    AgentContainer<GenericAgent> agents{};
    const Point start{0.0, 0.0};
    agents.emplace_back(make_agent(*geo, start, Point{2.0, 0.0}));

    NeighborhoodSearch<GenericAgent> neighborhoodSearch{2.2};
    neighborhoodSearch.Update(agents);

    OperationalDecisionSystem system{std::make_unique<ConstantVelocityModel>()};
    system.Run(0.5, 0.0, neighborhoodSearch, *geo, agents);

    // velocity=2.0, dt=0.5 --> move x by 1.0
    const auto& agent = agents.front();
    EXPECT_NEAR(agent.location.xy().x, start.x + 1.0, 1e-9);
    EXPECT_NEAR(agent.location.xy().y, start.y, 1e-9);
    EXPECT_NEAR(agent.location.z(), 0.0, 1e-9);
    EXPECT_EQ(agent.location.region(), 0u);
}

TEST(OperationalDecisionSystemLocation, ItStaysOnTheSheetOverManySteps)
{
    auto geo = flat_square();

    AgentContainer<GenericAgent> agents{};
    const Point start{-5.0, -3.0};
    const Point velocity{1.0, 0.5};
    const double dT = 0.1;
    const int steps = 20;
    agents.emplace_back(make_agent(*geo, start, velocity));

    NeighborhoodSearch<GenericAgent> neighborhoodSearch{2.2};
    OperationalDecisionSystem system{std::make_unique<ConstantVelocityModel>()};

    for(int step = 0; step < steps; ++step) {
        neighborhoodSearch.Update(agents);
        system.Run(dT, 0.0, neighborhoodSearch, *geo, agents);
        const auto& agent = agents.front();
        EXPECT_NEAR(agent.location.z(), 0.0, 1e-9);
        EXPECT_EQ(agent.location.region(), 0u);
    }
    // After `steps` steps the agent advanced by velocity * dT * steps.
    EXPECT_NEAR(agents.front().location.xy().x, start.x + velocity.x * dT * steps, 1e-9);
    EXPECT_NEAR(agents.front().location.xy().y, start.y + velocity.y * dT * steps, 1e-9);
}
