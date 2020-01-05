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
#include "routing/ff_router/UnivFFviaFM.h"

#include "general/Configuration.h"
#include "geometry/Crossing.h"
#include "geometry/Line.h"
#include "geometry/Room.h"
#include "geometry/SubRoom.h"
#include "geometry/Transition.h"
#include "geometry/Wall.h"

#include <catch2/catch.hpp>
#include <cmath>
TEST_CASE("routing/ff_router/UnivFFviaFM", "[routing][UnivFFviaFM]")
{
    SECTION("Constructor")
    {
        {
            Crossing sub1sub2;
            sub1sub2.SetPoint1(Point{1., 0.});
            sub1sub2.SetPoint2(Point{1., 1.});
            sub1sub2.SetState(DoorState::OPEN);
            Crossing sub2sub3;
            sub2sub3.SetPoint1(Point{2., 0.});
            sub2sub3.SetPoint2(Point{2., 1.});
            sub2sub3.SetState(DoorState::OPEN);
            Transition trans1;
            trans1.SetPoint1(Point{0., 0.});
            trans1.SetPoint2(Point{0., 1.});
            Transition trans3;
            trans3.SetPoint1(Point{3., 0.});
            trans3.SetPoint2(Point{3., 1.});
            Wall wall11;
            wall11.SetPoint1(Point{0., 0.});
            wall11.SetPoint2(Point{1., 0.});
            Wall wall12;
            wall12.SetPoint1(Point{0., 1.});
            wall12.SetPoint2(Point{1., 1.});
            Wall wall21;
            wall21.SetPoint1(Point{1., 0.});
            wall21.SetPoint2(Point{2., 0.});
            Wall wall22;
            wall22.SetPoint1(Point{1., 1.});
            wall22.SetPoint2(Point{2., 1.});
            Wall wall31;
            wall31.SetPoint1(Point{2., 0.});
            wall31.SetPoint2(Point{3., 0.});
            Wall wall32;
            wall32.SetPoint1(Point{2., 1.});
            wall32.SetPoint2(Point{3., 1.});
            auto sub1 = new NormalSubRoom();
            sub1->SetSubRoomID(1);
            sub1->AddCrossing(&sub1sub2);
            sub1->AddTransition(&trans1);
            sub1->AddWall(wall11);
            sub1->AddWall(wall12);
            sub1->ConvertLineToPoly(std::vector<Line *>{&sub1sub2, &trans1});
            sub1->CreateBoostPoly();
            auto sub2 = new NormalSubRoom();
            sub2->SetSubRoomID(2);
            sub2->AddCrossing(&sub1sub2);
            sub2->AddCrossing(&sub2sub3);
            sub2->AddWall(wall21);
            sub2->AddWall(wall22);
            sub2->ConvertLineToPoly(std::vector<Line *>{&sub1sub2, &sub2sub3});
            sub2->CreateBoostPoly();
            auto sub3 = new NormalSubRoom();
            sub3->SetSubRoomID(3);
            sub3->AddCrossing(&sub2sub3);
            sub3->AddTransition(&trans3);
            sub3->AddWall(wall31);
            sub3->AddWall(wall32);
            sub3->ConvertLineToPoly(std::vector<Line *>{&sub2sub3, &trans3});
            sub3->CreateBoostPoly();
            auto room = new Room();
            room->AddSubRoom(sub1);
            room->AddSubRoom(sub2);
            room->AddSubRoom(sub3);
            Configuration config;
            double hx              = 0.125;
            double wallAvoid       = 0.8;
            bool useWallDistance   = false;
            std::vector<int> doors = {trans1.GetUniqueID(),
                                      trans3.GetUniqueID(),
                                      sub1sub2.GetUniqueID(),
                                      sub2sub3.GetUniqueID()};
            REQUIRE_NOTHROW(UnivFFviaFM(room, &config, hx, wallAvoid, useWallDistance, doors));
            UnivFFviaFM univ(room, &config, hx, wallAvoid, useWallDistance, doors);
        }
        {
            // Test constructor with a subroom without doors, which should throw an exception
            Transition trans1;
            trans1.SetPoint1(Point{0., 0.});
            trans1.SetPoint2(Point{0., 1.});
            Transition trans3;
            trans3.SetPoint1(Point{3., 0.});
            trans3.SetPoint2(Point{3., 1.});
            Wall wall11;
            wall11.SetPoint1(Point{0., 0.});
            wall11.SetPoint2(Point{1., 0.});
            Wall wall12;
            wall12.SetPoint1(Point{0., 1.});
            wall12.SetPoint2(Point{1., 1.});
            Wall wall21;
            wall21.SetPoint1(Point{1., 0.});
            wall21.SetPoint2(Point{2., 0.});
            Wall wall22;
            wall22.SetPoint1(Point{1., 1.});
            wall22.SetPoint2(Point{2., 1.});
            Wall wall23;
            wall23.SetPoint1(Point{1., 0.});
            wall23.SetPoint2(Point{1., 1.});
            Wall wall24;
            wall24.SetPoint1(Point{2., 0.});
            wall24.SetPoint2(Point{2., 1.});
            Wall wall31;
            wall31.SetPoint1(Point{2., 0.});
            wall31.SetPoint2(Point{3., 0.});
            Wall wall32;
            wall32.SetPoint1(Point{2., 1.});
            wall32.SetPoint2(Point{3., 1.});
            auto sub1 = new NormalSubRoom();
            sub1->SetSubRoomID(1);
            sub1->AddTransition(&trans1);
            sub1->AddWall(wall11);
            sub1->AddWall(wall23);
            sub1->AddWall(wall12);
            sub1->ConvertLineToPoly(std::vector<Line *>{&trans1});
            sub1->CreateBoostPoly();
            auto sub2 = new NormalSubRoom();
            sub2->SetSubRoomID(2);
            sub2->AddWall(wall21);
            sub2->AddWall(wall22);
            sub2->AddWall(wall23);
            sub2->AddWall(wall24);
            sub2->ConvertLineToPoly(std::vector<Line *>{});
            sub2->CreateBoostPoly();
            auto sub3 = new NormalSubRoom();
            sub3->SetSubRoomID(3);
            sub3->AddTransition(&trans3);
            sub3->AddWall(wall31);
            sub3->AddWall(wall32);
            sub3->AddWall(wall24);
            sub3->ConvertLineToPoly(std::vector<Line *>{&trans3});
            sub3->CreateBoostPoly();
            auto room = new Room();
            room->AddSubRoom(sub1);
            room->AddSubRoom(sub2);
            room->AddSubRoom(sub3);
            Configuration config;
            double hx              = 0.125;
            double wallAvoid       = 0.8;
            bool useWallDistance   = false;
            std::vector<int> doors = {trans1.GetUniqueID(), trans3.GetUniqueID()};
            REQUIRE_THROWS(UnivFFviaFM(room, &config, hx, wallAvoid, useWallDistance, doors));
        }
        {
            // Test constructor with a tiny subroom where no inside point could be found, which should throw an exception
            Transition trans1;
            trans1.SetPoint1(Point{0., 0.});
            trans1.SetPoint2(Point{0., 1.});
            Wall wall11;
            wall11.SetPoint1(Point{0., 0.});
            wall11.SetPoint2(Point{0.1, 0.});
            Wall wall12;
            wall12.SetPoint1(Point{0., 1.});
            wall12.SetPoint2(Point{0.1, 1.});
            Wall wall13;
            wall13.SetPoint1(Point{0.1, 1.});
            wall13.SetPoint2(Point{0.1, 1.});

            auto sub1 = new NormalSubRoom();
            sub1->SetSubRoomID(1);
            sub1->AddTransition(&trans1);
            sub1->AddWall(wall11);
            sub1->AddWall(wall12);
            sub1->AddWall(wall13);
            sub1->ConvertLineToPoly(std::vector<Line *>{&trans1});
            sub1->CreateBoostPoly();
            auto room = new Room();
            room->AddSubRoom(sub1);

            Configuration config;
            double hx              = 0.125;
            double wallAvoid       = 0.8;
            bool useWallDistance   = false;
            std::vector<int> doors = {trans1.GetUniqueID()};
            REQUIRE_THROWS(UnivFFviaFM(room, &config, hx, wallAvoid, useWallDistance, doors));
        }
    }
}
