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
#pragma once

#include "general/Format.h"
#include "general/Logger.h"
#include "geometry/Point.h"

// geometric interpretation of index in "directNeighbor"
//          x (1)              ^ y
//          |                  |
// (2) x----0----x (0)         o---> x
//          |
//          x (3)

typedef struct directNeighbor_t {
    long int key[4];
} directNeighbor;


class RectGrid
{
public:
    RectGrid() { this->isInitialized = false; }

    virtual ~RectGrid() = default;

    RectGrid(const RectGrid & other)
    {
        nPoints       = other.nPoints;
        xMin          = other.xMin;
        yMin          = other.yMin;
        xMax          = other.xMax;
        yMax          = other.yMax;
        hx            = other.hx;
        hy            = other.hy;
        iMax          = other.iMax;
        jMax          = other.jMax;
        isInitialized = other.isInitialized;
    }

    RectGrid(
        long int nPointsArg,
        double xMinArg,
        double yMinArg,
        double xMaxArg,
        double yMaxArg,
        double hxArg,
        double hyArg,
        long int iMaxArg, // indices must be smaller than iMax
        long int jMaxArg, // indices must be smaller than jMax
        bool isInitializedArg)
    {
        nPoints       = nPointsArg; // must not be corrupted..
        xMin          = xMinArg;
        yMin          = yMinArg;
        xMax          = xMaxArg;
        yMax          = yMaxArg;
        hx            = hxArg;
        hy            = hyArg;
        iMax          = iMaxArg;
        jMax          = jMaxArg;
        isInitialized = isInitializedArg;
    }

    [[nodiscard]] long int GetnPoints() const { return nPoints; }
    //void SetnPoints(long int val) { nPoints = val; }
    [[nodiscard]] double GetxMin() const { return xMin; }
    void SetxMin(double val)
    {
        if(!isInitialized)
            xMin = val;
    }
    [[nodiscard]] double GetyMin() const { return yMin; }
    void SetyMin(double val)
    {
        if(!isInitialized)
            yMin = val;
    }
    [[nodiscard]] double GetxMax() const { return xMax; }
    void SetxMax(double val)
    {
        if(!isInitialized)
            xMax = val;
    }
    [[nodiscard]] double GetyMax() const { return yMax; }
    void SetyMax(double val)
    {
        if(!isInitialized)
            yMax = val;
    }
    [[nodiscard]] long int GetiMax() const { return iMax; }

    [[nodiscard]] long int GetjMax() const { return jMax; }

    [[nodiscard]] double Gethx() const { return hx; }

    [[nodiscard]] double Gethy() const { return hy; }


    [[nodiscard]] double get_x_fromKey(long int key) const { return (key % iMax) * hx + xMin; }
    [[nodiscard]] double get_y_fromKey(long int key) const { return (key / iMax) * hy + yMin; }
    [[nodiscard]] double get_i_fromKey(long int key) const { return (key % iMax); }
    [[nodiscard]] double get_j_fromKey(long int key) const { return (key / iMax); }

    [[nodiscard]] long int getKeyAtXY(const double x, const double y) const
    { //key = index in (extern managed) array
        //Point nearest = getNearestGridPoint(Point(x,y));
        long int i = (long int) (((x - xMin) / hx) + .5);
        long int j = (long int) (((y - yMin) / hy) + .5);
        if((i >= 0) && (i <= iMax) && (j >= 0) &&
           (j <= jMax))            //@todo: ar.graf: check in #ifdef block
            return (j * iMax + i); // 0-based; index of (closest gridpoint)

        Logging::Error(fmt::format(check_fmt("ERROR 1 in RectGrid::getKeyAtPoint with:")));
        Logging::Error(fmt::format(check_fmt("Point: {:.2f} {:.2f}"), x, y));
        Logging::Error(fmt::format(check_fmt("xMin, yMin: {:.2f} {:.2f}"), xMin, yMin));
        Logging::Error(fmt::format(check_fmt("xMax, yMax: {:.2f} {:.2f}"), xMax, yMax));
        Logging::Error(fmt::format(
            check_fmt("Point is out of Grid-Scope, Tip: check if correct Floorfield is called")));

        return -1; // invalid indices
    }

    [[nodiscard]] long int getKeyAtPoint(const Point p) const
    {
        auto i = (long int) (((p._x - xMin) / hx) + .5);
        auto j = (long int) (((p._y - yMin) / hy) + .5);
        //if ((i >= 0) && (i <= iMax) && (j >= 0) && (j <= jMax)) //@todo: ar.graf: check in #ifdef block
        if(includesPoint(p))       //@todo: ar.graf: this if must be made work
            return (j * iMax + i); // 0-based; index of (closest gridpoint)
        else {
            if(p._x < xMin) {
                Logging::Error(
                    fmt::format(check_fmt("Out of left bound by: {:.2f}"), (xMin - p._x)));
                i = 0;
            }
            if(p._x > xMax) {
                Logging::Error(
                    fmt::format(check_fmt("Out of right bound by: {:.2f}"), (p._x - xMax)));
                i = iMax;
            }
            if(p._y < yMin) {
                Logging::Error(
                    fmt::format(check_fmt("lower of lower bound by: {:.2f}"), (yMin - p._y)));
                j = 0;
            }
            if(p._y > yMax) {
                Logging::Error(
                    fmt::format(check_fmt("lower of upper bound by: {:.2f}"), (p._y - yMax)));
                j = jMax;
            }
            return (j * iMax + i);
        }
    }

    void
    setBoundaries(const double xMinA, const double yMinA, const double xMaxA, const double yMaxA)
    {
        if(!isInitialized) {
            xMin = xMinA;
            xMax = xMaxA;
            yMin = yMinA;
            yMax = yMaxA;
        }
    }

    void setBoundaries(const Point xy_min, const Point xy_max)
    {
        if(!isInitialized) {
            xMin = xy_min._x;
            xMax = xy_max._x;
            yMin = xy_min._y;
            yMax = xy_max._y;
        }
    }

    void setSpacing(const double h_x, const double h_y)
    {
        if(!isInitialized) {
            hy = h_y;
            hx = h_x;
        }
    }

    void createGrid()
    { //what if cast chops off float, if any changes: get_x_fromKey still correct?
        if(!isInitialized) {
            iMax = (long int) ((xMax - xMin) / hx) +
                   2; //check plus 2 (one for ceil, one for starting point)
            jMax    = (long int) ((yMax - yMin) / hy) + 2;
            nPoints = iMax * jMax;
            //@todo: see if necessary to align xMax/yMax
            xMax          = xMin + iMax * hx;
            yMax          = yMin + jMax * hy;
            isInitialized = true;
        }
    }

    [[nodiscard]] Point getNearestGridPoint(const Point & currPoint) const
    {
        //if ((currPoint._x > xMax) || (currPoint._y > yMax) ||
        //    (currPoint._x < xMin) || (currPoint._y < yMin)) {
        if(!includesPoint(currPoint)) {
            Logging::Error(fmt::format(check_fmt("ERROR 3 in RectGrid::getKeyAtPoint with:")));
            Logging::Error(
                fmt::format(check_fmt("Point: {:.2f} {:.2f}"), currPoint._x, currPoint._y));
            Logging::Error(fmt::format(check_fmt("xMin, yMin: {:.2f} {:.2f}"), xMin, yMin));
            Logging::Error(fmt::format(check_fmt("xMax, yMax: {:.2f} {:.2f}"), xMax, yMax));
            Logging::Error(fmt::format(check_fmt(
                "Point is out of Grid-Scope, Tip: check if correct Floorfield is called")));

            return Point(-7, -7);
        }
        long int i = (long int) (((currPoint._x - xMin) / hx) + .5);
        long int j = (long int) (((currPoint._y - yMin) / hy) + .5);
        return Point(i * hx + xMin, j * hy + yMin);
    }

    [[nodiscard]] Point getPointFromKey(const long int key) const
    {
        long int i = key % iMax;
        long int j = key / iMax; //integer division

        return Point(i * hx + xMin, j * hy + yMin);
    }

    [[nodiscard]] directNeighbor getNeighbors(const long int key) const
    {
        directNeighbor neighbors = {{-1, -1, -1, -1}}; //curleybrackets for struct, then for int[4]
        long int i               = get_i_fromKey(key);
        long int j               = get_j_fromKey(key);

        //right                       //-2 marks invalid neighbor
        neighbors.key[0] = (i == (iMax - 1)) ? -2 : (j * iMax + i + 1);
        //upper
        neighbors.key[1] = (j == (jMax - 1)) ? -2 : ((j + 1) * iMax + i);
        //left
        neighbors.key[2] = (i == 0) ? -2 : (j * iMax + i - 1);
        //lower
        neighbors.key[3] = (j == 0) ? -2 : ((j - 1) * iMax + i);

        return neighbors;
    }

    [[nodiscard]] bool includesPoint(const Point & point) const
    {
        return !(
            (point._x < (xMin - hx / 2)) || (point._x > (xMax + hx / 2)) ||
            (point._y < (yMin - hy / 2)) || (point._y > (yMax + hy / 2)));
    }

protected:
private:
    long int nPoints{};
    double xMin{};
    double yMin{};
    double xMax{};
    double yMax{};
    double hx{};
    double hy{};
    long int iMax{}; // indices must be smaller than iMax
    long int jMax{}; // indices must be smaller than jMax
    bool isInitialized;
};
