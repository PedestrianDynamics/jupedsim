// SPDX-License-Identifier: LGPL-3.0-or-later
#include "AgentView.hpp"
#include "EnvironmentQuery.hpp"
#include "GenericAgent.hpp"
#include "Geometry/Geometry3D.hpp"
#include "GeometryBuilder.hpp"
#include "NeighborhoodSearch.hpp"
#include "OperationalModels/WarpDriver/WarpDriverModel.hpp"

#include <gtest/gtest.h>

#include <memory>
#include <vector>

namespace
{
using State = WarpDriverModel::State;

/// Open ground with one thin wall block standing across x = 1.
std::unique_ptr<Geometry3D> WalledGeometry()
{
    GeometryBuilder b{};
    b.AddAccessibleArea({{-100, -100}, {100, -100}, {100, 100}, {-100, 100}});
    b.ExcludeFromAccessibleArea({{0.9, -50}, {1.1, -50}, {1.1, 50}, {0.9, 50}});
    return std::make_unique<Geometry3D>(b.Build().Polygon());
}

GenericAgent MakeAgent(Point position, Point target)
{
    State s{};
    // Heading where it is going: the anticipation reads the neighbour's orientation, so an
    // agent facing away predicts no collision and would make the test pass for free.
    s.orientation = (target - position).Normalized();
    GenericAgent agent(
        GenericAgent::ID::Invalid,
        jps::UniqueID<Journey>::Invalid,
        jps::UniqueID<BaseStage>::Invalid,
        position,
        std::move(s));
    agent.nextTarget = target;
    return agent;
}

/// One step of the first agent, with whatever company it has been given.
Point step_of_first(const Geometry3D& geo, AgentContainer<GenericAgent>& agents)
{
    for(auto& agent : agents) {
        agent.location = geo.get_location(agent.Position().x, agent.Position().y, 0.0);
    }
    NeighborhoodSearch<GenericAgent> search{10.0};
    search.Update(agents);
    const EnvironmentQuery query{geo, search};

    const WarpDriverModel model{0.3};
    OperationalModelState next = agents[0].state;
    return model.ComputeNextState(agents[0].state, next, AgentStep{query, agents[0], 0.05});
}

} // namespace

TEST(WarpDriverModel, AnAgentBehindAWallDoesNotSteerUs)
{
    // The wall stands at x = 1. The walker approaches it from the left, the other agent
    // stands behind it, well inside the anticipation range but out of sight. Anticipating a
    // collision with someone you cannot reach would steer the walker for no reason.
    const auto geo = WalledGeometry();

    AgentContainer<GenericAgent> alone{};
    alone.push_back(MakeAgent({-1.0, 0.0}, {5.0, 0.0}));
    const Point without = step_of_first(*geo, alone);

    AgentContainer<GenericAgent> accompanied{};
    accompanied.push_back(MakeAgent({-1.0, 0.0}, {5.0, 0.0}));
    accompanied.push_back(MakeAgent({2.0, 0.0}, {-5.0, 0.0}));
    const Point with = step_of_first(*geo, accompanied);

    EXPECT_EQ(with, without) << "the agent behind the wall changed the step";
}

TEST(WarpDriverModel, AnAgentInPlainSightStillDoes)
{
    // The same setup with the other agent on this side of the wall: that one has to matter,
    // or the test above would pass for the wrong reason.
    const auto geo = WalledGeometry();

    AgentContainer<GenericAgent> alone{};
    alone.push_back(MakeAgent({-1.0, 0.0}, {5.0, 0.0}));
    const Point without = step_of_first(*geo, alone);

    AgentContainer<GenericAgent> accompanied{};
    accompanied.push_back(MakeAgent({-1.0, 0.0}, {5.0, 0.0}));
    accompanied.push_back(MakeAgent({-0.2, 0.0}, {-5.0, 0.0}));
    const Point with = step_of_first(*geo, accompanied);

    EXPECT_NE(with, without);
}
