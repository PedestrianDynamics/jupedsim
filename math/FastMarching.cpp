/**
 * \file        math/FastMarching.cpp
 * \date        Jan 20, 2015
 * \version
 * \copyright   <2009-2014> Forschungszentrum Jülich GmbH. All rights reserved.
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

#include "FastMarching.h"
#include "../geometry/Building.h"

RectGrid::RectGrid():
    gridID(NULL),
    isInner(NULL),
    nGridpoints(0),
    xMin(0.),
    yMin(0.),
    xMax(1.),
    yMax(1.),
    hx(1.),
    hy(1.),
    iMax(-1),
    jMax(-1)
{
    //Konstruktor
}

RectGrid::~RectGrid(){};

int RectGrid::setBoundaries(const double xMinA, const double yMinA,
                        const double xMaxA, const double yMaxA) {
    xMin = xMinA;
    xMax = xMaxA;
    yMin = yMinA;
    yMax = yMaxA;
    return 1;
}

int RectGrid::setBoundaries(const Point xy_min, const Point xy_max) {
    xMin = xy_min.GetX();
    xMax = xy_max.GetX();
    yMin = xy_min.GetY();
    yMax = xy_max.GetY();
    return 1;
}

int RectGrid::setSpacing(const double h_y, const double h_x) {
    hy = h_y;
    hx = h_x;
    return 1;
}

int RectGrid::createGrid(){
    iMax = (int)((xMax-xMin)/hx) + 1;
    jMax = (int)((yMax-yMin)/hy) + 1;
    nGridpoints = iMax * jMax;
    isInner = new int[nGridpoints];
    for (int i = 0; i < nGridpoints; ++i) {
        isInner[i] = 0;
    }
    return 1;
}

int RectGrid::isGridPoint(const Point& testPoint) const {
    Point nearest = getNearestGridPoint(testPoint);
    if ( (abs( nearest.GetX() - testPoint.GetX() ) <= .01) && (abs( nearest.GetY() - testPoint.GetY() ) <= .01) )
        return 1;
    return 0;
}

int RectGrid::isInnerPoint(const Point& testPoint) const {
    Point nearest = getNearestGridPoint(testPoint);
    return isInner[getKeyAtXY(nearest.GetX(), nearest.GetY())];
}

Point RectGrid::getNearestGridPoint(const Point& currPoint) const {
    if ((currPoint.GetX() > xMax) || (currPoint.GetY() > yMax))
        return Point(-7, -7); // @todo: ar.graf : find good false indicator
    int i = (int)(((currPoint.GetX()-xMin)/hx)+.5);
    int j = (int)(((currPoint.GetY()-yMin)/hy)+.5);
    return Point(i*hx+xMin, j*hy+yMin);
}

int RectGrid::getNearestGridPointsKey(const Point& currPoint) const {
    return getKeyAtXY(currPoint.GetX(), currPoint.GetY());
}

int RectGrid::getKeyAtIndex(const int i, const int j) const {//key = index in 1D array by giving indices of 2D array
    if ((i <= iMax) && (j <= jMax))                         //      zB um schnell die Nachbarpunkte-keys zu finden
        return (j*iMax+i); // 0-based
    return -1; // invalid indices
}

int RectGrid::getKeyAtXY(const double x, const double y) const {//key = index in (extern managed) array
    Point nearest = getNearestGridPoint(Point(x,y));
    int i = (int)(((nearest.GetX()-xMin)/hx)+.5);
    int j = (int)(((nearest.GetY()-yMin)/hy)+.5);
    if ((i <= iMax) && (j <= jMax))
        return (j*iMax+i); // 0-based
    return -1; // invalid indices
}

Point RectGrid::getPointFromKey(const int key) const {
    int i = key%iMax;
    int j = key/iMax; //integer division

    return Point(i*hx, j*hy);
}


int RectGrid::getNumOfElements() const {
    return nGridpoints;
}

directNeighbor RectGrid::getNeighbors(const Point& currPoint) const{
    directNeighbor neighbors = {{-1, -1, -1, -1}};
    int i = (int)(((nearest.GetX()-xMin)/hx)+.5);
    int j = (int)(((nearest.GetY()-yMin)/hy)+.5);

    //upper
    neighbors.key[0] = ((j+1)*iMax+i);
    //lower
    neighbors.key[1] = ((j-1)*iMax+i);
    //left
    neighbors.key[2] = (j*iMax+i-1);
    //right
    neighbors.key[3] = (j*iMax+i+1);

    return neighbors;
}
//        int setAsInner(const Point& innerP); //input in xy world coordinates
//        int setAsOuter(const Point& outerP); //input in xy world coordinates
//
//        int getGridID() const;
//        directNeighbor getNeighbors(const Point currPoint) const;
//




FastMarching::FastMarcher()
{
    //ctor
}

FastMarching::~FastMarcher()
{
    //dtor
}
