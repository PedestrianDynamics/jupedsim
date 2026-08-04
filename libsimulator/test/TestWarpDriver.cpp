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

/// The same ground with nothing on it.
std::unique_ptr<Geometry3D> OpenGeometry()
{
    GeometryBuilder b{};
    b.AddAccessibleArea({{-100, -100}, {100, -100}, {100, 100}, {-100, 100}});
    return std::make_unique<Geometry3D>(b.Build().Polygon());
}

/// Open ground with a 0.1 m block just left of the origin -- thin enough that one long step
/// clears it and lands in free space on the far side.
std::unique_ptr<Geometry3D> ThinWallLeftOfOrigin()
{
    GeometryBuilder b{};
    b.AddAccessibleArea({{-100, -100}, {100, -100}, {100, 100}, {-100, 100}});
    b.ExcludeFromAccessibleArea({{-0.2, -50}, {-0.1, -50}, {-0.1, 50}, {-0.2, 50}});
    return std::make_unique<Geometry3D>(b.Build().Polygon());
}

GenericAgent MakeAgent(const Geometry3D& geo, Point position, Point target, State s = State{})
{
    // Heading where it is going: the anticipation reads the neighbour's orientation, so an
    // agent facing away predicts no collision and would make the test pass for free.
    s.orientation = (target - position).Normalized();
    GenericAgent agent(
        GenericAgent::ID::Invalid,
        jps::UniqueID<Journey>::Invalid,
        jps::UniqueID<BaseStage>::Invalid,
        *geo.get_location(position.x, position.y, 0.0),
        std::move(s));
    agent.nextTarget = target;
    return agent;
}

/// One step of the first agent, with whatever company it has been given.
Point step_of_first(const Geometry3D& geo, AgentContainer<GenericAgent>& agents, double dt = 0.05)
{
    NeighborhoodSearch<GenericAgent> search{10.0};
    search.Update(agents);
    const EnvironmentQuery query{geo, search};

    const WarpDriverModel model{0.3};
    OperationalModelState next = agents[0].state;
    return model.ComputeNextState(agents[0].state, next, AgentStep{query, agents[0], dt});
}

} // namespace

TEST(WarpDriverModel, AnAgentBehindAWallDoesNotSteerUs)
{
    // The wall stands at x = 1. The walker approaches it from the left, the other agent
    // stands behind it, well inside the anticipation range but out of sight. Anticipating a
    // collision with someone you cannot reach would steer the walker for no reason.
    const auto geo = WalledGeometry();

    AgentContainer<GenericAgent> alone{};
    alone.push_back(MakeAgent(*geo, {-1.0, 0.0}, {5.0, 0.0}));
    const Point without = step_of_first(*geo, alone);

    AgentContainer<GenericAgent> accompanied{};
    accompanied.push_back(MakeAgent(*geo, {-1.0, 0.0}, {5.0, 0.0}));
    accompanied.push_back(MakeAgent(*geo, {2.0, 0.0}, {-5.0, 0.0}));
    const Point with = step_of_first(*geo, accompanied);

    EXPECT_EQ(with, without) << "the agent behind the wall changed the step";
}

TEST(WarpDriverModel, ADetourTakesTheOtherSideWhenTheWayIsBlocked)
{
    // An agent already breaking a deadlock sideways. Heading towards +y, side +1 veers
    // towards -x, which is where the block stands.
    State detouring{};
    detouring.detourTime = 1.0;
    detouring.detourSide = 1;

    // The step is long enough to clear the block outright and land in free space beyond it.
    // Judging where it lands would wave that through; judging the way there does not.
    constexpr double longStep = 0.5;

    const auto open = OpenGeometry();
    AgentContainer<GenericAgent> unobstructed{};
    unobstructed.push_back(MakeAgent(*open, {0, 0}, {0, 10}, detouring));
    EXPECT_LT(step_of_first(*open, unobstructed, longStep).x, 0.0);

    const auto blocked = ThinWallLeftOfOrigin();
    AgentContainer<GenericAgent> obstructed{};
    obstructed.push_back(MakeAgent(*blocked, {0, 0}, {0, 10}, detouring));
    EXPECT_GT(step_of_first(*blocked, obstructed, longStep).x, 0.0);
}

TEST(WarpDriverModel, AnAgentInPlainSightStillDoes)
{
    // The same setup with the other agent on this side of the wall: that one has to matter,
    // or the test above would pass for the wrong reason.
    const auto geo = WalledGeometry();

    AgentContainer<GenericAgent> alone{};
    alone.push_back(MakeAgent(*geo, {-1.0, 0.0}, {5.0, 0.0}));
    const Point without = step_of_first(*geo, alone);

    AgentContainer<GenericAgent> accompanied{};
    accompanied.push_back(MakeAgent(*geo, {-1.0, 0.0}, {5.0, 0.0}));
    accompanied.push_back(MakeAgent(*geo, {-0.2, 0.0}, {-5.0, 0.0}));
    const Point with = step_of_first(*geo, accompanied);

    EXPECT_NE(with, without);
}
