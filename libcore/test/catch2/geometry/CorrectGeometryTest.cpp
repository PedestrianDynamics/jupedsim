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

#include "geometry/helper/CorrectGeometry.cpp"

#include "geometry/Building.h"
#include "geometry/Crossing.h"
#include "geometry/Line.h"
#include "geometry/SubRoom.h"
#include "geometry/Wall.h"

#include <catch2/catch.hpp>

TEST_CASE("geometry/Building/correct", "[geometry][Building][correct]")
{
    SECTION("SplitWall")
    {
        SECTION("No split possible")
        {
            NormalSubRoom subRoom;
            REQUIRE_FALSE(geometry::helper::SplitWall(subRoom, Wall{}));

            Wall bigWall(Point(0, 0), Point(20, 0));
            REQUIRE_FALSE(geometry::helper::SplitWall(subRoom, bigWall));

            Transition transition;
            subRoom.AddTransition(&transition);
            REQUIRE_FALSE(geometry::helper::SplitWall(subRoom, bigWall));

            transition.SetPoint2(Point(0, 10));
            REQUIRE_FALSE(geometry::helper::SplitWall(subRoom, bigWall));

            Crossing crossing;
            subRoom.AddCrossing(&crossing);
            REQUIRE_FALSE(geometry::helper::SplitWall(subRoom, bigWall));
            crossing.SetPoint1(Point(100, 0));
            crossing.SetPoint2(Point(100, 1));

            Wall wall;
            subRoom.AddWall(wall);
            REQUIRE_FALSE(geometry::helper::SplitWall(subRoom, bigWall));

            wall.SetPoint1(Point(-100, 0));
            REQUIRE_FALSE(geometry::helper::SplitWall(subRoom, bigWall));

            REQUIRE_FALSE(geometry::helper::SplitWall(subRoom, Wall{}));
        }
    }


    SECTION("GetSplitPoint")
    {
        Wall testWall(Point(0, 0), Point(0, 10));
        SECTION("No Split Point")
        {
            // default Walls (0,0)-(0,0)
            REQUIRE_FALSE(geometry::helper::GetSplitPoint(Wall(), Line()));
            REQUIRE_FALSE(geometry::helper::GetSplitPoint(Wall(), Wall()));

            // No intersection
            REQUIRE_FALSE(geometry::helper::GetSplitPoint(testWall, Line()));
            REQUIRE_FALSE(
                geometry::helper::GetSplitPoint(testWall, Line(Point(1, 1), Point(1, 2))));
            REQUIRE_FALSE(
                geometry::helper::GetSplitPoint(testWall, Line(Point(0, -1), Point(1, -1))));

            // Overlapping
            REQUIRE_FALSE(
                geometry::helper::GetSplitPoint(testWall, Line(Point(0, 0), Point(0, 2))));
            REQUIRE_FALSE(
                geometry::helper::GetSplitPoint(testWall, Line(Point(0, -1), Point(0, 4))));

            // Common starting/ending Point
            REQUIRE_FALSE(
                geometry::helper::GetSplitPoint(testWall, Line(Point(0, 0), Point(1, 2))));
            REQUIRE_FALSE(
                geometry::helper::GetSplitPoint(testWall, Line(Point(0, 10), Point(1, -1))));

            // Same Line
            REQUIRE_FALSE(
                geometry::helper::GetSplitPoint(testWall, Line(Point(0, 0), Point(0, 10))));
            REQUIRE_FALSE(geometry::helper::GetSplitPoint(testWall, testWall));

            // Line starting or ending on bigWall
            REQUIRE_FALSE(
                geometry::helper::GetSplitPoint(testWall, Line(Point(-1, 0), Point(1, 0))));
            REQUIRE_FALSE(
                geometry::helper::GetSplitPoint(testWall, Line(Point(-1, 10), Point(2, 10))));
        }

        SECTION("Split Point")
        {
            // Some "normal" intersection points
            REQUIRE(
                Point(0, 5) ==
                geometry::helper::GetSplitPoint(testWall, Line(Point(-1, 5), Point(1, 5))).value());
            REQUIRE(
                Point(0, 0.5) ==
                geometry::helper::GetSplitPoint(testWall, Line(Point(-0.5, 0), Point(0.5, 1)))
                    .value());

            // line starting or ending on bigwall
            REQUIRE(
                Point(0, 0.5) ==
                geometry::helper::GetSplitPoint(testWall, Line(Point(0, 0.5), Point(10, 1)))
                    .value());
            REQUIRE(
                Point(0, 1) ==
                geometry::helper::GetSplitPoint(testWall, Line(Point(1, 1), Point(0, 1))).value());
        }
    }
}
