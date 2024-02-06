// Copyright © 2012-2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "Point.hpp"

struct Triangle {
    /// Points of the triangle in CCW order.
    Point points[3]{};

    Triangle() = default;
    Triangle(Point a, Point b, Point c);

    bool Inside(Point p) const;
};
