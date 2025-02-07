// SPDX-License-Identifier: LGPL-3.0-or-later
#include "Point.hpp"

#include <Logger.hpp>
#include <limits>

bool Point::isZeroLength() const
{
    constexpr double epsilon = 1e-6;
    return (std::abs(x) < epsilon && std::abs(y) < epsilon);
}

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

/**
 * Transforms a point from the global Cartesian coordinate system (S_1)
 * into the ellipse-local coordinate system (S_2).
 *
 * Steps:
 * 1. **Translation:** Move the point relative to the ellipse center.
 * 2. **Rotation:** Rotate by angle -phi to align with S_2 axes.
 *
 * @param center  The center of the ellipse in Cartesian coordinates (xc, yc)
 * @param cphi    Cosine of rotation angle phi
 * @param sphi    Sine of rotation angle phi
 * @return        Point transformed into the ellipse-local frame (S_2)
 */
Point Point::TransformToEllipseCoordinates(const Point& center, double cphi, double sphi) const
{
    Point p = Point(x, y);
    return (p - center).Rotate(cphi, -sphi);
}
/**
 * Transforms coordinates from ellipse-local system (S_2) back to Cartesian system (S_1).
 * This is the inverse operation of TransformToEllipseCoordinates().
 *
 * The transformation follows these steps:
 * 1. Take point P(x,y) in ellipse system S_2
 * 2. Rotate by -phi to align with Cartesian axes: M(-phi) * [x_2 ; y_2]
 * 3. Add ellipse center coordinates to translate: + [xc ; yc]
 *
 * @param center The ellipse center coordinates (xc,yc) in Cartesian system S_1
 * @param cphi   cos(phi), where phi is the angle between S_1 and S_2
 * @param sphi   sin(phi)
 * @return       Point in Cartesian coordinates (S_1)
 */
Point Point::TransformToCartesianCoordinates(const Point& center, double cphi, double sphi) const
{

    Point p = Point(x, y);
    return (p.Rotate(cphi, sphi) + center);
}

/**
 * CCW rotate Point (interpreted as vector) around origin.
 * @param ctheta Cosine of rotation angle
 * @param stetha Sine of rotation angle
 * @return Rotated point
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

Point Point::operator-() const
{
    return Point(-x, -y);
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
