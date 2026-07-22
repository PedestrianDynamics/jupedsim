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

// Build NeighborhoodSearch populated with all agents in the container.
// Returns both so the caller can keep both alive.
struct Fixture {
    AgentContainer<GenericAgent> agents{};
    NeighborhoodSearch<GenericAgent> nsearch{5.0};

    void add(GenericAgent a) { agents.push_back(std::move(a)); }

    void commit() { nsearch.Update(agents); }
};
} // namespace

TEST(EnvironmentQuery, AgentsInRangeExcludesSelf)
{
    Fixture f{};
    f.add(MakeAgent({0, 0}));
    f.commit();

    const auto geo = OpenGeometry();
    const EnvironmentQuery q{geo, f.nsearch};

    const auto result = q.AgentsInRange(f.agents[0].model, 100.0);
    EXPECT_TRUE(result.empty());
}

TEST(EnvironmentQuery, AgentsInRangeNoFilterReturnsAllInRadius)
{
    Fixture f{};
    f.add(MakeAgent({0, 0})); // querying agent
    f.add(MakeAgent({1, 0}));
    f.add(MakeAgent({0, 1}));
    f.add(MakeAgent({-1, 0}));
    f.commit();

    const auto geo = OpenGeometry();
    const EnvironmentQuery q{geo, f.nsearch};

    const auto result = q.AgentsInRange(f.agents[0].model, 5.0);
    EXPECT_EQ(result.size(), 3u);
}

TEST(EnvironmentQuery, AgentsInRangeCustomFilterRejectsAll)
{
    Fixture f{};
    f.add(MakeAgent({0, 0}));
    f.add(MakeAgent({1, 0}));
    f.add(MakeAgent({0, 1}));
    f.commit();

    const auto geo = OpenGeometry();
    const EnvironmentQuery q{geo, f.nsearch};

    const auto result =
        q.AgentsInRange(f.agents[0].model, 5.0, [](const GenericAgent&) { return false; });
    EXPECT_TRUE(result.empty());
}

TEST(EnvironmentQuery, AgentsInRangeCustomFilterSelectsSubset)
{
    Fixture f{};
    f.add(MakeAgent({0, 0})); // querying agent (radius 0.2)
    f.add(MakeAgent({1, 0}, 0.2)); // small radius — filtered out
    f.add(MakeAgent({0, 1}, 0.4)); // large radius — kept
    f.add(MakeAgent({-1, 0}, 0.4)); // large radius — kept
    f.commit();

    const auto geo = OpenGeometry();
    const EnvironmentQuery q{geo, f.nsearch};

    const auto result = q.AgentsInRange(f.agents[0].model, 5.0, [](const GenericAgent& a) {
        return std::get<State>(a.model).radius > 0.25;
    });

    ASSERT_EQ(result.size(), 2u);
    for(const auto& neighbor : result) {
        EXPECT_GT(std::get<State>(neighbor.model).radius, 0.25);
    }
}

TEST(EnvironmentQuery, AgentsInRangeCustomFilterReceivesNoSelf)
{
    // Verify the filter is never called with the querying agent itself.
    Fixture f{};
    f.add(MakeAgent({0, 0}));
    f.add(MakeAgent({1, 0}));
    f.commit();

    const auto geo = OpenGeometry();
    const EnvironmentQuery q{geo, f.nsearch};

    const auto selfId = f.agents[0].id;
    bool selfSeen = false;
    q.AgentsInRange(f.agents[0].model, 5.0, [&](const GenericAgent& a) {
        if(a.id == selfId) {
            selfSeen = true;
        }
        return true;
    });

    EXPECT_FALSE(selfSeen);
}

TEST(EnvironmentQuery, AgentsInRangeOutOfRadiusNotReturned)
{
    Fixture f{};
    f.add(MakeAgent({0, 0}));
    f.add(MakeAgent({50, 0})); // far away
    f.commit();

    const auto geo = OpenGeometry();
    const EnvironmentQuery q{geo, f.nsearch};

    const auto result =
        q.AgentsInRange(f.agents[0].model, 1.0, [](const GenericAgent&) { return true; });
    EXPECT_TRUE(result.empty());
}
