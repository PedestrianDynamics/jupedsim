// Copyright © 2012-2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "Point.hpp"

#include "Macros.hpp"

#include <Logger.hpp>
#include <limits>

double Point::Norm() const
{
    return sqrt(NormSquare());
}

Point Point::Normalized() const
{
    return std::get<1>(NormAndNormalized());
}

std::tuple<double, Point> Point::NormAndNormalized() const
{
    const double norm = Norm();
    if(norm > std::numeric_limits<double>::epsilon())
        return std::make_tuple(norm, (Point(x, y) / norm));
    else
        return std::make_tuple(0.0, Point(0.0, 0.0));
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

 [x ; y] = [xc ; yc] +  [x_3 ; y_3]   : (1) ( with [x_i ; y_i] coordinats of P in S_i and i in
{1,2,3} )

[x_2 ; y_2] = M(phi) * [x_3 ; y_3]  : (2)


(1) in (2)--->

-->  [x_2 ; y_2] = M(phi) * ([x ; y] - [xc ; yc])



after rotation:
OC = OO1 +O1C
OC  = -O1O +O1C

xnew = -xc + x

*/
Point Point::TransformToEllipseCoordinates(const Point& center, double cphi, double sphi) const
{
    Point p = Point(x, y);
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

Point Point::TransformToCartesianCoordinates(const Point& center, double cphi, double sphi) const
{
    Point p = Point(x, y);
    return (p.Rotate(cphi, sphi) + center);
}

/*rotate a two-dimensional vector by an angle of theta

Rotation-matrix=[cos(theta)  -sin(theta)]
                [ sin(theta)  cos(theta)]

*/
Point Point::Rotate(double ctheta, double stheta) const
{
    return Point(x * ctheta - y * stheta, x * stheta + y * ctheta);
}

Point Point::Rotate90Deg() const
{
    return {-y, x};
}

bool Point::IsUnitLength() const
{
    return std::abs(1 - NormSquare()) <= std::numeric_limits<double>::epsilon();
}

const Point Point::operator+(const Point& p) const
{
    return Point(x + p.x, y + p.y);
}

const Point Point::operator-(const Point& p) const
{
    return Point(x - p.x, y - p.y);
}

bool Point::operator==(const Point& p) const
{
    return x == p.x && y == p.y;
}

bool Point::operator!=(const Point& p) const
{
    return !(*this == p);
}

const Point operator*(const Point& p, double f)
{
    return Point(p.x * f, p.y * f);
}

Point& Point::operator+=(const Point& p)
{
    x += p.x;
    y += p.y;
    return *this;
}

const Point operator/(const Point& p, double f)
{
    static auto constexpr eps =
        std::numeric_limits<double>::epsilon() * std::numeric_limits<double>::epsilon();
    if(f > eps)
        return Point(p.x / f, p.y / f);
    else {
        LOG_WARNING("Point::operator/ dividend {} is too small. Using 1 instead.", f);
        return Point(p.x, p.y);
    }
}

bool Point::operator<(const Point& rhs) const
{
    if(x < rhs.x)
        return true;
    else if((x == rhs.x) && (y < rhs.y))
        return true;
    return false;
}

bool Point::operator>(const Point& rhs) const
{
    return rhs < *this;
}

bool Point::operator<=(const Point& rhs) const
{
    return !(rhs < *this);
}

bool Point::operator>=(const Point& rhs) const
{
    return !(*this < rhs);
}

double Distance(const Point& point1, const Point& point2)
{
    return (point1 - point2).Norm();
}

double DistanceSquared(const Point& a, const Point& b)
{
    return (a - b).NormSquare();
}
