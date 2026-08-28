// SPDX-License-Identifier: LGPL-3.0-or-later
#include "AgentView.hpp"
#include "EnvironmentQuery.hpp"
#include "GenericAgent.hpp"
#include "Geometry/Geometry3D.hpp"
#include "GeometryBuilder.hpp"
#include "MeshFixtures.hpp"
#include "NeighborhoodSearch.hpp"
#include "OperationalModels/CollisionFreeSpeedModel/CollisionFreeSpeedModel.hpp"
#include "TestCommon.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <limits>
#include <memory>
#include <vector>

namespace
{
using State = CollisionFreeSpeedModel::State;

GenericAgent MakeAgent(const Geometry3D& geo, Point pos, double radius = 0.2, double z = 0.0)
{
    State s{};
    s.radius = radius;
    return GenericAgent(
        GenericAgent::ID::Invalid,
        jps::UniqueID<Journey>::Invalid,
        jps::UniqueID<BaseStage>::Invalid,
        *geo.get_location(pos.x, pos.y, z),
        std::move(s));
}

std::unique_ptr<Geometry3D> OpenGeometry()
{
    GeometryBuilder b{};
    b.AddAccessibleArea({{-100, -100}, {100, -100}, {100, 100}, {-100, 100}});
    return std::make_unique<Geometry3D>(b.Build().Polygon());
}

// Geometry with a thin wall at x≈1 that blocks line-of-sight across it.
std::unique_ptr<Geometry3D> WalledGeometry()
{
    GeometryBuilder b{};
    b.AddAccessibleArea({{-100, -100}, {100, -100}, {100, 100}, {-100, 100}});
    b.ExcludeFromAccessibleArea({{0.9, -50}, {1.1, -50}, {1.1, 50}, {0.9, 50}});
    return std::make_unique<Geometry3D>(b.Build().Polygon());
}

struct Environment {
    AgentContainer<GenericAgent> agents{};
    NeighborhoodSearch<GenericAgent> neighborhood_search{5.0};
    std::vector<std::pair<Point, double>> requested{};

    void add_agent(Point pos, double radius = 0.2) { requested.emplace_back(pos, radius); }

    // Agents are asked for before the geometry exists, so they are made here -- only the
    // geometry can put them onto the surface, the same way Simulation::AddAgent does.
    EnvironmentQuery query(const Geometry3D& geo)
    {
        for(const auto& [pos, radius] : requested) {
            agents.push_back(MakeAgent(geo, pos, radius));
        }
        requested.clear();
        neighborhood_search.Update(agents);
        return {geo, neighborhood_search};
    }

    // The first agent added is the one every test queries from.
    AgentView FirstAgentView(const EnvironmentQuery& q) const { return {q, agents[0]}; }

    const Location& FirstAgentLocation() const { return agents[0].location; }
};
} // namespace

TEST(AgentView, OtherAgentsInRangeExcludesSelf)
{
    Environment env{};
    env.add_agent({0, 0});
    const auto geo = OpenGeometry();
    const auto q = env.query(*geo);

    const auto result = env.FirstAgentView(q).OtherAgentsInRange(100.0);
    EXPECT_TRUE(result.empty());
}

TEST(AgentView, OtherAgentsInRangeNoFilterReturnsAllInRadius)
{
    Environment env{};
    env.add_agent({0, 0}); // querying agent
    env.add_agent({1, 0});
    env.add_agent({0, 1});
    env.add_agent({-1, 0});
    const auto geo = OpenGeometry();
    const auto q = env.query(*geo);

    const auto result = env.FirstAgentView(q).OtherAgentsInRange(5.0);
    EXPECT_EQ(result.size(), 3u);
}

TEST(AgentView, OtherAgentsInRangeCustomFilterRejectsAll)
{
    Environment env{};
    env.add_agent({0, 0});
    env.add_agent({1, 0});
    env.add_agent({0, 1});
    const auto geo = OpenGeometry();
    const auto q = env.query(*geo);

    const auto result =
        env.FirstAgentView(q).OtherAgentsInRange(5.0, [](const NeighborView&) { return false; });
    EXPECT_TRUE(result.empty());
}

TEST(AgentView, OtherAgentsInRangeCustomFilterSelectsSubset)
{
    Environment env{};
    env.add_agent({0, 0}); // querying agent
    env.add_agent({1, 0}); // positive x — kept
    env.add_agent({0, 1}); // positive y — kept
    env.add_agent({-1, 0}); // negative x — filtered out
    const auto geo = OpenGeometry();
    const auto q = env.query(*geo);

    const auto result = env.FirstAgentView(q).OtherAgentsInRange(
        5.0, [](const NeighborView& n) { return n.RelativePosition.x >= 0.0; });

    ASSERT_EQ(result.size(), 2u);
    for(const auto& neighbor : result) {
        EXPECT_GE(neighbor.RelativePosition.x, 0.0);
    }
}

TEST(AgentView, NoGeometryBetweenFiltersOccludedAgents)
{
    Environment env{};
    env.add_agent({0, 0}); // querying agent
    env.add_agent({2, 0}); // behind wall — occluded
    env.add_agent({0, 1}); // same side as querying agent — visible
    const auto geo = WalledGeometry();
    const auto q = env.query(*geo);

    const auto view = env.FirstAgentView(q);
    const auto result = view.OtherAgentsInRange(
        5.0, [&](const NeighborView& n) { return view.NoGeometryBetween(n); });

    ASSERT_EQ(result.size(), 1u);
    EXPECT_EQ(result[0].RelativePosition, Point(0, 1));
}

TEST(AgentView, OtherAgentsInRangeCustomFilterReceivesNoSelf)
{
    // Verify the filter is never called with the querying agent itself.
    Environment env{};
    env.add_agent({0, 0});
    env.add_agent({1, 0});
    const auto geo = OpenGeometry();
    const auto q = env.query(*geo);

    int calls = 0;
    env.FirstAgentView(q).OtherAgentsInRange(5.0, [&](const NeighborView&) {
        ++calls;
        return true;
    });
    EXPECT_EQ(calls, 1);
}

TEST(AgentView, OtherAgentsInRangeOutOfRadiusNotReturned)
{
    Environment env{};
    env.add_agent({0, 0});
    env.add_agent({50, 0}); // far away
    const auto geo = OpenGeometry();
    const auto q = env.query(*geo);

    const auto result =
        env.FirstAgentView(q).OtherAgentsInRange(1.0, [](const NeighborView&) { return true; });
    EXPECT_TRUE(result.empty());
}

TEST(AgentView, AgentsOnTheSamePositionSeeEachOther)
{
    Environment env{};
    env.add_agent({3, 4});
    env.add_agent({3, 4});
    const auto geo = OpenGeometry();
    const auto q = env.query(*geo);

    const auto result = env.FirstAgentView(q).OtherAgentsInRange(1.0);

    ASSERT_EQ(result.size(), 1u);
    EXPECT_EQ(result[0].state, &env.agents[1].state);
}

// Every other test here queries from {0,0} and would not notice a centre that
// silently defaults to the origin.
TEST(AgentView, OtherAgentsInRangeCentresOnTheQueryingAgentNotTheOrigin)
{
    Environment env{};
    env.add_agent({50, 30}); // querying agent, deliberately away from the origin
    env.add_agent({51, 30}); // its actual neighbor
    env.add_agent({0.5, 0}); // decoy next to the origin
    const auto geo = OpenGeometry();
    const auto q = env.query(*geo);

    const auto result = env.FirstAgentView(q).OtherAgentsInRange(2.0);

    ASSERT_EQ(result.size(), 1u);
    EXPECT_EQ(result[0].RelativePosition, Point(1, 0));
}

TEST(AgentView, WallsInRangeAreRelativeToTheAgent)
{
    Environment env{};
    const Point pos{-1.0, 2.0};
    env.add_agent(pos);
    const auto geo = WalledGeometry();
    const auto q = env.query(*geo);

    std::vector<LineSegment> expected{};
    for(const auto& segment : q.LineSegmentsInRange(env.FirstAgentLocation(), 3.0)) {
        expected.push_back({segment.p1 - pos, segment.p2 - pos});
    }
    ASSERT_FALSE(expected.empty());

    const auto view = env.FirstAgentView(q);
    std::vector<LineSegment> actual{};
    double closest = std::numeric_limits<double>::max();
    for(const auto& wall : view.WallsInRange(3.0)) {
        actual.push_back(wall.segment);
        closest = std::min(closest, wall.distance);
    }

    EXPECT_EQ(actual, expected);

    // The near face of the wall block sits at x = 0.9, so measured from the agent it is
    // 1.9 away. Anything computed against the origin instead would report 0.9.
    EXPECT_DOUBLE_EQ(closest, 1.9);
}

TEST(AgentView, WallViewProjectsOntoTheWallAndPointsBackAtTheAgent)
{
    Environment env{};
    const Point pos{-1.0, 2.0};
    env.add_agent(pos);
    const auto geo = WalledGeometry();
    const auto q = env.query(*geo);

    const auto view = env.FirstAgentView(q);
    auto walls = view.WallsInRange(3.0);
    ASSERT_FALSE(walls.empty());
    // The near face of the wall block spans the agent's y, so the agent faces it head on.
    const WallView nearest =
        *std::ranges::min_element(walls, {}, [](const WallView& w) { return w.distance; });

    EXPECT_DOUBLE_EQ(nearest.closest_point.x, 1.9);
    EXPECT_NEAR(nearest.closest_point.y, 0.0, 1e-12);
    EXPECT_DOUBLE_EQ(nearest.distance, 1.9);
    // The wall is to the agent's right, so a normal pointing back at the agent faces -x.
    EXPECT_DOUBLE_EQ(nearest.normal.x, -1.0);
    EXPECT_NEAR(nearest.normal.y, 0.0, 1e-12);
}

TEST(AgentView, AgentsOnAnotherStoreyAreNotNeighbours)
{
    // Two floors sharing a footprint, one agent on each, standing at the same (x, y) three
    // metres apart in height. The neighbourhood grid searches by (x, y) and offers them to
    // each other; one is standing well above the other's head and cannot touch them.
    Geometry3D geo{fixtures::stacked_floors()};

    AgentContainer<GenericAgent> agents{};
    agents.push_back(MakeAgent(geo, {5.0, 5.0}));
    agents.push_back(MakeAgent(geo, {5.0, 5.0}, 0.2, 3.0));

    NeighborhoodSearch<GenericAgent> search{5.0};
    search.Update(agents);
    const EnvironmentQuery query{geo, search};

    EXPECT_TRUE(AgentView(query, agents[0]).OtherAgentsInRange(10.0).empty());
    EXPECT_TRUE(AgentView(query, agents[1]).OtherAgentsInRange(10.0).empty());
}

TEST(AgentView, AgentsOnTheSameStoreyStillAre)
{
    // The same mesh, both agents on the lower floor: the filter must not swallow those.
    Geometry3D geo{fixtures::stacked_floors()};

    AgentContainer<GenericAgent> agents{};
    agents.push_back(MakeAgent(geo, {5.0, 5.0}));
    agents.push_back(MakeAgent(geo, {6.0, 5.0}));

    NeighborhoodSearch<GenericAgent> search{5.0};
    search.Update(agents);
    const EnvironmentQuery query{geo, search};

    EXPECT_EQ(AgentView(query, agents[0]).OtherAgentsInRange(10.0).size(), 1u);
}

TEST(AgentView, ANeighbourCloseEnoughToTouchCanStillBeOnAnotherStorey)
{
    // A mezzanine 1.5 m up, so the height band keeps both as candidates and what has to tell
    // them apart is which sheet each stands on. Nothing is between them in plan either: no
    // wall on either floor, and no seam joining the two.
    Geometry3D geo{fixtures::stacked_floors(1.5)};

    AgentContainer<GenericAgent> agents{};
    agents.push_back(MakeAgent(geo, {2.0, 5.0}));
    agents.push_back(MakeAgent(geo, {4.0, 5.0}));
    agents.push_back(MakeAgent(geo, {6.0, 5.0}, 0.2, 1.5));

    NeighborhoodSearch<GenericAgent> search{5.0};
    search.Update(agents);
    const EnvironmentQuery query{geo, search};

    const AgentView view{query, agents[0]};
    const auto seen = view.OtherAgentsInRange(
        10.0, [&](const NeighborView& n) { return view.NoGeometryBetween(n); });

    ASSERT_EQ(seen.size(), 1u);
    EXPECT_EQ(seen[0].RelativePosition, Point(2.0, 0.0));
}
