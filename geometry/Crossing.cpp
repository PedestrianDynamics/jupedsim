/**
 * File:   Crossing.cpp
 *
 *
 * Created on 16. November 2010, 12:56
 *
 *  @section LICENSE
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
 *
 */


#include "Crossing.h"
#include "Room.h"
#include "SubRoom.h"


Crossing::Crossing(){
    pID = -1;
    pRoom1 = NULL;
    pSubRoom1 = NULL;
    pSubRoom2 = NULL;
	pCaption = "";

}

Crossing::~Crossing() {
}


void Crossing::SetIndex(int ID) {
    pID = ID;
}

void Crossing::SetRoom1(Room* r) {
    pRoom1 = r;
}

void Crossing::SetSubRoom1(SubRoom* r1) {
    pSubRoom1 = r1;
}

void Crossing::SetSubRoom2(SubRoom* r2) {
    pSubRoom2 = r2;
}

void Crossing::SetCaption(string s) {
	pCaption = s;
}
// Getter-Funktionen

int Crossing::GetIndex() const {
    return pID;
}
string Crossing::GetCaption() const {
	return pCaption;
}
Room* Crossing::GetRoom1() const {
    return pRoom1;
}

// gibt den SubRoom mit ID zurück

SubRoom* Crossing::GetSubRoom(int ID) const {
    if (pSubRoom1 != NULL && pSubRoom1->GetSubRoomID() == ID)
        return pSubRoom1;
    else if (pSubRoom2 != NULL && pSubRoom2->GetSubRoomID() == ID)
        return pSubRoom2;
    else {
        Log->write("ERROR: \tCrossing::GetSubRoom() wrong ID");
        exit(0);
    }
}

SubRoom* Crossing::GetSubRoom1() const {
    return pSubRoom1;
}

SubRoom* Crossing::GetSubRoom2() const {
    return pSubRoom2;
}
// Sonstiges

// prüft ob Ausgnag nach draußen, für Croosings IMMER false

bool Crossing::IsExit() const {
    return false;
}

// prüft ob Ausgnag nach draußen, für Croosings IMMER false

bool Crossing::IsOpen() const {
    return true;
}

bool Crossing::IsTransition() const {
	return false;
}

// Prüft, ob Crossing in Raum mit roomID

bool Crossing::IsInRoom(int roomID) const {
    return pRoom1->GetRoomID() == roomID;
}

// Prüft, ob Crossing in SubRoom mit subroomID

bool Crossing::IsInSubRoom(int subroomID) const {
    bool r1, r2;
    if (pSubRoom1 != NULL)
        r1 = pSubRoom1->GetSubRoomID() == subroomID;
    else
        r1 = false;
    if (pSubRoom2 != NULL)
        r2 = pSubRoom2->GetSubRoomID() == subroomID;
    else
        r2 = false;
    return (r1 || r2);
}

/* gibt den ANDEREN Subroom != subroomID zurück
 * roomID wird hier nicht benötigt, aber in Transition::GetOtherSubRoom()
 * (virtuelle Funktion) */
SubRoom* Crossing::GetOtherSubRoom(int roomID, int subroomID) const {
    if (pSubRoom1->GetSubRoomID() == subroomID)
        return pSubRoom2;
    else if (pSubRoom2->GetSubRoomID() == subroomID)
        return pSubRoom1;
    else {
    	 char tmp[CLENGTH];
    	    sprintf(tmp,"ERROR: \tCrossing::GetOtherSubRoom No exit found "
    	    		"on the other side\n ID=%hd, roomID=%hd, subroomID=%hd\n",GetIndex(),roomID,subroomID);
        Log->write(tmp);
        exit(0);
    }
}

/* Setzt Fußgänger von einem SubRoom in den zugehörigen nächsten SubRoom
 * Parameter:
 *    - RoomID: aktuelle RoomID des Fußgängers
 *    - SubID: aktuelle SubRoomID des Fußgängers
 *    - PedID: ID des Fußgängers
 *    - goal_new: neues Ziel des Fußgängers, kann Crossing oder Transition sein,
 *          wird in Routing bestimmt und hier gesetzt
 * */

void Crossing::UpdatePedestrian(int RoomID, int SubID, int PedID, Crossing* goal_new) const {
    SubRoom* sub = GetSubRoom(SubID);
    Pedestrian* ped = sub->GetPedestrian(PedID);
    SubRoom* other_sub = GetOtherSubRoom(RoomID, SubID);
    // Lösche Fußgänger aus aktuellem SubRoom
    sub->DeletePedestrian(PedID);
    // neues Ziel setzten
    ped->SetExitIndex(goal_new->GetIndex());
    ped->SetExitLine(goal_new);
    // Setzte Fußgänger in neuen SubRoom
    other_sub->AddPedestrian(ped);
}
// Ausgabe

void Crossing::WriteToErrorLog() const {
    string s;
    char tmp[CLENGTH];
    sprintf(tmp, "\t\tCROSS: %d (%f, %f) -- (%f, %f)\n", GetIndex(), GetPoint1().GetX(),
            GetPoint1().GetY(), GetPoint2().GetX(), GetPoint2().GetY());
    s.append(tmp);
    sprintf(tmp, "\t\t\t\tSubRoom: %d <-> SubRoom: %d\n", GetSubRoom1()->GetSubRoomID(),
            GetSubRoom2()->GetSubRoomID());
    s.append(tmp);
    Log->write(s);
}

// TraVisTo Ausgabe

string Crossing::WriteElement() const {
	//return "";
    string geometry;
    char tmp[CLENGTH] = "";
    sprintf(tmp,"\t\t<door ID=\"%d\" color = \"250\" caption=\"%d\">\n",pID,pID);
    geometry.append(tmp);
    //geometry.append("\t\t<door color=\"250\">\n");
    sprintf(tmp, "\t\t\t<point xPos=\"%.2f\" yPos=\"%.2f\"/>\n",
            (GetPoint1().GetX()) * FAKTOR,
            (GetPoint1().GetY()) * FAKTOR);
    geometry.append(tmp);
    sprintf(tmp, "\t\t\t<point xPos=\"%.2f\" yPos=\"%.2f\"/>\n",
            (GetPoint2().GetX()) * FAKTOR,
            (GetPoint2().GetY()) * FAKTOR);
    geometry.append(tmp);
    geometry.append("\t\t</door>\n");
    return geometry;
}
