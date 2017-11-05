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
#include "../IO/OutputHandler.h"

using namespace std;

/************************************************************
 Transition (abgeleitet von Crossing)
 ************************************************************/

Transition::Transition() : Crossing()
{
     _isOpen = true;
     _doorUsage=0;
     _lastPassingTime=0;
     _room2 = nullptr;
//     _lastTickTime1 = 0;
//     _lastTickTime2 = 0;
//     _refresh1 = 0;
//     _refresh2 = 0;
}

Transition::~Transition()
{
}




void Transition::SetType(string type)
{
     _type=type;
}

void Transition::SetRoom2(Room* r)
{
     _room2 = r;
}

bool Transition::IsOpen() const
{
     return Crossing::IsOpen();
}


Room* Transition::GetRoom2() const
{
     return _room2;
}

string Transition::GetType() const
{
     return _type;
}
// Sonstiges

// gibt den ANDEREN room != roomID zurück
Room* Transition::GetOtherRoom(int roomID) const
{
     if (GetRoom1()!=nullptr && GetRoom1()->GetID() == roomID) {
          return GetRoom2();
     } else if (GetRoom2()!=nullptr && GetRoom2()->GetID() == roomID) {
          return GetRoom1();
     } else {
          char msg[CLENGTH];
          sprintf(msg,"ERROR: \tTransition::GetOtherRoom() wrong roomID [%d]",roomID);
          Log->Write(msg);
          exit(0);
     }

}
// virtuelle Funktionen

// prüft ob Ausgang nach draußen
bool Transition::IsExit() const
{
     if(GetRoom1()!=nullptr && _room2!=nullptr)
          return false;
     else
          return true;
}
// prüft, ob Transition in Raum mit roomID
bool Transition::IsInRoom(int roomID) const
{
     bool c1 = false;
     bool c2 = false;
     if (GetRoom1() != nullptr && GetRoom1()->GetID() == roomID)
          c1 = true;
     if (GetRoom2() != nullptr && GetRoom2()->GetID() == roomID)
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
SubRoom* Transition::GetOtherSubRoom(int roomID, int subroomID) const
{
     if ((GetRoom1() != nullptr) && (GetRoom1()->GetID() == roomID))
          return GetSubRoom2();
     else if ((GetRoom2() != nullptr) && (GetRoom2()->GetID() == roomID))
          return GetSubRoom1();
     else {
          Log->Write("ERROR: \tTransition::GetOtherSubRoom No exit found "
                     "on the other side\n ID=%d, roomID=%d, subroomID=%d\n",GetUniqueID(),roomID,subroomID);
          exit(EXIT_FAILURE);
     }
}

// Ein-Ausgbae

void Transition::WriteToErrorLog() const
{
     string s;
     char tmp[CLENGTH];
     sprintf(tmp, "\t\tTRANS: %d [%s] (%f, %f) -- (%f, %f)\n", GetID(), GetCaption().c_str(),
             GetPoint1()._x, GetPoint1()._y, GetPoint2()._x, GetPoint2()._y);
     s.append(tmp);
     // erster Raum
     if (GetRoom1() != nullptr) {
          sprintf(tmp, "\t\t\t\tRoom: %d [%s] SubRoom: %d", GetRoom1()->GetID(),
                  GetRoom1()->GetCaption().c_str(), GetSubRoom1()->GetSubRoomID());
     } else {
          sprintf(tmp, "\t\t\t\tAusgang");
     }
     s.append(tmp);
     // zweiter Raum
     if (GetRoom2() != nullptr) {
          sprintf(tmp, " <->\tRoom: %d [%s] SubRoom: %d\n", GetRoom2()->GetID(),
                  GetRoom2()->GetCaption().c_str(), GetSubRoom2()->GetSubRoomID());
     } else {
          sprintf(tmp, " <->\tAusgang\n");
     }
     s.append(tmp);
     Log->Write(s);
}

// TraVisTo Ausgabe
string Transition::GetDescription() const
{
     string geometry;
     char tmp[CLENGTH] = "";

     sprintf(tmp,"\t\t<door ID=\"%d\" color=\"180\" caption=\"%d_%d_%s\">\n",GetUniqueID(),GetID(),GetUniqueID(),GetCaption().c_str());
     geometry.append(tmp);
     sprintf(tmp, "\t\t\t<point xPos=\"%.2f\" yPos=\"%.2f\" zPos=\"%.2f\"/>\n",
             (GetPoint1()._x) * FAKTOR,
             (GetPoint1()._y) * FAKTOR,
             GetSubRoom1()->GetElevation(GetPoint1())*FAKTOR);
     geometry.append(tmp);
     sprintf(tmp, "\t\t\t<point xPos=\"%.2f\" yPos=\"%.2f\" zPos=\"%.2f\"/>\n",
             (GetPoint2()._x) * FAKTOR,
             (GetPoint2()._y) * FAKTOR,
             GetSubRoom1()->GetElevation(GetPoint2())*FAKTOR);
     geometry.append(tmp);
     geometry.append("\t\t</door>\n");
     return geometry;
}

void Transition::IncreaseDoorUsage(int number, double time)
{
     _doorUsage+=number;
     _lastPassingTime=time;
     _flowAtExit+=to_string(time)+"  "+to_string(_doorUsage)+"\n";
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
