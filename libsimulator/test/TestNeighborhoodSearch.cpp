#include "NeighborhoodSearch.hpp"
#include "Pedestrian.hpp"

#include <gtest/gtest.h>
#include <iostream>

TEST(NeighborhoodSearch, GetNeighboringAgents)
{
    NeighborhoodSearch neighborhood_search(2.2);

    std::vector<std::unique_ptr<Pedestrian>> pedestrians{};
    for(int counter = 0; counter < 10; ++counter) {
        pedestrians.emplace_back(std::make_unique<Pedestrian>());
        pedestrians.back()->SetPos(Point(0, 0));
    }
    std::vector<Pedestrian*> pedestrians_as_raw{};
    std::transform(
        std::begin(pedestrians),
        std::end(pedestrians),
        std::back_inserter(pedestrians_as_raw),
        [](const auto& e) { return e.get(); });

    neighborhood_search.Update(pedestrians);

    std::vector<Pedestrian*> neighborhood;

    Pedestrian special_ped;
    special_ped.SetPos(Point(0, 0));

    for(auto neighbor : neighborhood_search.GetNeighboringAgents(special_ped.GetPos(), 2.2)) {
        neighborhood.push_back(neighbor);
    }
    EXPECT_EQ(pedestrians_as_raw, neighborhood);
    neighborhood.clear();

    special_ped.SetPos(Point(10, 10));
    for(auto neighbor : neighborhood_search.GetNeighboringAgents(special_ped.GetPos(), 2.2)) {
        neighborhood.push_back(neighbor);
    }
    EXPECT_TRUE(neighborhood.empty());
    neighborhood.clear();

    special_ped.SetPos(Point(0, 4.4));
    for(auto neighbor : neighborhood_search.GetNeighboringAgents(special_ped.GetPos(), 2.2)) {
        neighborhood.push_back(neighbor);
    }
    EXPECT_TRUE(neighborhood.empty());
    neighborhood.clear();

    special_ped.SetPos(Point(0, 4.3));
    for(auto neighbor : neighborhood_search.GetNeighboringAgents(special_ped.GetPos(), 5)) {
        neighborhood.push_back(neighbor);
    }
    EXPECT_EQ(pedestrians_as_raw, neighborhood);
    neighborhood.clear();

    for(auto neighbor :
        neighborhood_search.GetNeighboringAgents(pedestrians_as_raw.front()->GetPos(), 2.2)) {
        neighborhood.push_back(neighbor);
    }
    EXPECT_EQ(pedestrians_as_raw, neighborhood);
    neighborhood.clear();
}

TEST(NeighborhoodSearch, GetNeighboringAgentsRadius)
{
    NeighborhoodSearch neighborhood_search(2);

    std::vector<std::unique_ptr<Pedestrian>> pedestrians{};
    pedestrians.emplace_back(std::make_unique<Pedestrian>());
    pedestrians.back()->SetPos(Point(0, 0));

    std::vector<Pedestrian*> pedestrians_as_raw{};
    std::transform(
        std::begin(pedestrians),
        std::end(pedestrians),
        std::back_inserter(pedestrians_as_raw),
        [](const auto& e) { return e.get(); });

    neighborhood_search.Update(pedestrians);

    std::vector<Pedestrian*> neighborhood;
    Pedestrian special_ped;
    // one level radius
    special_ped.SetPos(Point(2.5, 2.3));
    for(auto neighbor : neighborhood_search.GetNeighboringAgents(special_ped.GetPos(), 4)) {
        neighborhood.push_back(neighbor);
    }
    EXPECT_EQ(pedestrians_as_raw, neighborhood);
    neighborhood.clear();

    // Pedestrians should be filtered by the neighborhood level already
    special_ped.SetPos(Point(4.5, 4.3));
    EXPECT_TRUE(neighborhood_search.GetNeighboringAgents(special_ped.GetPos(), 2).empty());
}
