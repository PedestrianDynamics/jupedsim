/// Copyright © 2012-2022 Forschungszentrum Jülich GmbH
/// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/register/point.hpp>
#include <fmt/format.h>

#include <cmath>
#include <iostream>
#include <string>

class Point
{
public:
    double x = 0.; //*<x-coordinate of a 2-d point*/
    double y = 0.; //*<y-coordinate of a 2-d point*/

public:
    /**
     * **Ctor**
     * Constructs a new point with given x and y.
     * x and y are 0 if not given.
     * @param [in] x: x-coordinate as double
     * @param [in] y: y-coordinate as double
     */
    Point(double x = 0, double y = 0) : x(x), y(y){};

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
    inline double ScalarProduct(const Point& v) const { return x * v.x + y * v.y; }

    inline double CrossProduct(const Point& p) const { return Determinant(p); }

    /// determinant of the square matrix formed by the vectors [ this, v]
    inline double Determinant(const Point& v) const { return x * v.y - y * v.x; }

    Point TransformToEllipseCoordinates(const Point& center, double cphi, double sphi) const;
    /// translation and rotation in cartesian system
    Point TransformToCartesianCoordinates(const Point& center, double cphi, double sphi) const;
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
    /// Assignement
    Point& operator+=(const Point& p);
    /// nice formating of the point
    std::string toString() const;

    bool operator<(const Point& rhs) const;

    bool operator>(const Point& rhs) const;

    bool operator<=(const Point& rhs) const;

    bool operator>=(const Point& rhs) const;

    /**
     * @param [in/out] ostream& : ostream to write the point as xml-format into
     * @return the given ostream with point as xml-format written into
     */
    std::ostream& SaveToXml(std::ostream&) const;
};

BOOST_GEOMETRY_REGISTER_POINT_2D(Point, double, cs::cartesian, x, y);

/**
 * Calculates the distance between the 2 given points.
 * @param [in] point1
 * @param [in] point2
 * @return distance between point1 and point2
 * @see
 * [boost/geometry](http://www.boost.org/doc/libs/1_60_0/libs/geometry/doc/html/geometry/reference/algorithms/distance.html)
 */
double Distance(const Point&, const Point&);

/// multiplication
const Point operator*(const Point& p, const double f);
/// division
const Point operator/(const Point& p, const double f);

std::ostream& operator<<(std::ostream& out, const Point& p);

template <>
struct fmt::formatter<Point> {
    char presentation{'f'};

    constexpr auto parse(format_parse_context& ctx)
    {
        auto it = ctx.begin(), end = ctx.end();
        if(it != end && (*it == 'f' || *it == 'e')) {
            presentation = *it++;
        }
        if(it != end && *it != '}') {
            throw format_error("invalid format");
        }
        return it;
    }

    template <typename FormatContext>
    auto format(const Point& p, FormatContext& ctx) const
    {
        return presentation == 'f' ? fmt::format_to(ctx.out(), "({:f}, {:f})", p.x, p.y) :
                                     fmt::format_to(ctx.out(), "({:e}, {:e})", p.x, p.y);
    }
};
