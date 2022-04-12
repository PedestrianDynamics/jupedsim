#pragma once

#include "geometry/Point.hpp"

#include "GeometricFunctions.hpp"

#include <algorithm>
#include <iterator>

/// TODO(kkratz): Add Invariant check, i.e. if this is really a CCW ordered convex polygon.
/// Represents a simple concvex polygon without holes.
struct ConvexPolygon {
    /// CCW ordered points
    std::vector<Point> points{};

    ConvexPolygon() = default;

    template <typename Container>
    ConvexPolygon(const Container& container)
    {
        points.reserve(std::size(container));
        std::copy(std::begin(container), std::end(container), std::back_inserter(points));
    }

    bool Inside(Point p) const;
    Point Centroid() const;
};
