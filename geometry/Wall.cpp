/**
 * \file        Wall.cpp
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
#include "Wall.h"

/************************************************************
 Wall
 ************************************************************/

Wall::Wall() : Line()
{
}

Wall::Wall(const Point& p1, const Point& p2, const std::string& type) : Line(p1, p2), _type(type)
{
}

void Wall::WriteToErrorLog() const
{
     char tmp[CLENGTH];
     sprintf(tmp, "\t\tWALL: (%f, %f) -- (%f, %f)\n", GetPoint1()._x,
             GetPoint1()._y, GetPoint2()._x, GetPoint2()._y);
     Log->Write(tmp);
}

std::string Wall::Write() const
{
     std::string geometry;
     char wall[500] = "";
     geometry.append("\t\t<wall>\n");
     sprintf(wall, "\t\t\t<point xPos=\"%.2f\" yPos=\"%.2f\"/>\n",
             (GetPoint1()._x) * FAKTOR,
             (GetPoint1()._y) * FAKTOR);
     geometry.append(wall);
     sprintf(wall, "\t\t\t<point xPos=\"%.2f\" yPos=\"%.2f\"/>\n",
             (GetPoint2()._x) * FAKTOR,
             (GetPoint2()._y) * FAKTOR);
     geometry.append(wall);
     geometry.append("\t\t</wall>\n");
     return geometry;
}

const std::string& Wall::GetType() const
{
     return _type;
}

void Wall::SetType(const std::string& type)
{
     _type=type;
}
