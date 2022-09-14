/// Copyright © 2012-2022 Forschungszentrum Jülich GmbH
/// SPDX-License-Identifier: LGPL-3.0-or-later
#include "CollisionGeometry.hpp"

#include "IteratorPair.hpp"

#include <algorithm>
#include <vector>

double dist(Line l, Point p)
{
    return l.DistTo(p);
}

CollisionGeometry::CollisionGeometry(std::vector<Line>&& segments) : _segments(std::move(segments))
{
}

CollisionGeometry::LineSegmentRange
CollisionGeometry::LineSegmentsInDistanceTo(double distance, Point p) const
{
    return LineSegmentRange{
        DistanceQueryIterator<Line>{distance, p, _segments.cbegin(), _segments.cend()},
        DistanceQueryIterator<Line>{distance, p, _segments.cend(), _segments.cend()}};
}

bool CollisionGeometry::IntersectsAny(Line linesegment) const
{
    return std::find_if(_segments.cbegin(), _segments.cend(), [&linesegment](const auto& segment) {
               return linesegment.IntersectionWith(segment);
           }) != _segments.end();
}

void CollisionGeometry::AddLineSegment(Line l)
{
    _segments.push_back(l);
}

void CollisionGeometry::RemoveLineSegment(Line l)
{
    _segments.erase(std::remove(_segments.begin(), _segments.end(), l), _segments.end());
}

CollisionGeometryBuilder&
CollisionGeometryBuilder::AddLineSegment(double x1, double y1, double x2, double y2)
{
    _segements.emplace_back(Point{x1, y1}, Point{x2, y2});
    return *this;
}

CollisionGeometry CollisionGeometryBuilder::Build()
{
    _segements.shrink_to_fit();
    return CollisionGeometry{std::move(_segements)};
}
