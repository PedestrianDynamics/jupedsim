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

#include "geometry/Building.h"

#include "general/Configuration.h"
#include "geometry/Crossing.h"
#include "geometry/Line.h"
#include "geometry/Room.h"
#include "geometry/SubRoom.h"
#include "geometry/Transition.h"
#include "geometry/Wall.h"
#include "pedestrian/Pedestrian.h"

#include <catch2/catch.hpp>
#include <cmath>


TEST_CASE("geometry/Building", "[geometry][building][closestTransition]")
{
    SECTION("Closest transition")
    {
        Crossing sub1sub2;
        sub1sub2.SetPoint1(Point{10., -10.});
        sub1sub2.SetPoint2(Point{0., 0.});
        sub1sub2.SetState(DoorState::OPEN);

        Crossing sub1sub4;
        sub1sub4.SetPoint1(Point{-10., -10.});
        sub1sub4.SetPoint2(Point{0., 0.});
        sub1sub4.SetState(DoorState::OPEN);

        Crossing sub2sub3;
        sub2sub3.SetPoint1(Point{10., 10.});
        sub2sub3.SetPoint2(Point{0., 0.});
        sub2sub3.SetState(DoorState::OPEN);

        Crossing sub3sub4;
        sub3sub4.SetPoint1(Point{-10., 10.});
        sub3sub4.SetPoint2(Point{0., 0.});
        sub3sub4.SetState(DoorState::OPEN);

        Transition trans1;
        trans1.SetID(1);
        trans1.SetPoint1(Point{-2., -10.});
        trans1.SetPoint2(Point{2., -10.});

        Transition trans2;
        trans2.SetID(2);
        trans2.SetPoint1(Point{10., -2.});
        trans2.SetPoint2(Point{10., 2.});

        Transition trans3;
        trans3.SetID(3);
        trans3.SetPoint1(Point{-2., 10.});
        trans3.SetPoint2(Point{2., 10.});

        Transition trans4;
        trans4.SetID(4);
        trans4.SetPoint1(Point{-10., -2.});
        trans4.SetPoint2(Point{-10., 2.});

        Wall wall11;
        wall11.SetPoint1(Point{-10., -10.});
        wall11.SetPoint2(Point{-2., -10.});
        Wall wall12;
        wall12.SetPoint1(Point{2., -10.});
        wall12.SetPoint2(Point{10., -10.});

        Wall wall21;
        wall21.SetPoint1(Point{10., -10.});
        wall21.SetPoint2(Point{10., -2.});
        Wall wall22;
        wall22.SetPoint1(Point{10., 2.});
        wall22.SetPoint2(Point{10., 10.});

        Wall wall31;
        wall31.SetPoint1(Point{-10., 10.});
        wall31.SetPoint2(Point{-2., 10.});
        Wall wall32;
        wall32.SetPoint1(Point{2., 10.});
        wall32.SetPoint2(Point{10., 10.});

        Wall wall41;
        wall41.SetPoint1(Point{-10., -10.});
        wall41.SetPoint2(Point{-10., -2.});
        Wall wall42;
        wall42.SetPoint1(Point{-10., 2.});
        wall42.SetPoint2(Point{-10., 10.});

        auto sub1 = new NormalSubRoom();
        sub1->SetSubRoomID(1);
        sub1->AddCrossing(&sub1sub2);
        sub1->AddCrossing(&sub1sub4);
        sub1->AddTransition(&trans1);
        sub1->AddWall(wall11);
        sub1->AddWall(wall12);
        sub1->ConvertLineToPoly(std::vector<Line *>{&sub1sub2, &sub1sub4, &trans1});
        sub1->CreateBoostPoly();

        auto sub2 = new NormalSubRoom();
        sub2->SetSubRoomID(2);
        sub2->AddCrossing(&sub1sub2);
        sub2->AddCrossing(&sub2sub3);
        sub2->AddTransition(&trans2);
        sub2->AddWall(wall21);
        sub2->AddWall(wall22);
        sub2->ConvertLineToPoly(std::vector<Line *>{&sub1sub2, &sub2sub3, &trans2});
        sub2->CreateBoostPoly();

        auto sub3 = new NormalSubRoom();
        sub3->SetSubRoomID(3);
        sub3->AddCrossing(&sub2sub3);
        sub3->AddCrossing(&sub3sub4);
        sub3->AddTransition(&trans3);
        sub3->AddWall(wall31);
        sub3->AddWall(wall32);
        sub3->ConvertLineToPoly(std::vector<Line *>{&sub2sub3, &sub3sub4, &trans3});
        sub3->CreateBoostPoly();

        auto sub4 = new NormalSubRoom();
        sub4->SetSubRoomID(4);
        sub4->AddCrossing(&sub1sub4);
        sub4->AddCrossing(&sub3sub4);
        sub4->AddTransition(&trans4);
        sub4->AddWall(wall41);
        sub4->AddWall(wall42);
        sub4->ConvertLineToPoly(std::vector<Line *>{&sub1sub4, &sub3sub4, &trans4});
        sub4->CreateBoostPoly();

        auto room = new Room();
        room->SetID(0);
        room->AddSubRoom(sub1);
        room->AddSubRoom(sub2);
        room->AddSubRoom(sub3);
        room->AddSubRoom(sub4);
        Building building;
        building.AddRoom(room);
        Pedestrian ped;
        ped.SetRoomID(0, "");

        SECTION("No cutoff distance given, closest to 1")
        {
            ped.SetSubRoomID(0);
            ped.SetPos({0, -5}, true);
            auto closest = building.FindClosestTransition(ped);
            REQUIRE(closest.has_value());
            REQUIRE(closest.value()->GetID() == 1);
        }

        SECTION("Equal distance to all")
        {
            ped.SetPos({10, 10}, true);
            auto closest = building.FindClosestTransition(ped);
            REQUIRE(closest.has_value());
            REQUIRE(closest.value()->GetID() >= 1);
            REQUIRE(closest.value()->GetID() <= 4);
        }

        SECTION("Equal distance to 1 and 2")
        {
            ped.SetPos({2, 2}, true);
            auto closest = building.FindClosestTransition(ped);
            REQUIRE(closest.has_value());
            REQUIRE(closest.value()->GetID() >= 1);
            REQUIRE(closest.value()->GetID() <= 2);
        }

        SECTION("Cutoff distance given, with 1 in range")
        {
            ped.SetSubRoomID(0);
            ped.SetPos({0, -9}, true);
            auto closest = building.FindClosestTransition(ped, 2);
            REQUIRE(closest.has_value());
            REQUIRE(closest.value()->GetID() == 1);
        }

        SECTION("Cutoff distance given, with no transition in range")
        {
            ped.SetSubRoomID(0);
            ped.SetPos({0, 0}, true);
            auto closest = building.FindClosestTransition(ped, 2);
            REQUIRE_FALSE(closest.has_value());
        }
    }
}
