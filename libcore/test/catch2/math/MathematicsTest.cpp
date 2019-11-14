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

#include "math/Mathematics.h"

#include <catch2/catch.hpp>

TEST_CASE("math/Mathematics", "[math][Mathematics]")
{
    SECTION("Sign")
    {
        // positive
        REQUIRE(sign(1.0) == 1);
        // negative
        REQUIRE(sign(-1.0) == -1);
        // zero is positive
        REQUIRE(sign(0.0) == 1);
    }
}
