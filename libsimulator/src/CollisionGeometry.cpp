/// Copyright © 2012-2022 Forschungszentrum Jülich GmbH
/// SPDX-License-Identifier: LGPL-3.0-or-later
#include "CollisionGeometry.hpp"

#include "AABB.hpp"
#include "GeometricFunctions.hpp"
#include "IteratorPair.hpp"
#include "LineSegment.hpp"

#include <CGAL/Boolean_set_operations_2.h>
#include <CGAL/Point_2.h>
#include <CGAL/Polygon_2_algorithms.h>
#include <CGAL/enum.h>
#include <CGAL/number_utils.h>

#include <algorithm>
#include <vector>

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

CollisionGeometry::CollisionGeometry(PolyWithHoles accessibleArea) : _accessibleArea(accessibleArea)
{
    _segments.reserve(CountLineSegments(accessibleArea));
    ExtractSegmentsFromPolygon(accessibleArea.outer_boundary(), _segments);
    for(const auto& hole : accessibleArea.holes()) {
        ExtractSegmentsFromPolygon(hole, _segments);
    }
}

CollisionGeometry::LineSegmentRange
CollisionGeometry::LineSegmentsInDistanceTo(double distance, Point p) const
{
    return LineSegmentRange{
        DistanceQueryIterator<LineSegment>{distance, p, _segments.cbegin(), _segments.cend()},
        DistanceQueryIterator<LineSegment>{distance, p, _segments.cend(), _segments.cend()}};
}

bool CollisionGeometry::IntersectsAny(LineSegment linesegment) const
{
    return std::find_if(_segments.cbegin(), _segments.cend(), [&linesegment](const auto candidate) {
               // ad hoc check for AABB overlap
               const AABB a(linesegment.p1, linesegment.p2);
               const AABB b(candidate.p1, candidate.p2);
               if(!a.Overlap(b)) {
                   return false;
               }
               return intersects(linesegment, candidate);
           }) != _segments.end();
}

bool CollisionGeometry::InsideGeometry(Point p) const
{
    return CGAL::oriented_side(Kernel::Point_2(p.x, p.y), _accessibleArea) ==
           CGAL::ON_POSITIVE_SIDE;
}
