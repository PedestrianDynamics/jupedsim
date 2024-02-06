// Copyright © 2012-2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "CollisionGeometry.hpp"

#include "AABB.hpp"
#include "GeometricFunctions.hpp"
#include "IteratorPair.hpp"
#include "LineSegment.hpp"
#include "Mathematics.hpp"
#include "Point.hpp"

#include <CGAL/Boolean_set_operations_2.h>
#include <CGAL/Point_2.h>
#include <CGAL/Polygon_2_algorithms.h>
#include <CGAL/enum.h>
#include <CGAL/number_utils.h>

#include <algorithm>
#include <cmath>
#include <iterator>
#include <vector>

Cell makeCell(Point p)
{
    return {floor(p.x / CELL_EXTEND) * CELL_EXTEND, floor(p.y / CELL_EXTEND) * CELL_EXTEND};
}

bool IsN8Adjacent(const Cell& a, const Cell& b)
{
    const auto dx = static_cast<int>(abs(a.x - b.x) / CELL_EXTEND);
    const auto dy = static_cast<int>(abs(a.y - b.y) / CELL_EXTEND);
    if((dx == 0 && dy == 0) || dx > 1 || dy > 1) {
        return false;
    }
    return true;
}

std::set<Cell> cellsFromLineSegment(LineSegment ls)
{
    const auto firstCell = makeCell(ls.p1);
    const auto lastCell = makeCell(ls.p2);
    if(firstCell == lastCell) {
        return {firstCell};
    }

    if(IsN8Adjacent(firstCell, lastCell)) {
        return {firstCell, lastCell};
    }

    std::set<Cell> cells{firstCell, lastCell};

    const auto toMultiple = [](double x) { return ceil(x / CELL_EXTEND) * CELL_EXTEND; };
    const AABB bounds(ls.p1, ls.p2);
    const auto vec_p1p2 = ls.p2 - ls.p1;
    std::vector<Point> intersections{};
    for(double x_intersect = toMultiple(bounds.xmin); x_intersect <= bounds.xmax;
        x_intersect += CELL_EXTEND) {
        const double fact = (x_intersect - ls.p1.x) / vec_p1p2.x;
        intersections.emplace_back(x_intersect, ls.p1.y + fact * vec_p1p2.y);
    }
    for(double y_intersect = toMultiple(bounds.ymin); y_intersect <= bounds.ymax;
        y_intersect += CELL_EXTEND) {
        const double fact = (y_intersect - ls.p1.y) / vec_p1p2.y;
        intersections.emplace_back(ls.p1.x + fact * vec_p1p2.x, y_intersect);
    }
    std::sort(std::begin(intersections), std::end(intersections));
    for(size_t index = 1; index < intersections.size(); ++index) {
        cells.insert(makeCell((intersections[index - 1] + intersections[index]) / 2));
    }
    return cells;
}

double dist(LineSegment l, Point p)
{
    return l.DistTo(p);
}

size_t CountLineSegments(const PolyWithHoles& poly)
{
    auto count = poly.outer_boundary().size();
    for(const auto& hole : poly.holes()) {
        count += hole.size();
    }
    return count;
}

Point fromPoint_2(const Kernel::Point_2& p)
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

CollisionGeometry::CollisionGeometry(PolyWithHoles accessibleArea)
    : _accessibleAreaPolygon(accessibleArea)
{
    _segments.reserve(CountLineSegments(accessibleArea));
    ExtractSegmentsFromPolygon(accessibleArea.outer_boundary(), _segments);
    for(const auto& hole : accessibleArea.holes()) {
        ExtractSegmentsFromPolygon(hole, _segments);
    }

    for(const auto& ls : _segments) {
        const auto cells = cellsFromLineSegment(ls);
        for(const auto& cell : cells) {
            _grid[cell].insert(ls);
        }

        insertIntoApproximateGrid(ls);
    }

    for(auto& [_, vec] : _approximateGrid) {
        vec.shrink_to_fit();
    }

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

const std::vector<LineSegment>& CollisionGeometry::LineSegmentsInApproxDistanceTo(Point p) const
{
    const auto cell = makeCell(p);
    if(const auto it = _approximateGrid.find(cell); it != _approximateGrid.end()) {
        return it->second;
    }
    static const std::vector<LineSegment> empty{};
    return empty;
}

void CollisionGeometry::insertIntoApproximateGrid(const LineSegment& ls)
{
    constexpr double searchRadius = 4.;

    const auto searchExtend = Point(searchRadius, searchRadius);
    const AABB lineSegmentBounds({ls.p1, ls.p2});
    const AABB searchBounds(
        lineSegmentBounds.BottomLeft() - searchExtend, lineSegmentBounds.TopRight() + searchExtend);

    auto cellBottomLeft = makeCell(searchBounds.BottomLeft());
    auto cellTopRight = makeCell(searchBounds.TopRight());

    for(double x = cellBottomLeft.x; x <= cellTopRight.x; x += CELL_EXTEND) {
        for(double y = cellBottomLeft.y; y <= cellTopRight.y; y += CELL_EXTEND) {
            const auto cell = makeCell({x, y});

            const AABB bbWithSearchRadius(
                {cell.x - searchRadius, cell.y - searchRadius},
                {cell.x + searchRadius + CELL_EXTEND, cell.y + searchRadius + CELL_EXTEND});

            if(bbWithSearchRadius.Intersects(ls)) {
                auto& vec = _approximateGrid[cell];
                vec.push_back(ls);
            }
        }
    }
}

CollisionGeometry::LineSegmentRange
CollisionGeometry::LineSegmentsInDistanceTo(double distance, Point p) const
{
    return LineSegmentRange{
        DistanceQueryIterator<LineSegment>{distance, p, _segments.cbegin(), _segments.cend()},
        DistanceQueryIterator<LineSegment>{distance, p, _segments.cend(), _segments.cend()}};
}

bool CollisionGeometry::IntersectsAny(const LineSegment& linesegment) const
{
    const auto cellsToQuery = cellsFromLineSegment(linesegment);
    for(const auto& cell : cellsToQuery) {
        const auto iter = _grid.find(cell);
        if(iter == std::end(_grid)) {
            continue;
        }
        if(std::find_if(
               iter->second.cbegin(), iter->second.cend(), [&linesegment](const auto candidate) {
                   return intersects(linesegment, candidate);
               }) != iter->second.end()) {
            return true;
        }
    }
    return false;
}

bool CollisionGeometry::InsideGeometry(Point p) const
{
    return CGAL::oriented_side(Kernel::Point_2(p.x, p.y), _accessibleAreaPolygon) !=
           CGAL::ON_NEGATIVE_SIDE;
}

const std::tuple<std::vector<Point>, std::vector<std::vector<Point>>>&
CollisionGeometry::AccessibleArea() const
{
    return _accessibleArea;
}
