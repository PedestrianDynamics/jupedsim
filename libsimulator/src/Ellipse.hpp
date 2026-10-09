// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "Point.hpp"

class Ellipse
{
public:
    double av{0.53};
    double amin{0.18};
    double bmax{0.25};
    double bmin{0.20};

public:
    double get_ea(double speed) const; // ellipse semi-axis in the direction of the velocity
    // ellipse semi-axis in the orthogonal direction of the velocity
    double get_eb(double scale) const;
    // Effective distance between two ellipses
    double effective_distance_to_ellipse(
        const Ellipse& other,
        Point center_first,
        Point center_second,
        double scale_first,
        double scale_second,
        double speed_first,
        double speed_second,
        const Point& orientation_first,
        const Point& orientation_second) const;
    // Schnittpunkt der Ellipse mit der Gerade durch P und AP (=ActionPoint von E)
    Point point_on_ellipse(
        const Point& p,
        double scale,
        const Point& center,
        double speed,
        const Point& orientation) const;
};
