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

TEST_CASE("neighborhood/Grid2D", "[neighborhood][neightbor-search][grid]")
{
    SECTION("Grid2D")
    {
        Grid2D<std::deque<Pedestrian *>> grid(10, 100);

        for(unsigned i = 0; i < 10; ++i) {
            REQUIRE(grid[i].size() == 100);
        }
    }
}
