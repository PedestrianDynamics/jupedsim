/**
 * \file        RectGrid.h
 * \date        Mar 05, 2014
 * \version     v0.5
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
 * Header of class for rectangular grids.
 *
 *
 **/

#ifndef RECTGRID_H
#define RECTGRID_H

#include "../geometry/Point.h"

// geometric interpretation of index in "directNeighbor"
//          x (1)              ^ y
//          |                  |
// (2) x----0----x (0)         o---> x
//          |
//          x (3)

typedef struct directNeighbor_t {
    int key[4];
} directNeighbor;


class RectGrid
{
    public:
        RectGrid() {
            isInitialized = false;
        }
        virtual ~RectGrid();
        RectGrid(const RectGrid& other) {
            nPoints = other.nPoints;
            xMin = other.xMin;
            yMin = other.yMin;
            xMax = other.xMax;
            yMax = other.yMax;
            hx = other.hx;
            hy = other.hy;
            iMax = other.iMax;
            jMax = other.jMax;
            isInitialized = other.isInitialized;
        }

        unsigned long int GetnPoints() const { return nPoints; }
        //void SetnPoints(unsigned long int val) { nPoints = val; }
        double GetxMin() const { return xMin; }
        void SetxMin(double val) { if (!isInitialized) xMin = val; }
        double GetyMin() const { return yMin; }
        void SetyMin(double val) { if (!isInitialized) yMin = val; }
        double GetxMax() const { return xMax; }
        void SetxMax(double val) { if (!isInitialized) xMax = val; }
        double GetyMax() const { return yMax; }
        void SetyMax(double val) { if (!isInitialized) yMax = val; }
        unsigned long int GetiMax() const { return iMax; }
        //void SetiMax(unsigned long int val) { iMax = val; }
        unsigned long int GetjMax() const { return jMax; }
        //void SetjMax(unsigned long int val) { jMax = val; }
        double Gethx() const { return hx; }
        //void Sethx(double val) { hx = val; }
        double Gethy() const { return hy; }
        //void Sethy(double val) { hy = val; }

        double get_x_fromKey (unsigned long int key) const { return (key%iMax)*hx+xMin}
        double get_y_fromKey (unsigned long int key) const { return (key/iMax)*hy+yMin}
        double get_i_fromKey (unsigned long int key) const { return (key%iMax) }
        double get_j_fromKey (unsigned long int key) const { return (key/iMax) }

        void setBoundaries(const double xMinA, const double yMinA,
                           const double xMaxA, const double yMaxA) {
            if (!isInitialized) {
                xMin = xMinA;
                xMax = xMaxA;
                yMin = yMinA;
                yMax = yMaxA;
            }
        }

        void setBoundaries(const Point xy_min, const Point xy_max) {
            if (!isInitialized) {
                xMin = xy_min.GetX();
                xMax = xy_max.GetX();
                yMin = xy_min.GetY();
                yMax = xy_max.GetY();
            }
        }

        void setSpacing(const double h_x, const double h_y) {
            if (!isInitialized) {
                hy = h_y;
                hx = h_x;
            }
        }

        void createGrid(){ // @todo ar.graf : what if cast chops off float, if any changes: get_x_fromKey still correct?
            if (!isInitialized) {
                iMax = (unsigned long int)((xMax-xMin)/hx) + 1;
                jMax = (unsigned long int)((yMax-yMin)/hy) + 1;
                nPoints = iMax * jMax;
                isInitialized = true;
            }
        }

        Point getNearestGridPoint(const Point& currPoint) const {
            if ((currPoint.GetX() > xMax) || (currPoint.GetY() > yMax))
                return Point(-7, -7); // @todo: ar.graf : find good false indicator
            unsigned long int i = (unsigned long int)(((currPoint.GetX()-xMin)/hx)+.5);
            unsigned long int j = (unsigned long int)(((currPoint.GetY()-yMin)/hy)+.5);
            return Point(i*hx+xMin, j*hy+yMin);
        }

        directNeighbor getNeighbors(const unsigned long int key) const {
            directNeighbor neighbors = {{-1, -1, -1, -1}}; //curleybrackets for struct, then for int[4]
            unsigned long int i = get_i_fromKey(key);
            unsigned long int j = get_j_fromKey(key);

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

    protected:
    private:
        unsigned long int nPoints;
        double xMin;
        double yMin;
        double xMax;
        double yMax;
        double hx;
        double hy;
        unsigned long int iMax;
        unsigned long int jMax;
        bool isInitialized;
};

#endif // RECTGRID_H
