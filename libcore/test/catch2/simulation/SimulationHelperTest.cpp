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

TEST_CASE(
    "SimulationHelper::UpdatePedestrianRoomInformation",
    "[SimulationHelper][UpdatePedestrianRoomInformation]")
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
        ped.SetRoomID(sub11->GetRoomID(), "");
        ped.SetSubRoomID(sub11->GetSubRoomID());
        ped.SetSubRoomUID(sub11->GetUID());

        auto ret = SimulationHelper::UpdatePedestrianRoomInformation(building, ped);
        REQUIRE(ret == PedRelocation::NOT_NEEDED);
        REQUIRE(ped.GetRoomID() == sub11->GetRoomID());
        REQUIRE(ped.GetSubRoomID() == sub11->GetSubRoomID());
        REQUIRE(ped.GetSubRoomUID() == sub11->GetUID());
    }

    SECTION("Pedestrian in same room on crossing")
    {
        Pedestrian ped;
        ped.SetPos({-6, -1}, true);
        ped.SetRoomID(sub11->GetRoomID(), "");
        ped.SetSubRoomID(sub11->GetSubRoomID());
        ped.SetSubRoomUID(sub11->GetUID());

        auto ret = SimulationHelper::UpdatePedestrianRoomInformation(building, ped);
        REQUIRE(ret == PedRelocation::NOT_NEEDED);
        REQUIRE(ped.GetRoomID() == sub11->GetRoomID());
        REQUIRE(ped.GetSubRoomID() == sub11->GetSubRoomID());
        REQUIRE(ped.GetSubRoomUID() == sub11->GetUID());
    }

    SECTION("Pedestrian in same room on transition")
    {
        Pedestrian ped;
        ped.SetPos({2, -1}, true);
        ped.SetRoomID(sub13->GetRoomID(), "");
        ped.SetSubRoomID(sub13->GetSubRoomID());
        ped.SetSubRoomUID(sub13->GetUID());

        auto ret = SimulationHelper::UpdatePedestrianRoomInformation(building, ped);
        REQUIRE(ret == PedRelocation::NOT_NEEDED);
        REQUIRE(ped.GetRoomID() == sub13->GetRoomID());
        REQUIRE(ped.GetSubRoomID() == sub13->GetSubRoomID());
        REQUIRE(ped.GetSubRoomUID() == sub13->GetUID());
    }

    SECTION("Pedestrian in neighboring subroom")
    {
        Pedestrian ped;
        ped.SetPos({-8, -1}, true);
        ped.SetRoomID(sub12->GetRoomID(), "");
        ped.SetSubRoomID(sub12->GetSubRoomID());
        ped.SetSubRoomUID(sub12->GetUID());

        auto ret = SimulationHelper::UpdatePedestrianRoomInformation(building, ped);
        REQUIRE(ret == PedRelocation::SUCCESSFUL);
        REQUIRE(ped.GetRoomID() == sub11->GetRoomID());
        REQUIRE(ped.GetSubRoomID() == sub11->GetSubRoomID());
        REQUIRE(ped.GetSubRoomUID() == sub11->GetUID());
    }

    SECTION("Pedestrian in neighboring room")
    {
        Pedestrian ped;
        ped.SetPos({1, -1}, true);
        ped.SetRoomID(sub21->GetRoomID(), "");
        ped.SetSubRoomID(sub21->GetSubRoomID());
        ped.SetSubRoomUID(sub21->GetUID());

        auto ret = SimulationHelper::UpdatePedestrianRoomInformation(building, ped);
        REQUIRE(ret == PedRelocation::SUCCESSFUL);
        REQUIRE(ped.GetRoomID() == sub13->GetRoomID());
        REQUIRE(ped.GetSubRoomID() == sub13->GetSubRoomID());
        REQUIRE(ped.GetSubRoomUID() == sub13->GetUID());
        REQUIRE(ped.ChangedSubRoom());
    }

    SECTION("Pedestrian not in neighboring subroom")
    {
        Pedestrian ped;
        ped.SetPos({-8, -1}, true);
        ped.SetRoomID(sub13->GetRoomID(), "");
        ped.SetSubRoomID(sub13->GetSubRoomID());
        ped.SetSubRoomUID(sub13->GetUID());

        auto ret = SimulationHelper::UpdatePedestrianRoomInformation(building, ped);
        REQUIRE(ret == PedRelocation::FAILED);
        REQUIRE(ped.GetRoomID() == -1);
        REQUIRE(ped.GetSubRoomID() == -1);
        REQUIRE(ped.GetSubRoomUID() == -1);
    }

    SECTION("Pedestrian not in neighboring room")
    {
        Pedestrian ped;
        ped.SetPos({1, -1}, true);
        ped.SetRoomID(sub31->GetRoomID(), "");
        ped.SetSubRoomID(sub31->GetSubRoomID());
        ped.SetSubRoomUID(sub31->GetUID());

        auto ret = SimulationHelper::UpdatePedestrianRoomInformation(building, ped);
        REQUIRE(ret == PedRelocation::FAILED);
        REQUIRE(ped.GetRoomID() == -1);
        REQUIRE(ped.GetSubRoomID() == -1);
        REQUIRE(ped.GetSubRoomUID() == -1);
    }
}

TEST_CASE(
    "SimulationHelper::FindPedestriansReachedFinalGoal",
    "[SimulationHelper][FindPedestriansReachedFinalGoal]")
{
    // Create following geometry:
    // -| = Walls
    // +  = Transition
    // x  = Goal
    //
    // (-5, 2) |----------| (5, 2)
    //         |    xxx   +  xxx
    //         |          +  xxx
    //         |    xxx   +  xxx
    // (-5,-2) |----------| (5, -2)
    Wall wall111;
    wall111.SetPoint1(Point{5., -2.});
    wall111.SetPoint2(Point{-5., -2.});
    Wall wall112;
    wall112.SetPoint1(Point{-5., -2.});
    wall112.SetPoint2(Point{-5., 2.});
    Wall wall113;
    wall113.SetPoint1(Point{-5., 2.});
    wall113.SetPoint2(Point{5., 2.});
    auto trans11 = new Transition();
    trans11->SetID(1);
    trans11->SetPoint1(Point{5., -2.});
    trans11->SetPoint2(Point{5., 2.});

    auto sub11 = new NormalSubRoom();
    sub11->SetRoomID(1);
    sub11->SetSubRoomID(1);
    sub11->AddTransition(trans11);
    sub11->AddWall(wall111);
    sub11->AddWall(wall112);
    sub11->AddWall(wall113);
    sub11->ConvertLineToPoly(std::vector<Line *>{trans11});
    sub11->CreateBoostPoly();

    // Create room
    auto room1 = new Room();
    room1->SetID(1);
    room1->AddSubRoom(sub11);

    // Create goals
    auto goalInsideFinal = new Goal();
    goalInsideFinal->SetIsFinalGoal(true);
    goalInsideFinal->SetId(1);
    goalInsideFinal->SetRoomID(1);
    goalInsideFinal->SetSubRoomID(1);
    goalInsideFinal->AddWall(Wall({-2, 0.5}, {-2, 1.5}));
    goalInsideFinal->AddWall(Wall({-2, 1.5}, {2, 1.5}));
    goalInsideFinal->AddWall(Wall({2, 1.5}, {2, 0.5}));
    goalInsideFinal->AddWall(Wall({2, 0.5}, {-2, 0.5}));
    goalInsideFinal->ConvertLineToPoly();

    auto goalInside = new Goal();
    goalInside->SetIsFinalGoal(false);
    goalInside->SetId(2);
    goalInside->SetRoomID(1);
    goalInside->SetSubRoomID(1);
    goalInside->AddWall(Wall({-2, -0.5}, {-2, -1.5}));
    goalInside->AddWall(Wall({-2, -1.5}, {2, -1.5}));
    goalInside->AddWall(Wall({2, -1.5}, {2, -0.5}));
    goalInside->AddWall(Wall({2, -0.5}, {-2, -0.5}));
    goalInside->ConvertLineToPoly();

    auto goalOutside = new Goal();
    goalOutside->SetIsFinalGoal(true);
    goalOutside->SetId(3);
    goalOutside->SetRoomID(1);
    goalOutside->SetSubRoomID(1);
    goalOutside->AddWall(Wall({6, -1}, {6, 1}));
    goalOutside->AddWall(Wall({6, 1}, {8, 1}));
    goalOutside->AddWall(Wall({8, 1}, {8, -1}));
    goalOutside->AddWall(Wall({8, -1}, {6, -1}));
    goalOutside->ConvertLineToPoly();

    sub11->AddGoalID(1);
    sub11->AddGoalID(2);

    Building building;
    building.AddRoom(room1);
    building.AddTransition(trans11);
    building.AddGoal(goalInside);
    building.AddGoal(goalInsideFinal);
    building.AddGoal(goalOutside);

    REQUIRE(building.InitGeometry());
    REQUIRE(building.GetAllRooms().size() == 1);
    REQUIRE(building.GetAllTransitions().size() == 1);
    REQUIRE(building.GetAllGoals().size() == 3);

    // Goal outside
    Pedestrian pedOutsideGoalOutsideInGoalOutside;
    pedOutsideGoalOutsideInGoalOutside.SetPos({7, 1.5});
    pedOutsideGoalOutsideInGoalOutside.SetFinalDestination(goalOutside->GetId());
    Pedestrian pedOutsideGoalOutsideNotInGoalOutside;
    pedOutsideGoalOutsideNotInGoalOutside.SetPos({5.5, -1.5});
    pedOutsideGoalOutsideNotInGoalOutside.SetFinalDestination(goalOutside->GetId());
    Pedestrian pedInsideGoalOutsideInGoalFinal;
    pedInsideGoalOutsideInGoalFinal.SetPos({-0.5, 1.});
    pedInsideGoalOutsideInGoalFinal.SetFinalDestination(goalOutside->GetId());
    pedInsideGoalOutsideInGoalFinal.SetRoomID(1, "");
    pedInsideGoalOutsideInGoalFinal.SetSubRoomID(1);
    Pedestrian pedInsideGoalOutsideInGoalNotFinal;
    pedInsideGoalOutsideInGoalNotFinal.SetPos({-1.5, -1.});
    pedInsideGoalOutsideInGoalNotFinal.SetFinalDestination(goalOutside->GetId());
    pedInsideGoalOutsideInGoalNotFinal.SetRoomID(1, "");
    pedInsideGoalOutsideInGoalNotFinal.SetSubRoomID(1);
    Pedestrian pedInsideGoalOutsideInRoomLeft;
    pedInsideGoalOutsideInRoomLeft.SetPos({-3.5, -0.5});
    pedInsideGoalOutsideInRoomLeft.SetFinalDestination(goalOutside->GetId());
    pedInsideGoalOutsideInRoomLeft.SetRoomID(1, "");
    pedInsideGoalOutsideInRoomLeft.SetSubRoomID(1);
    Pedestrian pedInsideGoalOutsideInRoomRight;
    pedInsideGoalOutsideInRoomRight.SetPos({3.5, 0.5});
    pedInsideGoalOutsideInRoomRight.SetFinalDestination(goalOutside->GetId());
    pedInsideGoalOutsideInRoomRight.SetRoomID(1, "");
    pedInsideGoalOutsideInRoomRight.SetSubRoomID(1);

    // no goal
    Pedestrian pedOutsideNoGoalOutsideInGoalOutside;
    pedOutsideNoGoalOutsideInGoalOutside.SetPos({7, 0.5});
    pedOutsideNoGoalOutsideInGoalOutside.SetFinalDestination(FINAL_DEST_OUT);
    Pedestrian pedOutsideNoGoalOutsideNotInGoalOutside;
    pedOutsideNoGoalOutsideNotInGoalOutside.SetPos({5.5, 1.5});
    pedOutsideNoGoalOutsideNotInGoalOutside.SetFinalDestination(FINAL_DEST_OUT);
    Pedestrian pedInsideNoGoalOutsideInGoalFinal;
    pedInsideNoGoalOutsideInGoalFinal.SetPos({-1.5, 1.});
    pedInsideNoGoalOutsideInGoalFinal.SetFinalDestination(FINAL_DEST_OUT);
    pedInsideNoGoalOutsideInGoalFinal.SetRoomID(1, "");
    pedInsideNoGoalOutsideInGoalFinal.SetSubRoomID(1);
    Pedestrian pedInsideNoGoalOutsideInGoalNotFinal;
    pedInsideNoGoalOutsideInGoalNotFinal.SetPos({0.5, -1.});
    pedInsideNoGoalOutsideInGoalNotFinal.SetFinalDestination(FINAL_DEST_OUT);
    pedInsideNoGoalOutsideInGoalNotFinal.SetRoomID(1, "");
    pedInsideNoGoalOutsideInGoalNotFinal.SetSubRoomID(1);
    Pedestrian pedInsideNoGoalOutsideInRoomLeft;
    pedInsideNoGoalOutsideInRoomLeft.SetPos({-3.5, 1.5});
    pedInsideNoGoalOutsideInRoomLeft.SetFinalDestination(FINAL_DEST_OUT);
    pedInsideNoGoalOutsideInRoomLeft.SetRoomID(1, "");
    pedInsideNoGoalOutsideInRoomLeft.SetSubRoomID(1);
    Pedestrian pedInsideNoGoalOutsideInRoomRight;
    pedInsideNoGoalOutsideInRoomRight.SetPos({3.5, -1.5});
    pedInsideNoGoalOutsideInRoomRight.SetFinalDestination(FINAL_DEST_OUT);
    pedInsideNoGoalOutsideInRoomRight.SetRoomID(1, "");
    pedInsideNoGoalOutsideInRoomRight.SetSubRoomID(1);


    // goal final inside
    Pedestrian pedOutsideGoalFinalInsideInGoalOutside;
    pedOutsideGoalFinalInsideInGoalOutside.SetPos({7, -0.5});
    pedOutsideGoalFinalInsideInGoalOutside.SetFinalDestination(goalInsideFinal->GetId());
    Pedestrian pedOutsideGoalFinalInsideNotInGoalOutside;
    pedOutsideGoalFinalInsideNotInGoalOutside.SetPos({5.5, 0.5});
    pedOutsideGoalFinalInsideNotInGoalOutside.SetFinalDestination(goalInsideFinal->GetId());
    Pedestrian pedInsideGoalFinalInsideInGoalFinal;
    pedInsideGoalFinalInsideInGoalFinal.SetPos({1.5, 1.});
    pedInsideGoalFinalInsideInGoalFinal.SetFinalDestination(goalInsideFinal->GetId());
    pedInsideGoalFinalInsideInGoalFinal.SetRoomID(1, "");
    pedInsideGoalFinalInsideInGoalFinal.SetSubRoomID(1);
    Pedestrian pedInsideGoalFinalInsideInGoalNotFinal;
    pedInsideGoalFinalInsideInGoalNotFinal.SetPos({-0.5, -1.});
    pedInsideGoalFinalInsideInGoalNotFinal.SetFinalDestination(goalInsideFinal->GetId());
    pedInsideGoalFinalInsideInGoalNotFinal.SetRoomID(1, "");
    pedInsideGoalFinalInsideInGoalNotFinal.SetSubRoomID(1);
    Pedestrian pedInsideGoalFinalInsideInRoomLeft;
    pedInsideGoalFinalInsideInRoomLeft.SetPos({-3.5, 0.5});
    pedInsideGoalFinalInsideInRoomLeft.SetFinalDestination(goalInsideFinal->GetId());
    pedInsideGoalFinalInsideInRoomLeft.SetRoomID(1, "");
    pedInsideGoalFinalInsideInRoomLeft.SetSubRoomID(1);
    Pedestrian pedInsideGoalFinalInsideInRoomRight;
    pedInsideGoalFinalInsideInRoomRight.SetPos({3.5, -0.5});
    pedInsideGoalFinalInsideInRoomRight.SetFinalDestination(goalInsideFinal->GetId());
    pedInsideGoalFinalInsideInRoomRight.SetRoomID(1, "");
    pedInsideGoalFinalInsideInRoomRight.SetSubRoomID(1);


    // goal not final inside
    Pedestrian pedOutsideGoalNotFinalInsideInGoalOutside;
    pedOutsideGoalNotFinalInsideInGoalOutside.SetPos({7, -1.5});
    pedOutsideGoalNotFinalInsideInGoalOutside.SetFinalDestination(goalInside->GetId());
    Pedestrian pedOutsideGoalNotFinalInsideNotInGoalOutside;
    pedOutsideGoalNotFinalInsideNotInGoalOutside.SetPos({5.5, -0.5});
    pedOutsideGoalNotFinalInsideNotInGoalOutside.SetFinalDestination(goalInside->GetId());
    Pedestrian pedInsideGoalNotFinalInsideInGoalFinal;
    pedInsideGoalNotFinalInsideInGoalFinal.SetPos({0.5, 1.});
    pedInsideGoalNotFinalInsideInGoalFinal.SetFinalDestination(goalInside->GetId());
    pedInsideGoalNotFinalInsideInGoalFinal.SetRoomID(1, "");
    pedInsideGoalNotFinalInsideInGoalFinal.SetSubRoomID(1);
    Pedestrian pedInsideGoalNotFinalInsideInGoalNotFinal;
    pedInsideGoalNotFinalInsideInGoalNotFinal.SetPos({1.5, -1.});
    pedInsideGoalNotFinalInsideInGoalNotFinal.SetFinalDestination(goalInside->GetId());
    pedInsideGoalNotFinalInsideInGoalNotFinal.SetRoomID(1, "");
    pedInsideGoalNotFinalInsideInGoalNotFinal.SetSubRoomID(1);
    Pedestrian pedInsideGoalNotFinalInsideInRoomLeft;
    pedInsideGoalNotFinalInsideInRoomLeft.SetPos({-3.5, -1.5});
    pedInsideGoalNotFinalInsideInRoomLeft.SetFinalDestination(goalInside->GetId());
    pedInsideGoalNotFinalInsideInRoomLeft.SetRoomID(1, "");
    pedInsideGoalNotFinalInsideInRoomLeft.SetSubRoomID(1);
    Pedestrian pedInsideGoalNotFinalInsideInRoomRight;
    pedInsideGoalNotFinalInsideInRoomRight.SetPos({3.5, 1.5});
    pedInsideGoalNotFinalInsideInRoomRight.SetFinalDestination(goalInside->GetId());
    pedInsideGoalNotFinalInsideInRoomRight.SetRoomID(1, "");
    pedInsideGoalNotFinalInsideInRoomRight.SetSubRoomID(1);


    SECTION("Without outside room")
    {
        SECTION("No peds reached final goal")
        {
            auto pedsReachedFinalGoal = SimulationHelper::FindPedestriansReachedFinalGoal(
                building,
                {&pedInsideGoalOutsideInGoalFinal,
                 &pedInsideGoalOutsideInGoalNotFinal,
                 &pedInsideGoalOutsideInRoomLeft,
                 &pedInsideGoalOutsideInRoomRight,
                 &pedInsideNoGoalOutsideInGoalFinal,
                 &pedInsideNoGoalOutsideInGoalNotFinal,
                 &pedInsideNoGoalOutsideInRoomLeft,
                 &pedInsideNoGoalOutsideInRoomRight,
                 &pedInsideGoalFinalInsideInGoalNotFinal,
                 &pedInsideGoalFinalInsideInRoomLeft,
                 &pedInsideGoalFinalInsideInRoomRight,
                 &pedInsideGoalNotFinalInsideInGoalFinal,
                 &pedInsideGoalNotFinalInsideInRoomLeft,
                 &pedInsideGoalNotFinalInsideInRoomRight});
            REQUIRE(pedsReachedFinalGoal.empty());
        }

        SECTION("Multiple peds inside final goal, one wants to get there")
        {
            auto pedsReachedFinalGoal = SimulationHelper::FindPedestriansReachedFinalGoal(
                building,
                {&pedInsideGoalOutsideInGoalFinal,
                 &pedInsideNoGoalOutsideInGoalFinal,
                 &pedInsideGoalFinalInsideInGoalFinal,
                 &pedInsideGoalNotFinalInsideInGoalFinal});
            REQUIRE(pedsReachedFinalGoal.size() == 1);
            REQUIRE(
                std::find(
                    std::begin(pedsReachedFinalGoal),
                    std::end(pedsReachedFinalGoal),
                    &pedInsideGoalFinalInsideInGoalFinal) != pedsReachedFinalGoal.end());
        }
    }
}

TEST_CASE("SimulationHelper::FindOutsidePedestrians", "[SimulationHelper][FindOutsidePedestrians]")
{
    // Create following geometry:
    // -| = Walls
    // +  = Transition
    //
    // (-5, 2) |----------| (5, 2)
    //         |   +      +
    //         |   +      +
    //         |   +      +
    // (-5,-2) |----------| (5, -2)
    Wall wall111;
    wall111.SetPoint1(Point{5., -2.});
    wall111.SetPoint2(Point{-2., -2.});
    Wall wall112;
    wall112.SetPoint1(Point{-2., -2.});
    wall112.SetPoint2(Point{-2., 2.});
    Wall wall113;
    wall113.SetPoint1(Point{-2., 2.});
    wall113.SetPoint2(Point{5., 2.});
    auto trans11 = new Transition();
    trans11->SetID(1);
    trans11->SetPoint1(Point{5., -2.});
    trans11->SetPoint2(Point{5., 2.});

    auto sub11 = new NormalSubRoom();
    sub11->SetRoomID(1);
    sub11->SetSubRoomID(1);
    sub11->AddTransition(trans11);
    sub11->AddWall(wall111);
    sub11->AddWall(wall112);
    sub11->AddWall(wall113);
    sub11->ConvertLineToPoly(std::vector<Line *>{trans11});
    sub11->CreateBoostPoly();

    auto room1 = new Room();
    room1->SetID(1);
    room1->AddSubRoom(sub11);
    room1->AddTransitionID(trans11->GetUniqueID());

    Wall wall211;
    wall211.SetPoint1(Point{-2., -2.});
    wall211.SetPoint2(Point{-5., -2.});
    Wall wall212;
    wall212.SetPoint1(Point{-5., -2.});
    wall212.SetPoint2(Point{-5., 2.});
    Wall wall213;
    wall213.SetPoint1(Point{-5., 2.});
    wall213.SetPoint2(Point{-2., 2.});
    auto trans12 = new Transition();
    trans12->SetID(2);
    trans12->SetPoint1(Point{-2., -2.});
    trans12->SetPoint2(Point{-2., 2.});

    auto sub21 = new NormalSubRoom();
    sub21->SetRoomID(2);
    sub21->SetSubRoomID(1);
    sub21->AddTransition(trans12);
    sub21->AddWall(wall211);
    sub21->AddWall(wall212);
    sub21->AddWall(wall213);
    sub21->ConvertLineToPoly(std::vector<Line *>{trans12});
    sub21->CreateBoostPoly();

    auto room2 = new Room();
    room2->SetID(2);
    room2->AddSubRoom(sub21);
    room2->AddTransitionID(trans12->GetUniqueID());

    Building building;
    building.AddRoom(room1);
    building.AddRoom(room2);
    building.AddTransition(trans11);
    building.AddTransition(trans12);

    trans11->SetRoom1(room1);
    trans12->SetRoom1(room1);
    trans12->SetRoom2(room2);
    REQUIRE(building.InitGeometry());
    REQUIRE(building.GetAllRooms().size() == 2);
    REQUIRE(building.GetAllTransitions().size() == 2);
    REQUIRE(building.GetAllGoals().empty());

    SECTION("Normal transition crossed")
    {
        Pedestrian ped;
        ped.SetRoomID(1, "");
        ped.SetSubRoomID(1);
        ped.SetPos({-1.7, -1.}, false);
        ped.SetPos({-2.2, -1.}, false);
        ped.UpdateRoom(-1, -1);

        std::vector<Pedestrian *> peds{&ped};
        auto outsidePeds = SimulationHelper::FindPedestriansOutside(building, peds);
        REQUIRE(peds.size() == 1);
        REQUIRE(outsidePeds.empty());
    }

    SECTION("Exit crossed")
    {
        Pedestrian ped;
        ped.SetRoomID(1, "");
        ped.SetSubRoomID(1);
        ped.SetPos({4., -1.}, false);
        ped.SetPos({5.2, -1.}, false);
        ped.UpdateRoom(-1, -1);

        std::vector<Pedestrian *> peds{&ped};
        auto outsidePeds = SimulationHelper::FindPedestriansOutside(building, peds);
        REQUIRE(peds.empty());
        REQUIRE(outsidePeds.size() == 1);
    }

    SECTION("Runtime")
    {
        size_t largeNumber = 100000;
        std::vector<Pedestrian *> peds;
        for(size_t i = 0; i < largeNumber; ++i) {
            auto ped = new Pedestrian();
            ped->SetRoomID(1, "");
            ped->SetSubRoomID(1);
            ped->SetPos({4., -1.}, false);
            ped->SetPos({5.2, -1.}, false);
            ped->UpdateRoom(-1, -1);
            peds.push_back(ped);
        }

        auto outsidePeds = SimulationHelper::FindPedestriansOutside(building, peds);
        REQUIRE(peds.empty());
        REQUIRE(outsidePeds.size() == largeNumber);

        for(auto ped : peds) {
            delete ped;
        }
        peds.clear();
    }


    SECTION("Walked through wall")
    {
        Pedestrian ped;
        ped.SetRoomID(1, "");
        ped.SetSubRoomID(1);
        ped.SetPos({4., -1.}, false);
        ped.SetPos({4, -3.}, false);
        ped.UpdateRoom(-1, -1);

        std::vector<Pedestrian *> peds{&ped};
        auto outsidePeds = SimulationHelper::FindPedestriansOutside(building, peds);
        REQUIRE(peds.size() == 1);
        REQUIRE(outsidePeds.empty());
    }

    SECTION("Walked through closed door")
    {
        Pedestrian ped;
        ped.SetRoomID(1, "");
        ped.SetSubRoomID(1);
        ped.SetPos({4., -1.}, false);
        ped.SetPos({5.2, -1.}, false);
        ped.UpdateRoom(-1, -1);
        trans11->Close();

        std::vector<Pedestrian *> peds{&ped};
        auto outsidePeds = SimulationHelper::FindPedestriansOutside(building, peds);
        REQUIRE(peds.size() == 1);
        REQUIRE(outsidePeds.empty());
    }
}

TEST_CASE("SimulationHelper::UpdateFlowAtDoors", "[SimulationHelper][UpdateFlowAtDoors]")
{
    // Create following geometry:
    // -| = Walls
    // +  = Transition
    //
    // (-5, 2) |----------| (5, 2)
    //         |   +      +
    //         |   +      +
    //         |   +      +
    // (-5,-2) |----------| (5, -2)
    Wall wall111;
    wall111.SetPoint1(Point{5., -2.});
    wall111.SetPoint2(Point{-2., -2.});
    Wall wall113;
    wall113.SetPoint1(Point{-2., 2.});
    wall113.SetPoint2(Point{5., 2.});
    auto trans11 = new Transition();
    trans11->SetID(1);
    trans11->SetPoint1(Point{5., -2.});
    trans11->SetPoint2(Point{5., 2.});


    Wall wall211;
    wall211.SetPoint1(Point{-2., -2.});
    wall211.SetPoint2(Point{-5., -2.});
    Wall wall212;
    wall212.SetPoint1(Point{-5., -2.});
    wall212.SetPoint2(Point{-5., 2.});
    Wall wall213;
    wall213.SetPoint1(Point{-5., 2.});
    wall213.SetPoint2(Point{-2., 2.});
    auto trans12 = new Transition();
    trans12->SetID(2);
    trans12->SetPoint1(Point{-2., -2.});
    trans12->SetPoint2(Point{-2., 2.});

    auto sub11 = new NormalSubRoom();
    sub11->SetRoomID(1);
    sub11->SetSubRoomID(1);
    sub11->AddTransition(trans11);
    sub11->AddTransition(trans12);
    sub11->AddWall(wall111);
    sub11->AddWall(wall113);
    sub11->ConvertLineToPoly(std::vector<Line *>{trans11});
    sub11->CreateBoostPoly();

    auto sub21 = new NormalSubRoom();
    sub21->SetRoomID(2);
    sub21->SetSubRoomID(1);
    sub21->AddTransition(trans12);
    sub21->AddWall(wall211);
    sub21->AddWall(wall212);
    sub21->AddWall(wall213);
    sub21->ConvertLineToPoly(std::vector<Line *>{trans12});
    sub21->CreateBoostPoly();

    auto room1 = new Room();
    room1->SetID(1);
    room1->AddSubRoom(sub11);
    room1->AddTransitionID(trans11->GetUniqueID());
    room1->AddTransitionID(trans12->GetUniqueID());

    auto room2 = new Room();
    room2->SetID(2);
    room2->AddSubRoom(sub21);
    room2->AddTransitionID(trans12->GetUniqueID());

    Building building;
    building.AddRoom(room1);
    building.AddRoom(room2);
    building.AddTransition(trans11);
    building.AddTransition(trans12);

    trans11->SetRoom1(room1);
    trans12->SetRoom1(room1);
    trans12->SetRoom2(room2);
    REQUIRE(building.InitGeometry());
    REQUIRE(building.GetAllRooms().size() == 2);
    REQUIRE(building.GetAllTransitions().size() == 2);
    REQUIRE(building.GetAllGoals().empty());
    REQUIRE(trans11->GetDoorUsage() == 0);
    REQUIRE(trans11->GetPartialDoorUsage() == 0);
    REQUIRE(trans12->GetDoorUsage() == 0);
    REQUIRE(trans12->GetPartialDoorUsage() == 0);

    SECTION("inside ped")
    {
        SECTION("inside ped passed transition")
        {
            Pedestrian ped;
            ped.SetRoomID(2, "");
            ped.SetSubRoomID(1);
            ped.SetExitIndex(trans12->GetUniqueID());
            ped.SetPos({-2.05, -1.}, false);
            ped.SetPos({-1.95, -1.}, false);
            ped.UpdateRoom(1, 1);
            std::vector<Pedestrian *> peds{&ped};
            SimulationHelper::UpdateFlowAtDoors(building, peds);
            REQUIRE(trans11->GetDoorUsage() == 0);
            REQUIRE(trans11->GetPartialDoorUsage() == 0);
            REQUIRE(trans12->GetDoorUsage() == 1);
            REQUIRE(trans12->GetPartialDoorUsage() == 1);
        }

        SECTION("used unindented door")
        {
            Pedestrian ped;
            ped.SetRoomID(1, "");
            ped.SetSubRoomID(1);
            ped.SetExitIndex(trans11->GetUniqueID());
            ped.SetPos({-2.05, -1.}, false);
            ped.SetPos({-1.95, -1.}, false);
            ped.UpdateRoom(-1, -1);
            std::vector<Pedestrian *> peds{&ped};
            SimulationHelper::UpdateFlowAtDoors(building, peds);
            REQUIRE(trans11->GetDoorUsage() == 0);
            REQUIRE(trans11->GetPartialDoorUsage() == 0);
            REQUIRE(trans12->GetDoorUsage() == 0);
            REQUIRE(trans12->GetPartialDoorUsage() == 0);
        }
    }

    SECTION("outside ped")
    {
        SECTION("outside ped passed exit")
        {
            Pedestrian ped;
            ped.SetRoomID(1, "");
            ped.SetSubRoomID(1);
            ped.SetExitIndex(trans11->GetUniqueID());
            ped.SetPos({4.95, -1.}, false);
            ped.SetPos({5.05, -1.}, false);
            ped.UpdateRoom(-1, -1);
            std::vector<Pedestrian *> peds{&ped};
            SimulationHelper::UpdateFlowAtDoors(building, peds);
            REQUIRE(trans11->GetDoorUsage() == 1);
            REQUIRE(trans11->GetPartialDoorUsage() == 1);
            REQUIRE(trans12->GetDoorUsage() == 0);
            REQUIRE(trans12->GetPartialDoorUsage() == 0);
        }
    }
}

TEST_CASE("SimulationHelper::FindPassedDoor", "[SimulationHelper][FindPassedDoor]")
{
    // Create following geometry:
    // -| = Walls
    // +  = Transition
    // :  = Crossing
    //
    // (-10, 2) |-------------------| (6, 2)
    //          |    :    :    +    +
    //          |    :    :    +    +
    //          |    :    :    +    +
    // (-10,-2) |-------------------| (6, -2)

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

    auto room2 = new Room();
    room2->SetID(2);
    room2->AddSubRoom(sub21);

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
    trans23->SetRoom2(nullptr);
    trans23->SetSubRoom2(nullptr);

    Building building;
    building.AddRoom(room1);
    building.AddRoom(room2);
    building.AddTransition(trans12);
    building.AddTransition(trans23);
    building.AddCrossing(cross12);
    building.AddCrossing(cross23);
    REQUIRE(building.InitGeometry());
    REQUIRE(building.GetAllRooms().size() == 2);
    REQUIRE(building.GetAllTransitions().size() == 2);
    REQUIRE(building.GetAllCrossings().size() == 2);

    SECTION("ped inside")
    {
        SECTION("transition passed")
        {
            Pedestrian ped;
            ped.SetRoomID(1, "");
            ped.SetSubRoomID(3);
            ped.SetPos({1.95, -1.2});
            ped.UpdateRoom(2, 1);
            ped.SetPos({2.05, -1.2});
            auto passedTrans = SimulationHelper::FindPassedDoor(building, ped);
            REQUIRE(passedTrans.has_value());
            REQUIRE(passedTrans.value()->GetUniqueID() == trans12->GetUniqueID());
        }

        SECTION("step ends on transition")
        {
            Pedestrian ped;
            ped.SetRoomID(1, "");
            ped.SetSubRoomID(3);
            ped.SetPos({1.95, -1.2});
            ped.SetPos({2.00, -1.2});
            auto passedTrans = SimulationHelper::FindPassedDoor(building, ped);
            REQUIRE_FALSE(passedTrans.has_value());
        }

        SECTION("step starts on transition")
        {
            Pedestrian ped;
            ped.SetRoomID(1, "");
            ped.SetSubRoomID(3);
            ped.SetPos({2.00, -1.2});
            ped.SetPos({2.05, -1.2});
            auto passedTrans = SimulationHelper::FindPassedDoor(building, ped);
            REQUIRE(passedTrans.has_value());
            REQUIRE(passedTrans.value()->GetUniqueID() == trans12->GetUniqueID());
        }

        SECTION("step on transition")
        {
            Pedestrian ped;
            ped.SetRoomID(1, "");
            ped.SetSubRoomID(3);
            ped.SetPos({2.00, -1.2});
            ped.SetPos({2.00, -1.15});
            auto passedTrans = SimulationHelper::FindPassedDoor(building, ped);
            REQUIRE_FALSE(passedTrans.has_value());
        }


        SECTION("crossing passed")
        {
            Pedestrian ped;
            ped.SetRoomID(1, "");
            ped.SetSubRoomID(3);
            ped.SetPos({-1.95, -1.2});
            ped.UpdateRoom(1, 2);
            ped.SetPos({-2.05, -1.2});
            auto passedTrans = SimulationHelper::FindPassedDoor(building, ped);
            REQUIRE_FALSE(passedTrans.has_value());
        }

        SECTION("ped still in same subroom")
        {
            Pedestrian ped;
            ped.SetRoomID(1, "");
            ped.SetSubRoomID(3);
            ped.SetPos({-1.95, -1.2});
            ped.SetPos({-1.90, -1.2});
            auto passedTrans = SimulationHelper::FindPassedDoor(building, ped);
            REQUIRE_FALSE(passedTrans.has_value());
        }
    }

    SECTION("ped outside")
    {
        SECTION("transition passed")
        {
            Pedestrian ped;
            ped.SetRoomID(2, "");
            ped.SetSubRoomID(1);
            ped.SetPos({5.95, -1.2});
            ped.UpdateRoom(-1, -1);
            ped.SetPos({6.05, -1.2});
            auto passedTrans = SimulationHelper::FindPassedDoor(building, ped);
            REQUIRE(passedTrans.has_value());
            REQUIRE(passedTrans.value()->GetUniqueID() == trans23->GetUniqueID());
        }

        SECTION("step starts on transition")
        {
            Pedestrian ped;
            ped.SetRoomID(2, "");
            ped.SetSubRoomID(1);
            ped.SetPos({6.00, -1.2});
            ped.UpdateRoom(-1, -1);
            ped.SetPos({6.05, -1.2});
            auto passedTrans = SimulationHelper::FindPassedDoor(building, ped);
            REQUIRE(passedTrans.has_value());
            REQUIRE(passedTrans.value()->GetUniqueID() == trans23->GetUniqueID());
        }
    }
}

TEST_CASE("SimulationHelper::RemovePedestrians", "[SimulationHelper][RemovePedestrians]")
{
    // Create following geometry:
    // -| = Walls
    // +  = Transition
    //
    // (-5, 2) |----------| (5, 2)
    //         |          +
    //         |          +
    //         |          +
    // (-5,-2) |----------| (5, -2)

    Wall wall111;
    wall111.SetPoint1(Point{5., -2.});
    wall111.SetPoint2(Point{-5., -2.});
    Wall wall112;
    wall112.SetPoint1(Point{-5., -2.});
    wall112.SetPoint2(Point{-5., 2.});
    Wall wall113;
    wall113.SetPoint1(Point{-5., 2.});
    wall113.SetPoint2(Point{5., 2.});
    auto trans11 = new Transition();
    trans11->SetID(1);
    trans11->SetPoint1(Point{5., -2.});
    trans11->SetPoint2(Point{5., 2.});

    auto sub11 = new NormalSubRoom();
    sub11->SetRoomID(1);
    sub11->SetSubRoomID(1);
    sub11->AddTransition(trans11);
    sub11->AddWall(wall111);
    sub11->AddWall(wall112);
    sub11->AddWall(wall113);
    sub11->ConvertLineToPoly(std::vector<Line *>{trans11});
    sub11->CreateBoostPoly();

    // Create room
    auto room1 = new Room();
    room1->SetID(1);
    room1->AddSubRoom(sub11);

    Building building;
    building.AddRoom(room1);
    building.AddTransition(trans11);

    REQUIRE(building.InitGeometry());
    REQUIRE(building.GetAllRooms().size() == 1);
    REQUIRE(building.GetAllTransitions().size() == 1);

    std::vector<Pedestrian *> pedsToRemove;

    SECTION("Remove all pedestrians")
    {
        for(unsigned int i = 0; i < 10; ++i) {
            auto ped = new Pedestrian();
            building.AddPedestrian(ped);
            pedsToRemove.emplace_back(ped);
        }
        REQUIRE(building.GetAllPedestrians().size() == 10);
        REQUIRE(pedsToRemove.size() == 10);
        SimulationHelper::RemovePedestrians(building, pedsToRemove);
        REQUIRE(building.GetAllPedestrians().empty());
        REQUIRE(pedsToRemove.empty());
    }

    SECTION("Remove no pedestrians")
    {
        for(unsigned int i = 0; i < 10; ++i) {
            auto ped = new Pedestrian();
            building.AddPedestrian(ped);
        }
        REQUIRE(building.GetAllPedestrians().size() == 10);
        REQUIRE(pedsToRemove.empty());
        SimulationHelper::RemovePedestrians(building, pedsToRemove);
        REQUIRE(building.GetAllPedestrians().size() == 10);
        REQUIRE(pedsToRemove.empty());
    }

    SECTION("Remove 5 of 10 pedestrians")
    {
        std::vector<Pedestrian *> pedsRemaining;

        for(unsigned int i = 0; i < 10; ++i) {
            auto ped = new Pedestrian();
            building.AddPedestrian(ped);
            if(i < 5) {
                pedsToRemove.emplace_back(ped);
            } else {
                pedsRemaining.emplace_back(ped);
            }
        }
        REQUIRE(building.GetAllPedestrians().size() == 10);
        REQUIRE(pedsToRemove.size() == 5);
        REQUIRE(pedsRemaining.size() == 5);
        SimulationHelper::RemovePedestrians(building, pedsToRemove);
        REQUIRE(building.GetAllPedestrians().size() == 5);
        REQUIRE(pedsToRemove.empty());
        REQUIRE_THAT(building.GetAllPedestrians(), Catch::Matchers::UnorderedEquals(pedsRemaining));
    }
}
