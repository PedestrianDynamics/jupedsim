/**
 * \file        FastMarching.h
 * \date        Jan 15, 2015
 * \version     --
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
 * \section given any geometry, this module shall create an appropriate mesh (point-wise, no cells)
 * to discretize domain and perform a fast-marching-algorithm.
 *
 *
 **/

#ifndef FASTMARCHING_H
#define FASTMARCHING_H

#include "../geometry/Building.h"

// geometric interpretation of index in "directNeighbor"
//          x (1)              ^ y
//          |                  |
// (2) x----0----x (0)         o---> x
//          |
//          x (3)

typedef struct directNeighbor_t {
    int key[4];
} directNeighbor;


//
class RectGrid
{
    public:
        RectGrid();
        virtual ~RectGrid();

        int setBoundaries(const double xMinA, const double yMinA,
                          const double xMaxA, const double yMaxA);
        int setBoundaries(const Point xy_min, const Point xy_max);
        int setSpacing(const double h_y, const double h_x);
        int createGrid();

        int getKeyAtXY(const double x, const double y) const;     //key = indef in (extern managed) array
        int getKeyAtIndex(const int i, const int j) const;  //key = index in 1D array by giving indices of 2D array
                                                            //      zB um schnell die Nachbarpunkte-keys zu finden

        Point getPointFromKey(const int key) const;
        Point getNearestGridPoint(const Point& currPoint) const;
        int getNearestGridPointsKey(const Point& currPoint) const;  //key = index in array
        //Point getNearestGridPoint2Dindices (const Point& currPoint) const;

        int isInnerPoint(const Point& testPoint) const;
        int isGridPoint(const Point& testPoint) const;

        int setAsInner(const Point& innerP); //input in xy world coordinates
        int setAsOuter(const Point& outerP); //input in xy world coordinates

        int getGridID() const;
        int getNumOfElements() const;
        directNeighbor getNeighbors(const Point& currPoint) const;
        directNeighbor getNeighbors(const int key) const;

#ifdef LINKEDGRIDS
        RectGrid* next;
#endif // LINKEDGRIDS

    protected:
        int gridID;
        int* isInner;
        int nGridpoints;
        double xMin, xMax;
        double yMin, yMax;
        double hx, hy;
        int iMax, jMax;
    private:

};


class FastMarcher
{
    public:
        FastMarcher();
        virtual ~FastMarcher();

        //grid related / first with rectangular meshes only

        //setzt zum einen die eigenen boundary-attribute des RectGrid mittels "setBoundaries"
        //und initialisiert die Slowness/Speedfunktion der Waende
        int setGeometry(const Building * const building);

        //for testing only
        int setGrid(RectGrid* givenGrid);
        int setSpeedArray(double* givenSpeed);
        int setGradientArray(Point* givenGradient);
        int setCostArray(double* givenCost);
        RectGrid* getGrid() {return myGrid;}

        //calculate floor field
        int calculateFloorfield();

        //access floorfield data
        Point getFloorfieldAt(const Point currPos) const;
        double getTimecostAt(const Point currPos) const;
        double getTimecostAt(const int x, const int y) const;
        double getSpeedAt(const Point currPos) const;
        double getSpeedAt(const int x, const int y) const;

    protected:
    private:
        RectGrid* myGrid;
        double* myCost;
        Point* myGradient;
        double* mySpeed;

};

#endif // FASTMARCHING_H
