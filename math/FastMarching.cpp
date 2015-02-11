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
#include <limits>
#include <math.h>

RectGrid::RectGrid():
    gridID(0),
    isInner(nullptr),
    nGridpoints(0),
    xMin(0.),
    xMax(1.),
    yMin(0.),
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

int RectGrid::setSpacing(const double h_x, const double h_y) {
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

    return Point(i*hx+xMin, j*hy+yMin);
}


int RectGrid::getNumOfElements() const {
    return this->nGridpoints;
}

directNeighbor RectGrid::getNeighbors(const Point& currPoint) const{
    directNeighbor neighbors = {{-1, -1, -1, -1}};
    int i = (int)(((currPoint.GetX()-xMin)/hx)+.5);
    int j = (int)(((currPoint.GetY()-yMin)/hy)+.5);

    //right                       //-2 marks invalid neighbor
    neighbors.key[0] = (i == (iMax-1)) ? -2 : (j*iMax+i+1);
    //upper
    neighbors.key[1] = (j == (jMax-1)) ? -2 : ((j+1)*iMax+i);
    //left
    neighbors.key[2] = (i == 0) ? -2 : (j*iMax+i-1);
    //lower
    neighbors.key[3] = (j == 0) ? -2 : ((j-1)*iMax+i);

    return neighbors;
}

directNeighbor RectGrid::getNeighbors(const int key) const {
    directNeighbor neighbors = {{-1, -1, -1, -1}}; //curleybrackets for struct, then for int[4]
    int i = key%iMax;
    int j = key/iMax;

    //right                       //-2 marks invalid neighbor
    neighbors.key[0] = (i == (iMax-1)) ? -2 : (j*iMax+i+1);
    //upper
    neighbors.key[1] = (j == (jMax-1)) ? -2 : ((j+1)*iMax+i);
    //left
    neighbors.key[2] = (i == 0) ? -2 : (j*iMax+i-1);
    //lower
    neighbors.key[3] = (j == 0) ? -2 : ((j-1)*iMax+i);


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
    myGrid = 0;
    myCost = 0;
    myGradient = 0;
    mySpeed = 0;

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
    int numElem = myGrid->getNumOfElements();
    HeapTree<int>* narrowband = new HeapTree<int> (myCost, numElem);//myGrid->getNumOfElements());

    //suche alle Punkte mit cost = 0
    for (int iKey = 0; iKey < numElem; ++iKey) {
        //alle nachbarn eines jeden dieser punkte wird (falls cost = -2.) dem narrowband zugefuegt
        //(evtl mit cost = -1., um es als zugefuegt zu markieren)
        if (myCost[iKey] == 0.) {
            directNeighbor neighbor = myGrid->getNeighbors(iKey);
            for (int neighKey = 0; neighKey < 4; ++neighKey) {
                if (neighbor.key[neighKey] > -1) { //valid neighbor
                    if (myCost[neighbor.key[neighKey]] == -2.) { //indicating a non calculated cost (as it is calc below, no point gets in twice
                                                                 //thus not yet in narrow band
                        calcCostAt(neighbor.key[neighKey]);      //saves result in myCost[]
                        narrowband->Add(neighbor.key[neighKey]); //getting added into correct heap pos. (lowest is root)
                    }
                }
            }
        }
    }
    //jetzt habe ich das erste narrowband mit minimum im root knoten

    while (narrowband->GetNumOfElem() > 0) {
        //berechne alle Werte neu:
        int token = 0; //token ist die aktuelle Berechnungsposition im Feld
        int* narrowbandField = narrowband->GetArray();
        int lastField = narrowband->GetNumOfElem();
        while(token < lastField) {
            bool isNewCellunderToken = false;
            calcCostAt(narrowbandField[token]);
            if ((token > 0) && (myCost[narrowbandField[token]] < myCost[narrowbandField[(token-1)/2]])) { //only if token is not root
                //shiftUp
                int Item    = narrowbandField[token];
                int Current = token;
                int Parent  = (token-1)/2;
                while (Current > 0) { // While Current is not the RootNode
                    if (myCost[narrowbandField[Parent]] > myCost[Item]) {//smaller values move up
                        narrowbandField[Current] = narrowbandField[Parent];
                        Current = Parent;
                        Parent = (Current-1)/2;
                    } else
                        break;
                }
                narrowbandField[Current] = Item;
            } else {
                if (
                    ( ((token*2 + 1) < lastField) && (myCost[narrowbandField[token]] > myCost[narrowbandField[token*2 + 1]]) )
                        ||
                    ( ((token*2 + 2) < lastField) && (myCost[narrowbandField[token]] > myCost[narrowbandField[token*2 + 2]]) )
                ) { //shiftDown
                    int Current = token;
                    int Child   = Current*2 + 1;
                    int Item   = narrowbandField[Current];    // Used to compare values

                    while (Child < lastField) {
                        if (Child < (lastField - 1))
                            if (myCost[narrowbandField[Child]] > myCost[narrowbandField[Child+1]])  // Set Child to smalest Child node
                                ++Child;

                        if (myCost[Item] > myCost[narrowbandField[Child]]) {    // larger values travel down
                            // Switch the Current node and the Child node
                            isNewCellunderToken = true;
                            narrowbandField[Current] = narrowbandField[Child];
                            Current       = Child;
                            Child         = Current*2 + 1;
                        } else
                            break;
                    }
                    narrowbandField[Current] = Item;
                }
            }
            //last loop-command
            if (!isNewCellunderToken) {
                ++token;
            }
        }
        //suche im narrowband das cost-minimum (heap root) und
        //fuege nun das minimum den bekannten hinzu (indem es aus dem narrowband rausgenommen wird)
        int currentMinimum = narrowband->Remove();
        std::cerr << currentMinimum << " ist berechnet.";
        std::cerr << "\n";
        directNeighbor neighbor = myGrid->getNeighbors(currentMinimum);
        for (int neighKey = 0; neighKey < 4; ++neighKey) {
            if (neighbor.key[neighKey] > -1) { //valid neighbor
                //fuege seine nachbarn dem narrowband hinzu, falls nachbar = -2.
                if (myCost[neighbor.key[neighKey]] == -2.) { //indicating a non calculated cost (as it is calc below, no point gets in twice
                                                             //thus not yet in narrow band
                    calcCostAt(neighbor.key[neighKey]);
                    narrowband->Add(neighbor.key[neighKey]); //getting added into correct heap pos. (lowest is root)
                }
            }
        }
    }

    std::cerr<<"calcFloorfield done\n";

    return 0;
}

//falls Rueckgabewert -1 -> schwerer fehler (diskriminante negativ)
inline double quadrSolutionMax (double c1, double c2, double h1, double h2, double speed) {
    if (c1 == std::numeric_limits<double>::max()) return std::numeric_limits<double>::max();
    if (c2 == std::numeric_limits<double>::max()) return std::numeric_limits<double>::max();
    //berechne ausdruck unter der wurzel, dann fallunterscheidung < 0?
    double discriminant = (c1+c2)*(c1+c2) / 4.  -  (c1*c1 + c2*c2 - ((h1*h1)/(speed*speed))) / 2.;
    if (discriminant < 0.) return -1.; //negative values indicate error //alternative: {std::cerr << "exit"; exit(1);} (sollte nicht vorkommen)
    if (h1 != h2) return -1.; //not yet with different h in x and y
    double rooteval = sqrt(discriminant);

    double ret = (c1+c2)/2. + rooteval;
    return ret;
}

inline double quadrSolutionMin (double c1, double c2, double h1, double h2, double speed) {
    if (c1 == std::numeric_limits<double>::max()) return std::numeric_limits<double>::max();
    if (c2 == std::numeric_limits<double>::max()) return std::numeric_limits<double>::max();
    //berechne ausdruck unter der wurzel, dann fallunterscheidung < 0?
    double discriminant = (c1+c2)*(c1+c2) / 4.  -  (c1*c1 + c2*c2 - ((h1*h1)/(speed*speed))) / 2.;
    if (discriminant < 0.) return -1.; //negative values indicate error //alternative: {std::cerr << "exit"; exit(1);}
    if (h1 != h2) return -1.; //not yet with different h in x and y
    double rooteval = sqrt(discriminant);

    double ret = (c1+c2)/2. - rooteval;
    return ret;
}

double FastMarcher::calcCostAt(int key) {
    directNeighbor neighbor = myGrid->getNeighbors(key);
    double* neighCost = new double[4];
    for (int i = 0; i < 4; ++i) {
        if (neighbor.key[i] > -1) { //valid neighbor
            //save cost values of neighbors, set uncalculated (<- negative) neighbor costs to max
            //bei der berechnung sollen alle negativen cost werte als std::numeric_limits<double>::max() genutzt werden
            neighCost[i] = (myCost[neighbor.key[i]] >= 0.) ? myCost[neighbor.key[i]] : std::numeric_limits<double>::max();
        } else {                    //invalid neighbor
            neighCost[i] = std::numeric_limits<double>::max();
        }
    }

    //berechne pro punkt alle vier kost-werte und waehle zu jedem punkt das minimum der vier
    //Problem: ich kann nicht double::max * double::max rechnen (geloest), ich muss richtigen gradient abspeichern
    double hx = myGrid->getHx();
    double hy = myGrid->getHy();
    double minCost = std::numeric_limits<double>::max();
    double oneOfFour = quadrSolutionMax(neighCost[0], neighCost[1], hx, hy, mySpeed[key]);
    if (oneOfFour == -1.) {
        //diskriminate bei quadrSolutionMax war negativ
        oneOfFour = std::numeric_limits<double>::max();
    } else if (oneOfFour < minCost) {
        minCost = oneOfFour;
        myGradient[key].SetX((neighCost[0] - minCost)/hx);
        myGradient[key].SetY((neighCost[1] - minCost)/hy);
    }

    oneOfFour = quadrSolutionMax(neighCost[2], neighCost[1], hx, hy, mySpeed[key]);
    if (oneOfFour == -1.) {
        //diskriminate bei quadrSolutionMax war negativ
        oneOfFour = std::numeric_limits<double>::max();
    } else if (oneOfFour < minCost) {
        minCost = oneOfFour;
        myGradient[key].SetX((minCost - neighCost[2])/hx);
        myGradient[key].SetY((neighCost[1] - minCost)/hy);
    }

    oneOfFour = quadrSolutionMax(neighCost[2], neighCost[3], hx, hy, mySpeed[key]);
    if (oneOfFour == -1.) {
        //diskriminate bei quadrSolutionMax war negativ
        oneOfFour = std::numeric_limits<double>::max();
    } else if (oneOfFour < minCost) {
        minCost = oneOfFour;
        myGradient[key].SetX((minCost - neighCost[2])/hx);
        myGradient[key].SetY((minCost - neighCost[3])/hy);
    }

    oneOfFour = quadrSolutionMax(neighCost[0], neighCost[3], hx, hy, mySpeed[key]);
    if (oneOfFour == -1.) {
        //diskriminate bei quadrSolutionMax war negativ
        oneOfFour = std::numeric_limits<double>::max();
    } else if (oneOfFour < minCost) {
        minCost = oneOfFour;
        myGradient[key].SetX((neighCost[0] - minCost)/hx);
        myGradient[key].SetY((minCost - neighCost[3])/hy);
    }

    // try to update by one-sided update if minCost noch <double>::max()
    if (minCost == std::numeric_limits<double>::max()) {
        if (neighCost[0] != std::numeric_limits<double>::max()) {
            oneOfFour = neighCost[0] + hx/mySpeed[key];
            if (oneOfFour < minCost) {
                minCost = oneOfFour;
                myGradient[key].SetX(-1./mySpeed[key]);
                myGradient[key].SetY(0.);
            }
        }

        if (neighCost[1] != std::numeric_limits<double>::max()) {
            oneOfFour = neighCost[1] + hy/mySpeed[key];
            if (oneOfFour < minCost) {
                minCost = oneOfFour;
                myGradient[key].SetX(0);
                myGradient[key].SetY(-1./mySpeed[key]);
            }
        }

        if (neighCost[2] != std::numeric_limits<double>::max()) {
            oneOfFour = neighCost[2] + hx/mySpeed[key];
            if (oneOfFour < minCost) {
                minCost = oneOfFour;
                myGradient[key].SetY(0);
                myGradient[key].SetX(1./mySpeed[key]);
            }
        }

        if (neighCost[3] != std::numeric_limits<double>::max()) {
            oneOfFour = neighCost[3] + hy/mySpeed[key];
            if (oneOfFour < minCost) {
                minCost = oneOfFour;
                myGradient[key].SetX(0);
                myGradient[key].SetY(1./mySpeed[key]);
            }
        }
    }
    //das minimum wird in dem cost array gespeichert, das neu berechnete element muss an die richtige stelle im baum
    if (minCost == std::numeric_limits<double>::max()) {
        //minCost noch immer unendlich: dieser fall darf nicht eintreten, da min. ein Nachbar einen myCost Wert hat und
        //spaetestens das oneSided Update mit diesem Nachbarn einen gueltigen Costwert in minCost ablegen sollte!
        std::cerr<< "kein gueltiges update in calcCostAt()";
    }
    myCost[key] = minCost;
    return minCost;
}

//////////////////////////////////////////////////////////////////////////////////////////
// HEAP Structure
//////////////////////////////////////////////////////////////////////////////////////////
