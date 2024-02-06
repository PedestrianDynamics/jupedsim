// Copyright © 2012-2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "LineSegment.hpp"
#include "Mathematics.hpp"

class Ellipse
{
public:
    double Av{0.53};
    double Amin{0.18};
    double Bmax{0.25};
    double Bmin{0.20};

public:
    double GetEA(double speed) const; // ellipse semi-axis in the direction of the velocity
    // ellipse semi-axis in the orthogonal direction of the velocity
    double GetEB(double scale) const;
    // Effective distance between two ellipses
    double EffectiveDistanceToEllipse(
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
    Point PointOnEllipse(
        const Point& p,
        double scale,
        const Point& center,
        double speed,
        const Point& orientation) const;
};
