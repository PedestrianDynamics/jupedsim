/* 
 * File:   Wall.cpp
 * Author: andrea
 * 
 * Created on 30. September 2010, 11:47
 */

#include "Wall.h"

/************************************************************
  Konstruktoren
 ************************************************************/

Wall::Wall() {
    pLine = Line();
}

Wall::Wall(const Wall& orig) {
    pLine = orig.GetLine();
}

Wall::~Wall() {
}

/*************************************************************
 Setter-Funktionen
 ************************************************************/

void Wall::SetLine(Line l) {
    pLine = l;
}

/*************************************************************
 Getter-Funktionen
 ************************************************************/

const Line& Wall::GetLine() const {
    return pLine;
}

/*************************************************************
 Ausgabe
 ************************************************************/

void Wall::WriteToErrorLog() const {
    char tmp[300];
    Line l = GetLine();
    sprintf(tmp, "\t\tWALL: (%f, %f) -- (%f, %f)\n", l.GetPoint1().GetX()
            , l.GetPoint1().GetY(), l.GetPoint2().GetX(), l.GetPoint2().GetY());
    printf("%s", tmp);
}
