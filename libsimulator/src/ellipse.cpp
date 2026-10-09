// SPDX-License-Identifier: LGPL-3.0-or-later
#include "ellipse.hpp"

#include "macros.hpp"
#include "point.hpp"

#include <cmath>

/// Calculates the semi-major axis (EA) of an ellipse based on the given speed.
///
/// The ellipse adapts dynamically depending on the agent's speed. This method computes
/// the semi-axis length in the direction of movement, reflecting how the ellipse
/// elongates as speed increases.
///
/// @param speed The current speed of the object (must be non-negative).
/// @return The computed semi-axis length in the velocity direction.
double Ellipse::get_ea(double speed) const
{
    return amin + speed * av;
}

/// Calculates the semi-minor axis (EB) of an ellipse orthogonal to the direction of velocity.
///
/// This method computes the ellipse's semi-axis length perpendicular to the object's movement,
/// allowing the ellipse to contract or expand based on the provided scaling factor.
///
/// @param scale A scaling factor typically in the range \f$ [0, 1] \f$.
///        - \f$ \text{scale} = 0 \f$ → returns \f$ B_{\text{max}} \f$.
///        - \f$ \text{scale} = 1 \f$ → returns \f$ B_{\text{min}} \f$.
/// @return The computed semi-axis length orthogonal to the velocity direction.
///
/// @note Values of `scale` outside the \f$ [0, 1] \f$ range may produce unexpected results.
/// @warning No input validation is performed on the `scale` parameter.
double Ellipse::get_eb(double scale) const
{
    const double delta_b = bmax - bmin;
    return bmax - delta_b * scale;
}

/// Calculates the effective distance between two ellipses.
///
/// This function computes the shortest distance between two ellipses. It does so by:
///
/// 1. **Coordinate Transformation:**
///    Transforms the center of each ellipse into the local coordinate system of the other
///    using their orientation vectors.
///
/// 2. **Closest Point Determination:**
///    Identifies the closest point on each ellipse to the other ellipse in their respective
///    coordinate systems.
///
/// 3. **Effective Distance Calculation:**
///    Calculates the Euclidean distance between these two closest points on the ellipses.
double Ellipse::effective_distance_to_ellipse(
    const Ellipse& e2,
    Point center_first,
    Point center_second,
    double scale_first,
    double scale_second,
    double speed_first,
    double speed_second,
    const Point& orientation_first,
    const Point& orientation_second) const
{
    Point e2in_e1 = center_second.transform_to_ellipse_coordinates(
        center_first, orientation_first.x, orientation_first.y);
    Point e1in_e2 = center_first.transform_to_ellipse_coordinates(
        center_second, orientation_second.x, orientation_second.y);

    Point r1 =
        this->point_on_ellipse(e2in_e1, scale_first, center_first, speed_first, orientation_first);
    Point r2 =
        e2.point_on_ellipse(e1in_e2, scale_second, center_second, speed_second, orientation_second);

    return (r1 - r2).norm();
}

/// Computes the point on the ellipse boundary along the line from the ellipse center to point P.
///
/// Given a point \( P \) in the local coordinate system of the ellipse, this function finds the
/// corresponding point on the ellipse that lies on the same line extending from the center through
/// \( P \).
///
/// **Behavior:**
/// - If \( P \) is very close to the ellipse center, it defaults to returning the point \((a, 0)\)
/// on the ellipse.
/// - Otherwise, it scales the direction from the center to \( P \) by the ellipse’s semi-major and
/// semi-minor axes.
///
/// @return Point on the ellipse boundary, transformed into the global coordinate system.
Point Ellipse::point_on_ellipse(
    const Point& p,
    double scale,
    const Point& center,
    double speed,
    const Point& orientation) const
{
    double x = p.x, y = p.y;
    double r = x * x + y * y;

    // Handle degenerate case when P is very close to the ellipse center
    if(r < j_eps * j_eps) {
        Point cp(this->get_ea(speed), 0);
        return cp.transform_to_cartesian_coordinates(center, orientation.x, orientation.y);
    }

    r = sqrt(r);

    double cos_theta = x / r;
    double sin_theta = y / r;

    double a = get_ea(speed);
    double b = get_eb(scale);
    Point s;
    s.x = a * cos_theta;
    s.y = b * sin_theta;

    return s.transform_to_cartesian_coordinates(center, orientation.x, orientation.y);
}
