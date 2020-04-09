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

#include "pedestrian/Pedestrian.h"

#include <catch2/catch.hpp>
#include <cmath>
#include <cstdlib>

TEST_CASE("Pedestrian::UpdateRoom", "[Pedestrian][UpdateRoom]")
{
    SECTION("Update room")
    {
        Pedestrian ped;
        int oldRoomID    = 1;
        int oldSubRoomID = 1;
        ped.SetRoomID(oldRoomID, "");
        ped.SetSubRoomID(oldSubRoomID);

        int newRoomID    = oldRoomID + 5;
        int newSubRoomID = oldSubRoomID + 5;

        ped.UpdateRoom(newRoomID, newSubRoomID);
        REQUIRE(ped.GetOldRoomID() == oldRoomID);
        REQUIRE(ped.GetOldSubRoomID() == oldSubRoomID);
        REQUIRE(ped.GetRoomID() == newRoomID);
        REQUIRE(ped.GetSubRoomID() == newSubRoomID);
    }
}

TEST_CASE("Pedestrian::ChangedRoom", "[Pedestrian][ChangedRoom]")
{
    SECTION("Same room and subroom")
    {
        Pedestrian ped;
        ped.SetRoomID(1, "");
        ped.SetSubRoomID(1);

        int oldRoomID    = ped.GetRoomID();
        int oldSubRoomID = ped.GetSubRoomID();
        int newRoomID    = oldRoomID;
        int newSubRoomID = oldSubRoomID;
        ped.UpdateRoom(newRoomID, newSubRoomID);
        REQUIRE(ped.GetOldRoomID() == oldRoomID);
        REQUIRE(ped.GetOldSubRoomID() == oldSubRoomID);
        REQUIRE(ped.GetRoomID() == newRoomID);
        REQUIRE(ped.GetSubRoomID() == newSubRoomID);
        REQUIRE_FALSE(ped.ChangedRoom());
    }

    SECTION("Same room and changed subroom")
    {
        Pedestrian ped;
        ped.SetRoomID(1, "");
        ped.SetSubRoomID(1);

        int oldRoomID    = ped.GetRoomID();
        int oldSubRoomID = ped.GetSubRoomID();
        int newRoomID    = oldRoomID;
        int newSubRoomID = oldSubRoomID + 1;
        ped.UpdateRoom(newRoomID, newSubRoomID);
        REQUIRE(ped.GetOldRoomID() == oldRoomID);
        REQUIRE(ped.GetOldSubRoomID() == oldSubRoomID);
        REQUIRE(ped.GetRoomID() == newRoomID);
        REQUIRE(ped.GetSubRoomID() == newSubRoomID);
        REQUIRE_FALSE(ped.ChangedRoom());
    }

    SECTION("Changed room and same subroom")
    {
        Pedestrian ped;
        ped.SetRoomID(1, "");
        ped.SetSubRoomID(1);

        int oldRoomID    = ped.GetRoomID();
        int oldSubRoomID = ped.GetSubRoomID();
        int newRoomID    = oldRoomID + 1;
        int newSubRoomID = oldSubRoomID;
        ped.UpdateRoom(newRoomID, newSubRoomID);
        REQUIRE(ped.GetOldRoomID() == oldRoomID);
        REQUIRE(ped.GetOldSubRoomID() == oldSubRoomID);
        REQUIRE(ped.GetRoomID() == newRoomID);
        REQUIRE(ped.GetSubRoomID() == newSubRoomID);
        REQUIRE(ped.ChangedRoom());
    }

    SECTION("Changed room and changed subroom")
    {
        Pedestrian ped;
        ped.SetRoomID(1, "");
        ped.SetSubRoomID(1);

        int oldRoomID    = ped.GetRoomID();
        int oldSubRoomID = ped.GetSubRoomID();
        int newRoomID    = oldRoomID + 1;
        int newSubRoomID = oldSubRoomID + 1;
        ped.UpdateRoom(newRoomID, newSubRoomID);
        REQUIRE(ped.GetOldRoomID() == oldRoomID);
        REQUIRE(ped.GetOldSubRoomID() == oldSubRoomID);
        REQUIRE(ped.GetRoomID() == newRoomID);
        REQUIRE(ped.GetSubRoomID() == newSubRoomID);
        REQUIRE(ped.ChangedRoom());
    }

    SECTION("Old RoomID not set yet")
    {
        Pedestrian ped;
        ped.SetRoomID(1, "");
        ped.SetSubRoomID(1);
        REQUIRE_FALSE(ped.ChangedRoom());
    }
}

TEST_CASE("Pedestrian::ChangedSubRoom", "[Pedestrian][ChangedSubRoom]")
{
    SECTION("Same room and subroom")
    {
        Pedestrian ped;
        ped.SetRoomID(1, "");
        ped.SetSubRoomID(1);

        int oldRoomID    = ped.GetRoomID();
        int oldSubRoomID = ped.GetSubRoomID();
        int newRoomID    = oldRoomID;
        int newSubRoomID = oldSubRoomID;
        ped.UpdateRoom(newRoomID, newSubRoomID);
        REQUIRE(ped.GetOldRoomID() == oldRoomID);
        REQUIRE(ped.GetOldSubRoomID() == oldSubRoomID);
        REQUIRE(ped.GetRoomID() == newRoomID);
        REQUIRE(ped.GetSubRoomID() == newSubRoomID);
        REQUIRE_FALSE(ped.ChangedSubRoom());
    }

    SECTION("Same room and changed subroom")
    {
        Pedestrian ped;
        ped.SetRoomID(1, "");
        ped.SetSubRoomID(1);

        int oldRoomID    = ped.GetRoomID();
        int oldSubRoomID = ped.GetSubRoomID();
        int newRoomID    = oldRoomID;
        int newSubRoomID = oldSubRoomID + 1;
        ped.UpdateRoom(newRoomID, newSubRoomID);
        REQUIRE(ped.GetOldRoomID() == oldRoomID);
        REQUIRE(ped.GetOldSubRoomID() == oldSubRoomID);
        REQUIRE(ped.GetRoomID() == newRoomID);
        REQUIRE(ped.GetSubRoomID() == newSubRoomID);
        REQUIRE(ped.ChangedSubRoom());
    }

    SECTION("Changed room and same subroom")
    {
        Pedestrian ped;
        ped.SetRoomID(1, "");
        ped.SetSubRoomID(1);

        int oldRoomID    = ped.GetRoomID();
        int oldSubRoomID = ped.GetSubRoomID();
        int newRoomID    = oldRoomID + 1;
        int newSubRoomID = oldSubRoomID;
        ped.UpdateRoom(newRoomID, newSubRoomID);
        REQUIRE(ped.GetOldRoomID() == oldRoomID);
        REQUIRE(ped.GetOldSubRoomID() == oldSubRoomID);
        REQUIRE(ped.GetRoomID() == newRoomID);
        REQUIRE(ped.GetSubRoomID() == newSubRoomID);
        REQUIRE(ped.ChangedSubRoom());
    }

    SECTION("Changed room and changed subroom")
    {
        Pedestrian ped;
        ped.SetRoomID(1, "");
        ped.SetSubRoomID(1);

        int oldRoomID    = ped.GetRoomID();
        int oldSubRoomID = ped.GetSubRoomID();
        int newRoomID    = oldRoomID + 1;
        int newSubRoomID = oldSubRoomID + 1;
        ped.UpdateRoom(newRoomID, newSubRoomID);
        REQUIRE(ped.GetOldRoomID() == oldRoomID);
        REQUIRE(ped.GetOldSubRoomID() == oldSubRoomID);
        REQUIRE(ped.GetRoomID() == newRoomID);
        REQUIRE(ped.GetSubRoomID() == newSubRoomID);
        REQUIRE(ped.ChangedSubRoom());
    }

    SECTION("Old RoomID not set yet")
    {
        Pedestrian ped;
        ped.SetRoomID(1, "");
        ped.SetSubRoomID(1);
        REQUIRE_FALSE(ped.ChangedSubRoom());
    }

    SECTION("Old SubRoomID not set yet")
    {
        Pedestrian ped;
        ped.SetRoomID(1, "");
        ped.SetSubRoomID(std::numeric_limits<int>::min());
        ped.UpdateRoom(1, 1);
        REQUIRE_FALSE(ped.ChangedSubRoom());
    }
}

TEST_CASE("Pedestrian::SetPos", "[Pedestrian][SetPos]")
{
    Pedestrian ped;
    ped.Setdt(0.1);

    SECTION("global timer larger premovement time and not initial")
    {
        Pedestrian::SetGlobalTime(10.);
        ped.SetPremovementTime(Pedestrian::GetGlobalTime() - 1.);
        ped.SetRecordingTime(10.);

        Point p{-1, 5};
        ped.SetPos(p, true);
        REQUIRE(ped.GetLastPositions().size() == 1);
        REQUIRE(ped.GetLastPositions().back() == p);
    }

    SECTION("global timer larger premovement time and initial")
    {
        Pedestrian::SetGlobalTime(10.);
        ped.SetPremovementTime(Pedestrian::GetGlobalTime() - 1.);
        ped.SetRecordingTime(10.);

        Point p{-1, 5};
        ped.SetPos(p, true);
        REQUIRE(ped.GetLastPositions().size() == 1);
        REQUIRE(ped.GetLastPositions().back() == p);
    }

    SECTION("global timer smaller premovement time and not initial")
    {
        Pedestrian::SetGlobalTime(10.);
        ped.SetPremovementTime(Pedestrian::GetGlobalTime() + 1.);
        ped.SetRecordingTime(10.);

        Point p{-1, 5};
        ped.SetPos(p, false);
        REQUIRE(ped.GetLastPositions().empty());
    }

    SECTION("global timer smaller premovement time and initial")
    {
        Pedestrian::SetGlobalTime(10.);
        ped.SetPremovementTime(Pedestrian::GetGlobalTime() + 1.);
        ped.SetRecordingTime(10.);

        Point p{-1, 5};
        ped.SetPos(p, true);
        REQUIRE(ped.GetLastPositions().size() == 1);
        REQUIRE(ped.GetLastPositions().back() == p);
    }

    SECTION("max size not exceeded")
    {
        Pedestrian::SetGlobalTime(10.);
        ped.SetPremovementTime(Pedestrian::GetGlobalTime() - 1.);
        ped.SetRecordingTime(10.);

        double min   = -10;
        double max   = 10;
        size_t steps = 5;
        double step  = std::abs(max - min) / steps;

        for(size_t i = 0; i < steps; ++i) {
            Point p{i * step, -1. * i * step};
            ped.SetPos(p, false);
            REQUIRE(ped.GetLastPositions().back() == p);
        }

        REQUIRE(ped.GetLastPositions().size() == steps);
    }

    SECTION("max size reached")
    {
        Pedestrian::SetGlobalTime(10.);
        ped.SetPremovementTime(Pedestrian::GetGlobalTime() - 1.);
        ped.SetRecordingTime(10.);

        size_t steps = std::floor(ped.GetRecordingTime() / ped.Getdt());
        double min   = -10;
        double max   = 10;
        double step  = std::abs(max - min) / steps;

        for(size_t i = 0; i < steps; ++i) {
            Point p{i * step, -1. * i * step};
            ped.SetPos(p, false);
            REQUIRE(ped.GetLastPositions().back() == p);
        }

        REQUIRE(ped.GetLastPositions().size() == steps);
    }

    SECTION("max size exceeded")
    {
        Pedestrian::SetGlobalTime(10.);
        ped.SetPremovementTime(Pedestrian::GetGlobalTime() - 1.);
        ped.SetRecordingTime(10.);

        size_t maxSize = std::floor(ped.GetRecordingTime() / ped.Getdt());
        int steps      = maxSize + 1;
        double min     = -10;
        double max     = 10;
        double step    = std::abs(max - min) / steps;

        for(int i = 0; i < steps; ++i) {
            Point p{i * step, -1. * i * step};
            ped.SetPos(p, false);
            REQUIRE(ped.GetLastPositions().back() == p);
        }

        REQUIRE(ped.GetLastPositions().size() == maxSize);
    }
}
