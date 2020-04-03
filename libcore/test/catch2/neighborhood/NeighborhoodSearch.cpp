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


#include "neighborhood/NeighborhoodSearch.h"

#include "geometry/Point.h"
#include "pedestrian/Pedestrian.h"

#include <algorithm>
#include <catch2/catch.hpp>
#include <deque>
#include <vector>

TEST_CASE("neighborhood/NeighborSearch", "[neighborhood][neightor-search]")
{
    SECTION("Empty NeighborhoodSearch")
    {
        NeighborhoodSearch lcgrid(0, 10, 0, 10, 2.2);

        Pedestrian special_ped;
        special_ped.SetPos(Point(0, 0));

        REQUIRE(lcgrid.GetNeighbourhood(&special_ped).empty());

        special_ped.SetPos(Point(10, 0));
        REQUIRE(lcgrid.GetNeighbourhood(&special_ped).empty());
        special_ped.SetPos(Point(5, 5));
        REQUIRE(lcgrid.GetNeighbourhood(&special_ped).empty());
        special_ped.SetPos(Point(0, 10));
        REQUIRE(lcgrid.GetNeighbourhood(&special_ped).empty());
    }

    SECTION("GetNeighbors")
    {
        NeighborhoodSearch neighborhood_search(0, 10, 0, 10, 2.2);

        std::vector<Pedestrian> pedestrians(10);
        for(auto & ped : pedestrians) {
            ped.SetPos(Point(0, 0));
        }

        std::vector<Pedestrian *> ped_pointers;
        std::transform(
            pedestrians.begin(),
            pedestrians.end(),
            std::back_inserter(ped_pointers),
            [](Pedestrian & ped) -> Pedestrian * { return &ped; });

        neighborhood_search.Update(ped_pointers);

        std::vector<Pedestrian *> neighborhood;

        Pedestrian special_ped;
        special_ped.SetPos(Point(0, 0));

        neighborhood = neighborhood_search.GetNeighbourhood(&special_ped);
        REQUIRE_THAT(ped_pointers, Catch::Matchers::UnorderedEquals(neighborhood));

        special_ped.SetPos(Point(10, 10));
        neighborhood = neighborhood_search.GetNeighbourhood(&special_ped);
        REQUIRE(neighborhood.empty());

        special_ped.SetPos(Point(0, 4.4));
        neighborhood = neighborhood_search.GetNeighbourhood(&special_ped);
        REQUIRE(neighborhood.empty());

        special_ped.SetPos(Point(0, 4.3));
        neighborhood = neighborhood_search.GetNeighbourhood(&special_ped);
        REQUIRE_THAT(ped_pointers, Catch::Matchers::UnorderedEquals(neighborhood));
    }
}
