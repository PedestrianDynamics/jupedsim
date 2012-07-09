/* 
 * File:   Building.cpp
 * Author: andrea
 * 
 * Created on 1. Oktober 2010, 09:25
 */

#include "Building.h"

/************************************************************
  Konstruktoren
 ************************************************************/

Building::Building() {
    pCaption = "no caption";
    pRooms = vector<Room > ();
}

Building::Building(const Building& orig) {
    pCaption = orig.GetCaption();
    pRooms = orig.GetAllRooms();
}

Building::~Building() {
    if (pRooms.size() > 0) pRooms.clear();
}

/*************************************************************
 Setter-Funktionen
 ************************************************************/
void Building::SetCaption(string s) {
    pCaption = s;
}

void Building::SetAllRooms(const vector<Room>& rooms) {
    pRooms = rooms;
}

void Building::SetRoom(const Room& room, int index) {
    if ((index >= 0) && (index < (int) pRooms.size())) {
        pRooms[index] = room;
    } else {
        printf("ERROR: \tWrong Index in CBuilding.SetRoom()");
        exit(0);
    }
}

/*************************************************************
 Getter-Funktionen
 ************************************************************/

string Building::GetCaption() const {
    return pCaption;
}

const vector<Room>& Building::GetAllRooms() const {
    return pRooms;
}

const Room& Building::GetRoom(int index) const {
    if ((index >= 0) && (index < (int) pRooms.size())) {
        return pRooms[index];
    } else {
      printf("ERROR: Wrong 'index'=%d in CBuiling::GetRoom()\n",index);
        exit(0);
    }
}

/*************************************************************
 Sonstiges
 ************************************************************/
void Building::InitGeometry() {
    for (int i = 0; i < pRooms.size(); i++) {
        Room room = GetRoom(i);
        vector<Transition> trans = room.GetAllTransitions();
        // alle Tueren öffnen
        for (int j = 0; j < trans.size(); j++) {
            trans[j].Open();
            room.SetTransition(trans[j], j);
        }
        // Polygone berechnen
        room.ConvertLineToPoly();
        SetRoom(room, i);
    }
}

void Building::AddRoom(const Room& room) {
    pRooms.push_back(room);
}

int Building::RoomIndexForThisID(int ID) const {
    int i;
    for (i = 0; i < pRooms.size(); i++) {
        if (pRooms[i].GetRoomID() == ID)
            break;
    }
    if (i == pRooms.size()) {
        printf("ERROR: \tBuilding::RoomIndexForThisID() can not find ID!");
        exit(0);
    }
    return i;
}

/*************************************************************
 Ein-Ausgabe
 ************************************************************/

void Building::LoadFromFile(string filename) {
    ifstream buildingfile;
    string line;

    // Datei oeffnen
    buildingfile.open(filename.c_str(), fstream::in);
    if (!buildingfile) {
      printf("ERROR: \tCannot load building file: %s", filename.c_str());
        exit(0);
    } else {
        int i = 0;
        while (getline(buildingfile, line)) {
            i++; // Zeienindex zum debuggen, nach jedem getline() erhöhen
            if (line.find("<header>") != string::npos) {
                LoadHeader(&buildingfile, &i);
            } else if (line.find("<rooms>") != string::npos) {
                LoadRooms(&buildingfile, &i);
            } else if (line.find("<transitions>") != string::npos) {
                LoadTransitions(&buildingfile, &i);
            } else {
                char tmp[100];
                sprintf(tmp, "ERROR: \tWrong object in building file: [%s] line %d ", line.c_str(), i);
                printf("%s",tmp);
                exit(0);
            }
        }
        buildingfile.close();
        buildingfile.clear();
    }
}

void Building::WriteToErrorLog() const {
    printf("GEOMETRY: ");
    for (int i = 0; i < pRooms.size(); i++) {
        Room r = GetRoom(i);
        r.WriteToErrorLog();
    }
    printf("\n");
}

/*************************************************************
 private Funktionen
 ************************************************************/
void Building::LoadHeader(ifstream* buildingfile, int* i) {
    string line;

    getline(*buildingfile, line);
    (*i)++;
    while (line.find("</header>") == string::npos) {
        if (line.find("caption") != string::npos) {
            istringstream iss(line, istringstream::in);
            string tmp; // Schlüsselwort, hier: "caption"
            iss >> tmp >> pCaption;
        } else if (line.find("version") != string::npos) {
            istringstream iss(line, istringstream::in);
            string tmp; // Schlüsselwort, hier: "version"
            double version;
            iss >> tmp >> version;
            if (version != 0.1) {
                char tmp[100];
                sprintf(tmp, "ERROR: \tneue Version im Geometrieformat!!! %f != %f", version, VERSION);
                printf(tmp);
                exit(0);
            }
        } else {
            char tmp[100];
            sprintf(tmp, "ERROR: \tWrong object in building file <header>: [%s] line %d "
                    , line.c_str(), *i);
            printf("%s", tmp);
            exit(0);
        }
        getline(*buildingfile, line);
        (*i)++;
    }
}

void Building::LoadRooms(ifstream* buildingfile, int* i) {
    string line;
    getline(*buildingfile, line);
    (*i)++;
    int elements = 0;
    while (line.find("</rooms>") == string::npos) {
        if (line.find("elements") != string::npos) {
            istringstream iss(line, istringstream::in);
            string tmp; // Schlüsselwort
            iss >> tmp >> elements;
        } else if (line.find("<room>") != string::npos) {
            LoadRoom(buildingfile, i);
        } else {
            char tmp[100];
            sprintf(tmp, "ERROR: \tWrong object in building file <rooms>: [%s] line %d "
                    , line.c_str(), i);
            printf("%s", tmp);
            exit(0);
        }
        getline(*buildingfile, line);
        (*i)++;
    }
    if (elements != pRooms.size()) {
        char tmp[100];
        sprintf(tmp, "ERROR: \tWrong number of rooms: %d != %d", elements, pRooms.size());
        printf("%s", tmp);
        exit(0);
    }

}

void Building::LoadRoom(ifstream* buildingfile, int* i) {
    string line;
    getline(*buildingfile, line);
    (*i)++;
    Room room = Room();
    while (line.find("</room>") == string::npos) {
        if (line.find("caption") != string::npos) {
            istringstream iss(line, istringstream::in);
            string tmp, caption; // Schlüsselwort, hier: "caption"
            iss >> tmp >> caption;
            room.SetCaption(caption);
        } else if (line.find("index") != string::npos) {
            istringstream iss(line, istringstream::in);
            string tmp; // Schlüsselwort
            int ID;
            iss >> tmp >> ID;
            room.SetRoomID(ID);
        } else if (line.find("zpos") != string::npos) {
            istringstream iss(line, istringstream::in);
            string tmp; // Schlüsselwort
            int zpos;
            iss >> tmp >> zpos;
            room.SetZPos(zpos);
        } else if (line.find("<contdata>") != string::npos) {
            room.LoadWalls(buildingfile, i);
        } else {
            char tmp[100];
            sprintf(tmp, "ERROR: \tWrong object in building file <room> :"
                    "[%s] line %d ", line.c_str(), i);
            printf("%s", tmp);
            exit(0);
        }
        getline(*buildingfile, line);
        (*i)++;
    }
    AddRoom(room);
}

void Building::LoadTransitions(ifstream* buildingfile, int* i) {
    string line;
    getline(*buildingfile, line);
    (*i)++;
    int elements = 0;
    int anz = 0;
    while (line.find("</transitions>") == string::npos) {
        if (line.find("elements") != string::npos) {
            istringstream iss(line, istringstream::in);
            string tmp; // Schlüsselwort
            iss >> tmp >> elements;
        } else if (line.find("<transition>") != string::npos) {
            anz++;
            LoadTransition(buildingfile, i);
        } else {
            char tmp[100];
            sprintf(tmp, "ERROR: \tWrong object in building file <transition>: [%s] line %d ",
                    line.c_str(), i);
	    printf("%s", tmp);
            exit(0);
        }
        getline(*buildingfile, line);
        (*i)++;
    }
    if (elements != anz) {
        char tmp[100];
        sprintf(tmp, "ERROR: \tWrong number of rooms: %d != %d", elements, pRooms.size());
        printf("%s", tmp);
        exit(0);
    }
}

void Building::LoadTransition(ifstream* buildingfile, int* i) {
    string line;
    getline(*buildingfile, line);
    Transition t;
    (*i)++;
    while (line.find("</transition>") == string::npos) {
        if (line.find("index") != string::npos) {
            istringstream iss(line, istringstream::in);
            string tmp; // Schlüsselwort
            int ID;
            iss >> tmp >> ID;
            t.SetID(ID);
        } else if (line.find("caption") != string::npos) {
            istringstream iss(line, istringstream::in);
            string tmp, caption;
            iss >> tmp >> caption;
            t.SetCaption(caption);
        } else if (line.find("trans") != string::npos) {
            istringstream iss(line, istringstream::in);
            string tmp;
            float x1, y1, x2, y2;
            iss >> tmp >> x1 >> y1 >> x2 >> y2;
            t.SetLine(Line(CPoint(x1, y1), CPoint(x2, y2)));
        } else if (line.find("room1") != string::npos) {
            istringstream iss(line, istringstream::in);
            string tmp;
            int room1;
            iss >> tmp >> room1;
            t.SetRoom1(room1);
        } else if (line.find("room2") != string::npos) {
            stringstream iss(line, istringstream::in);
            string tmp;
            int room2;
            iss >> tmp >> room2;
            t.SetRoom2(room2);
        } else {
            char tmp[100];
            sprintf(tmp, "ERROR: \tWrong object in building file <transition> <transition>: [%s] line %d ",
                    line.c_str(), i);
            printf("%s", tmp);
            exit(0);
        }
        getline(*buildingfile, line);
        (*i)++;
    }
    int ID1 = t.GetRoom1();
    int ID2 = t.GetRoom2();
    if (ID1 == ID2) { // spaeter vielleicht Sondertüren einfügen
      if(ID1 >=0){
	Room r = GetRoom(ID1);
	r.AddTransition(t);
	SetRoom(r, ID1);
      }
    } else { // Transition zu beiden Räumen hinzufügen
        if (ID1 != -1) { // -1 bedeutet Ausgang
            Room r = GetRoom(ID1);
            r.AddTransition(t);
            SetRoom(r, ID1);
        }
        if (ID2 != -1) {
            Room r = GetRoom(ID2);
            r.AddTransition(t);
            SetRoom(r, ID2);
        }
    }
}
