/**
 * \file        Crossing.cpp
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


#include "Crossing.h"

#include "Room.h"
#include "SubRoom.h"

#include <Logger.h>

using namespace std;


Crossing::Crossing()
{
    _id = -1;
}

Crossing::~Crossing() {}

void Crossing::SetID(int ID)
{
    _id = ID;
}

//void Crossing::SetSubRoom2(SubRoom* r2)
//{
//     _subRoom2 = r2;
//}

// Getter-Funktionen

int Crossing::GetID() const
{
    return _id;
}

//SubRoom* Crossing::GetSubRoom2() const
//{
//     return _subRoom2;
//}
// Sonstiges


bool Crossing::IsExit() const
{
    return false;
}


bool Crossing::IsOpen() const
{
    return true;
}

bool Crossing::IsTransition() const
{
    return false;
}

bool Crossing::IsInSubRoom(int subroomID) const
{
    bool r1, r2;
    if(_subRoom1 != NULL)
        r1 = _subRoom1->GetSubRoomID() == subroomID;
    else
        r1 = false;
    if(_subRoom2 != NULL)
        r2 = _subRoom2->GetSubRoomID() == subroomID;
    else
        r2 = false;
    return (r1 || r2);
}

/* gibt den ANDEREN Subroom != subroomID zurück
 * roomID wird hier nicht benötigt, aber in Transition::GetOtherSubRoom()
 * (virtuelle Funktion) */
SubRoom * Crossing::GetOtherSubRoom(int roomID, int subroomID) const
{
    if(_subRoom1->GetSubRoomID() == subroomID)
        return _subRoom2;
    else if(_subRoom2->GetSubRoomID() == subroomID)
        return _subRoom1;
    else {
        LOG_WARNING(
            "Crossing::GetOtherSubRoom No exit found "
            "on the other side\n ID={}, roomID={}, subroomID={}\n",
            GetID(),
            roomID,
            subroomID);
        return NULL;
    }
}


// Ausgabe
void Crossing::WriteToErrorLog() const
{
    LOG_ERROR(
        "\t\tCROSS: {} ({:.2f}, {:.2f}) -- ({:.2f}, {:.2f})\n\t\t\t\tSubRoom: {} <-> SubRoom: {}\n",
        GetID(),
        GetPoint1().GetX(),
        GetPoint1().GetY(),
        GetPoint2().GetX(),
        GetPoint2().GetY(),
        GetSubRoom1()->GetSubRoomID(),
        GetSubRoom2()->GetSubRoomID());
}

// TraVisTo Ausgabe
string Crossing::GetDescription() const
{
    //return "";
    string geometry;
    char tmp[CLENGTH] = "";
    sprintf(
        tmp,
        "\t\t<crossing ID=\"%d\" color = \"250\" caption=\"%d_%d\">\n",
        GetUniqueID(),
        GetID(),
        GetUniqueID());
    geometry.append(tmp);
    //geometry.append("\t\t<door color=\"250\">\n");
    sprintf(
        tmp,
        "\t\t\t<point xPos=\"%.2f\" yPos=\"%.2f\" zPos=\"%.2f\" />\n",
        (GetPoint1().GetX()) * FAKTOR,
        (GetPoint1().GetY()) * FAKTOR,
        _subRoom1->GetElevation(GetPoint1()) * FAKTOR);
    geometry.append(tmp);
    sprintf(
        tmp,
        "\t\t\t<point xPos=\"%.2f\" yPos=\"%.2f\" zPos=\"%.2f\" />\n",
        (GetPoint2().GetX()) * FAKTOR,
        (GetPoint2().GetY()) * FAKTOR,
        _subRoom1->GetElevation(GetPoint2()) * FAKTOR);
    geometry.append(tmp);
    geometry.append("\t\t</crossing>\n");
    return geometry;
}
