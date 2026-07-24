// SPDX-License-Identifier: LGPL-3.0-or-later
#include "EnvironmentQuery.hpp"
#include "GenericAgent.hpp"
#include "GeometryBuilder.hpp"
#include "NeighborhoodSearch.hpp"
#include "OperationalModels/CollisionFreeSpeedModel/CollisionFreeSpeedModel.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <vector>

namespace
{
using State = CollisionFreeSpeedModel::State;

GenericAgent MakeAgent(Point pos, double radius = 0.2)
{
    State s{};
    s.position = pos;
    s.radius = radius;
    return GenericAgent(
        GenericAgent::ID::Invalid,
        jps::UniqueID<Journey>::Invalid,
        jps::UniqueID<BaseStage>::Invalid,
        std::move(s));
}

CollisionGeometry OpenGeometry()
{
    GeometryBuilder b{};
    b.AddAccessibleArea({{-100, -100}, {100, -100}, {100, 100}, {-100, 100}});
    return b.Build();
}

// Geometry with a thin wall at x≈1 that blocks line-of-sight across it.
CollisionGeometry WalledGeometry()
{
    GeometryBuilder b{};
    b.AddAccessibleArea({{-100, -100}, {100, -100}, {100, 100}, {-100, 100}});
    b.ExcludeFromAccessibleArea({{0.9, -50}, {1.1, -50}, {1.1, 50}, {0.9, 50}});
    return b.Build();
}

struct Environment {
    AgentContainer<GenericAgent> agents{};
    NeighborhoodSearch<GenericAgent> neighborhood_search{5.0};

    void add_agent(Point pos, double radius = 0.2) { agents.push_back(MakeAgent(pos, radius)); }

    EnvironmentQuery query(const CollisionGeometry& geo)
    {
        neighborhood_search.Update(agents);
        return {geo, neighborhood_search};
    }
};
} // namespace

TEST(EnvironmentQuery, AgentsInRangeExcludesSelf)
{
    Environment f{};
    f.add_agent({0, 0});
    const auto geo = OpenGeometry();
    const auto q = f.query(geo);

    const auto result = q.AgentsInRange(f.agents[0].model, 100.0);
    EXPECT_TRUE(result.empty());
}

TEST(EnvironmentQuery, AgentsInRangeNoFilterReturnsAllInRadius)
{
    Environment f{};
    f.add_agent({0, 0}); // querying agent
    f.add_agent({1, 0});
    f.add_agent({0, 1});
    f.add_agent({-1, 0});
    const auto geo = OpenGeometry();
    const auto q = f.query(geo);

    const auto result = q.AgentsInRange(f.agents[0].model, 5.0);
    EXPECT_EQ(result.size(), 3u);
}

TEST(EnvironmentQuery, AgentsInRangeCustomFilterRejectsAll)
{
    Environment f{};
    f.add_agent({0, 0});
    f.add_agent({1, 0});
    f.add_agent({0, 1});
    const auto geo = OpenGeometry();
    const auto q = f.query(geo);

    const auto result = q.AgentsInRange(f.agents[0].model, 5.0, [](const Point&) { return false; });
    EXPECT_TRUE(result.empty());
}

TEST(EnvironmentQuery, AgentsInRangeCustomFilterSelectsSubset)
{
    Environment f{};
    f.add_agent({0, 0}); // querying agent
    f.add_agent({1, 0}); // positive x — kept
    f.add_agent({0, 1}); // positive y — kept
    f.add_agent({-1, 0}); // negative x — filtered out
    const auto geo = OpenGeometry();
    const auto q = f.query(geo);

    const auto result =
        q.AgentsInRange(f.agents[0].model, 5.0, [](const Point& to) { return to.x >= 0.0; });

    ASSERT_EQ(result.size(), 2u);
    for(const auto& neighbor : result) {
        EXPECT_GE(std::get<State>(neighbor.model).position.x, 0.0);
    }
}

TEST(EnvironmentQuery, NoGeometryBetweenFiltersOccludedAgents)
{
    Environment f{};
    f.add_agent({0, 0}); // querying agent
    f.add_agent({2, 0}); // behind wall — occluded
    f.add_agent({0, 1}); // same side as querying agent — visible
    const auto geo = WalledGeometry();
    const auto q = f.query(geo);

    const auto from = f.agents[0].position();
    const auto result = q.AgentsInRange(
        f.agents[0].model, 5.0, [&](const Point& to) { return q.NoGeometryBetween(from, to); });

    ASSERT_EQ(result.size(), 1u);
    EXPECT_EQ(std::get<State>(result[0].model).position, Point(0, 1));
}

TEST(EnvironmentQuery, AgentsInRangeCustomFilterReceivesNoSelf)
{
    // Verify the filter is never called with the querying agent itself.
    Environment f{};
    f.add_agent({0, 0});
    f.add_agent({1, 0});
    const auto geo = OpenGeometry();
    const auto q = f.query(geo);

    const auto selfPos = f.agents[0].position();
    q.AgentsInRange(f.agents[0].model, 5.0, [&](const Point& to) {
        if(to == selfPos) {
            ADD_FAILURE() << "filter was called with the querying agent's own position";
        }
        return true;
    });
}

TEST(EnvironmentQuery, AgentsInRangeOutOfRadiusNotReturned)
{
    Environment f{};
    f.add_agent({0, 0});
    f.add_agent({50, 0}); // far away
    const auto geo = OpenGeometry();
    const auto q = f.query(geo);

    const auto result = q.AgentsInRange(f.agents[0].model, 1.0, [](const Point&) { return true; });
    EXPECT_TRUE(result.empty());
}
