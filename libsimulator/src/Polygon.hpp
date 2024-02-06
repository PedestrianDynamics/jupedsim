// Copyright © 2012-2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "Point.hpp"

#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/Polygon_2.h>

class Polygon
{
    using PolygonType = CGAL::Polygon_2<CGAL::Exact_predicates_exact_constructions_kernel>;

    PolygonType _polygon;

public:
    explicit Polygon(const std::vector<Point>& points);
    ~Polygon() = default;
    Polygon(const Polygon& other) = default;
    Polygon& operator=(const Polygon& other) = default;
    Polygon(Polygon&& other) = default;
    Polygon& operator=(Polygon&& other) = default;
    bool IsConvex() const;
    bool IsInside(Point p) const;
    Point Centroid() const;
    std::tuple<Point, double> ContainingCircle() const;

    operator PolygonType() const { return _polygon; }
};
