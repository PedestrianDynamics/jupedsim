// Copyright © 2012-2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "Ellipse.hpp"
#include "Macros.hpp"

// ellipse  semi-axis in the direction of the velocity
double Ellipse::GetEA(double speed) const
{
    return Amin + speed * Av;
}

// ellipse semi-axis in the orthogonal direction of the velocity
double Ellipse::GetEB(double scale) const
{
    const double deltaB = Bmax - Bmin;
    return Bmax - deltaB * scale;
}

double Ellipse::EffectiveDistanceToEllipse(
    const Ellipse& E2,
    Point center_first,
    Point center_second,
    double scale_first,
    double scale_second,
    double speed_first,
    double speed_second,
    const Point& orientation_first,
    const Point& orientation_second) const
{
    Point R1, R2;
    Point E1inE2, // center of E1 in coordinate system of E2
        E2inE1;
    E2inE1 = center_second.TransformToEllipseCoordinates(
        center_first, orientation_first.x, orientation_first.y);
    E1inE2 = center_first.TransformToEllipseCoordinates(
        center_second, orientation_second.x, orientation_second.y);
    // distance between centers of E1 and E2
    const auto dist = (center_first - center_second).Norm();
    R1 = this->PointOnEllipse(E2inE1, scale_first, center_first, speed_first, orientation_first);
    R2 = E2.PointOnEllipse(E1inE2, scale_second, center_second, speed_second, orientation_second);
    // effective distance
    return dist - (center_first - R1).Norm() - (center_second - R2).Norm();
}

// input: P is a point in the ellipse world.
// output: The point on the ellipse (in cartesian coord) that lays on the same line OP
// O being the center of the ellipse
// if P approx equal to Center of ellipse return cartesian coordinats of the point (a,0)/ellipse
Point Ellipse::PointOnEllipse(
    const Point& P,
    double scale,
    const Point& center,
    double speed,
    const Point& orientation) const
{
    double x = P.x, y = P.y;
    double r = x * x + y * y;
    if(r < J_EPS * J_EPS) {
        Point CP(this->GetEA(speed), 0);
        return CP.TransformToCartesianCoordinates(center, orientation.x, orientation.y);
    }
    r = sqrt(r);

    double cosTheta = x / r;
    double sinTheta = y / r;
    double a = GetEA(speed);
    double b = GetEB(scale);
    Point S;
    S.x = a * cosTheta;
    S.y = b * sinTheta;
    return S.TransformToCartesianCoordinates(center, orientation.x, orientation.y);
}
