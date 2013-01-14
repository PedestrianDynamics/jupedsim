/**
 * File:   Room.cpp
 * 
 * Created on 30. September 2010, 11:58
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

#include <vector>

#include "Room.h"

/************************************************************
  Konstruktoren
 ************************************************************/

Room::Room() {
    pRoomID = -1;
    pState=0; //smoke-free
    pCaption = "no room caption";
    pZPos = -1.0;
    pSubRooms = vector<SubRoom* > ();
}

Room::Room(const Room& orig) {
    pRoomID = orig.GetRoomID();
    pCaption = orig.GetCaption();
    pZPos = orig.GetZPos();
    pSubRooms = orig.GetAllSubRooms();

}

Room::~Room() {
    for (int i = 0; i < GetAnzSubRooms(); i++)
        delete pSubRooms[i];
}

/*************************************************************
 Setter-Funktionen
 ************************************************************/
void Room::SetRoomID(int ID) {
    pRoomID = ID;
}

void Room::SetCaption(string s) {
    pCaption = s;
}

void Room::SetZPos(double z) {
    pZPos = z;
}

void Room::SetAllSubRooms(const vector<SubRoom*>& subrooms) {
    pSubRooms = subrooms;
}

void Room::SetSubRoom(SubRoom* subroom, int index) {
    if ((index >= 0) && (index < GetAnzSubRooms())) {
        pSubRooms[index] = subroom;
    } else {
        Log->Write("ERROR: Wrong Index in Room::SetSubRoom()");
        exit(0);
    }
}

void Room::SetRoomState(int state) {
	pState=state;
}
/*************************************************************
 Getter-Funktionen
 ************************************************************/
int Room::GetRoomID() const {
    return pRoomID;
}

string Room::GetCaption() const {
    return pCaption;
}

double Room::GetZPos() const {
    //if(pCaption=="070") return pZPos+1.0;
	return pZPos;
}

int Room::GetAnzSubRooms() const {
    return pSubRooms.size();
}

const vector<SubRoom*>& Room::GetAllSubRooms() const {
    return pSubRooms;
}

SubRoom* Room::GetSubRoom(int index) const {
    if ((index >= 0) && (index < (int) pSubRooms.size()))
        return pSubRooms[index];
    else {
    	char tmp[CLENGTH];
        sprintf(tmp,"ERROR: Room::GetSubRoom() Wrong subroom index [%d] for room index [%d] ",index,pRoomID);
        Log->Write(tmp);
        exit(0);
    }
}

int Room::GetAnzPedestrians() const {
    int sum = 0;
    for (int i = 0; i < GetAnzSubRooms(); i++) {
        sum += GetSubRoom(i)->GetAnzPedestrians();
    }
    return sum;
}

int Room::GetRoomState() const {
	return pState;
}
/*************************************************************
 Sonstige Funktionen
 ************************************************************/
void Room::AddSubRoom(SubRoom* r) {
    pSubRooms.push_back(r);
}

void Room::DeleteSubRoom(int index) {
    if ((index >= 0) && (index < (int) pSubRooms.size()))
        pSubRooms.erase(pSubRooms.begin() + index);
    else {
        Log->Write("ERROR: Wrong Index in Room::DeleteSubRoom()");
        exit(0);
    }
}

/*************************************************************
 Ein-Ausgabe
 ************************************************************/
void Room::LoadNormalSubRoom(ifstream* buildingfile, int* i) {
    string line;
    int NWalls = 0;
    getline(*buildingfile, line);
    (*i)++;
    NormalSubRoom* subroom = new NormalSubRoom();

    while (line.find("</subroom>") == string::npos) {
        istringstream iss(line, istringstream::in);
        string tmp; // Schlüsselwort, z.B.: "caption"
        if (line.find("index") != string::npos) {
            int ID;
            iss >> tmp >> ID;
            subroom->SetSubRoomID(ID);
            subroom->SetRoomID(pRoomID);
        } else if (line.find("Welements") != string::npos) {
            iss >> tmp >> NWalls;
        } else if (line.find("wall") != string::npos) {
            subroom->LoadWall(line);
        } else {
            char tmp[CLENGTH];
            sprintf(tmp, "ERROR: \tRoom::LoadNormalSubRoom()"
                    "Wrong object in building file <subroom>: [%s] line %d ", line.c_str(), *i);
            Log->Write(tmp);
            exit(0);
        }
        getline(*buildingfile, line);
        (*i)++;
    }
    if (NWalls != subroom->GetAnzWalls()) {
        char tmp[CLENGTH];
        sprintf(tmp, "ERROR: \tRoom::LoadNormalSubRoom()"
                "Wrong number of Walls: %d != %d", NWalls, subroom->GetAnzWalls());
        Log->Write(tmp);
        exit(0);
    }
    AddSubRoom(subroom);
}

void Room::LoadStair(ifstream* buildingfile, int* i) {
    string line;
    int NWalls = 0;
    getline(*buildingfile, line);
    (*i)++;
    Stair* stair = new Stair();
    while (line.find("</stair>") == string::npos) {
        istringstream iss(line, istringstream::in);
        string tmp; // Schlüsselwort, z.B.: "caption"
        if (line.find("index") != string::npos) {
            int ID;
            iss >> tmp >> ID;
            stair->SetSubRoomID(ID);
            stair->SetRoomID(pRoomID);
        } else if (line.find("Welements") != string::npos) {
            iss >> tmp >> NWalls;
        } else if (line.find("wall") != string::npos) {
            stair->LoadWall(line);
        } else if (line.find("up") != string::npos) {
            double x_up, y_up;
            iss >> tmp >> x_up >> y_up;
            stair->SetUp(Point(x_up, y_up));
        } else if (line.find("down") != string::npos) {
            double x_down, y_down;
            iss >> tmp >> x_down >> y_down;
            stair->SetDown(Point(x_down, y_down));
        } else {
            char tmp[CLENGTH];
            sprintf(tmp, "ERROR: \tRoom::LoadStair() Wrong object in building file <stair>: [%s] line %d "
                    , line.c_str(), *i);
            Log->Write(tmp);
            exit(0);
        }
        getline(*buildingfile, line);
        (*i)++;
    }
    if (NWalls != stair->GetAnzWalls()) {
        char tmp[CLENGTH];
        sprintf(tmp, "ERROR: \tWrong number of Walls: %d != %d", NWalls, stair->GetAnzWalls());
        Log->Write(tmp);
        exit(0);
    }
    AddSubRoom(stair);
}

void Room::WriteToErrorLog() const {
    char tmp[CLENGTH];
    string s;
    sprintf(tmp, "\tRaum: %d [%s]:\n", pRoomID, pCaption.c_str());
    s.append(tmp);
    Log->Write(s);
    // SubRooms
    for (int i = 0; i < GetAnzSubRooms(); i++) {
        SubRoom* s = GetSubRoom(i);
        s->WriteToErrorLog();
    }

}

const vector<int>& Room::GetAllTransitionsIDs() const {
	return pTransitionsIDs;
}

void Room::AddTransitionID(int ID){
	pTransitionsIDs.push_back(ID);
}

void Room::SetOutputHandler(OutputHandler* oh){
	pOutputFile=oh;
}

OutputHandler* Room::GetOutputHandler() const {
	return pOutputFile;
}
