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
#include "../math/HeapTree.h"

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

RectGrid::~RectGrid(){
    if (isInner) delete[] isInner;
};

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
        isInner[i] = 1;
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
    int i = (int)(((currPoint.GetX()-xMin)/hx)+.5);
    int j = (int)(((currPoint.GetY()-yMin)/hy)+.5);

    //upper                        //-2 marks invalid neighbor
    neighbors.key[0] = (j == jMax) ? -2 : ((j+1)*iMax+i); //bei oberen Rand => out of Bound
    //lower
    neighbors.key[1] = (j == 0) ? -2 : ((j-1)*iMax+i); //bei unterem Rand => negative
    //left
    neighbors.key[2] = (i == 0) ? -2 : (j*iMax+i-1); // @todo: ar.graf Fehler falls Point am linken Rand
    //right
    neighbors.key[3] = (i == iMax) ? -2 : (j*iMax+i+1); // @todo: ar.graf Fehler falls Point am rechten Rand

    return neighbors;
}

directNeighbor RectGrid::getNeighbors(const int key) const {
    directNeighbor neighbors = {{-1, -1, -1, -1}};
    int i = key%iMax;
    int j = key/iMax;

    //upper                        //-2 marks invalid neighbor
    neighbors.key[0] = (j == jMax) ? -2 : ((j+1)*iMax+i); //bei oberen Rand => out of Bound
    //lower
    neighbors.key[1] = (j == 0) ? -2 : ((j-1)*iMax+i); //bei unterem Rand => negative
    //left
    neighbors.key[2] = (i == 0) ? -2 : (j*iMax+i-1); // @todo: ar.graf Fehler falls Point am linken Rand
    //right
    neighbors.key[3] = (i == iMax) ? -2 : (j*iMax+i+1); // @todo: ar.graf Fehler falls Point am rechten Rand

    return neighbors;

}

int RectGrid::setAsInner(const Point& innerP) {
    return 0; // @todo: ar.graf
}

int RectGrid::setAsOuter(const Point& outerP) {
    return 0; // @todo: ar.graf
}
//        int setAsInner(const Point& innerP); //input in xy world coordinates
//        int setAsOuter(const Point& outerP); //input in xy world coordinates
//
//        int getGridID() const;



FastMarcher::FastMarcher()
{
    //ctor
}

FastMarcher::~FastMarcher()
{
    //dtor
}

int FastMarcher::setCostArray(double* givenCost) {
    myCost = givenCost;
    return 1;
}

int FastMarcher::setGradientArray(Point* givenGradient) {
    myGradient = givenGradient;
    return 1;
}

int FastMarcher::setSpeedArray(double* givenSpeed) {
    mySpeed = givenSpeed;
    return 1;
}

int FastMarcher::setGrid(RectGrid* givenGrid) {
    myGrid = givenGrid;
    return 1;
}

int FastMarcher::setGeometry(const Building* const building) {
    return 0;   // @todo: ar.graf
}

// @todo: ar.graf : get all access methods with key parameter (do not calc key for each get-meth)

Point FastMarcher::getFloorfieldAt(const Point currPos) const {
    int key = myGrid->getKeyAtXY(currPos.GetX(), currPos.GetY());
    return myGradient[key];
}

double FastMarcher::getTimecostAt(const Point currPos) const {
    int key = myGrid->getKeyAtXY(currPos.GetX(), currPos.GetY());
    return myCost[key];
}

double FastMarcher::getTimecostAt(const int x, const int y) const {
    int key = myGrid->getKeyAtXY(x, y);
    return myCost[key];
}

double FastMarcher::getSpeedAt(const Point currPos) const {
    int key = myGrid->getKeyAtXY(currPos.GetX(), currPos.GetY());
    return mySpeed[key];
}

double FastMarcher::getSpeedAt(const int x, const int y) const {
    int key = myGrid->getKeyAtXY(x, y);
    return mySpeed[key];
}

int FastMarcher::calculateFloorfield() {
    // @todo: ar.graf
    //definiere max heap //dyn heap??
    HeapTree<int>* narrowband = new HeapTree<int> (myCost, myGrid->getNumOfElements());

    //suche alle Punkte mit cost = 0
    for (int iKey = 0; iKey < myGrid->getNumOfElements(); ++iKey) {
        //alle nachbarn eines jeden dieser punkte wird (falls cost = -2.) dem narrowband zugefuegt
        //(evtl mit cost = -1., um es als zugefuegt zu markieren)
        if (myCost[iKey] == 0.) {
            narrowband->Add(iKey);
        }
        directNeighbor neighbor = myGrid->getNeighbors(iKey);
    }
    //jetzt habe ich das erste narrowband
    //berechne pro punkt alle vier kost-werte und waehle zu jedem punkt das minimum der vier
    //das minimum wird in dem cost array gespeichert, das neu berechnete element muss an die richtige stelle im baum
    //suche im narrowband das cost-minimum (heap root) und
        //fuege seine nachbarn dem narrowband hinzu, falls nachbar = -2.
        //fuege nun das minimum den bekannten hinzu (indem es aus dem narrowband rausgenommen wird)



    return 0;
}
