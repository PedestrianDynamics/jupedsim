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

#include "geometry/Room.h"

#include <catch2/catch.hpp>
#include <string>

TEST_CASE("geometry/Room", "[geometry][Room]")
{
    SECTION("Setter and Getter")
    {
        Room R1;
        for(int i = 1; i < 10; ++i) {
            R1.SetID(i);
            R1.SetCaption("Room" + std::to_string(i));
            R1.SetZPos(10.0 / i);
            R1.SetEgressTime(i * 10.0 / 3);
            R1.SetState(RoomState(i % 2));
            Room R2(R1);
            REQUIRE(R2.GetID() == i);
            REQUIRE(R2.GetCaption() == "Room" + std::to_string(i));
            REQUIRE(R2.GetZPos() == 10.0 / i);
            REQUIRE(R2.GetEgressTime() == i * 10.0 / 3);
            REQUIRE(R2.GetState() == i % 2);
        }
    }
}
