// SPDX-License-Identifier: LGPL-3.0-or-later
#include "agent_view.hpp"
#include "environment_query.hpp"
#include "generic_agent.hpp"
#include "geometry/geometry.hpp"
#include "geometry_fixtures.hpp"
#include "neighborhood_search.hpp"
#include "operational_models/collision_free_speed_model/collision_free_speed_model.hpp"
#include "test_common.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <limits>
#include <memory>
#include <vector>

namespace
{
using State = CollisionFreeSpeedModel::State;

GenericAgent make_agent(const Geometry& geo, Point pos, double radius = 0.2, double z = 0.0)
{
    State s{};
    s.radius = radius;
    return GenericAgent(
        GenericAgent::ID::invalid,
        jps::UniqueID<Journey>::invalid,
        jps::UniqueID<BaseStage>::invalid,
        *geo.get_location_near_z(pos.x, pos.y, z),
        std::move(s));
}

std::unique_ptr<Geometry> open_geometry()
{
    return test_geometries::rectangle({-100, -100}, {100, 100});
}

// Geometry with a thin wall at x≈1 that blocks line-of-sight across it.
std::unique_ptr<Geometry> walled_geometry()
{
    return test_geometries::rectangle_with_hole({-100, -100}, {100, 100}, {0.9, -50}, {1.1, 50});
}

struct Environment {
    AgentContainer<GenericAgent> agents{};
    NeighborhoodSearch<GenericAgent> neighborhood_search{5.0};
    std::vector<std::pair<Point, double>> requested{};

    void add_agent(Point pos, double radius = 0.2) { requested.emplace_back(pos, radius); }

    // Agents are asked for before the geometry exists, so they are made here -- only the
    // geometry can put them onto the surface, the same way Simulation::add_agent does.
    EnvironmentQuery query(const Geometry& geo)
    {
        for(const auto& [pos, radius] : requested) {
            agents.push_back(make_agent(geo, pos, radius));
        }
        requested.clear();
        neighborhood_search.update(agents);
        return {geo, neighborhood_search};
    }

    // The first agent added is the one every test queries from.
    AgentView first_agent_view(const EnvironmentQuery& q) const { return {q, agents[0]}; }

    const Location& first_agent_location() const { return agents[0].location; }
};
} // namespace

TEST(AgentView, OtherAgentsInRangeExcludesSelf)
{
    Environment env{};
    env.add_agent({0, 0});
    const auto geo = open_geometry();
    const auto q = env.query(*geo);

    const auto result = env.first_agent_view(q).other_agents_in_range(100.0);
    EXPECT_TRUE(result.empty());
}

TEST(AgentView, OtherAgentsInRangeNoFilterReturnsAllInRadius)
{
    Environment env{};
    env.add_agent({0, 0}); // querying agent
    env.add_agent({1, 0});
    env.add_agent({0, 1});
    env.add_agent({-1, 0});
    const auto geo = open_geometry();
    const auto q = env.query(*geo);

    const auto result = env.first_agent_view(q).other_agents_in_range(5.0);
    EXPECT_EQ(result.size(), 3u);
}

TEST(AgentView, OtherAgentsInRangeCustomFilterRejectsAll)
{
    Environment env{};
    env.add_agent({0, 0});
    env.add_agent({1, 0});
    env.add_agent({0, 1});
    const auto geo = open_geometry();
    const auto q = env.query(*geo);

    const auto result = env.first_agent_view(q).other_agents_in_range(
        5.0, [](const NeighborView&) { return false; });
    EXPECT_TRUE(result.empty());
}

TEST(AgentView, OtherAgentsInRangeCustomFilterSelectsSubset)
{
    Environment env{};
    env.add_agent({0, 0}); // querying agent
    env.add_agent({1, 0}); // positive x — kept
    env.add_agent({0, 1}); // positive y — kept
    env.add_agent({-1, 0}); // negative x — filtered out
    const auto geo = open_geometry();
    const auto q = env.query(*geo);

    const auto result = env.first_agent_view(q).other_agents_in_range(
        5.0, [](const NeighborView& n) { return n.relative_position.x >= 0.0; });

    ASSERT_EQ(result.size(), 2u);
    for(const auto& neighbor : result) {
        EXPECT_GE(neighbor.relative_position.x, 0.0);
    }
}

TEST(AgentView, NoGeometryBetweenFiltersOccludedAgents)
{
    Environment env{};
    env.add_agent({0, 0}); // querying agent
    env.add_agent({2, 0}); // behind wall — occluded
    env.add_agent({0, 1}); // same side as querying agent — visible
    const auto geo = walled_geometry();
    const auto q = env.query(*geo);

    const auto view = env.first_agent_view(q);
    const auto result = view.other_agents_in_range(
        5.0, [&](const NeighborView& n) { return view.no_geometry_between(n); });

    ASSERT_EQ(result.size(), 1u);
    EXPECT_EQ(result[0].relative_position, Point(0, 1));
}

TEST(AgentView, OtherAgentsInRangeCustomFilterReceivesNoSelf)
{
    // Verify the filter is never called with the querying agent itself.
    Environment env{};
    env.add_agent({0, 0});
    env.add_agent({1, 0});
    const auto geo = open_geometry();
    const auto q = env.query(*geo);

    int calls = 0;
    env.first_agent_view(q).other_agents_in_range(5.0, [&](const NeighborView&) {
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
    const auto geo = open_geometry();
    const auto q = env.query(*geo);

    const auto result = env.first_agent_view(q).other_agents_in_range(
        1.0, [](const NeighborView&) { return true; });
    EXPECT_TRUE(result.empty());
}

TEST(AgentView, AgentsOnTheSamePositionSeeEachOther)
{
    Environment env{};
    env.add_agent({3, 4});
    env.add_agent({3, 4});
    const auto geo = open_geometry();
    const auto q = env.query(*geo);

    const auto result = env.first_agent_view(q).other_agents_in_range(1.0);

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
    const auto geo = open_geometry();
    const auto q = env.query(*geo);

    const auto result = env.first_agent_view(q).other_agents_in_range(2.0);

    ASSERT_EQ(result.size(), 1u);
    EXPECT_EQ(result[0].relative_position, Point(1, 0));
}

TEST(AgentView, WallsInRangeAreRelativeToTheAgent)
{
    Environment env{};
    const Point pos{-1.0, 2.0};
    env.add_agent(pos);
    const auto geo = walled_geometry();
    const auto q = env.query(*geo);

    std::vector<LineSegment> expected{};
    for(const auto& segment : q.line_segments_in_range(env.first_agent_location(), 3.0)) {
        expected.push_back({segment.p1 - pos, segment.p2 - pos});
    }
    ASSERT_FALSE(expected.empty());

    const auto view = env.first_agent_view(q);
    std::vector<LineSegment> actual{};
    double closest = std::numeric_limits<double>::max();
    for(const auto& wall : view.walls_in_range(3.0)) {
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
    const auto geo = walled_geometry();
    const auto q = env.query(*geo);

    const auto view = env.first_agent_view(q);
    auto walls = view.walls_in_range(3.0);
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
    const auto geo = test_geometries::stacked_floors({0, 0}, {10, 10}, 3.0);

    AgentContainer<GenericAgent> agents{};
    agents.push_back(make_agent(*geo, {5.0, 5.0}));
    agents.push_back(make_agent(*geo, {5.0, 5.0}, 0.2, 3.0));

    NeighborhoodSearch<GenericAgent> search{5.0};
    search.update(agents);
    const EnvironmentQuery query{*geo, search};

    EXPECT_TRUE(AgentView(query, agents[0]).other_agents_in_range(10.0).empty());
    EXPECT_TRUE(AgentView(query, agents[1]).other_agents_in_range(10.0).empty());
}

TEST(AgentView, AgentsOnTheSameStoreyStillAre)
{
    // The same mesh, both agents on the lower floor: the filter must not swallow those.
    const auto geo = test_geometries::stacked_floors({0, 0}, {10, 10}, 3.0);

    AgentContainer<GenericAgent> agents{};
    agents.push_back(make_agent(*geo, {5.0, 5.0}));
    agents.push_back(make_agent(*geo, {6.0, 5.0}));

    NeighborhoodSearch<GenericAgent> search{5.0};
    search.update(agents);
    const EnvironmentQuery query{*geo, search};

    EXPECT_EQ(AgentView(query, agents[0]).other_agents_in_range(10.0).size(), 1u);
}

TEST(AgentView, ANeighbourCloseEnoughToTouchCanStillBeOnAnotherStorey)
{
    // A mezzanine 1.5 m up, so the height band keeps both as candidates and what has to tell
    // them apart is which sheet each stands on. Nothing is between them in plan either: no
    // wall on either floor, and no seam joining the two.
    const auto geo = test_geometries::stacked_floors({0, 0}, {10, 10}, 1.5);

    AgentContainer<GenericAgent> agents{};
    agents.push_back(make_agent(*geo, {2.0, 5.0}));
    agents.push_back(make_agent(*geo, {4.0, 5.0}));
    agents.push_back(make_agent(*geo, {6.0, 5.0}, 0.2, 1.5));

    NeighborhoodSearch<GenericAgent> search{5.0};
    search.update(agents);
    const EnvironmentQuery query{*geo, search};

    const AgentView view{query, agents[0]};
    const auto seen = view.other_agents_in_range(
        10.0, [&](const NeighborView& n) { return view.no_geometry_between(n); });

    ASSERT_EQ(seen.size(), 1u);
    EXPECT_EQ(seen[0].relative_position, Point(2.0, 0.0));
}
