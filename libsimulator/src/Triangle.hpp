#pragma once

#include "Point.hpp"

struct Triangle {
    /// Points of the triangle in CCW order.
    Point points[3]{};

    Triangle() = default;
    Triangle(Point a, Point b, Point c);

    bool Inside(Point p) const;
};
