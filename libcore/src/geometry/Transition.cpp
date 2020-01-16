/**
 * \file        Transition.cpp
 * \date        Nov 16, 2010
 * \version     v0.7
 * \copyright   <2009-2015> Forschungszentrum Jülich GmbH. All rights reserved.
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
 *
 * \section Description
 *
 *
 **/
#include "Transition.h"

#include "Room.h"
#include "SubRoom.h"
#include "general/Logger.h"

Transition::Transition() : Crossing()
{
    _room2 = nullptr;
}

Transition::~Transition() {}


void Transition::SetType(std::string type)
{
    _type = type;
}

void Transition::SetRoom2(Room * r)
{
    _room2 = r;
}

Room * Transition::GetRoom2() const
{
    return _room2;
}

std::string Transition::GetType() const
{
    return _type;
}
// Sonstiges

// gibt den ANDEREN room != roomID zurück
Room * Transition::GetOtherRoom(int roomID) const
{
    if(GetRoom1() != nullptr && GetRoom1()->GetID() == roomID) {
        return GetRoom2();
    } else if(GetRoom2() != nullptr && GetRoom2()->GetID() == roomID) {
        return GetRoom1();
    } else {
        LOG_ERROR("Transition::GetOtherRoom() wrong roomID [{}]", roomID);

        exit(0);
    }
}
// virtuelle Funktionen

// prüft ob Ausgang nach draußen
bool Transition::IsExit() const
{
    if(GetRoom1() != nullptr && _room2 != nullptr)
        return false;
    else
        return true;
}
// prüft, ob Transition in Raum mit roomID
bool Transition::IsInRoom(int roomID) const
{
    bool c1 = false;
    bool c2 = false;
    if(GetRoom1() != nullptr && GetRoom1()->GetID() == roomID)
        c1 = true;
    if(GetRoom2() != nullptr && GetRoom2()->GetID() == roomID)
        c2 = true;
    return c1 || c2;
}

bool Transition::IsTransition() const
{
    return true;
}


/* gibt den ANDEREN Subroom mit GetRoomID() != roomID zurück
 * subroomID wird hier nicht benötigt, aber in Crossings::GetOtherSubRoom()
 * (virtuelle Funktion) */
SubRoom * Transition::GetOtherSubRoom(int roomID, int subroomID) const
{
    if((GetRoom1() != nullptr) && (GetRoom1()->GetID() == roomID))
        return GetSubRoom2();
    else if((GetRoom2() != nullptr) && (GetRoom2()->GetID() == roomID))
        return GetSubRoom1();
    else {
        LOG_ERROR(
            "Transition::GetOtherSubRoom No exit found on the other side, ID={}, "
            "roomID={}, subroomID={}.",
            GetUniqueID(),
            roomID,
            subroomID);
        exit(EXIT_FAILURE);
    }
}


// TraVisTo Ausgabe
std::string Transition::GetDescription() const
{
    std::string geometry;
    char tmp[1024] = "";

    sprintf(
        tmp,
        "\t\t<door ID=\"%d\" color=\"180\" caption=\"%d_%d_%s\">\n",
        GetUniqueID(),
        GetID(),
        GetUniqueID(),
        GetCaption().c_str());
    geometry.append(tmp);
    sprintf(
        tmp,
        "\t\t\t<point xPos=\"%.2f\" yPos=\"%.2f\" zPos=\"%.2f\"/>\n",
        (GetPoint1()._x),
        (GetPoint1()._y),
        GetSubRoom1()->GetElevation(GetPoint1()));
    geometry.append(tmp);
    sprintf(
        tmp,
        "\t\t\t<point xPos=\"%.2f\" yPos=\"%.2f\" zPos=\"%.2f\"/>\n",
        (GetPoint2()._x),
        (GetPoint2()._y),
        GetSubRoom1()->GetElevation(GetPoint2()));
    geometry.append(tmp);
    geometry.append("\t\t</door>\n");
    return geometry;
}
