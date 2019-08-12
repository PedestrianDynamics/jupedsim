/**
 * \file        Hline.cpp
 * \date        Aug 1, 2012
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
#include "Hline.h"

#include "Room.h"
#include "SubRoom.h"


Hline::Hline()
{
     _id=-1;
}

Hline::~Hline()
{
}

void Hline::SetID(int ID)
{
     _id=ID;
}

void Hline::SetRoom1(Room* r)
{
     _room1=r;
}

void Hline::SetCaption(std::string s)
{
     _caption=s;
}

void Hline::SetSubRoom1(SubRoom* s)
{
     _subRoom1=s;
}

int Hline::GetID() const
{
     return _id;
}

std::string Hline::GetCaption() const
{
     return _caption;
}

Room* Hline::GetRoom1() const
{
     return _room1;
}

SubRoom* Hline::GetSubRoom1() const
{
     return _subRoom1;
}

void Hline::SetSubRoom2(SubRoom* r2)
{
     _subRoom2 = r2;
}


SubRoom* Hline::GetSubRoom2() const
{
     return _subRoom2;
}

bool Hline::IsInSubRoom(int subroomID) const
{
     return _subRoom1->GetSubRoomID() == subroomID;
}

bool Hline::IsInRoom(int roomID) const
{
     return _room1->GetID() == roomID;
}

void Hline::WriteToErrorLog() const
{
     std::string s;
     char tmp[CLENGTH];
     sprintf(tmp, "\t\tHline: %d (%f, %f) -- (%f, %f)\n", GetID(), GetPoint1()._x,
             GetPoint1()._y, GetPoint2()._x, GetPoint2()._y);
     s.append(tmp);
     sprintf(tmp, "\t\t\t\tRoom: %d <-> SubRoom: %d\n", _room1->GetID(),
             _subRoom1->GetSubRoomID());
     s.append(tmp);
     Log->Write(s);
}

// TraVisTo Ausgabe
std::string Hline::GetDescription() const
{
     std::string geometry;
     char tmp[CLENGTH] = "";
     sprintf(tmp,"\t\t<hline ID=\"%d\" room_id=\"%d\" subroom_id=\"%d\" color = \"250\" caption=\"h_%d_%d\">\n",GetUniqueID(),_room1->GetID(),_subRoom1->GetSubRoomID(),GetID(),GetUniqueID());
     geometry.append(tmp);
     //geometry.append("\t\t<door color=\"250\">\n");
     sprintf(tmp, "\t\t\t<point xPos=\"%.2f\" yPos=\"%.2f\" zPos=\"%.2f\"/>\n",
             (GetPoint1()._x) * FAKTOR,
             (GetPoint1()._y) * FAKTOR,
             _subRoom1->GetElevation(GetPoint1())*FAKTOR);
     geometry.append(tmp);
     sprintf(tmp, "\t\t\t<point xPos=\"%.2f\" yPos=\"%.2f\" zPos=\"%.2f\"/>\n",
             (GetPoint2()._x) * FAKTOR,
             (GetPoint2()._y) * FAKTOR,
             _subRoom1->GetElevation(GetPoint2())*FAKTOR);
     geometry.append(tmp);
     geometry.append("\t\t</hline>\n");
     return geometry;
}
