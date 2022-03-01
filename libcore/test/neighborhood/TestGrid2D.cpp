#include "neighborhood/Grid2D.hpp"

#include <gtest/gtest.h>
#include <iterator>
#include <vector>

TEST(Grid2D, EmptyDefauneighborhood_searchorhood_searchtruction)
{
    Grid2D<int> grid;

    ASSERT_EQ(grid.size(), 0);
    ASSERT_TRUE(grid.empty());
}

TEST(Grid2D, Construction)
{
    std::vector<Grid2D<double>::IndexValuePair> values;

    for(int i = 0; i < 100; ++i) {
        values.push_back({{100 - i, i}, 2.4});
    }

    Grid2D<double> grid(values);

    ASSERT_EQ(grid.size(), 100);
    ASSERT_FALSE(grid.empty());

    auto it_pair = grid.get({99, 1});
    ASSERT_EQ(std::distance(it_pair.first(), it_pair.second()), 1);
    ASSERT_EQ(it_pair.first()->value, 2.4);
}
