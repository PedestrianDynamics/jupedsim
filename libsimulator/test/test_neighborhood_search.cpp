// SPDX-License-Identifier: LGPL-3.0-or-later
#include "generic_agent.hpp"
#include "kernel.hpp"
#include "neighborhood_search.hpp"
#include "simulation_error.hpp"
#include "test_common.hpp"

#include <gtest/gtest.h>

#include <cmath>
#include <limits>
#include <set>
#include <vector>

/// Stands in for an agent's Location: of a stored value the grid only ever asks where it is
/// in plan, and building a real one would need a geometry this test has no use for.
struct PlanarLocation {
    PlanarLocation(double x = 0.0, double y = 0.0) : p(x, y) {}
    Point p;
    Point xy() const { return p; }
};

template <typename T>
struct ValueWithPos {
    PlanarLocation location{};
    T val;
};

namespace
{
std::set<int> item_ids_in_range(
    const NeighborhoodSearch<ValueWithPos<int>>& neighborhood,
    Point pos,
    double radius)
{
    std::set<int> ids{};
    neighborhood.for_each_in_range(
        pos, radius, [&ids](const ValueWithPos<int>& item) { ids.insert(item.val); });
    return ids;
}
} // namespace

TEST(NeighborhoodSearch, ReturnsEmptyOnEmpty)
{
    NeighborhoodSearch<ValueWithPos<int>> neighborhood{3};
    ASSERT_TRUE(item_ids_in_range(neighborhood, {0, 0}, 10).empty());
}

TEST(NeighborhoodSearch, ReturnsOneValueInRange)
{
    NeighborhoodSearch<ValueWithPos<int>> neighborhood{3};
    const AgentContainer<ValueWithPos<int>> agents{{{0, 0}, 1}};
    neighborhood.update(agents);

    ASSERT_EQ(item_ids_in_range(neighborhood, {0, 0}, 10), (std::set<int>{1}));
}

TEST(NeighborhoodSearch, ReturnsMultipleValuesInRange)
{
    NeighborhoodSearch<ValueWithPos<int>> neighborhood{3};
    const AgentContainer<ValueWithPos<int>> agents{{{0, 0}, 1}, {{0, 0}, 0}};
    neighborhood.update(agents);

    ASSERT_EQ(item_ids_in_range(neighborhood, {0, 0}, 10), (std::set<int>{1, 0}));
}

TEST(NeighborhoodSearch, ReturnsValuesFromDifferentInternalGridCells)
{
    NeighborhoodSearch<ValueWithPos<int>> neighborhood{3};
    const AgentContainer<ValueWithPos<int>> agents{{{0, 0}, 1}, {{-3, 0}, 0}, {{4, 4}, 6}};
    neighborhood.update(agents);

    ASSERT_EQ(item_ids_in_range(neighborhood, {0, 0}, 10), (std::set<int>{1, 0, 6}));
}

TEST(NeighborhoodSearch, RejectesValuesInGridCellsTooFarAway)
{
    NeighborhoodSearch<ValueWithPos<int>> neighborhood{3};
    const AgentContainer<ValueWithPos<int>> agents{
        {{0, 0}, 1}, {{-3, 0}, 0}, {{4, 4}, 6}, {{10, 10}, 7}};
    neighborhood.update(agents);

    ASSERT_EQ(item_ids_in_range(neighborhood, {0, 0}, 10), (std::set<int>{1, 0, 6}));
}

TEST(NeighborhoodSearch, RejectsValuesFromSelectedGridThatareTooFarAway)
{
    NeighborhoodSearch<ValueWithPos<int>> neighborhood{3};
    const AgentContainer<ValueWithPos<int>> agents{{{0, 0}, 1}, {{0.5, 0.5}, 2}, {{0.4, 0.4}, 3}};
    neighborhood.update(agents);

    ASSERT_EQ(item_ids_in_range(neighborhood, {0, 0}, 0.41 * sqrt(2.0)), (std::set<int>{1, 3}));
}

TEST(NeighborhoodSearch, ReturnsValueExactlyDistanceAwayFromQueryPoint)
{
    NeighborhoodSearch<ValueWithPos<int>> neighborhood{3};
    const AgentContainer<ValueWithPos<int>> agents{{{1, 0}, 1}};
    neighborhood.update(agents);

    ASSERT_EQ(item_ids_in_range(neighborhood, {0, 0}, 1), (std::set<int>{1}));
}

TEST(NeighborhoodSearch, FindsValuesOnBothSidesOfTheAxes)
{
    NeighborhoodSearch<ValueWithPos<int>> neighborhood{3};
    const AgentContainer<ValueWithPos<int>> agents{
        {{-0.5, -0.5}, 1}, {{0.5, 0.5}, 2}, {{-5.9, 0}, 3}, {{0, 5.9}, 4}, {{-6.1, 0}, 5}};
    neighborhood.update(agents);

    ASSERT_EQ(item_ids_in_range(neighborhood, {0, 0}, 6), (std::set<int>{1, 2, 3, 4}));
    ASSERT_EQ(item_ids_in_range(neighborhood, {-6, 0}, 0.2), (std::set<int>{3, 5}));
}

TEST(NeighborhoodSearch, FindsValuesFromAPointOffTheGrid)
{
    NeighborhoodSearch<ValueWithPos<int>> neighborhood{3};
    const AgentContainer<ValueWithPos<int>> agents{{{0, 0}, 1}, {{1, 1}, 2}};
    neighborhood.update(agents);

    ASSERT_EQ(item_ids_in_range(neighborhood, {-4, -4}, 8), (std::set<int>{1, 2}));
    ASSERT_TRUE(item_ids_in_range(neighborhood, {100, 100}, 10).empty());
}

TEST(NeighborhoodSearch, AHugeRadiusFindsEverything)
{
    NeighborhoodSearch<ValueWithPos<int>> neighborhood{3};
    const AgentContainer<ValueWithPos<int>> agents{{{0, 0}, 1}, {{50, -50}, 2}};
    neighborhood.update(agents);

    ASSERT_EQ(item_ids_in_range(neighborhood, {1e12, 1e12}, 1e13), (std::set<int>{1, 2}));
}

TEST(NeighborhoodSearch, AddedValuesAreFoundUntilTheNextUpdate)
{
    NeighborhoodSearch<ValueWithPos<int>> neighborhood{3};
    AgentContainer<ValueWithPos<int>> agents{{{0, 0}, 1}};
    neighborhood.update(agents);
    agents.push_back({{20, 20}, 2});
    neighborhood.add_agent(agents.back());

    ASSERT_EQ(item_ids_in_range(neighborhood, {20, 20}, 1), (std::set<int>{2}));
    ASSERT_EQ(item_ids_in_range(neighborhood, {10, 10}, 20), (std::set<int>{1, 2}));

    agents.pop_back();
    neighborhood.update(agents);
    ASSERT_TRUE(item_ids_in_range(neighborhood, {20, 20}, 1).empty());
}

TEST(NeighborhoodSearch, ManyAddedValuesAreSortedInBehindTheStoredOnes)
{
    NeighborhoodSearch<ValueWithPos<int>> neighborhood{3};
    AgentContainer<ValueWithPos<int>> agents{{{0, 0}, 0}};
    neighborhood.update(agents);
    // Enough added values to be sorted in, one far off the grid, and one more added after that.
    agents.push_back({{30, 30}, -1});
    neighborhood.add_agent(agents.back());
    std::vector<int> expected{0};
    for(int id = 1; id <= 1001; ++id) {
        agents.push_back({{0.0004 * id, 0.0004 * id}, id});
        neighborhood.add_agent(agents.back());
        expected.push_back(id);
    }

    std::vector<int> visited{};
    neighborhood.for_each_in_range(
        {0, 0}, 1, [&visited](const ValueWithPos<int>& item) { visited.push_back(item.val); });
    ASSERT_EQ(visited, expected);
    ASSERT_EQ(item_ids_in_range(neighborhood, {30, 30}, 1), (std::set<int>{-1}));
}

TEST(NeighborhoodSearch, VisitsValuesOfACellInContainerOrder)
{
    NeighborhoodSearch<ValueWithPos<int>> neighborhood{3};
    const AgentContainer<ValueWithPos<int>> agents{
        {{0.3, 0.3}, 3}, {{9, 9}, 9}, {{0.1, 0.1}, 1}, {{0.2, 0.2}, 2}};
    neighborhood.update(agents);

    std::vector<int> visited{};
    neighborhood.for_each_in_range(
        {0, 0}, 1, [&visited](const ValueWithPos<int>& item) { visited.push_back(item.val); });
    ASSERT_EQ(visited, (std::vector<int>{3, 1, 2}));
}

TEST(NeighborhoodSearch, FindsValuesOfGroupsFarApart)
{
    NeighborhoodSearch<ValueWithPos<int>> neighborhood{3};
    const AgentContainer<ValueWithPos<int>> agents{
        {{0.3, 0.3}, 3},
        {{1e5, 1e5}, 9},
        {{0.1, 0.1}, 1},
        {{-1e5, 2e5}, 8},
        {{0.2, 0.2}, 2},
        {{1e5 + 1, 1e5}, 10}};
    neighborhood.update(agents);

    std::vector<int> visited{};
    neighborhood.for_each_in_range(
        {0, 0}, 1, [&visited](const ValueWithPos<int>& item) { visited.push_back(item.val); });
    ASSERT_EQ(visited, (std::vector<int>{3, 1, 2}));
    ASSERT_EQ(item_ids_in_range(neighborhood, {1e5, 1e5}, 2), (std::set<int>{9, 10}));
    ASSERT_EQ(item_ids_in_range(neighborhood, {-1e5, 2e5}, 2), (std::set<int>{8}));
    ASSERT_TRUE(item_ids_in_range(neighborhood, {5e4, 5e4}, 100).empty());
    ASSERT_EQ(item_ids_in_range(neighborhood, {0, 0}, 1e13), (std::set<int>{1, 2, 3, 8, 9, 10}));
}

TEST(NeighborhoodSearch, ManyAddedValuesAreSortedInBehindTheStoredOnesOfGroupsFarApart)
{
    NeighborhoodSearch<ValueWithPos<int>> neighborhood{3};
    AgentContainer<ValueWithPos<int>> agents{{{0, 0}, 0}, {{1e5, 1e5}, -1}};
    neighborhood.update(agents);
    std::vector<int> expected{0};
    for(int id = 1; id <= 1001; ++id) {
        agents.push_back({{0.0004 * id, 0.0004 * id}, id});
        neighborhood.add_agent(agents.back());
        expected.push_back(id);
    }

    std::vector<int> visited{};
    neighborhood.for_each_in_range(
        {0, 0}, 1, [&visited](const ValueWithPos<int>& item) { visited.push_back(item.val); });
    ASSERT_EQ(visited, expected);
    ASSERT_EQ(item_ids_in_range(neighborhood, {1e5, 1e5}, 1), (std::set<int>{-1}));
}

TEST(NeighborhoodSearch, FindsValuesNextToHugeCoordinates)
{
    NeighborhoodSearch<ValueWithPos<int>> neighborhood{3};
    const AgentContainer<ValueWithPos<int>> agents{{{0, 0}, 1}, {{1e300, 0}, 2}, {{0, -1e300}, 3}};
    neighborhood.update(agents);

    ASSERT_EQ(item_ids_in_range(neighborhood, {0, 0}, 1), (std::set<int>{1}));
    ASSERT_EQ(item_ids_in_range(neighborhood, {1e300, 0}, 1), (std::set<int>{2}));
}

TEST(NeighborhoodSearch, RejectsPositionsThatAreNotFinite)
{
    NeighborhoodSearch<ValueWithPos<int>> neighborhood{3};
    const auto nan = std::numeric_limits<double>::quiet_NaN();
    const auto inf = std::numeric_limits<double>::infinity();
    for(const auto& bad : {Point{nan, 0}, Point{0, nan}, Point{inf, 0}, Point{0, -inf}}) {
        const AgentContainer<ValueWithPos<int>> agents{{{bad.x(), bad.y()}, 1}, {{0, 0}, 2}};
        EXPECT_THROW(neighborhood.update(agents), SimulationError);
    }
}
