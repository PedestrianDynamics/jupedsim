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
#include <numbers>
#include <string>
#include <tuple>
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

Polygon::Polygon(PolygonType polygon) : _polygon(std::move(polygon))
{
}

Polygon Polygon::from_circle(Point center, double radius)
{
    // An edge of the inscribed n-gon is 2 r sin(pi / n) long.
    constexpr double target_circle_edge_length = 0.4;
    constexpr int min_circle_corners = 4;
    const double max_sin = target_circle_edge_length / (2.0 * radius);
    const int corners = max_sin >= std::sin(std::numbers::pi / min_circle_corners) ?
                            min_circle_corners :
                            static_cast<int>(std::ceil(std::numbers::pi / std::asin(max_sin)));
    std::vector<Point> points{};
    points.reserve(corners);
    for(int i = 0; i < corners; ++i) {
        const double angle = 2.0 * std::numbers::pi * i / corners;
        points.push_back(center + Point{std::cos(angle), std::sin(angle)} * radius);
    }
    return Polygon{points};
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
