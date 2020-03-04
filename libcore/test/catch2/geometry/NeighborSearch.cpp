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


#include "geometry/Point.h"
#include "mpi/LCGrid.h"
#include "pedestrian/Pedestrian.h"

#include <catch2/catch.hpp>
#include <iostream>
#include <vector>

TEST_CASE("geometry/NeighborSearch", "[geometry][neightbor-search][lcgrid]")
{
    SECTION("GetNeighbors")
    {
        LCGrid lcgrid(0, 10, 0, 10, 2.2);

        std::vector<Pedestrian> pedestrians(10);
        for(auto & ped : pedestrians) {
            ped.SetPos(Point(0, 0));
        }

        std::vector<Pedestrian *> ped_pointers;
        for(auto & ped : pedestrians) {
            ped_pointers.push_back(&ped);
        }

        lcgrid.Update(ped_pointers);

        std::vector<Pedestrian *> neighborhood;

        Pedestrian special_ped;
        special_ped.SetPos(Point(0, 0));

        lcgrid.GetNeighbourhood(&special_ped, neighborhood);
        REQUIRE_THAT(ped_pointers, Catch::Matchers::UnorderedEquals(neighborhood));

        neighborhood.clear();
        special_ped.SetPos(Point(10, 10));
        lcgrid.GetNeighbourhood(&special_ped, neighborhood);
        REQUIRE(neighborhood.empty());

        neighborhood.clear();
        special_ped.SetPos(Point(0, 4.4));
        lcgrid.GetNeighbourhood(&special_ped, neighborhood);
        REQUIRE(neighborhood.empty());

        neighborhood.clear();
        special_ped.SetPos(Point(0, 4.3));
        lcgrid.GetNeighbourhood(&special_ped, neighborhood);
        REQUIRE_THAT(ped_pointers, Catch::Matchers::UnorderedEquals(neighborhood));
    }
}
