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


#include "neighborhood/NeighborhoodSearch.hpp"

#include "geometry/Point.hpp"
#include "pedestrian/Pedestrian.hpp"

#include <algorithm>
#include <catch2/catch.hpp>
#include <deque>
#include <iterator>
#include <memory>
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

        neighborhood = neighborhood_search.GetNeighbourhood(&special_ped);
        REQUIRE_THAT(pedestrians_as_raw, Catch::Matchers::UnorderedEquals(neighborhood));

        special_ped.SetPos(Point(10, 10));
        neighborhood = neighborhood_search.GetNeighbourhood(&special_ped);
        REQUIRE(neighborhood.empty());

        special_ped.SetPos(Point(0, 4.4));
        neighborhood = neighborhood_search.GetNeighbourhood(&special_ped);
        REQUIRE(neighborhood.empty());

        special_ped.SetPos(Point(0, 4.3));
        neighborhood = neighborhood_search.GetNeighbourhood(&special_ped);
        REQUIRE_THAT(pedestrians_as_raw, Catch::Matchers::UnorderedEquals(neighborhood));
    }
}
