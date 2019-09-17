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

#include "geometry/Wall.h"

#include <catch2/catch.hpp>


TEST_CASE("geometry/Wall", "[geometry][Wall]")
{
    SECTION("ctor")
    {
        Wall  W1;
        Point P1;
        REQUIRE(W1.GetPoint1() == P1);
        REQUIRE(W1.GetPoint2() == P1);
        std::string type[] = {"internal", "external"};
        for(int i = 0; i < 10; ++i) {
            Point P2(0, i * i);
            Point P3(static_cast<float>(i) / 1000000, i * i);

            Line L1(P2, P3);
            Wall W2(P2, P3, type[i % 2]);
            REQUIRE(W2 == L1);
            // GetType test
            REQUIRE(W2.GetType() == type[i % 2]);
        }
    }
}
