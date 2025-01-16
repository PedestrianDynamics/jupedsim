// Copyright © 2012-2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "Ellipse.hpp"
#include "Macros.hpp"

/**
 * @brief Calculates the semi-major axis (EA) of an ellipse based on the given speed.
 *
 * The ellipse adapts dynamically depending on the agent's speed. This method computes
 * the semi-axis length in the direction of movement, reflecting how the ellipse
 * elongates as speed increases.
 *
 * @param speed The current speed of the object (must be non-negative).
 * @return The computed semi-axis length in the velocity direction.
 *
 */
double Ellipse::GetEA(double speed) const
{
    return Amin + speed * Av;
}

/**
 * @brief Calculates the semi-minor axis (EB) of an ellipse orthogonal to the direction of velocity.
 *
 * This method computes the ellipse's semi-axis length perpendicular to the object's movement,
 * allowing the ellipse to contract or expand based on the provided scaling factor.
 *
 * @param scale A scaling factor typically in the range \f$ [0, 1] \f$.
 *        - \f$ \text{scale} = 0 \f$ → returns \f$ B_{\text{max}} \f$.
 *        - \f$ \text{scale} = 1 \f$ → returns \f$ B_{\text{min}} \f$.
 * @return The computed semi-axis length orthogonal to the velocity direction.
 *
 * @note Values of `scale` outside the \f$ [0, 1] \f$ range may produce unexpected results.
 * @warning No input validation is performed on the `scale` parameter.
 */
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
    // Transform centers into each other's coordinate systems using orientation as cos(φ) and sin(φ)
    Point E2inE1 = center_second.TransformToEllipseCoordinates(center_first, orientation_first.x, orientation_first.y);
    Point E1inE2 = center_first.TransformToEllipseCoordinates(center_second, orientation_second.x, orientation_second.y);

    // Find the closest points on each ellipse
    Point R1 = this->PointOnEllipse(E2inE1, scale_first, center_first, speed_first, orientation_first);
    Point R2 = E2.PointOnEllipse(E1inE2, scale_second, center_second, speed_second, orientation_second);

    // Compute the shortest distance between the closest points
    return (R1 - R2).Norm();
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
