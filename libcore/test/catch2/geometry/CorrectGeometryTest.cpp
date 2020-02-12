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

#include "geometry/helper/CorrectGeometry.h"

#include "general/Configuration.h"
#include "geometry/Building.h"
#include "geometry/Crossing.h"
#include "geometry/Line.h"
#include "geometry/SubRoom.h"
#include "geometry/Transition.h"
#include "geometry/Wall.h"
#include "pedestrian/PedDistributor.h"

#include <algorithm>
#include <catch2/catch.hpp>

TEST_CASE("geometry/Building/correct", "[geometry][Building][correct]")
{
    SECTION("CorrectInputGeometry")
    {
        Building b;
        Room * r          = new Room();
        NormalSubRoom * s = new NormalSubRoom();


        SECTION("empty rooms and building")
        {
            REQUIRE_NOTHROW(geometry::helper::CorrectInputGeometry(b));
            REQUIRE(b.GetAllGoals().empty());
            REQUIRE(b.GetAllRooms().empty());
            REQUIRE(b.GetAllHlines().empty());
            REQUIRE(b.GetAllCrossings().empty());
            REQUIRE(b.GetAllPedestrians().empty());
            REQUIRE(b.GetAllTransitions().empty());


            r->AddSubRoom(s);
            b.AddRoom(r);
            REQUIRE_NOTHROW(geometry::helper::CorrectInputGeometry(b));
            REQUIRE(b.GetAllGoals().empty());
            REQUIRE(b.GetAllRooms().size() == 1);
            REQUIRE(b.GetAllHlines().empty());
            REQUIRE(b.GetAllCrossings().empty());
            REQUIRE(b.GetAllPedestrians().empty());
            REQUIRE(b.GetAllTransitions().empty());


            // A single Wall is not connected in the rooms walls and should be removed
            s->AddWall(Wall(Point(0, 0), Point(0, 10)));
            REQUIRE_THROWS_WITH(
                geometry::helper::CorrectInputGeometry(b),
                "Cannot write corrected geometry. No Configuration object found.");
            REQUIRE(b.GetAllGoals().empty());
            REQUIRE(b.GetAllRooms().size() == 1);
            REQUIRE(b.GetAllHlines().empty());
            REQUIRE(b.GetAllCrossings().empty());
            REQUIRE(b.GetAllPedestrians().empty());
            REQUIRE(b.GetAllTransitions().empty());
            REQUIRE(s->GetAllWalls().empty());
        }

        SECTION("overlapping doors only")
        {
            r->AddSubRoom(s);
            b.AddRoom(r);
            s->AddWall(Wall(Point(0, 0), Point(0, 10)));
            s->AddWall(Wall(Point(0, 10), Point(10, 10)));
            s->AddWall(Wall(Point(10, 10), Point(10, 0)));
            s->AddWall(Wall(Point(10, 0), Point(0, 0)));

            Transition * door = new Transition();
            door->SetPoint1(Point(0, 1));
            door->SetPoint2(Point(0, 2));
            door->SetID(0);
            s->AddTransition(door);
            b.AddTransition(door);
            REQUIRE(s->GetAllWalls().size() == 4);

            // When the geometry was fixed, the new geometry file is saved. Since we do not have a configuration in the building, the CorrectInputGeometry should throw an excption.
            REQUIRE_THROWS_WITH(
                geometry::helper::CorrectInputGeometry(b),
                "Cannot write corrected geometry. No Configuration object found.");
            REQUIRE(b.GetAllGoals().empty());
            REQUIRE(b.GetAllRooms().size() == 1);
            REQUIRE(b.GetAllHlines().empty());
            REQUIRE(b.GetAllCrossings().empty());
            REQUIRE(b.GetAllPedestrians().empty());
            REQUIRE(b.GetAllTransitions().size() == 1);
            REQUIRE(s->GetAllTransitions().size() == 1);
            REQUIRE(s->GetAllWalls().size() == 5);

            // check for splitted walls
            auto it = std::find(
                s->GetAllWalls().begin(), s->GetAllWalls().end(), Wall(Point(0, 0), Point(0, 1)));
            REQUIRE(it != s->GetAllWalls().end());
            it = std::find(
                s->GetAllWalls().begin(), s->GetAllWalls().end(), Wall(Point(0, 2), Point(0, 10)));
            REQUIRE(it != s->GetAllWalls().end());

            Transition * door2 = new Transition();
            door2->SetPoint1(Point(0, 9));
            door2->SetPoint2(Point(0, 10));
            door2->SetID(1);

            s->AddTransition(door2);
            b.AddTransition(door2);
            REQUIRE_THROWS_WITH(
                geometry::helper::CorrectInputGeometry(b),
                "Cannot write corrected geometry. No Configuration object found.");
            REQUIRE(s->GetAllWalls().size() == 5);

            it = std::find(
                s->GetAllWalls().begin(), s->GetAllWalls().end(), Wall(Point(0, 0), Point(0, 1)));
            REQUIRE(it != s->GetAllWalls().end());
            it = std::find(
                s->GetAllWalls().begin(), s->GetAllWalls().end(), Wall(Point(0, 2), Point(0, 9)));
            REQUIRE(it != s->GetAllWalls().end());
        }

        SECTION("big walls only")
        {
            r->AddSubRoom(s);
            b.AddRoom(r);
            s->AddWall(Wall(Point(0, 0), Point(0, 10)));
            s->AddWall(Wall(Point(0, 10), Point(10, 10)));
            s->AddWall(Wall(Point(10, 10), Point(10, 0)));
            s->AddWall(Wall(Point(11, 0), Point(0, 0)));


            REQUIRE(s->GetAllWalls().size() == 4);

            REQUIRE_THROWS_WITH(
                geometry::helper::CorrectInputGeometry(b),
                "Cannot write corrected geometry. No Configuration object found.");
            REQUIRE(b.GetAllGoals().empty());
            REQUIRE(b.GetAllRooms().size() == 1);
            REQUIRE(b.GetAllHlines().empty());
            REQUIRE(b.GetAllCrossings().empty());
            REQUIRE(b.GetAllPedestrians().empty());
            REQUIRE(s->GetAllWalls().size() == 4);

            // check for new wall
            auto it = std::find(
                s->GetAllWalls().begin(), s->GetAllWalls().end(), Wall(Point(10, 0), Point(0, 0)));
            REQUIRE(it != s->GetAllWalls().end());
        }

        SECTION("big walls and overlapping door")
        {
            r->AddSubRoom(s);
            b.AddRoom(r);
            s->AddWall(Wall(Point(0, 0), Point(0, 10)));
            s->AddWall(Wall(Point(0, 10), Point(10, 10)));
            s->AddWall(Wall(Point(10, 10), Point(10, 0)));
            s->AddWall(Wall(Point(11, 0), Point(0, 0)));

            Transition * door = new Transition();
            door->SetPoint1(Point(8, 0));
            door->SetPoint2(Point(9, 0));
            door->SetID(0);
            s->AddTransition(door);
            b.AddTransition(door);

            REQUIRE(s->GetAllWalls().size() == 4);

            REQUIRE_THROWS_WITH(
                geometry::helper::CorrectInputGeometry(b),
                "Cannot write corrected geometry. No Configuration object found.");
            REQUIRE(b.GetAllGoals().empty());
            REQUIRE(b.GetAllRooms().size() == 1);
            REQUIRE(b.GetAllHlines().empty());
            REQUIRE(b.GetAllCrossings().empty());
            REQUIRE(b.GetAllPedestrians().empty());

            REQUIRE(s->GetAllWalls().size() == 5);

            // check for new wall
            auto it = std::find(
                s->GetAllWalls().begin(), s->GetAllWalls().end(), Wall(Point(8, 0), Point(0, 0)));
            REQUIRE(it != s->GetAllWalls().end());
            it = std::find(
                s->GetAllWalls().begin(), s->GetAllWalls().end(), Wall(Point(10, 0), Point(9, 0)));
            REQUIRE(it != s->GetAllWalls().end());
        }

        SECTION("big walls and overlapping door in corner")
        {
            r->AddSubRoom(s);
            b.AddRoom(r);
            s->AddWall(Wall(Point(0, 0), Point(0, 10)));
            s->AddWall(Wall(Point(0, 10), Point(10, 10)));
            s->AddWall(Wall(Point(10, 10), Point(10, 0)));
            s->AddWall(Wall(Point(11, 0), Point(0, 0)));

            Transition * door = new Transition();
            door->SetPoint1(Point(10, 0));
            door->SetPoint2(Point(9, 0));
            door->SetID(0);
            s->AddTransition(door);
            b.AddTransition(door);

            REQUIRE(s->GetAllWalls().size() == 4);

            REQUIRE_THROWS_WITH(
                geometry::helper::CorrectInputGeometry(b),
                "Cannot write corrected geometry. No Configuration object found.");
            REQUIRE(b.GetAllGoals().empty());
            REQUIRE(b.GetAllRooms().size() == 1);
            REQUIRE(b.GetAllHlines().empty());
            REQUIRE(b.GetAllCrossings().empty());
            REQUIRE(b.GetAllPedestrians().empty());

            REQUIRE(s->GetAllWalls().size() == 4);

            // check for new wall
            auto it = std::find(
                s->GetAllWalls().begin(), s->GetAllWalls().end(), Wall(Point(9, 0), Point(0, 0)));
            REQUIRE(it != s->GetAllWalls().end());
        }
    }


    SECTION("SplitWall")
    {
        SECTION("No split possible")
        {
            NormalSubRoom subRoom;
            Wall bigWall(Point(0, 0), Point(20, 0));

            SECTION("Empty SubRoom")
            {
                REQUIRE_FALSE(geometry::helper::SplitWall(subRoom, Wall{}));
                REQUIRE_FALSE(geometry::helper::SplitWall(subRoom, bigWall));
            }

            SECTION("Transition")
            {
                Transition transition;
                subRoom.AddTransition(&transition);
                REQUIRE_FALSE(geometry::helper::SplitWall(subRoom, bigWall));

                transition.SetPoint1(Point(0, 0));
                transition.SetPoint2(Point(0, 10));
                REQUIRE_FALSE(geometry::helper::SplitWall(subRoom, bigWall));

                transition.SetPoint1(Point(0, -2));
                transition.SetPoint2(Point(0, 0));
                REQUIRE_FALSE(geometry::helper::SplitWall(subRoom, bigWall));

                transition.SetPoint1(Point(20, 0));
                transition.SetPoint2(Point(20, 1));
                REQUIRE_FALSE(geometry::helper::SplitWall(subRoom, bigWall));

                transition.SetPoint1(Point(20, -1));
                transition.SetPoint2(Point(20, 0));
                REQUIRE_FALSE(geometry::helper::SplitWall(subRoom, bigWall));
            }

            SECTION("Crossing")
            {
                Crossing crossing;
                subRoom.AddCrossing(&crossing);
                REQUIRE_FALSE(geometry::helper::SplitWall(subRoom, bigWall));

                crossing.SetPoint1(Point(0, 0));
                crossing.SetPoint2(Point(0, 10));
                REQUIRE_FALSE(geometry::helper::SplitWall(subRoom, bigWall));

                crossing.SetPoint1(Point(0, -2));
                crossing.SetPoint2(Point(0, 0));
                REQUIRE_FALSE(geometry::helper::SplitWall(subRoom, bigWall));

                crossing.SetPoint1(Point(20, 0));
                crossing.SetPoint2(Point(20, 1));
                REQUIRE_FALSE(geometry::helper::SplitWall(subRoom, bigWall));

                crossing.SetPoint1(Point(20, -1));
                crossing.SetPoint2(Point(20, 0));
                REQUIRE_FALSE(geometry::helper::SplitWall(subRoom, bigWall));
            }
        }

        SECTION("Wall Splitted")
        {
            NormalSubRoom subRoom;
            Wall bigWall(Point(0, 0), Point(20, 0));

            SECTION("Transition")
            {
                // Wall should be splitted in two pieces
                Transition transition;
                subRoom.AddTransition(&transition);
                transition.SetPoint1(Point(10, 0));
                transition.SetPoint2(Point(10, 1));
                auto walls = geometry::helper::SplitWall(subRoom, bigWall);
                REQUIRE(walls);
                REQUIRE(walls->size() == 2);
                auto expected_wall1 = Wall(Point(0, 0), Point(10, 0));
                auto expected_wall2 = Wall(Point(10, 0), Point(20, 0));

                REQUIRE((expected_wall1 == walls->front() || expected_wall1 == walls->back()));
                REQUIRE((expected_wall2 == walls->front() || expected_wall2 == walls->back()));


                transition.SetPoint1(Point(10, -1));
                transition.SetPoint2(Point(10, 1));
                walls = geometry::helper::SplitWall(subRoom, bigWall);
                REQUIRE(walls);
                REQUIRE(walls->size() == 2);
                REQUIRE((expected_wall1 == walls->front() || expected_wall1 == walls->back()));
                REQUIRE((expected_wall2 == walls->front() || expected_wall2 == walls->back()));
            }

            SECTION("Crossing")
            {
                // Wall should be splitted in two pieces
                Crossing crossing;
                subRoom.AddCrossing(&crossing);
                crossing.SetPoint1(Point(10, 0));
                crossing.SetPoint2(Point(10, 1));
                auto walls = geometry::helper::SplitWall(subRoom, bigWall);
                REQUIRE(walls);
                REQUIRE(walls->size() == 2);
                auto expected_wall1 = Wall(Point(0, 0), Point(10, 0));
                auto expected_wall2 = Wall(Point(10, 0), Point(20, 0));

                REQUIRE((expected_wall1 == walls->front() || expected_wall1 == walls->back()));
                REQUIRE((expected_wall2 == walls->front() || expected_wall2 == walls->back()));


                crossing.SetPoint1(Point(10, -1));
                crossing.SetPoint2(Point(10, 1));
                walls = geometry::helper::SplitWall(subRoom, bigWall);
                REQUIRE(walls);
                REQUIRE(walls->size() == 2);
                REQUIRE((expected_wall1 == walls->front() || expected_wall1 == walls->back()));
                REQUIRE((expected_wall2 == walls->front() || expected_wall2 == walls->back()));
            }

            SECTION("MixedTransition and Crossing") {}
            SECTION("More then two Splitpoints") {}
        }
    }


    SECTION("ComputeSplitPoint")
    {
        Wall testWall(Point(0, 0), Point(0, 10));
        SECTION("No Split Point")
        {
            SECTION("Not Intersecting")
            {
                // default Walls (0,0)-(0,0)
                REQUIRE_FALSE(geometry::helper::ComputeSplitPoint(Wall(), Line()));
                REQUIRE_FALSE(geometry::helper::ComputeSplitPoint(Wall(), Wall()));

                REQUIRE_FALSE(geometry::helper::ComputeSplitPoint(testWall, Line()));
                REQUIRE_FALSE(
                    geometry::helper::ComputeSplitPoint(testWall, Line(Point(1, 1), Point(1, 2))));
                REQUIRE_FALSE(geometry::helper::ComputeSplitPoint(
                    testWall, Line(Point(0, -1), Point(1, -1))));
            }
            SECTION("Overlapping")
            {
                // Overlapping
                REQUIRE_FALSE(
                    geometry::helper::ComputeSplitPoint(testWall, Line(Point(0, 0), Point(0, 2))));
                REQUIRE_FALSE(
                    geometry::helper::ComputeSplitPoint(testWall, Line(Point(0, -1), Point(0, 4))));
            }
            SECTION("Common Start or End Point")
            {
                REQUIRE_FALSE(
                    geometry::helper::ComputeSplitPoint(testWall, Line(Point(0, 0), Point(1, 2))));
                REQUIRE_FALSE(geometry::helper::ComputeSplitPoint(
                    testWall, Line(Point(0, 10), Point(1, -1))));
            }
            SECTION("Same Line")
            {
                // Same Line
                REQUIRE_FALSE(
                    geometry::helper::ComputeSplitPoint(testWall, Line(Point(0, 0), Point(0, 10))));
                REQUIRE_FALSE(
                    geometry::helper::ComputeSplitPoint(testWall, Line(Point(0, 10), Point(0, 0))));
                REQUIRE_FALSE(geometry::helper::ComputeSplitPoint(testWall, testWall));
            }
            SECTION("Line Starting or Ending on BigWall")
            {
                REQUIRE_FALSE(
                    geometry::helper::ComputeSplitPoint(testWall, Line(Point(-1, 0), Point(1, 0))));
                REQUIRE_FALSE(geometry::helper::ComputeSplitPoint(
                    testWall, Line(Point(-1, 10), Point(2, 10))));
            }
        }
        SECTION("Split Points Available")
        {
            // Some "normal" intersection points
            REQUIRE(
                Point(0, 5) ==
                geometry::helper::ComputeSplitPoint(testWall, Line(Point(-1, 5), Point(1, 5)))
                    .value());
            REQUIRE(
                Point(0, 0.5) ==
                geometry::helper::ComputeSplitPoint(testWall, Line(Point(-0.5, 0), Point(0.5, 1)))
                    .value());

            // line starting or ending on bigwall
            REQUIRE(
                Point(0, 0.5) ==
                geometry::helper::ComputeSplitPoint(testWall, Line(Point(0, 0.5), Point(10, 1)))
                    .value());
            REQUIRE(
                Point(0, 1) ==
                geometry::helper::ComputeSplitPoint(testWall, Line(Point(1, 1), Point(0, 1)))
                    .value());
        }
    }

    SECTION("RemoveOverlappingWall")
    {
        NormalSubRoom subroom;
        subroom.AddWall(Wall(Point(0, 0), Point(10, 0)));

        SECTION("Not Overlapping")
        {
            REQUIRE_FALSE(
                geometry::helper::RemoveOverlappingWall(Line(Point(0, 0), Point(-6, 0)), subroom));
            REQUIRE_FALSE(
                geometry::helper::RemoveOverlappingWall(Line(Point(-3, 0), Point(-6, 0)), subroom));
            REQUIRE_FALSE(
                geometry::helper::RemoveOverlappingWall(Line(Point(-3, 3), Point(-6, 0)), subroom));
        }


        SECTION("Simple Overlapping")
        {
            REQUIRE(
                geometry::helper::RemoveOverlappingWall(Line(Point(5, 0), Point(6, 0)), subroom));
            REQUIRE(subroom.GetAllWalls().size() == 2);
            auto it = std::find(
                subroom.GetAllWalls().begin(),
                subroom.GetAllWalls().end(),
                Wall(Point(0, 0), Point(5, 0)));
            REQUIRE(it != subroom.GetAllWalls().end());
            it = std::find(
                subroom.GetAllWalls().begin(),
                subroom.GetAllWalls().end(),
                Wall(Point(6, 0), Point(10, 0)));
            REQUIRE(it != subroom.GetAllWalls().end());
        }

        SECTION("Overlapping at End of Wall")
        {
            REQUIRE(
                geometry::helper::RemoveOverlappingWall(Line(Point(0, 0), Point(1, 0)), subroom));
            REQUIRE(subroom.GetAllWalls().size() == 1);
            REQUIRE(subroom.GetAllWalls().front() == Line(Point(1, 0), Point(10, 0)));

            REQUIRE(
                geometry::helper::RemoveOverlappingWall(Line(Point(10, 0), Point(5, 0)), subroom));
            REQUIRE(subroom.GetAllWalls().size() == 1);
            REQUIRE(subroom.GetAllWalls().front() == Line(Point(1, 0), Point(5, 0)));
        }

        SECTION("Completely Overlapping")
        {
            REQUIRE(
                geometry::helper::RemoveOverlappingWall(Line(Point(0, 0), Point(10, 0)), subroom));
            REQUIRE(subroom.GetAllWalls().empty());
        }
    }
}
