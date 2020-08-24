/**
 * \file        Point.cpp
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


#include "Point.h"

#include "../general/Macros.h"

#include <Logger.h>
#include <cmath>
#include <iostream>
#include <sstream>
/************************************************************
  Konstruktoren
 ************************************************************/
Point::Point() : _x(0), _y(0) {}

Point::Point(double x, double y) : _x(x), _y(y) {}

std::string Point::toString() const
{
    std::stringstream tmp;
    tmp << "( " << _x << " : " << _y << " )";
    return tmp.str();
}

void Point::SetX(double x)
{
    _x = x;
}

void Point::SetY(double y)
{
    _y = y;
}

double Point::GetX() const
{
    return _x;
}

double Point::GetY() const
{
    return _y;
}

double Point::Norm() const
{
    return sqrt(NormSquare());
}

double Point::NormMolified() const
{
    double const eps_sq = 0.1;
    return sqrt(NormSquare() + eps_sq);
}

Point Point::NormalizedMolified() const
{
    double norm = NormMolified();
    if(norm > J_EPS_GOAL)
        return (Point(_x, _y) / norm);
    else
        return Point(0.0, 0.0);
}


Point Point::Normalized() const
{
    double norm = Norm();
    if(norm > J_EPS)
        return (Point(_x, _y) / norm);
    else
        return Point(0.0, 0.0);
}


/* Transformiert die "normalen" Koordinaten in Koordinaten der Ellipse
 * dazu verschieben des Koordinaten Ursprungs in Center und anschliessend drehen um phi
 * alle Punkte müssen in "normalen" Koordinaten gegeben sein
 * center: Center der Ellipse in deren System transformiert werden soll
 * phi: Winkel der Ellipse in deren System transformiert werden soll
 * */


/*coordinate transformation of the point P(x,y) expressed in coord system S1 to a new coord. sys S2

           A
           *
         |     S_2
     \   |   /
 |    \  |  /
 |     \ | /^phi
 | yc___\ /_)_________ S_3
 |       O1
 |       |
 |       |
 |       xc
 |
 |___________________________
O
S_1


////////////////////////////////////
S_1 is cartesian coordinate system!!
////////////////////////////////////

  input:
  - (x,y)        :  coordinates of the point A in S_1
  - (xc,yc)      : coordinate of the center in the  S_1 (Center of Ellipse)
  - phi          : angle between the S_1 and S_2

  output:
  +  (xnew,ynew) : new coordinate of the point A in the coord. sys S2

OA = OO1 + O1A

 [x ; y] = [xc ; yc] +  [x_3 ; y_3]   : (1) ( with [x_i ; y_i] coordinats of P in S_i and i in {1,2,3} )

[x_2 ; y_2] = M(phi) * [x_3 ; y_3]  : (2)


(1) in (2)--->

-->  [x_2 ; y_2] = M(phi) * ([x ; y] - [xc ; yc])



after rotation:
OC = OO1 +O1C
OC  = -O1O +O1C

xnew = -xc + x

*/
Point Point::TransformToEllipseCoordinates(const Point & center, double cphi, double sphi) const
{
    Point p = Point(_x, _y);
    return (p - center).Rotate(cphi, -sphi);
}

/*
This is the reverse funktion of TransformToEllipseCoordinates(),
where the coord. of a point are transformated to cart. coord.

 input:
  - (x,y)        :  coordinates of the point P in S_2
  - (xc,yc)      : coordinate of the center in the  S_1 (Center of Ellipse)
  - phi          : angle between the S_1 and S_2

  output:
  +  (xnew,ynew) : new coordinate of the point P in the coord. sys S_1

[x_2 ; y_2] = M(phi) * ([x ; y] - [xc ; yc]) (see comments in CoordTransToEllipse() )


----> [x ; y] =  M(-phi) * [x_2 ; y_2] +  [xc ; yc]

*/

Point Point::TransformToCartesianCoordinates(const Point & center, double cphi, double sphi) const
{
    Point p = Point(_x, _y);
    return (p.Rotate(cphi, sphi) + center);
}

/*rotate a two-dimensional vector by an angle of theta

Rotation-matrix=[cos(theta)  -sin(theta)]
                [ sin(theta)  cos(theta)]

*/
Point Point::Rotate(double ctheta, double stheta) const
{
    return Point(_x * ctheta - _y * stheta, _x * stheta + _y * ctheta);
}

//  sum
const Point Point::operator+(const Point & p) const
{
    //return Point(_x + p.GetX(), _y + p.GetY());
    return Point(_x + p._x, _y + p._y);
}

// sub
const Point Point::operator-(const Point & p) const
{
    // return Point(_x - p.GetX(), _y - p.GetY());
    return Point(_x - p._x, _y - p._y);
}

// equal
bool Point::operator==(const Point & p) const
{
    //    return (fabs(_x - p.GetX()) < J_EPS && fabs(_y - p.GetY()) < J_EPS);
    return (fabs(_x - p._x) < J_EPS && fabs(_y - p._y) < J_EPS);
}

// not equal
bool Point::operator!=(const Point & p) const
{
    //return (fabs(_x - p.GetX()) > J_EPS || fabs(_y - p.GetY()) > J_EPS);
    return (fabs(_x - p._x) > J_EPS || fabs(_y - p._y) > J_EPS);
}


// multiplication with scalar
const Point operator*(const Point & p, double f)
{
    //return Point(p.GetX() * f, p.GetY() * f);
    return Point(p._x * f, p._y * f);
}


// Assignment
Point & Point::operator+=(const Point & p)
{
    _x += p._x;
    _y += p._y;
    return *this;
}


// divition with scalar
const Point operator/(const Point & p, double f)
{
    if(f > J_EPS * J_EPS)
        return Point(p._x / f, p._y / f);
    else {
        LOG_WARNING("Point::operator/ dividend {} is too small. Using 1 instead.", f);
        return Point(p._x, p._y);
    }
    //return Point(p.GetX() / f, p.GetY() / f);
}
