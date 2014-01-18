/**
 * File:   Wall.cpp
 *
 * Created on 16. November 2010, 12:55
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

#include "Wall.h"

using namespace std;

/************************************************************
 Wall
 ************************************************************/

Wall::Wall() : Line() {
}

Wall::Wall(const Point& p1, const Point& p2) : Line(p1, p2) {

}

Wall::Wall(const Wall& orig) : Line(orig) {
}

void Wall::WriteToErrorLog() const {
    char tmp[CLENGTH];
    sprintf(tmp, "\t\tWALL: (%f, %f) -- (%f, %f)\n", GetPoint1().GetX(),
            GetPoint1().GetY(), GetPoint2().GetX(), GetPoint2().GetY());
    Log->Write(tmp);
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
