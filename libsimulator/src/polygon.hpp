// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "cfg_cgal.hpp"
#include "kernel.hpp"

#include <string>
#include <tuple>
#include <vector>

class Polygon
{
    using PolygonType = epick::Poly;

    PolygonType _polygon;

public:
    explicit Polygon(const std::vector<Point>& points);
    explicit Polygon(PolygonType polygon);
    ~Polygon() = default;
    Polygon(const Polygon& other) = default;
    Polygon& operator=(const Polygon& other) = default;
    Polygon(Polygon&& other) = default;
    Polygon& operator=(Polygon&& other) = default;
    bool is_convex() const;
    bool is_inside(Point p) const;
    Point centroid() const;
    std::tuple<Point, double> containing_circle() const;

    operator PolygonType() const { return _polygon; }
};

/// Throws if @p polygon is empty.
std::string as_wkt(const epick::PolyWithHoles& polygon);
