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

#include <Logger.h>

using namespace std;

/************************************************************
 Transition (abgeleitet von Crossing)
 ************************************************************/

Transition::Transition() : Crossing()
{
    _isOpen          = true;
    _doorUsage       = 0;
    _lastPassingTime = 0;
    _room2           = nullptr;
}

Transition::~Transition() {}


void Transition::Close()
{
    _isOpen = false;
}

void Transition::Open()
{
    _isOpen = true;
}

void Transition::SetType(string type)
{
    _type = type;
}

void Transition::SetRoom2(Room * r)
{
    _room2 = r;
}

bool Transition::IsOpen() const
{
    return _isOpen;
}


Room * Transition::GetRoom2() const
{
    return _room2;
}

string Transition::GetType() const
{
    return _type;
}
// Sonstiges

// gibt den ANDEREN room != roomID zurück
Room * Transition::GetOtherRoom(int roomID) const
{
    if(GetRoom1() != NULL && GetRoom1()->GetID() == roomID) {
        return GetRoom2();
    } else if(GetRoom2() != NULL && GetRoom2()->GetID() == roomID) {
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
    if(GetRoom1() != NULL && _room2 != NULL)
        return false;
    else
        return true;
}
// prüft, ob Transition in Raum mit roomID
bool Transition::IsInRoom(int roomID) const
{
    bool c1 = false;
    bool c2 = false;
    if(GetRoom1() != NULL && GetRoom1()->GetID() == roomID)
        c1 = true;
    if(GetRoom2() != NULL && GetRoom2()->GetID() == roomID)
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
    if((GetRoom1() != NULL) && (GetRoom1()->GetID() == roomID))
        return GetSubRoom2();
    else if((GetRoom2() != NULL) && (GetRoom2()->GetID() == roomID))
        return GetSubRoom1();
    else {
        LOG_ERROR(
            "Transition::GetOtherSubRoom No exit found "
            "on the other side\n ID={}, roomID={}, subroomID={}\n",
            GetUniqueID(),
            roomID,
            subroomID);
        exit(EXIT_FAILURE);
    }
}

// Ein-Ausgbae

void Transition::WriteToErrorLog() const
{
    LOG_ERROR(
        "\t\tTRANS: {} [{}] ({:.2f}, {:.2f}) -- ({:.2f}, {:.2f})\n",
        GetID(),
        GetCaption(),
        GetPoint1().GetX(),
        GetPoint1().GetY(),
        GetPoint2().GetX(),
        GetPoint2().GetY());
    // erster Raum
    if(GetRoom1() != NULL) {
        LOG_ERROR(
            "\t\t\t\tRoom: {} [{}] SubRoom: {}",
            GetRoom1()->GetID(),
            GetRoom1()->GetCaption(),
            GetSubRoom1()->GetSubRoomID());
    } else {
        LOG_ERROR("\t\t\t\tAusgang");
    }

    // zweiter Raum
    if(GetRoom2() != NULL) {
        LOG_ERROR(
            " <->\tRoom: {} [{}] SubRoom: {}\n",
            GetRoom2()->GetID(),
            GetRoom2()->GetCaption().c_str(),
            GetSubRoom2()->GetSubRoomID());
    } else {
        LOG_ERROR("\t\t\t\tAusgang");
    }
}

// TraVisTo Ausgabe
string Transition::GetDescription() const
{
    string geometry;
    char tmp[CLENGTH] = "";

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
        (GetPoint1().GetX()) * FAKTOR,
        (GetPoint1().GetY()) * FAKTOR,
        GetSubRoom1()->GetElevation(GetPoint1()) * FAKTOR);
    geometry.append(tmp);
    sprintf(
        tmp,
        "\t\t\t<point xPos=\"%.2f\" yPos=\"%.2f\" zPos=\"%.2f\"/>\n",
        (GetPoint2().GetX()) * FAKTOR,
        (GetPoint2().GetY()) * FAKTOR,
        GetSubRoom1()->GetElevation(GetPoint2()) * FAKTOR);
    geometry.append(tmp);
    geometry.append("\t\t</door>\n");
    return geometry;
}

void Transition::IncreaseDoorUsage(int number, double time)
{
    _doorUsage += number;
    _lastPassingTime = time;
    _flowAtExit += to_string(time) + "  " + to_string(_doorUsage) + "\n";
}

int Transition::GetDoorUsage() const
{
    return _doorUsage;
}

double Transition::GetLastPassingTime() const
{
    return _lastPassingTime;
}

const std::string & Transition::GetFlowCurve() const
{
    return _flowAtExit;
}
