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

#include <cmath>

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
    /**
     * Default constructor.
     */
    RectGrid() { this->_initialized = false; }

    /**
     * Copy constructor.
     * @param other RectGrid to copy.
     */
    RectGrid(const RectGrid & other)
    {
        _nPoints     = other._nPoints;
        _xMin        = other._xMin;
        _yMin        = other._yMin;
        _xMax        = other._xMax;
        _yMax        = other._yMax;
        _cellsizeX   = other._cellsizeX;
        _cellsizeY   = other._cellsizeY;
        _iMax        = other._iMax;
        _jMax        = other._jMax;
        _initialized = other._initialized;
    }

    /**
     * Constructor.
     * @param nPointsArg Number of grid cells.
     * @param xMinArg Minimal x coordinate of grid.
     * @param yMinArg Minimal y coordinate of grid.
     * @param xMaxArg Maximum x coordinate of grid.
     * @param yMaxArg Maximum y coordinate of grid.
     * @param hxArg Grid cell size in x direction.
     * @param hyArg Grid cell size in y direction.
     * @param iMaxArg Maximum index in grid in x direction.
     * @param jMaxArg Maximum index in grid in y direction.
     * @param isInitializedArg Grid is initialized.
     */
    RectGrid(
        long int nPointsArg,
        double xMinArg,
        double yMinArg,
        double xMaxArg,
        double yMaxArg,
        double hxArg,
        double hyArg,
        long int iMaxArg,
        long int jMaxArg,
        bool isInitializedArg)
    {
        _nPoints     = nPointsArg;
        _xMin        = xMinArg;
        _yMin        = yMinArg;
        _xMax        = xMaxArg;
        _yMax        = yMaxArg;
        _cellsizeX   = hxArg;
        _cellsizeY   = hyArg;
        _iMax        = iMaxArg;
        _jMax        = jMaxArg;
        _initialized = isInitializedArg;
    }

    /**
     * Default deconstructor
     */
    virtual ~RectGrid() = default;

    [[nodiscard]] long int GetnPoints() const { return _nPoints; }

    [[nodiscard]] double GetxMin() const { return _xMin; }

    [[nodiscard]] double GetyMin() const { return _yMin; }

    [[nodiscard]] double GetxMax() const { return _xMax; }

    [[nodiscard]] double GetyMax() const { return _yMax; }

    /**
     * Returns the maximum index in x direction.
     * @return maximum index in x direction.
     */
    [[nodiscard]] long int GetiMax() const { return _iMax; }

    /**
     * Returns the maximum index in y direction.
     * @return maximum index in y direction.
     */
    [[nodiscard]] long int GetjMax() const { return _jMax; }

    /**
     * Returns the grid cell size in x direction.
     * @return grid cell size in x direction.
     */
    [[nodiscard]] double Gethx() const { return _cellsizeX; }

    /**
     * Returns the grid cell size in y direction.
     * @return grid cell size in y direction.
     */
    [[nodiscard]] double Gethy() const { return _cellsizeY; }

    /**
     * Retunrs the x coordinate of cell \p key in grid.
     * @param key Index of cell in grid.
     * @return X coordinate of cell \p key
     */
    [[nodiscard]] double GetXFromKey(long int key) const
    {
        return std::fmod(key, _iMax) * _cellsizeX + _xMin;
    }

    /**
     * Retunrs the y coordinate of cell \p key in grid.
     * @param key Index of cell in grid.
     * @return Y coordinate of cell \p key
     */
    [[nodiscard]] double GetYFromKey(long int key) const
    {
        return (static_cast<double>(key) / _iMax) * _cellsizeY + _yMin;
    }

    /**
     * Returns the index in x direction of cell \p key in grid.
     * @param key Index of cell in grid.
     * @return Index in x direction of cell \p key.
     */
    [[nodiscard]] long int GetIFromKey(long int key) const { return std::fmod(key, _iMax); }

    /**
     * Returns the index in y direction of cell \p key in grid.
     * @param key Index of cell in grid.
     * @return Index in y direction of cell \p key.
     */
    [[nodiscard]] long int GetJFromKey(long int key) const
    {
        return static_cast<long int>(key / _iMax);
    }

    /**
     * Returns the key to a x- and y-coordinate.
     * @param x x-coordiante.
     * @param y y-coordinate.
     * @return Key to point (
     */
    [[nodiscard]] long int GetKeyAtXY(const double x, const double y) const
    {
        long int i = std::lround((x - _xMin) / _cellsizeX);
        long int j = std::lround((y - _yMin) / _cellsizeY);

        if(IncludesPoint(Point(x, y))) {
            return (j * _iMax + i); // 0-based; index of (closest gridpoint)}
        } else {
            if(x < _xMin) {
                Logging::Error(fmt::format(check_fmt("Out of left bound by: {:.2f}"), (_xMin - x)));
                i = 0;
            }
            if(x > _xMax) {
                Logging::Error(
                    fmt::format(check_fmt("Out of right bound by: {:.2f}"), (x - _xMax)));
                i = _iMax;
            }
            if(y < _yMin) {
                Logging::Error(
                    fmt::format(check_fmt("lower of lower bound by: {:.2f}"), (_yMin - y)));
                j = 0;
            }
            if(y > _yMax) {
                Logging::Error(
                    fmt::format(check_fmt("lower of upper bound by: {:.2f}"), (y - _yMax)));
                j = _jMax;
            }
            return (j * _iMax + i);
        }
    }

    /**
     * Returns the key to point \p p.
     * @param p Point in grid.
     * @return Key of \p p in grid, if \p p not in grid key of closest point inside grid.
     */
    [[nodiscard]] long int GetKeyAtPoint(const Point p) const { return GetKeyAtXY(p._x, p._y); }

    /**
     * Sets the boundaries of the grid if not already set.
     * @param xMin Minimal x value in grid.
     * @param yMin Minimal y value in grid.
     * @param xMax Maximum x value in grid.
     * @param yMax Maximum y value in grid.
     */
    void SetBoundaries(const double xMin, const double yMin, const double xMax, const double yMax)
    {
        if(!_initialized) {
            _xMin = xMin;
            _xMax = xMax;
            _yMin = yMin;
            _yMax = yMax;
        }
    }

    /**
     * Sets the grid cell size in x and y direction.
     * @param h_x Grid cell size in x direction.
     * @param h_y Grid cell size in y direction.
     */
    void SetSpacing(const double h_x, const double h_y)
    {
        if(!_initialized) {
            _cellsizeY = h_y;
            _cellsizeX = h_x;
        }
    }

    /**
     * Creates the grid.
     * @pre \a _xMin, \a _xMax, \a _cellsizeX, \a _yMin, \a _yMax, \a _cellsizeY need to be set.
     * @post \a _iMax, \a _jMax, \a _nPoints are set.
     */
    void CreateGrid()
    { //what if cast chops off float, if any changes: GetXFromKey still correct?
        if(!_initialized) {
            _iMax = (long int) ((_xMax - _xMin) / _cellsizeX) +
                    2; //check plus 2 (one for ceil, one for starting point)
            _jMax    = (long int) ((_yMax - _yMin) / _cellsizeY) + 2;
            _nPoints = _iMax * _jMax;
            //@todo: see if necessary to align _xMax/_yMax
            _xMax        = _xMin + _iMax * _cellsizeX;
            _yMax        = _yMin + _jMax * _cellsizeY;
            _initialized = true;
        }
    }

    /**
     * Returns the closest grid point to \p currPoint.
     * @param currPoint Point of which the closest grid point is desired.
     * @return Closest grid point to \p currPoint.
     */
    [[nodiscard]] Point GetNearestGridPoint(const Point & currPoint) const
    {
        if(!IncludesPoint(currPoint)) {
            Logging::Error(fmt::format(check_fmt("ERROR 3 in RectGrid::GetKeyAtPoint with:")));
            Logging::Error(
                fmt::format(check_fmt("Point: {:.2f} {:.2f}"), currPoint._x, currPoint._y));
            Logging::Error(fmt::format(check_fmt("_xMin, _yMin: {:.2f} {:.2f}"), _xMin, _yMin));
            Logging::Error(fmt::format(check_fmt("_xMax, _yMax: {:.2f} {:.2f}"), _xMax, _yMax));
            Logging::Error(fmt::format(check_fmt(
                "Point is out of Grid-Scope, Tip: check if correct Floorfield is called")));

            return Point(-7, -7);
        }
        long int i = std::lround((currPoint._x - _xMin) / _cellsizeX);
        long int j = std::lround((currPoint._y - _yMin) / _cellsizeY);

        return Point(
            static_cast<double>(i) * _cellsizeX + _xMin,
            static_cast<double>(j) * _cellsizeY + _yMin);
    }

    /**
     * Returns the point of grid cell \p key.
     * @param key Key of the grid cell.
     * @return Point at grid cell \p key.
     */
    [[nodiscard]] Point GetPointFromKey(const long int key) const
    {
        long int i = key % _iMax;
        long int j = key / _iMax; //integer division

        return Point(i * _cellsizeX + _xMin, j * _cellsizeY + _yMin);
    }

    /**
     * Returns the indices of the neighboring cells of cell with key \p key.
     * @param key Key of cell which neighbors should be returned.
     * @return the indices of the direct neighbors of cells \p key.
     */
    [[nodiscard]] directNeighbor GetNeighbors(const long int key) const
    {
        directNeighbor neighbors = {{-1, -1, -1, -1}}; //curleybrackets for struct, then for int[4]
        long int i               = GetIFromKey(key);
        long int j               = GetJFromKey(key);

        //right                       //-2 marks invalid neighbor
        neighbors.key[0] = (i == (_iMax - 1)) ? -2 : (j * _iMax + i + 1);
        //upper
        neighbors.key[1] = (j == (_jMax - 1)) ? -2 : ((j + 1) * _iMax + i);
        //left
        neighbors.key[2] = (i == 0) ? -2 : (j * _iMax + i - 1);
        //lower
        neighbors.key[3] = (j == 0) ? -2 : ((j - 1) * _iMax + i);

        return neighbors;
    }

    /**
     * Checks if \p point is included in grid.
     * @param point Point to check.
     * @return \p point is included in grid.
     */
    [[nodiscard]] bool IncludesPoint(const Point & point) const
    {
        return !(
            (point._x < (_xMin - _cellsizeX / 2)) || (point._x > (_xMax + _cellsizeX / 2)) ||
            (point._y < (_yMin - _cellsizeY / 2)) || (point._y > (_yMax + _cellsizeY / 2)));
    }

private:
    /**
     * Number of grid points.
     */
    long int _nPoints{};

    /**
     * Minimal x coordinate included in grid.
     */
    double _xMin{};

    /**
     * Maximum x coordinate included in grid.
     */
    double _xMax{};

    /**
     * Minimal y coordinate included in grid.
     */
    double _yMin{};

    /**
     * Maximum y coordinate included in grid.
     */
    double _yMax{};

    /**
     * Grid cell size in x direction.
     */
    double _cellsizeX{};

    /**
     * Grid cell size in y direction.
     */
    double _cellsizeY{};

    /**
     * Maximum index in x direction.
     */
    long int _iMax{};

    /**
     * Maximum index in y direction.
     */
    long int _jMax{};

    /**
     * Grid is initialized.
     */
    bool _initialized;
};
