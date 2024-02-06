// Copyright © 2012-2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "Triangle.hpp"

Triangle::Triangle(Point a, Point b, Point c) : points{a, b, c}
{
}

bool Triangle::Inside(Point p) const
{
    if((p - points[0]).CrossProduct(points[1] - points[0]) > 0.0f) {
        return false;
    }
    if((p - points[1]).CrossProduct(points[2] - points[1]) > 0.0f) {
        return false;
    }
    if((p - points[2]).CrossProduct(points[0] - points[2]) > 0.0f) {
        return false;
    }
    return true;
}
