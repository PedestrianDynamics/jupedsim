/**
 * \file        Point.h
 * \date        Sep 30, 2010
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


#ifndef _POINT_H
#define _POINT_H
#include <string>

class Point
{
public:
    double _x;
    double _y;

public:
    // constructors
    Point();
    Point(double x, double y);
    Point(const Point & orig);


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

    /// Norm molified see Koester2013
    double NormMolified() const;

    /// Norm square
    inline double NormSquare() const { return ScalarProduct(*this); }
    /// normalized vector
    Point Normalized() const;
    /// normalized vector usinf NormMolified
    Point NormalizedMolified() const;
    /// dot product
    inline double ScalarProduct(const Point & v) const { return _x * v._x + _y * v._y; }

    // since we have only 2D vectors (may be changed in the future), this function returns a scalar
    // (basically the third component of the vector (0,0,z) )
    inline double CrossProduct(const Point & p) const { return Determinant(p); }


    /// determinant of the square matrix formed by the vectors [ this, v]
    inline double Determinant(const Point & v) const { return _x * v._y - _y * v._x; }

    /// translation and rotation in Ellipse coordinate system
    Point TransformToEllipseCoordinates(const Point & center, double cphi, double sphi) const;
    /// translation and rotation in cartesian system
    Point TransformToCartesianCoordinates(const Point & center, double cphi, double sphi) const;
    /// rotate the vector by theta
    Point Rotate(double ctheta, double stheta) const;


    // operators
    /// addition
    const Point operator+(const Point & p) const;
    /// substraction
    const Point operator-(const Point & p) const;
    /// equal
    bool operator==(const Point & p) const;
    /// not equal
    bool operator!=(const Point & p) const;
    /// Assignement
    Point & operator+=(const Point & p);
    /// nice formating of the point
    std::string toString() const;
};

/// multiplication
const Point operator*(const Point & p, const double f);
/// division
const Point operator/(const Point & p, const double f);

#endif /* _POINT_H */
