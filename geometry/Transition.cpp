/*
 * File:   Transition.cpp
 * Author: andrea
 *
 * Created on 16. November 2010, 12:57
 */

#include "Transition.h"

/************************************************************
 Transition (abgeleitet von Crossing)
 ************************************************************/

Transition::Transition() : Crossing() {
	pIsOpen = true;
	pRoom2 = NULL;
}

Transition::Transition(const Transition& orig) : Crossing(orig) {
	pIsOpen = orig.IsOpen();
	pRoom2 = orig.GetRoom2();
}

Transition::~Transition() {
}

// Setter-Funktionen

void Transition::Close() {
	pIsOpen = false;
}

void Transition::Open() {
	pIsOpen = true;
}

void Transition::SetType(string type){
	pType=type;
}

void Transition::SetRoom2(Room* r) {
	pRoom2 = r;
}

// Getter-Funktionen

bool Transition::IsOpen() const {
	return pIsOpen;
}


Room* Transition::GetRoom2() const {
	return pRoom2;
}

string Transition::GetType() const {
	return pType;
}
// Sonstiges

// gibt den ANDEREN room != roomID zurück
Room* Transition::GetOtherRoom(int roomID) const {
	if (GetRoom1()!=NULL && GetRoom1()->GetRoomID() == roomID) {
		return GetRoom2();
	} else if (GetRoom2()!=NULL && GetRoom2()->GetRoomID() == roomID) {
		return GetRoom1();
	} else {
		char msg[CLENGTH];
		sprintf(msg,"ERROR: \tTransition::GetOtherRoom() wrong roomID [%d]",roomID);
		Log->write(msg);
		exit(0);
	}

}
// virtuelle Funktionen

// prüft ob Ausgang nach draußen
bool Transition::IsExit() const {
	if(GetRoom1()!=NULL && pRoom2!=NULL)
		return false;
	else
		return true;
}
// prüft, ob Transition in Raum mit roomID
bool Transition::IsInRoom(int roomID) const {
	bool c1 = false;
	bool c2 = false;
	if (GetRoom1() != NULL && GetRoom1()->GetRoomID() == roomID)
		c1 = true;
	if (GetRoom2() != NULL && GetRoom2()->GetRoomID() == roomID)
		c2 = true;
	return c1 || c2;
}

bool Transition::IsTransition() const {
	return true;
}

/* Setzt Fußgänger von einem SubRoom in den zugehörigen nächsten SubRoom in einem neuen Raum
 * Parameter:
 *    - RoomID: aktuelle RoomID des Fußgängers
 *    - SubID: aktuelle SubRoomID des Fußgängers
 *    - PedID: ID des Fußgängers
 *    - goal_new: neues Ziel des Fußgängers, kann Crossing oder Transition sein,
 *          wird in Routing bestimmt und hier gesetzt
 * */
void Transition::UpdatePedestrian(int RoomID, int SubID, int PedID, Crossing* goal_new) const {
	SubRoom* sub = GetSubRoom(SubID);
	Pedestrian* ped = sub->GetPedestrian(PedID);
	Room* other_room = GetOtherRoom(RoomID);
	if (other_room != NULL) {
		SubRoom* other_sub = GetOtherSubRoom(RoomID, SubID);
		// Fußgänger löschen
		sub->DeletePedestrian(PedID);
		// neues Ziel setzen
		ped->SetExitIndex(goal_new->GetIndex());
		ped->SetExitLine(goal_new);
		// in neuem SubRoom setzten
		other_sub->AddPedestrian(ped);
	} else {
		// Fußgänger löschen und NICHT neu setzen
		sub->DeletePedestrian(PedID);
	}
}
/* gibt den ANDEREN Subroom mit GetRoomID() != roomID zurück
 * subroomID wird hier nicht benötigt, aber in Crossings::GetOtherSubRoom()
 * (virtuelle Funktion) */
SubRoom* Transition::GetOtherSubRoom(int roomID, int subroomID) const {
	if ((GetRoom1() != NULL) && (GetRoom1()->GetRoomID() == roomID))
		return GetSubRoom2();
	else if ((GetRoom2() != NULL) && (GetRoom2()->GetRoomID() == roomID))
		return GetSubRoom1();
	else {
		char tmp[CLENGTH];
		sprintf(tmp,"ERROR: \tTransition::GetOtherSubRoom No exit found "
				"on the other side\n ID=%hd, roomID=%hd, subroomID=%hd\n",GetIndex(),roomID,subroomID);
		Log->write(tmp);
		exit(0);
	}
}

// Ein-Ausgbae

void Transition::WriteToErrorLog() const {
	string s;
	char tmp[CLENGTH];
	sprintf(tmp, "\t\tTRANS: %d [%s] (%f, %f) -- (%f, %f)\n", GetIndex(), GetCaption().c_str(),
			GetPoint1().GetX(), GetPoint1().GetY(), GetPoint2().GetX(), GetPoint2().GetY());
	s.append(tmp);
	// erster Raum
	if (GetRoom1() != NULL) {
		sprintf(tmp, "\t\t\t\tRoom: %d [%s] SubRoom: %d", GetRoom1()->GetRoomID(),
				GetRoom1()->GetCaption().c_str(), GetSubRoom1()->GetSubRoomID());
	} else {
		sprintf(tmp, "\t\t\t\tAusgang");
	}
	s.append(tmp);
	// zweiter Raum
	if (GetRoom2() != NULL) {
		sprintf(tmp, " <->\tRoom: %d [%s] SubRoom: %d\n", GetRoom2()->GetRoomID(),
				GetRoom2()->GetCaption().c_str(), GetSubRoom2()->GetSubRoomID());
	} else {
		sprintf(tmp, " <->\tAusgang\n");
	}
	s.append(tmp);
	Log->write(s);
}

// TraVisTo Ausgabe
string Transition::WriteElement() const {
	string geometry;
	char tmp[CLENGTH] = "";

	sprintf(tmp,"\t\t<door ID=\"%d\" color=\"180\" caption=\"%d_%s\">\n",GetIndex(),GetIndex(),GetCaption().c_str());
	geometry.append(tmp);
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
