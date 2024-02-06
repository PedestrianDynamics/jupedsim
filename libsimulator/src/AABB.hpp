// Copyright © 2012-2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "IteratorPair.hpp"
#include "LineSegment.hpp"
#include "Point.hpp"
#include "SimulationError.hpp"

#include <limits>

struct AABB {
    double xmin{std::numeric_limits<double>::max()};
    double xmax{std::numeric_limits<double>::lowest()};
    double ymin{std::numeric_limits<double>::max()};
    double ymax{std::numeric_limits<double>::lowest()};

    AABB() = default;

    template <typename Container>
    AABB(const Container& container)
    {
        if(std::empty(container)) {
            throw SimulationError("Cannot create a AABB from zero points");
        }
        for(const auto [x, y] : container) {
            xmin = std::min(x, xmin);
            xmax = std::max(x, xmax);
            ymin = std::min(y, ymin);
            ymax = std::max(y, ymax);
        };
    }

    AABB(const Point a, const Point b)
    {
        xmin = std::min(b.x, std::min(a.x, xmin));
        xmax = std::max(b.x, std::max(a.x, xmax));
        ymin = std::min(b.y, std::min(a.y, ymin));
        ymax = std::max(b.y, std::max(a.y, ymax));
    }

    bool Inside(Point p) const { return p.x >= xmin && p.x <= xmax && p.y >= ymin && p.y <= ymax; }

    bool Overlap(const AABB& other) const
    {
        return !(xmax < other.xmin || xmin > other.xmax || ymax < other.ymin || ymin > other.ymax);
    };

    bool Intersects(const LineSegment& lineSegment) const;

    Point TopLeft() const { return Point{xmin, ymax}; };
    Point TopRight() const { return Point{xmax, ymax}; };
    Point BottomLeft() const { return Point{xmin, ymin}; };
    Point BottomRight() const { return Point{xmax, ymin}; };
};
