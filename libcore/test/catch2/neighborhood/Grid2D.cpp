/*
 * This file is part of JuPedSim.
 *
 * JuPedSim is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * JuPedSim is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with JuPedSim. If not, see <http://www.gnu.org/licenses/>.
 *
 **/


#include "neighborhood/Grid2D.h"

#include "pedestrian/Pedestrian.h"

#include <catch2/catch.hpp>
#include <deque>

TEST_CASE("neighborhood/Grid2D", "[neighborhood][neighbor-search][grid2d]")
{
    SECTION("ctors")
    {
        SECTION("empty default")
        {
            Grid2D<int> grid;
            REQUIRE(grid.num_rows() == 0);
            REQUIRE(grid.num_columns() == 0);

            Grid2D<std::deque<Pedestrian *>> grid2;
            REQUIRE(grid2.num_rows() == 0);
            REQUIRE(grid2.num_columns() == 0);
        }

        SECTION("custom contructor")
        {
            Grid2D<std::deque<Pedestrian *>> grid(10, 100);
            REQUIRE(grid.num_rows() == 10);
            REQUIRE(grid.num_columns() == 100);

            for(unsigned i = 0; i < 10; ++i) {
                REQUIRE(grid[i].size() == 100);
            }

            Grid2D<std::deque<int>> grid2(7, 9);
            REQUIRE(grid2.num_rows() == 7);
            REQUIRE(grid2.num_columns() == 9);

            for(unsigned i = 0; i < 7; ++i) {
                REQUIRE(grid2[i].size() == 9);
            }
        }

        SECTION("copy")
        {
            SECTION("empty values")
            {
                Grid2D<std::deque<int>> grid(5, 10);
                REQUIRE(grid.num_rows() == 5);
                REQUIRE(grid.num_columns() == 10);
                for(unsigned i = 0; i < 5; ++i) {
                    for(unsigned j = 0; j < 10; ++j) {
                        // the deque should be empty
                        REQUIRE(grid[i][j].empty());
                    }
                }

                auto grid_copy = grid;
                // grid must be the same, it is a copy, not move
                REQUIRE(grid.num_rows() == 5);
                REQUIRE(grid.num_columns() == 10);
                for(unsigned i = 0; i < 5; ++i) {
                    for(unsigned j = 0; j < 10; ++j) {
                        // the deque should be empty
                        REQUIRE(grid[i][j].empty());
                    }
                }

                // now check the copy
                REQUIRE(grid_copy.num_rows() == 5);
                REQUIRE(grid_copy.num_columns() == 10);
                for(unsigned i = 0; i < 5; ++i) {
                    for(unsigned j = 0; j < 10; ++j) {
                        // the deque should be empty
                        REQUIRE(grid_copy[i][j].empty());
                    }
                }
            }
            SECTION("non empty")
            {
                Grid2D<int> grid(5, 10);
                REQUIRE(grid.num_rows() == 5);
                REQUIRE(grid.num_columns() == 10);
                for(unsigned i = 0; i < 5; ++i) {
                    for(unsigned j = 0; j < 10; ++j) {
                        grid[i][j] = i * j;
                    }
                }

                auto grid_copy = grid;
                // grid must be the same, it is a copy, not move
                REQUIRE(grid.num_rows() == 5);
                REQUIRE(grid.num_columns() == 10);
                for(unsigned i = 0; i < 5; ++i) {
                    for(unsigned j = 0; j < 10; ++j) {
                        REQUIRE(grid[i][j] == i * j);
                    }
                }

                // now check the copy
                REQUIRE(grid_copy.num_rows() == 5);
                REQUIRE(grid_copy.num_columns() == 10);
                for(unsigned i = 0; i < 5; ++i) {
                    for(unsigned j = 0; j < 10; ++j) {
                        REQUIRE(grid_copy[i][j] == i * j);
                    }
                }
            }
        }
    }
}
