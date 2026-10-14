// SPDX-License-Identifier: LGPL-3.0-or-later
#include "polygon.hpp"

#include "point.hpp"
#include "simulation_error.hpp"

#include <CGAL/enum.h>
#include <CGAL/number_utils.h>
#include <fmt/format.h>

#include <algorithm>
#include <cmath>
#include <iterator>
#include <string>
#include <tuple>
#include <vector>

Polygon::Polygon(const std::vector<Point>& points)
{
    for(const auto& p : points) {
        if(!std::isfinite(p.x) || !std::isfinite(p.y)) {
            throw SimulationError("Polygon coordinates must be finite");
        }
    }
    const bool closed = points.size() > 1 && (points.back() - points.front()).is_zero_length();
    const size_t count = closed ? points.size() - 1 : points.size();
    if(count < 3) {
        throw SimulationError("Polygon must have at least 3 points");
    }
    for(size_t index = 0; index < count; ++index) {
        _polygon.push_back({points[index].x, points[index].y});
    }

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

Polygon::Polygon(PolygonType polygon) : _polygon(std::move(polygon))
{
}

bool Polygon::is_convex() const
{
    return _polygon.is_convex();
}

bool Polygon::is_inside(Point p) const
{
    const auto side = _polygon.bounded_side(PolygonType::Point_2{p.x, p.y});
    return side != CGAL::Bounded_side::ON_UNBOUNDED_SIDE;
}

Point Polygon::centroid() const
{
    Point sum{};
    std::for_each(_polygon.begin(), _polygon.end(), [&sum](const auto& p) {
        sum += Point(CGAL::to_double(p.x()), CGAL::to_double(p.y()));
    });
    return sum / static_cast<double>(_polygon.size());
}

std::tuple<Point, double> Polygon::containing_circle() const
{
    const auto center = centroid();
    auto distance = 0.0;
    std::for_each(std::begin(_polygon), std::end(_polygon), [&distance, center](const auto& p) {
        const Point pt(CGAL::to_double(p.x()), CGAL::to_double(p.y()));
        distance = std::max(distance, (center - pt).norm());
    });
    return {center, distance};
}

std::string as_wkt(const PolyWithHoles& polygon)
{
    if(polygon.is_unbounded()) {
        throw SimulationError("Empty polygon.");
    }
    // fmt's "{}" is the shortest decimal that reads back to the same double.
    std::string wkt{"POLYGON ("};
    const auto append_ring = [&wkt](const Poly& ring) {
        wkt += '(';
        for(const Point2D& q : ring.container()) {
            fmt::format_to(std::back_inserter(wkt), "{} {}, ", q.x(), q.y());
        }
        const Point2D& first = ring.container().front();
        fmt::format_to(std::back_inserter(wkt), "{} {})", first.x(), first.y());
    };
    append_ring(polygon.outer_boundary());
    for(const Poly& hole : polygon.holes()) {
        wkt += ", ";
        append_ring(hole);
    }
    wkt += ')';
    return wkt;
}
