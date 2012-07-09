/* 
 * File:   Transition.cpp
 * Author: andrea
 * 
 * Created on 30. September 2010, 10:12
 */

#include "Transition.h"

/************************************************************
  Konstruktoren
 ************************************************************/

Transition::Transition() {
    pLine = Line();
    pIsOpen = true;
    pTransitionID = -1;
    pCaption = "no transition caption";
    pRoom1 = -1;
    pRoom2 = -1;
}

Transition::Transition(const Transition& orig) {
    pLine = orig.GetLine();
    pIsOpen = orig.GetIsOpen();
    pTransitionID = orig.GetID();
    pCaption = orig.GetCaption();
    pRoom1 = orig.GetRoom1();
    pRoom2 = orig.GetRoom2();
}

Transition::~Transition() {
}

/*************************************************************
 Setter-Funktionen
 ************************************************************/
void Transition::SetLine(const Line& l) {
    pLine = l;
}

void Transition::Close() {
    pIsOpen = false;
}

void Transition::Open() {
    pIsOpen = true;
}

void Transition::SetID(int ID) {
    pTransitionID = ID;
}

void Transition::SetCaption(string s) {
    pCaption = s;
}

void Transition::SetRoom1(int r1) {
    pRoom1 = r1;
}

void Transition::SetRoom2(int r2) {
    pRoom2 = r2;
}

/*************************************************************
 Getter-Funktionen
 ************************************************************/
const Line& Transition::GetLine() const {
    return pLine;
}

bool Transition::GetIsOpen() const {
    return pIsOpen;
}

int Transition::GetID() const {
    return pTransitionID;
}

string Transition::GetCaption() const {
    return pCaption;
}

int Transition::GetRoom1() const {
    return pRoom1;
}

int Transition::GetRoom2() const {
    return pRoom2;
}

/*************************************************************
 Sonstige Funktionen
 ************************************************************/


bool Transition::Connects(int r1id, int r2id) const {
    bool c1 = (r1id == pRoom1) && (r2id == pRoom2);
    bool c2 = (r1id == pRoom2) && (r2id == pRoom1);
    return c1 | c2;
}

int Transition::GetOtherRoom(int room_id) const {
    if (room_id == pRoom1) return pRoom2;
    else if (room_id == pRoom2) return pRoom1;
    else {
        printf("ERROR: \tNo exit found on the other side");
        exit(0);
    }
}

/*************************************************************
 Ein-Ausgabe
 ************************************************************/

void Transition::WriteToErrorLog() const {
    char tmp[300];
    Line l = GetLine();
    sprintf(tmp, "\t\tTRANS: %d [%s] (%f, %f) -- (%f, %f)\n", GetID(), GetCaption().c_str(), l.GetPoint1().GetX()
            , l.GetPoint1().GetY(), l.GetPoint2().GetX(), l.GetPoint2().GetY());
    printf("%s", tmp);
}
