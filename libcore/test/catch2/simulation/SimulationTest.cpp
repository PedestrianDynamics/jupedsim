/**
 * Copyright (c) 2020 Forschungszentrum Jülich GmbH. All rights reserved.
 *
 * \section License
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
 **/
#include "SimulationHelper.h"
#include "general/Configuration.h"
#include "geometry/Crossing.h"
#include "geometry/Line.h"
#include "geometry/Room.h"
#include "geometry/SubRoom.h"
#include "geometry/Transition.h"
#include "geometry/Wall.h"
#include "pedestrian/Pedestrian.h"

#include <catch2/catch.hpp>

TEST_CASE("Simulation", "[simulation][RelocatePedestrian]")
{
    SECTION("Relocation Pedestrian")
    {
        // Create following geometry:
        // -| = Walls
        // +  = Transition
        // :  = Crossing
        //
        // (-10, 2) |------------------------| (10, 2)
        //          |    :    :    +    +    |
        //          |    :    :    +    +    |
        //          |    :    :    +    +    |
        // (-10,-2) |------------------------| (10, -2)

        // Subroom
        Wall wall111;
        wall111.SetPoint1(Point{-6., -2.});
        wall111.SetPoint2(Point{-10., -2.});
        Wall wall112;
        wall112.SetPoint1(Point{-10., -2.});
        wall112.SetPoint2(Point{-10., 2.});
        Wall wall113;
        wall113.SetPoint1(Point{-10., 2.});
        wall113.SetPoint2(Point{-6., 2.});
        auto cross12 = new Crossing;
        cross12->SetPoint1(Point{-6., -2.});
        cross12->SetPoint2(Point{-6., 2.});
        auto sub11 = new NormalSubRoom();
        sub11->SetRoomID(1);
        sub11->SetSubRoomID(1);
        sub11->AddCrossing(cross12);
        sub11->AddWall(wall111);
        sub11->AddWall(wall112);
        sub11->AddWall(wall113);
        sub11->ConvertLineToPoly(std::vector<Line *>{cross12});
        sub11->CreateBoostPoly();

        // Subroom
        Wall wall121;
        wall121.SetPoint1(Point{-2., -2.});
        wall121.SetPoint2(Point{-6., -2.});
        Wall wall122;
        wall122.SetPoint1(Point{-2., 2.});
        wall122.SetPoint2(Point{-6., 2.});
        auto cross23 = new Crossing();
        cross23->SetPoint1(Point{-2., -2.});
        cross23->SetPoint2(Point{-2., 2.});
        auto sub12 = new NormalSubRoom();
        sub12->SetRoomID(1);
        sub12->SetSubRoomID(2);
        sub12->AddCrossing(cross12);
        sub12->AddCrossing(cross23);
        sub12->AddWall(wall121);
        sub12->AddWall(wall122);
        sub12->ConvertLineToPoly(std::vector<Line *>{cross12, cross23});
        sub12->CreateBoostPoly();

        //Subroom
        Wall wall131;
        wall131.SetPoint1(Point{-2., -2.});
        wall131.SetPoint2(Point{2., -2.});
        Wall wall132;
        wall132.SetPoint1(Point{-2., 2.});
        wall132.SetPoint2(Point{2., 2.});
        auto trans12 = new Transition();
        trans12->SetID(1);
        trans12->SetPoint1(Point{2., -2.});
        trans12->SetPoint2(Point{2., 2.});
        auto sub13 = new NormalSubRoom();
        sub13->SetRoomID(1);
        sub13->SetSubRoomID(3);
        sub13->AddCrossing(cross23);
        sub13->AddTransition(trans12);
        sub13->AddWall(wall131);
        sub13->AddWall(wall132);
        sub13->ConvertLineToPoly(std::vector<Line *>{cross23, trans12});
        sub13->CreateBoostPoly();

        // Create room
        auto room1 = new Room();
        room1->SetID(1);
        room1->AddSubRoom(sub11);
        room1->AddSubRoom(sub12);
        room1->AddSubRoom(sub13);

        //Subroom
        Wall wall211;
        wall211.SetPoint1(Point{2., -2.});
        wall211.SetPoint2(Point{6., -2.});
        Wall wall212;
        wall212.SetPoint1(Point{2., 2.});
        wall212.SetPoint2(Point{6., 2.});
        auto trans23 = new Transition();
        trans23->SetID(2);
        trans23->SetPoint1(Point{6., -2.});
        trans23->SetPoint2(Point{6., 2.});
        auto sub21 = new NormalSubRoom();
        sub21->SetRoomID(2);
        sub21->SetSubRoomID(1);
        sub21->AddTransition(trans12);
        sub21->AddTransition(trans23);
        sub21->AddWall(wall211);
        sub21->AddWall(wall212);
        sub21->ConvertLineToPoly(std::vector<Line *>{trans12, trans23});
        sub21->CreateBoostPoly();

        // Create room
        auto room2 = new Room();
        room2->SetID(2);
        room2->AddSubRoom(sub21);

        //Subroom
        Wall wall311;
        wall311.SetPoint1(Point{6., -2.});
        wall311.SetPoint2(Point{10., -2.});
        Wall wall312;
        wall312.SetPoint1(Point{10., -2.});
        wall312.SetPoint2(Point{10., 2.});
        Wall wall313;
        wall313.SetPoint1(Point{10., 2.});
        wall313.SetPoint2(Point{6., 2.});
        auto sub31 = new NormalSubRoom();
        sub31->SetRoomID(3);
        sub31->SetSubRoomID(1);
        sub31->AddTransition(trans23);
        sub31->AddWall(wall311);
        sub31->AddWall(wall312);
        sub31->AddWall(wall313);
        sub31->ConvertLineToPoly(std::vector<Line *>{trans23});
        sub31->CreateBoostPoly();

        // Create room
        auto room3 = new Room();
        room3->SetID(3);
        room3->AddSubRoom(sub31);

        //
        cross12->SetRoom1(room1);
        cross12->SetSubRoom1(sub11);
        cross12->SetSubRoom2(sub12);

        cross23->SetRoom1(room1);
        cross23->SetSubRoom1(sub12);
        cross23->SetSubRoom2(sub13);

        trans12->SetRoom1(room1);
        trans12->SetSubRoom1(sub13);
        trans12->SetRoom2(room2);
        trans12->SetSubRoom2(sub21);

        trans23->SetRoom1(room2);
        trans23->SetSubRoom1(sub21);
        trans23->SetRoom2(room3);
        trans23->SetSubRoom2(sub31);

        Building building;
        building.AddRoom(room1);
        building.AddRoom(room2);
        building.AddRoom(room3);
        building.AddTransition(trans12);
        building.AddTransition(trans23);
        building.AddCrossing(cross12);
        building.AddCrossing(cross23);
        REQUIRE(building.InitGeometry());
        REQUIRE(building.GetAllRooms().size() == 3);
        REQUIRE(building.GetAllTransitions().size() == 2);
        REQUIRE(building.GetAllCrossings().size() == 2);

        SECTION("Pedestrian in same room")
        {
            Pedestrian ped;
            ped.SetPos({-8, -1}, true);
            ped.SetRoomID(1, "");
            ped.SetSubRoomID(1);

            auto ret = SimulationHelper::RelocatePedestrian(building, ped);
            REQUIRE(ret.has_value());
            REQUIRE_FALSE(ret.value());
        }

        SECTION("Pedestrian in neighboring subroom")
        {
            Pedestrian ped;
            ped.SetPos({-8, -1}, true);
            ped.SetRoomID(1, "");
            ped.SetSubRoomID(2);

            auto ret = SimulationHelper::RelocatePedestrian(building, ped);
            REQUIRE(ret.has_value());
            REQUIRE(ret.value());
            REQUIRE(ped.GetRoomID() == 1);
            REQUIRE(ped.GetSubRoomID() == 1);
        }

        SECTION("Pedestrian in neighboring room")
        {
            Pedestrian ped;
            ped.SetPos({1, -1}, true);
            ped.SetRoomID(2, "");
            ped.SetSubRoomID(1);

            auto ret = SimulationHelper::RelocatePedestrian(building, ped);
            REQUIRE(ret.has_value());
            REQUIRE(ret.value());
            REQUIRE(ped.GetRoomID() == 1);
            REQUIRE(ped.GetSubRoomID() == 3);
        }

        SECTION("Pedestrian not in neighboring subroom")
        {
            Pedestrian ped;
            ped.SetPos({-8, -1}, true);
            ped.SetRoomID(1, "");
            ped.SetSubRoomID(3);

            auto ret = SimulationHelper::RelocatePedestrian(building, ped);
            REQUIRE_FALSE(ret.has_value());
        }

        SECTION("Pedestrian in neighboring room")
        {
            Pedestrian ped;
            ped.SetPos({1, -1}, true);
            ped.SetRoomID(3, "");
            ped.SetSubRoomID(1);

            auto ret = SimulationHelper::RelocatePedestrian(building, ped);
            REQUIRE_FALSE(ret.has_value());
        }
    };
}
