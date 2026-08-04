// SPDX-License-Identifier: LGPL-3.0-or-later
#include "GenericAgent.hpp"
#include "NeighborhoodSearch.hpp"

#include <gtest/gtest.h>

#include <cmath>
#include <set>

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
std::set<int>
ItemIdsInRange(const NeighborhoodSearch<ValueWithPos<int>>& neighborhood, Point pos, double radius)
{
    std::set<int> ids{};
    neighborhood.ForEachInRange(
        pos, radius, [&ids](const ValueWithPos<int>& item) { ids.insert(item.val); });
    return ids;
}
} // namespace

TEST(NeighborhoodSearch, ReturnsEmptyOnEmpty)
{
    NeighborhoodSearch<ValueWithPos<int>> neighborhood{3};
    ASSERT_TRUE(ItemIdsInRange(neighborhood, {0, 0}, 10).empty());
}

TEST(NeighborhoodSearch, ReturnsOneValueInRange)
{
    NeighborhoodSearch<ValueWithPos<int>> neighborhood{3};
    const AgentContainer<ValueWithPos<int>> agents{{{0, 0}, 1}};
    neighborhood.Update(agents);

    ASSERT_EQ(ItemIdsInRange(neighborhood, {0, 0}, 10), (std::set<int>{1}));
}

TEST(NeighborhoodSearch, ReturnsMultipleValuesInRange)
{
    NeighborhoodSearch<ValueWithPos<int>> neighborhood{3};
    const AgentContainer<ValueWithPos<int>> agents{{{0, 0}, 1}, {{0, 0}, 0}};
    neighborhood.Update(agents);

    ASSERT_EQ(ItemIdsInRange(neighborhood, {0, 0}, 10), (std::set<int>{1, 0}));
}

TEST(NeighborhoodSearch, ReturnsValuesFromDifferentInternalGridCells)
{
    NeighborhoodSearch<ValueWithPos<int>> neighborhood{3};
    const AgentContainer<ValueWithPos<int>> agents{{{0, 0}, 1}, {{-3, 0}, 0}, {{4, 4}, 6}};
    neighborhood.Update(agents);

    ASSERT_EQ(ItemIdsInRange(neighborhood, {0, 0}, 10), (std::set<int>{1, 0, 6}));
}

TEST(NeighborhoodSearch, RejectesValuesInGridCellsTooFarAway)
{
    NeighborhoodSearch<ValueWithPos<int>> neighborhood{3};
    const AgentContainer<ValueWithPos<int>> agents{
        {{0, 0}, 1}, {{-3, 0}, 0}, {{4, 4}, 6}, {{10, 10}, 7}};
    neighborhood.Update(agents);

    ASSERT_EQ(ItemIdsInRange(neighborhood, {0, 0}, 10), (std::set<int>{1, 0, 6}));
}

TEST(NeighborhoodSearch, RejectsValuesFromSelectedGridThatareTooFarAway)
{
    NeighborhoodSearch<ValueWithPos<int>> neighborhood{3};
    const AgentContainer<ValueWithPos<int>> agents{{{0, 0}, 1}, {{0.5, 0.5}, 2}, {{0.4, 0.4}, 3}};
    neighborhood.Update(agents);

    ASSERT_EQ(ItemIdsInRange(neighborhood, {0, 0}, 0.41 * sqrt(2.0)), (std::set<int>{1, 3}));
}

TEST(NeighborhoodSearch, ReturnsValueExactlyDistanceAwayFromQueryPoint)
{
    NeighborhoodSearch<ValueWithPos<int>> neighborhood{3};
    const AgentContainer<ValueWithPos<int>> agents{{{1, 0}, 1}};
    neighborhood.Update(agents);

    ASSERT_EQ(ItemIdsInRange(neighborhood, {0, 0}, 1), (std::set<int>{1}));
}
