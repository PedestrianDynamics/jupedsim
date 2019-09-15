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

#include "geometry/Obstacle.h"

#include "geometry/Point.h"
#include "geometry/Wall.h"

#include <catch2/catch.hpp>
#include <cmath>
#include <cstdlib>
#include <vector>

const double PI = std::acos(-1);

using Catch::Matchers::Equals;

TEST_CASE("geometry/Obstacle", "[geometry][Obstacle]")
{
    SECTION("ctor getter setter")
    {
        Obstacle          obs1;
        std::vector<Wall> walls_test;
        for(int i = 1; i < 10; ++i) {
            Point P1(cos(PI / i), sin(PI / i));
            Point P2(i, i * i);
            Wall  W1(P1, P2, "interior");
            obs1.AddWall(W1);
            walls_test.emplace_back(W1);
            obs1.SetCaption("obstacle" + std::to_string(i));
            obs1.SetHeight(-i); // logically incorrect
            obs1.SetId(i - 1);

            REQUIRE_THAT(obs1.GetCaption(), Equals("obstacle" + std::to_string(i)));
            REQUIRE(obs1.GetHeight() == -i); // logically incorrect
            REQUIRE(obs1.GetId() == (i - 1));
            REQUIRE_THAT(obs1.GetAllWalls(), Equals(walls_test));
        }
    }

    // Original Test was disabled.
    // WhichQuad is private.
    // SECTION("WhichQuad")
    // {
    //     Point    vertex;
    //     Point    hitPos;
    //     Obstacle obs;

    //     REQUIRE(obs.WhichQuad(vertex, hitPos) == 3);
    //     for(int i = 1, j = 1; i < 10; ++i, ++j) {
    //         vertex = Point(i, j);
    //         hitPos = Point(-i, -j);
    //         REQUIRE(obs.WhichQuad(vertex, hitPos) == 1);

    //         vertex = Point(-i, j);
    //         hitPos = Point(i, -j);
    //         REQUIRE(obs.WhichQuad(vertex, hitPos) == 2);

    //         vertex = Point(-i, -j);
    //         hitPos = Point(i, j);
    //         REQUIRE(obs.WhichQuad(vertex, hitPos) == 3);

    //         vertex = Point(i, -j);
    //         hitPos = Point(-i, j);
    //         REQUIRE(obs.WhichQuad(vertex, hitPos) == 4);
    //     }
    // }

    SECTION("Contains")
    {
        Point P1(0, 0);
        Point P2(0, 10);
        Point P3(10, 10);
        Point P4(10, 0);

        Obstacle obs1;

        Wall w1(P1, P2);
        Wall w2(P2, P3);
        Wall w3(P3, P4);
        Wall w4(P4, P1);

        obs1.AddWall(w1);
        obs1.AddWall(w2);
        obs1.AddWall(w3);
        obs1.AddWall(w4);

        obs1.ConvertLineToPoly();

        // inside the obstacle check
        for(int i = 1; i < 10; ++i)
            REQUIRE(obs1.Contains(Point(i, static_cast<float>(i) / 100)));

        // on the edge check
        for(int i = 0; i < 11; ++i) {
            REQUIRE(obs1.Contains(Point(0, i)));
            REQUIRE(obs1.Contains(Point(i, 0)));
            REQUIRE(obs1.Contains(Point(10, i)));
            REQUIRE(obs1.Contains(Point(i, 10)));
        }

        // outside the obstacle check
        for(int i = 1; i < 10; ++i)
            REQUIRE_FALSE(obs1.Contains(Point(-i, i * i)));
    }

    SECTION("ConvertLineToPoly")
    {
        Point P1;
        Point P2(2, 2);
        Point P3(2, 0);
        Point P4(0, 2);
        Point P5(2, -1);
        Point P6(-1, -1);

        Obstacle obs1;
        obs1.SetId(1);
        obs1.AddWall(Wall(P1, P3));
        obs1.AddWall(Wall(P2, P3));
        obs1.AddWall(Wall(P2, P4));
        obs1.AddWall(Wall(P4, P1));

        REQUIRE(obs1.ConvertLineToPoly());
        obs1.AddWall(Wall(P1, P6)); // Hanging edge
        REQUIRE_FALSE(obs1.ConvertLineToPoly());

        Obstacle obs2;
        obs2.SetId(2);
        obs2.AddWall(Wall(P1, P3));
        obs2.AddWall(Wall(P3, P2));
        obs2.AddWall(Wall(P2, P4));
        obs2.AddWall(Wall(P4, P1));

        obs2.AddWall(Wall(P1, P6));
        obs2.AddWall(Wall(P6, P5));
        obs2.AddWall(Wall(P5, P1)); // two closed polygon sharing the same starting vertex

        if(obs2.ConvertLineToPoly() == true) {
            REQUIRE(obs2.GetPolygon().size() == 6);
        }

        // if polygon creation is true all the vertex must be included
        // since there are no intersecting Walls this can be considered as a valid polygon,
        // provided all the vertex are added without duplicates of shared vertex.
    }

    SECTION("ConvertLineToPoly intersecting Walls")
    {
        Point P1;
        Point P2(2, 2);
        Point P3(2, 0);
        Point P4(0, 2);

        Obstacle obs1;
        obs1.SetId(1);
        obs1.AddWall(Wall(P1, P2));
        obs1.AddWall(Wall(P2, P3));
        obs1.AddWall(Wall(P3, P4));
        obs1.AddWall(Wall(P4, P1));

        REQUIRE_FALSE(obs1.ConvertLineToPoly());
    }

    SECTION("GetCentroid")
    {
        for(int i = 0; i < 10; ++i) {
            Point P1(i, i);
            Point P2(i + 10, i);
            Point P3(i + 5, i + 5);
            Point P4(i + 5, i - 5);

            Wall w1(P1, P2);
            Wall w2(P2, P3);
            Wall w3(P3, P1);

            Obstacle obs1;
            obs1.SetId(i - 1);

            obs1.AddWall(w1);
            obs1.AddWall(w2);
            obs1.AddWall(w3);

            REQUIRE(obs1.ConvertLineToPoly());
            REQUIRE(obs1.GetCentroid() == (P1 + P2 + P3) / 3);

            Line L1(P3, P4);
            REQUIRE(obs1.IntersectWithLine(L1));
        }
    }
}
