/* 
 * File:   Room.cpp
 * Author: andrea
 * 
 * Created on 30. September 2010, 11:58
 */

#include <vector>

#include "Room.h"
#include "Transition.h"

/************************************************************
  Konstruktoren
 ************************************************************/

Room::Room() {
    pRoomID = -1;
    pCaption = "no room caption";
    pZPos = -1;
    pWalls = vector<Wall > ();
    pTransitions = vector<Transition > ();
    pPoly = vector<CPoint > ();
}

Room::Room(const Room& orig) {
    pRoomID = orig.GetRoomID();
    pCaption = orig.GetCaption();
    pZPos = orig.GetZPos();
    pWalls = orig.GetAllWalls();
    pTransitions = orig.GetAllTransitions();
    pPoly = orig.GetPolygon();
}

Room::~Room() {
    if (pWalls.size() > 0) pWalls.clear();
    if (pTransitions.size() > 0) pTransitions.clear();
    if (pPoly.size() > 0) pPoly.clear();
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

void Room::SetZPos(int z) {
    pZPos = z;
}

void Room::SetAllWalls(const vector<Wall>& walls) {
    pWalls = walls;
}

void Room::SetWall(const Wall& wall, int index) {
    if ((index >= 0) && (index < (int) pWalls.size())) {
        pWalls[index] = wall;
    } else {
        printf("ERROR: Wrong Index in CRoom::SetWall()");
        exit(0);
    }
}

void Room::SetAllTransitions(const vector<Transition>& transitions) {
    pTransitions = transitions;
}

void Room::SetTransition(const Transition& transition, int index) {
    if ((index >= 0) && (index < (int) pTransitions.size())) {
        pTransitions[index] = transition;
    } else {
      printf("ERROR: Wrong Index in CRoom::SetTransition()");
        exit(0);
    }
}

void Room::SetPolygon(const vector<CPoint>& poly) {
    pPoly = poly;
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

int Room::GetZPos() const {
    return pZPos;
}

const vector<Wall>& Room::GetAllWalls() const {
    return pWalls;
}

const Wall Room::GetWall(int index) const {
    if ((index >= 0) && (index < (int) pWalls.size()))
        return pWalls[index];
    else {
        printf("ERROR: Wrong 'index' in CRoom::GetWall()");
        exit(0);
    }

}

const vector<Transition> Room::GetAllTransitions() const {
    return pTransitions;
}

const Transition Room::GetTransition(int index) const {
    if ((index >= 0) && (index < (int) pTransitions.size()))
        return pTransitions[index];
    else {
        printf("ERROR: Wrong 'index' in CRoom::GetTransition()");
        exit(0);
    }
}

const vector<CPoint>& Room::GetPolygon() const {
    return pPoly;
}


/*************************************************************
 Sonstige Funktionen
 ************************************************************/

void Room::AddWall(const Wall& w) {
    pWalls.push_back(w);
}

void Room::DeleteWall(int index) {
    if ((index >= 0) && (index < (int) pWalls.size()))
        pWalls.erase(pWalls.begin() + index);
    else {
        printf("ERROR: Wrong Index in CRoom::DeleteWall()");
        exit(0);
    }
}

void Room::AddTransition(const Transition& trans) {
    pTransitions.push_back(trans);
}

void Room::DeleteTransition(int index) {
    if ((index >= 0) && (index < (int) pTransitions.size()))
        pTransitions.erase(pTransitions.begin() + index);
    else {
        printf("ERROR: Wrong Index in CRoom::DeleteTransition()");
        exit(0);
    }
}

void Room::CloseTransition(int index) {
    Transition tmp = GetTransition(index);
    if (tmp.GetIsOpen()) {
        tmp.Close();
        SetTransition(tmp, index);
    } else {
        printf("ERROR: Wrong Index in CRoom::CloseTransition() Transition is closed!!!");
        exit(0);
    }
}


void Room::ConvertLineToPoly() {
    vector<Line> copy;
    CPoint point;
    Line line;

    // Alle Linienelemente in copy speichern Wände und Transitions
    for (int i = 0; i < pWalls.size(); i++) {
        copy.push_back(pWalls[i].GetLine());
    }
    for (int i = 0; i < pTransitions.size(); i++) {
        copy.push_back(pTransitions[i].GetLine());
    }

    line = copy[0];
    pPoly.push_back(line.GetPoint1());
    point = line.GetPoint2();
    copy.erase(copy.begin());

    for (int i = 0; i < (int) copy.size(); i++) {
        line = copy[i];
        if ((point - line.GetPoint1()).Norm()< TOLERANZ) {
            pPoly.push_back(line.GetPoint1());
            point = line.GetPoint2();
            copy.erase(copy.begin() + i);
            // von vorne suchen
            i = -1;
        } else if ((point - line.GetPoint2()).Norm()< TOLERANZ) {
            pPoly.push_back(line.GetPoint2());
            point = line.GetPoint1();
            copy.erase(copy.begin() + i);
            // von vorne suchen
            i = -1;
        }
    }
    if ((pPoly[0] - point).Norm()>TOLERANZ) {
        char tmp[100];
        sprintf(tmp,"ERROR: \tRoom::ConvertLineToPoly(): Raum %d Anfangspunkt ungleich Endpunkt!!!\n"
                "(%f, %f) != (%f, %f)\n", GetRoomID(), pPoly[0].GetX(), pPoly[0].GetY(), point.GetX(),
                point.GetY());
        printf("%s", tmp);
        exit(0);
    }
}


bool Room::IsStart() const {
    return pCaption.find("start") != string::npos;
}

bool Room::IsExit() const {
    return pCaption.find("exit") != string::npos;
}

CPoint Room::GetCentre() const {
    //This will only work for orthogonally orientated rectangular rooms
    vector<float> xs;
    vector<float> ys;
    vector<float>::iterator xmin, xmax, ymin, ymax;
    CPoint mid;

    for (int i = 0; i < pPoly.size(); i++) {
        xs.push_back(pPoly[i].GetX());
        ys.push_back(pPoly[i].GetY());
    }
    xmin = min_element(xs.begin(), xs.end());
    xmax = max_element(xs.begin(), xs.end());
    ymin = min_element(ys.begin(), ys.end());
    ymax = max_element(ys.begin(), ys.end());
    mid.SetX((*xmin + *xmax) / 2);
    mid.SetY((*ymin + *ymax) / 2);
    return mid;


}



/*************************************************************
 Ein-Ausgabe
 ************************************************************/
void Room::LoadWalls(ifstream* buildingfile, int* i) {
    string line;
    getline(*buildingfile, line);
    (*i)++;
    int elements;
    while (line.find("</contdata>") == string::npos) {
        if (line.find("<elements>") != string::npos) {
            getline(*buildingfile, line);
            (*i)++;
            while (line.find("</elements>") == string::npos) {
                if (line.find("wall") != string::npos) {
                    Wall wall = Wall();
                    istringstream iss(line, istringstream::in);
                    string tmp; // Schlüsselwort
                    float x1, y1, x2, y2;
                    iss >> tmp >> x1>> y1 >> x2 >> y2;
                    wall.SetLine(Line(CPoint(x1, y1), CPoint(x2, y2)));
                    AddWall(wall);
                } else {
                    char tmp[100];
                    sprintf(tmp, "ERROR: \tWrong object in building file <room> "
                            "<contdata> <element>: [%s] line %d ", line.c_str(), i);
                    printf("%s", tmp);
                    exit(0);
                }
                getline(*buildingfile, line);
                (*i)++;
            }
        } else if (line.find("elements") != string::npos) {
            istringstream iss(line, istringstream::in);
            string tmp; // Schlüsselwort
            iss >> tmp >> elements;
        } else {
            char tmp[100];
            sprintf(tmp, "ERROR: \tWrong object in building file <room> <contdata>:"
                    "[%s] line %d ", line.c_str(), i);
            printf("%s", tmp);
            exit(0);
        }
        getline(*buildingfile, line);
        (*i)++;
    }
    if (elements != pWalls.size()) {
        char tmp[100];
        sprintf(tmp, "ERROR: \tWrong number of rooms: %d != %d", elements, pWalls.size());
        printf("%s", tmp);
        exit(0);
    }
}

void Room::WriteToErrorLog() const {
    char tmp[300];
    string s;
    sprintf(tmp, "\tRaum: %d [%s]:", pRoomID, pCaption.c_str());
    s.append(tmp);
    printf("%s", tmp);
    // Wände
    for (int i = 0; i < pWalls.size(); i++) {
        Wall w = GetWall(i);
        w.WriteToErrorLog();
    }
    //Übergänge
    for (int i = 0; i < pTransitions.size(); i++) {
        Transition t = GetTransition(i);
        t.WriteToErrorLog();
    }
}
