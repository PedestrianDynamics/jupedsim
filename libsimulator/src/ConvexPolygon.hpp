/// Copyright © 2012-2022 Forschungszentrum Jülich GmbH
/// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "Point.hpp"

#include <algorithm>
#include <iterator>

/// TODO(kkratz): Add Invariant check, i.e. if this is really a CCW ordered convex polygon.
/// Represents a simple concvex polygon without holes.
class ConvexPolygon
{
    /// CCW ordered points
    std::vector<Point> points{};
    Point centroid{};

public:
    ConvexPolygon() = default;

    template <typename Container>
    ConvexPolygon(const Container& container)
    {
        points.reserve(std::size(container));
        std::copy(std::begin(container), std::end(container), std::back_inserter(points));
        const auto sum = std::accumulate(std::begin(points), std::end(points), Point{});
        centroid = sum / points.size();
    }

    bool IsInside(Point p) const;
    Point Centroid() const { return centroid; }
    std::tuple<Point, double> ContainingCircle() const;
};
