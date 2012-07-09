/*
 * File:   Wall.cpp
 * Author: andrea
 *
 * Created on 16. November 2010, 12:55
 */

#include "Wall.h"

/************************************************************
 Wall
 ************************************************************/

Wall::Wall() : Line() {
}

Wall::Wall(const Point& p1, const Point& p2) : Line(p1, p2) {

}

Wall::Wall(const Wall& orig) : Line(orig) {
}

//Wall::~Wall() {
//
//}

void Wall::WriteToErrorLog() const {
    char tmp[CLENGTH];
    sprintf(tmp, "\t\tWALL: (%f, %f) -- (%f, %f)\n", GetPoint1().GetX(),
            GetPoint1().GetY(), GetPoint2().GetX(), GetPoint2().GetY());
    Log->write(tmp);
}

string Wall::Write() const {
    string geometry;
    char wall[500] = "";
    geometry.append("\t\t<wall>\n");
    sprintf(wall, "\t\t\t<point xPos=\"%.2f\" yPos=\"%.2f\"/>\n",
            (GetPoint1().GetX()) * FAKTOR,
            (GetPoint1().GetY()) * FAKTOR);
    geometry.append(wall);
    sprintf(wall, "\t\t\t<point xPos=\"%.2f\" yPos=\"%.2f\"/>\n",
            (GetPoint2().GetX()) * FAKTOR,
            (GetPoint2().GetY()) * FAKTOR);
    geometry.append(wall);
    geometry.append("\t\t</wall>\n");
    return geometry;
}
