// Copyright © 2012-2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "Polygon.hpp"

#include "SimulationError.hpp"

#include <CGAL/enum.h>
#include <CGAL/number_utils.h>

#include <stdexcept>
#include <vector>

Polygon::Polygon(const std::vector<Point>& points)
{
    if(points.size() < 3) {
        throw SimulationError("Polygon must have at least 3 points");
    }
    _polygon.resize(points.size());
    std::transform(std::begin(points), std::end(points), _polygon.begin(), [](const auto& p) {
        return PolygonType::Point_2{p.x, p.y};
    });

    if(!_polygon.is_simple()) {
        throw SimulationError("Polygon is not simple");
    }

    switch(_polygon.orientation()) {
        case CGAL::Orientation::COLLINEAR:
            throw SimulationError("Polygon may not be collinear.");
        case CGAL::Orientation::CLOCKWISE:
            _polygon.reverse_orientation();
        case CGAL::Orientation::COUNTERCLOCKWISE:
            break;
    }
}

bool Polygon::IsConvex() const
{
    return _polygon.is_convex();
}

bool Polygon::IsInside(Point p) const
{
    const auto side = _polygon.bounded_side(PolygonType::Point_2{p.x, p.y});
    return side != CGAL::Bounded_side::ON_UNBOUNDED_SIDE;
}

Point Polygon::Centroid() const
{
    Point sum{};
    std::for_each(_polygon.begin(), _polygon.end(), [&sum](const auto& p) {
        sum += Point(CGAL::to_double(p.x()), CGAL::to_double(p.y()));
    });
    return sum / _polygon.size();
}

std::tuple<Point, double> Polygon::ContainingCircle() const
{
    const auto center = Centroid();
    auto distance = 0.0;
    std::for_each(std::begin(_polygon), std::end(_polygon), [&distance, center](const auto& p) {
        const Point pt(CGAL::to_double(p.x()), CGAL::to_double(p.y()));
        distance = std::max(distance, (center - pt).Norm());
    });
    return {center, distance};
}
