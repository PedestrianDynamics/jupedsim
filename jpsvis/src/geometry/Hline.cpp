/**
 * Hline.cpp
 *
 *  Created on: Aug 1, 2012
 *
 * @section LICENSE
 * This file is part of JuPedSim.
 *
 * JuPedSim is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * JuPedSim is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with JuPedSim. If not, see <http://www.gnu.org/licenses/>.
 *
 * @section DESCRIPTION
 *
 *
 *
 */


#include "Hline.h"

using namespace std;

Hline::Hline()
{
    _room=NULL;
    _subRoom=NULL;
    _id=-1;
}

Hline::~Hline()
{
}

void Hline::SetID(int ID)
{
    _id=ID;
}

void Hline::SetRoom(Room* r)
{
    _room=r;
}

void Hline::SetCaption(string s)
{
    _caption=s;
}

void Hline::SetSubRoom(SubRoom* s)
{
    _subRoom=s;
}

int Hline::GetID() const
{
    return _id;
}

string Hline::GetCaption() const
{
    return _caption;
}

Room* Hline::GetRoom() const
{
    return _room;
}

SubRoom* Hline::GetSubRoom() const
{
    return _subRoom;
}

bool Hline::IsInSubRoom(int subroomID) const
{
    return _subRoom->GetSubRoomID() == subroomID;
}

bool Hline::IsInRoom(int roomID) const
{
    return _room->GetID() == roomID;
}

void Hline::WriteToErrorLog() const
{
    string s;
    char tmp[CLENGTH];
    sprintf(tmp, "\t\tHline: %d (%f, %f) -- (%f, %f)\n", GetID(), GetPoint1().GetX(),
            GetPoint1().GetY(), GetPoint2().GetX(), GetPoint2().GetY());
    s.append(tmp);
    sprintf(tmp, "\t\t\t\tRoom: %d <-> SubRoom: %d\n", _room->GetID(),
            _subRoom->GetSubRoomID());
    s.append(tmp);
    Log->Write(s);
}

// TraVisTo Ausgabe

string Hline::WriteElement() const
{
    string geometry;
    char tmp[CLENGTH] = "";
    sprintf(tmp,"\t\t<door ID=\"%d\" color = \"250\" caption=\"%d_%d\">\n",GetUniqueID(),GetID(),GetUniqueID());
    geometry.append(tmp);
    //geometry.append("\t\t<door color=\"250\">\n");
    sprintf(tmp, "\t\t\t<point xPos=\"%.2f\" yPos=\"%.2f\" zPos=\"%.2f\"/>\n",
            (GetPoint1().GetX()) * FAKTOR,
            (GetPoint1().GetY()) * FAKTOR,
            _subRoom->GetElevation(GetPoint1())*FAKTOR);
    geometry.append(tmp);
    sprintf(tmp, "\t\t\t<point xPos=\"%.2f\" yPos=\"%.2f\" zPos=\"%.2f\"/>\n",
            (GetPoint2().GetX()) * FAKTOR,
            (GetPoint2().GetY()) * FAKTOR,
            _subRoom->GetElevation(GetPoint2())*FAKTOR);
    geometry.append(tmp);
    geometry.append("\t\t</door>\n");
    return geometry;
}


