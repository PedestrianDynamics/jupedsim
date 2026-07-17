// SPDX-License-Identifier: LGPL-3.0-or-later
#include "NeighborhoodSearch3D.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <random>
#include <ranges>
#include <set>
#include <span>
#include <vector>

namespace
{
/// "ranges" filter to check horizontally within radius and vertically against height.
auto passes_guards(std::span<const Point3D> positions, const Point3D& p, double r, double h)
{
    return [positions, g1 = within_horizontal_distance(p, r), g2 = within_vertical_band(p, h)](
               std::size_t i) { return g1(positions[i]) && g2(positions[i]); };
}

/// Brute force check: Walk over all positions and check against guards filter.
std::set<std::size_t>
oracle(std::span<const Point3D> positions, const Point3D& p, double r, double h)
{
    auto matches = std::views::iota(std::size_t{0}, positions.size()) |
                   std::views::filter(passes_guards(positions, p, r, h));
    return {matches.begin(), matches.end()};
}

std::set<std::size_t> filtered_search(
    const NeighborhoodSearch3D& search,
    std::span<const Point3D> positions,
    const Point3D& p,
    double r,
    double h)
{
    auto matches =
        search.candidates(p, r, h) | std::views::filter(passes_guards(positions, p, r, h));
    return {matches.begin(), matches.end()};
}
} // namespace

TEST(NeighborhoodSearch3D, EmptyIndexYieldsNoCandidates)
{
    NeighborhoodSearch3D search{2.0};
    ASSERT_TRUE(search.candidates({0, 0, 0}, 5.0, 2.2).empty());
}

TEST(NeighborhoodSearch3D, FlatRoomMatchesBruteForceOracle)
{
    // All agents at z=0: 3D search must reproduce plain 2D semantics.
    std::vector<Point3D> positions{};
    for(int x = 0; x < 10; ++x) {
        for(int y = 0; y < 10; ++y) {
            positions.emplace_back(x * 0.7, y * 0.7, 0.0);
        }
    }
    NeighborhoodSearch3D search{2.0};
    search.rebuild_index(positions);

    // Test neighborhood search against oracle for different positions
    for(const auto& q :
        {Point3D{0, 0, 0}, Point3D{3.5, 3.5, 0}, Point3D{6.3, 0.7, 0}, Point3D{-1, -1, 0}}) {
        EXPECT_EQ(filtered_search(search, positions, q, 2.5, 2.2), oracle(positions, q, 2.5, 2.2));
    }
}

TEST(NeighborhoodSearch3D, CandidatesAreASuperset)
{
    std::vector<Point3D> positions{
        {0, 0, 0}, {1, 1, 0}, {2.4, 0, 0.5}, {0, 2.4, 1.0}, {5, 5, 0}, {0, 0, 3.0}};
    NeighborhoodSearch3D search{2.0};
    search.rebuild_index(positions);

    const Point3D q{0.5, 0.5, 0};
    EXPECT_THAT(
        search.candidates(q, 2.5, 2.2), testing::IsSupersetOf(oracle(positions, q, 2.5, 2.2)));
}

TEST(NeighborhoodSearch3D, StackedAgentsAreNotNeighbors)
{
    // Same (x,y), but large dz: Those should be filtered.
    const std::vector<Point3D> positions{{5, 5, 0}, {5, 5, 3.0}, {5.5, 5, 0}};
    NeighborhoodSearch3D search{2.0};
    search.rebuild_index(positions);

    const auto neighbors = filtered_search(search, positions, {5, 5, 0}, 2.0, 2.2);
    EXPECT_EQ(neighbors, (std::set<std::size_t>{0, 2}));
}

TEST(NeighborhoodSearch3D, AgentsOnARampStayNeighbors)
{
    // Small dz: Must still return as candidate.
    const std::vector<Point3D> positions{{0, 0, 0}, {0, 1, 0.5}, {0, 2, 1.0}};
    NeighborhoodSearch3D search{2.0};
    search.rebuild_index(positions);

    const auto neighbors = filtered_search(search, positions, {0, 0, 0}, 2.5, 2.2);
    EXPECT_EQ(neighbors, (std::set<std::size_t>{0, 1, 2})); // all entries
}

TEST(NeighborhoodSearch3D, RandomizedOracleComparison)
{
    // Multi-floor randomized points around origin (incl. negative coordinates).
    std::mt19937 rng{42};
    std::uniform_real_distribution<double> xy{-50.0, 50.0};
    std::uniform_real_distribution<double> z{-4.0, 12.0};

    std::vector<Point3D> positions{};
    positions.reserve(500);
    for(int i = 0; i < 500; ++i) {
        positions.emplace_back(xy(rng), xy(rng), z(rng));
    }
    NeighborhoodSearch3D search{2.0};
    search.rebuild_index(positions);

    for(int i = 0; i < 50; ++i) {
        const Point3D q{xy(rng), xy(rng), z(rng)};
        EXPECT_EQ(filtered_search(search, positions, q, 2.5, 2.2), oracle(positions, q, 2.5, 2.2))
            << "query " << q;
    }
}

TEST(NeighborhoodSearch3D, RebuildIndexAcceptsAnyPoint3DRange)
{
    // Test how a call with agents would look like.
    // [RL, TODO] This requires AggAgent to call a complete rebuild - same with removal.
    //            Still working on the semantics - maybe we need to change this again.
    //            Basically we get back indices into the container used at the time of
    //            calling "rebuild_index()".
    struct Agent {
        int id;
        Point3D pos;
    };
    const std::vector<Agent> agents{{7, {0, 0, 0}}, {8, {0.5, 0, 0}}, {9, {40, 40, 0}}};
    NeighborhoodSearch3D search{2.0};
    search.rebuild_index(agents | std::views::transform([](const Agent& a) { return a.pos; }));

    const auto near_origin = search.candidates({0, 0, 0}, 2.0, 2.2);
    EXPECT_EQ(
        std::set<std::size_t>(near_origin.begin(), near_origin.end()),
        (std::set<std::size_t>{0, 1}));
}

TEST(NeighborhoodSearch3D, RebuildIndexReplacesPreviousState)
{
    NeighborhoodSearch3D search{2.0};
    const std::vector<Point3D> initial{{0, 0, 0}, {1, 0, 0}};
    search.rebuild_index(initial);
    ASSERT_EQ(filtered_search(search, initial, {0, 0, 0}, 2.0, 2.2).size(), 2);

    const std::vector<Point3D> replaced{{40, 40, 0}};
    search.rebuild_index(replaced);
    EXPECT_TRUE(search.candidates({0, 0, 0}, 2.0, 2.2).empty());
    EXPECT_EQ(filtered_search(search, replaced, {40, 40, 0}, 2.0, 2.2), (std::set<std::size_t>{0}));
}
