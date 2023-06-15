/// Copyright © 2012-2022 Forschungszentrum Jülich GmbH
/// SPDX-License-Identifier: LGPL-3.0-or-later
#include "NeighborhoodSearch.hpp"

#include <cmath>
#include <gtest/gtest.h>
#include <iostream>
#include <iterator>
#include <memory>

TEST(Grid2D, EmptyDefauneighborhood_searchorhood_searchtruction)
{
    Grid2D<int> grid;

    ASSERT_EQ(grid.size(), 0);
    ASSERT_TRUE(grid.empty());
}

TEST(Grid2D, Construction)
{
    std::vector<Grid2D<double>::Entry> values;

    for(int i = 0; i < 100; ++i) {
        values.push_back({{100 - i, i}, {100.0 - i, static_cast<double>(i)}, 2.4});
    }

    Grid2D<double> grid(values);

    ASSERT_EQ(grid.size(), 100);
    ASSERT_FALSE(grid.empty());

    auto it_pair = grid.get({99, 1});
    ASSERT_EQ(std::distance(it_pair.first(), it_pair.second()), 1);
    ASSERT_EQ(it_pair.first()->value, 2.4);
}

template <typename T>
struct ValueWithPos {
    Point pos{};
    T val;
};

TEST(NeighborhoodSearch, ReturnsEmptyOnEmpty)
{
    NeighborhoodSearch<int> neighborhood{3};
    const auto range = neighborhood.GetNeighboringAgents({0, 0}, 10);
    ASSERT_EQ(std::begin(range), std::end(range));
}

TEST(NeighborhoodSearch, ReturnsOneValueInRange)
{
    NeighborhoodSearch<ValueWithPos<int>> neighborhood{3};
    const std::vector<ValueWithPos<int>> agents{{{0, 0}, 1}};
    neighborhood.Update(agents);

    const auto expected = std::set<int>{1};
    const auto result = neighborhood.GetNeighboringAgents({0, 0}, 10);
    std::set<int> actual{};
    std::transform(
        std::begin(result),
        std::end(result),
        std::inserter(actual, std::begin(actual)),
        [](const auto& v) { return v->val; });
    ASSERT_EQ(actual, expected);
}

TEST(NeighborhoodSearch, ReturnsMultipleValuesInRange)
{
    NeighborhoodSearch<ValueWithPos<int>> neighborhood{3};
    const std::vector<ValueWithPos<int>> agents{{{0, 0}, 1}, {{0, 0}, 0}};
    neighborhood.Update(agents);

    const auto expected = std::set<int>{1, 0};
    const auto result = neighborhood.GetNeighboringAgents({0, 0}, 10);
    std::set<int> actual{};
    std::transform(
        std::begin(result),
        std::end(result),
        std::inserter(actual, std::begin(actual)),
        [](const auto& v) { return v->val; });
    ASSERT_EQ(actual, expected);
}

TEST(NeighborhoodSearch, ReturnsValuesFromDifferentInternalGridCells)
{
    NeighborhoodSearch<ValueWithPos<int>> neighborhood{3};
    const std::vector<ValueWithPos<int>> agents{{{0, 0}, 1}, {{-3, 0}, 0}, {{4, 4}, 6}};
    neighborhood.Update(agents);

    const auto expected = std::set<int>{1, 0, 6};
    const auto result = neighborhood.GetNeighboringAgents({0, 0}, 10);
    std::set<int> actual{};
    std::transform(
        std::begin(result),
        std::end(result),
        std::inserter(actual, std::begin(actual)),
        [](const auto& v) { return v->val; });
    ASSERT_EQ(actual, expected);
}

TEST(NeighborhoodSearch, RejectesValuesInGridCellsTooFarAway)
{
    NeighborhoodSearch<ValueWithPos<int>> neighborhood{3};
    const std::vector<ValueWithPos<int>> agents{
        {{0, 0}, 1}, {{-3, 0}, 0}, {{4, 4}, 6}, {{10, 10}, 7}};
    neighborhood.Update(agents);

    const auto expected = std::set<int>{1, 0, 6};
    const auto result = neighborhood.GetNeighboringAgents({0, 0}, 10);
    std::set<int> actual{};
    std::transform(
        std::begin(result),
        std::end(result),
        std::inserter(actual, std::begin(actual)),
        [](const auto& v) { return v->val; });
    ASSERT_EQ(actual, expected);
}

TEST(NeighborhoodSearch, RejectsValuesFromSelectedGridThatareTooFarAway)
{
    NeighborhoodSearch<ValueWithPos<int>> neighborhood{3};
    const std::vector<ValueWithPos<int>> agents{{{0, 0}, 1}, {{0.5, 0.5}, 2}, {{0.4, 0.4}, 3}};
    neighborhood.Update(agents);

    const auto expected = std::set<int>{1, 3};
    const auto result = neighborhood.GetNeighboringAgents({0, 0}, 0.41 * sqrt(2.0));
    std::set<int> actual{};
    std::transform(
        std::begin(result),
        std::end(result),
        std::inserter(actual, std::begin(actual)),
        [](const auto& v) { return v->val; });
    ASSERT_EQ(actual, expected);
}

TEST(NeighborhoodSearch, ReturnsValueExactlyDistanceAwayFromQueryPoint)
{
    NeighborhoodSearch<ValueWithPos<int>> neighborhood{3};
    const std::vector<ValueWithPos<int>> agents{{{1, 0}, 1}};
    neighborhood.Update(agents);

    const auto expected = std::set<int>{1};
    const auto result = neighborhood.GetNeighboringAgents({0, 0}, 1);
    std::set<int> actual{};
    std::transform(
        std::begin(result),
        std::end(result),
        std::inserter(actual, std::begin(actual)),
        [](const auto& v) { return v->val; });
    ASSERT_EQ(actual, expected);
}
