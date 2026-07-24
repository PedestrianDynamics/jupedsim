// SPDX-License-Identifier: LGPL-3.0-or-later
#include "Geometry/Geometry2D.hpp"

#include "CfgCgal.hpp"
#include "LineSegment.hpp"
#include "Point.hpp"

#include <CGAL/Boolean_set_operations_2/oriented_side.h>
#include <CGAL/enum.h>
#include <CGAL/number_utils.h>

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <tuple>
#include <vector>

namespace
{
size_t CountLineSegments(const PolyWithHoles& poly)
{
    auto count = poly.outer_boundary().size();
    for(const auto& hole : poly.holes()) {
        count += hole.size();
    }
    return count;
}

Point fromPoint_2(const K::Point_2& p)
{
    return {CGAL::to_double(p.x()), CGAL::to_double(p.y())};
}

void ExtractSegmentsFromPolygon(const Poly& p, std::vector<LineSegment>& segments)
{
    const auto& boundary = p.container();
    for(size_t index = 1; index < boundary.size(); ++index) {
        segments.emplace_back(fromPoint_2(boundary[index - 1]), fromPoint_2(boundary[index]));
    }
    segments.emplace_back(fromPoint_2(boundary.back()), fromPoint_2(boundary.front()));
}

std::vector<LineSegment> SegmentsFromPolygon(const PolyWithHoles& poly)
{
    std::vector<LineSegment> segments{};
    segments.reserve(CountLineSegments(poly));
    ExtractSegmentsFromPolygon(poly.outer_boundary(), segments);
    for(const auto& hole : poly.holes()) {
        ExtractSegmentsFromPolygon(hole, segments);
    }
    return segments;
}
} // namespace

Geometry2D::Geometry2D(PolyWithHoles accessibleArea)
    : _accessibleAreaPolygon(accessibleArea), _wallGrid(SegmentsFromPolygon(accessibleArea))
{
    const auto cvt = [](const auto& c) {
        std::vector<Point> out{};
        out.reserve(c.size());
        std::transform(std::begin(c), std::end(c), std::back_inserter(out), [](auto&& p) {
            return fromPoint_2(p);
        });
        return out;
    };
    std::vector<Point> exterior = cvt(_accessibleAreaPolygon.outer_boundary().container());
    std::vector<std::vector<Point>> holes{};
    holes.reserve(_accessibleAreaPolygon.holes().size());
    std::transform(
        std::begin(_accessibleAreaPolygon.holes()),
        std::end(_accessibleAreaPolygon.holes()),
        std::back_inserter(holes),
        [&cvt](auto&& c) { return cvt(c); });
    _accessibleArea = std::make_tuple(exterior, holes);
}

const std::vector<LineSegment>& Geometry2D::LineSegmentsInApproxDistanceTo(Point p) const
{
    return _wallGrid.LineSegmentsInApproxDistanceTo(p);
}

Geometry2D::LineSegmentRange Geometry2D::LineSegmentsInDistanceTo(double distance, Point p) const
{
    return _wallGrid.LineSegmentsInDistanceTo(distance, p);
}

bool Geometry2D::IntersectsAny(const LineSegment& linesegment) const
{
    return _wallGrid.IntersectsAny(linesegment);
}

bool Geometry2D::InsideGeometry(Point p) const
{
    return CGAL::oriented_side(K::Point_2(p.x, p.y), _accessibleAreaPolygon) !=
           CGAL::ON_NEGATIVE_SIDE;
}

const std::tuple<std::vector<Point>, std::vector<std::vector<Point>>>&
Geometry2D::AccessibleArea() const
{
    return _accessibleArea;
}
