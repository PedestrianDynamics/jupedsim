/**
 * File:   Point.h
 *
 * Created on 30. September 2010, 09:21
 *
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

#ifndef _POINT_H
#define	_POINT_H
#include <string>

class Point {
public:
    double _x;
    double _y;

public:
    // constructors
    Point();
    Point(double x, double y);
    Point(const Point& orig);


    /**
     * Set/Get the x component
     */
    void SetX(double x);

    /**
     * Set/Get the y component
     */
    void SetY(double y);


    /**
     * Set/Get the x component
     */
    double GetX() const;

    /**
     * Set/Get the y component
     */
    double GetY() const;

    /// Norm
    double Norm() const;
    /// Norm square
    double NormSquare() const;
     /// normalized vector
    Point Normalized() const;
    /// dot product
    double ScalarP(const Point& v) const;
    /// determinant of the square matrix formed by the vectors [ this, v]
    double Det(const Point& v) const;
    /// translation and rotation in Ellipse coordinate system
    Point CoordTransToEllipse(const Point& center, double cphi, double sphi) const;
    /// translation and rotation in cartesian system
    Point CoordTransToCart(const Point& center, double cphi, double sphi) const;
    /// rotate the vector by theta
    Point Rotate(double ctheta, double stheta) const;


    // operators
    /// addition
    const Point operator+(const Point& p) const;
    /// substraction
    const Point operator-(const Point& p) const;
    /// equal
    bool operator==(const Point& p) const;
    /// not equal
    bool operator!=(const Point& p) const;

    /// nice formating of the point
    std::string toString() const;
};

/// multiplication
const Point operator*(const Point& p, const double f);
/// division
const Point operator/(const Point& p, const double f);

#endif	/* _POINT_H */

