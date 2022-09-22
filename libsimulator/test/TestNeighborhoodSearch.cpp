/// Copyright © 2012-2022 Forschungszentrum Jülich GmbH
/// SPDX-License-Identifier: LGPL-3.0-or-later
#include "NeighborhoodSearch.hpp"

#include "Agent.hpp"

#include <gtest/gtest.h>
#include <iostream>

TEST(NeighborhoodSearch, GetNeighboringAgents)
{
    NeighborhoodSearch neighborhood_search(2.2);

    std::vector<std::unique_ptr<Agent>> pedestrians{};
    for(int counter = 0; counter < 10; ++counter) {
        pedestrians.emplace_back(std::make_unique<Agent>());
        pedestrians.back()->pos = Point(0, 0);
    }
    std::vector<Agent*> pedestrians_as_raw{};
    std::transform(
        std::begin(pedestrians),
        std::end(pedestrians),
        std::back_inserter(pedestrians_as_raw),
        [](const auto& e) { return e.get(); });

    neighborhood_search.Update(pedestrians);

    std::vector<Agent*> neighborhood;

    Agent special_ped;
    special_ped.pos = Point(0, 0);

    for(auto neighbor : neighborhood_search.GetNeighboringAgents(special_ped.pos, 2.2)) {
        neighborhood.push_back(neighbor);
    }
    EXPECT_EQ(pedestrians_as_raw, neighborhood);
    neighborhood.clear();

    special_ped.pos = Point(10, 10);
    for(auto neighbor : neighborhood_search.GetNeighboringAgents(special_ped.pos, 2.2)) {
        neighborhood.push_back(neighbor);
    }
    EXPECT_TRUE(neighborhood.empty());
    neighborhood.clear();

    special_ped.pos = Point(0, 4.4);
    for(auto neighbor : neighborhood_search.GetNeighboringAgents(special_ped.pos, 2.2)) {
        neighborhood.push_back(neighbor);
    }
    EXPECT_TRUE(neighborhood.empty());
    neighborhood.clear();

    special_ped.pos = Point(0, 4.3);
    for(auto neighbor : neighborhood_search.GetNeighboringAgents(special_ped.pos, 5)) {
        neighborhood.push_back(neighbor);
    }
    EXPECT_EQ(pedestrians_as_raw, neighborhood);
    neighborhood.clear();

    for(auto neighbor :
        neighborhood_search.GetNeighboringAgents(pedestrians_as_raw.front()->pos, 2.2)) {
        neighborhood.push_back(neighbor);
    }
    EXPECT_EQ(pedestrians_as_raw, neighborhood);
    neighborhood.clear();
}

TEST(NeighborhoodSearch, GetNeighboringAgentsRadius)
{
    NeighborhoodSearch neighborhood_search(2);

    std::vector<std::unique_ptr<Agent>> pedestrians{};
    pedestrians.emplace_back(std::make_unique<Agent>());
    pedestrians.back()->pos = Point(0, 0);

    std::vector<Agent*> pedestrians_as_raw{};
    std::transform(
        std::begin(pedestrians),
        std::end(pedestrians),
        std::back_inserter(pedestrians_as_raw),
        [](const auto& e) { return e.get(); });

    neighborhood_search.Update(pedestrians);

    std::vector<Agent*> neighborhood;
    Agent special_ped;
    // one level radius
    special_ped.pos = Point(2.5, 2.3);
    for(auto neighbor : neighborhood_search.GetNeighboringAgents(special_ped.pos, 4)) {
        neighborhood.push_back(neighbor);
    }
    EXPECT_EQ(pedestrians_as_raw, neighborhood);
    neighborhood.clear();

    // Pedestrians should be filtered by the neighborhood level already
    special_ped.pos = Point(4.5, 4.3);
    EXPECT_TRUE(neighborhood_search.GetNeighboringAgents(special_ped.pos, 2).empty());
}
