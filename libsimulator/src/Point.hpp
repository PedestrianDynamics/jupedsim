/// Copyright © 2012-2022 Forschungszentrum Jülich GmbH
/// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

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

    /// Create a new vector rotated by +90 degree (ccw rotation)
    /// @return rotated vector
    Point Rotate90Deg() const;

    /// Tests that the vector is length 1
    /// @return length == 1
    bool IsUnitLength() const;

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
    std::string toString() const;

    bool operator<(const Point& rhs) const;

    bool operator>(const Point& rhs) const;

    bool operator<=(const Point& rhs) const;

    bool operator>=(const Point& rhs) const;
};

/// Euclidean distance between 'a' and 'b'
/// @param [in] Point a
/// @param [in] Point b
/// @return distance between 'a' and 'b'
double Distance(const Point& a, const Point& b);

/// Squared euclidean distance between 'a' and 'b'
/// @param [in] Point a
/// @param [in] Point b
/// @return distance between 'a' and 'b'
double DistanceSquared(const Point& a, const Point& b);

/// multiplication
const Point operator*(const Point& p, const double f);
/// division
const Point operator/(const Point& p, const double f);

std::ostream& operator<<(std::ostream& out, const Point& p);

template <>
struct fmt::formatter<Point> {
    char presentation{'f'};

    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const Point& p, FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(), "({}, {})", p.x, p.y);
    }
};
