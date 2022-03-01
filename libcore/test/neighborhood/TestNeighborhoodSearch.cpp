#include "neighborhood/NeighborhoodSearch.hpp"
#include "pedestrian/Pedestrian.hpp"

#include <gtest/gtest.h>

TEST(NeighborhoodSearch, Construction)
{
    NeighborhoodSearch neighborhood_search(2.2);

    Pedestrian special_ped;
    special_ped.SetPos(Point(0, 0));

    EXPECT_TRUE(neighborhood_search.GetNeighborhood(&special_ped).empty());

    special_ped.SetPos(Point(10, 0));
    EXPECT_TRUE(neighborhood_search.GetNeighborhood(&special_ped).empty());
    special_ped.SetPos(Point(5, 5));
    EXPECT_TRUE(neighborhood_search.GetNeighborhood(&special_ped).empty());
    special_ped.SetPos(Point(0, 10));
    EXPECT_TRUE(neighborhood_search.GetNeighborhood(&special_ped).empty());
}

TEST(NeighborhoodSearch, GetNeighborhood)
{
    NeighborhoodSearch neighborhood_search(2.2);

    std::vector<std::unique_ptr<Pedestrian>> pedestrians{};
    for(int counter = 0; counter < 10; ++counter) {
        pedestrians.emplace_back(std::make_unique<Pedestrian>());
        pedestrians.back()->SetPos(Point(0, 0));
    }
    std::vector<Pedestrian *> pedestrians_as_raw{};
    std::transform(
        std::begin(pedestrians),
        std::end(pedestrians),
        std::back_inserter(pedestrians_as_raw),
        [](const auto & e) { return e.get(); });

    neighborhood_search.Update(pedestrians);

    std::vector<Pedestrian *> neighborhood;

    Pedestrian special_ped;
    special_ped.SetPos(Point(0, 0));

    neighborhood = neighborhood_search.GetNeighborhood(&special_ped);
    EXPECT_EQ(pedestrians_as_raw, neighborhood);

    special_ped.SetPos(Point(10, 10));
    neighborhood = neighborhood_search.GetNeighborhood(&special_ped);
    EXPECT_TRUE(neighborhood.empty());

    special_ped.SetPos(Point(0, 4.4));
    neighborhood = neighborhood_search.GetNeighborhood(&special_ped);
    EXPECT_TRUE(neighborhood.empty());

    special_ped.SetPos(Point(0, 4.3));
    neighborhood = neighborhood_search.GetNeighborhood(&special_ped);
    EXPECT_EQ(pedestrians_as_raw, neighborhood);
}
